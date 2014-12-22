/*
 * HashCheck.cpp
 *
 *  Created on: 2014-09-09
 *      Author: macote
 */

#include "HashCheck.h"

LPCWSTR HashCheck::kChecksumBaseFileName = L"checksum";

void HashCheck::Initialize()
{
	silent_ = FALSE;
	checking_ = FALSE;
	updating_ = FALSE;
	skipcheck_ = FALSE;

	hashtype_ = Undefined;

	appfilename_ = GetAppFileName(args_[0].c_str());
	args_.erase(args_.begin());

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	if (args_.size() > 0)
	{
		std::vector<std::wstring>::iterator it;
		it = std::find(args_.begin(), args_.end(), L"-u");
		if (it != args_.end())
		{
			updating_ = TRUE;
			args_.erase(it);
		}
		it = std::find(args_.begin(), args_.end(), L"-sm");
		if (it != args_.end())
		{
			skipcheck_ = TRUE;
			args_.erase(it);
		}
		it = std::find(args_.begin(), args_.end(), L"-sha1");
		if (it != args_.end())
		{
			hashtype_ = SHA1;
			args_.erase(it);
		}
		it = std::find(args_.begin(), args_.end(), L"-md5");
		if (it != args_.end())
		{
			hashtype_ = MD5;
			args_.erase(it);
		}
		it = std::find(args_.begin(), args_.end(), L"-crc32");
		if (it != args_.end())
		{
			hashtype_ = CRC32;
			args_.erase(it);
		}
	}

	if (args_.size() > 0)
	{
		std::wstring tmp(args_[0]);
		if (*(tmp.end() - 1) != L'\\')
		{
			tmp += L'\\';
		}
		tmp += L"*";
		hFind = FindFirstFile(tmp.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);
			basepath_ = args_[0] + L'\\';
		}
		else
		{
			silent_ = true;
		}
	}

	std::wstring baseFilename = kChecksumBaseFileName;
	if (hashtype_ == SHA1)
		checksumfilename_ = baseFilename + L".sha1";
	else if (hashtype_ == MD5)
		checksumfilename_ = baseFilename + L".md5";
	else if (hashtype_ == CRC32)
		checksumfilename_ = baseFilename + L".crc32";
	else {
		checksumfilename_ = baseFilename + L".sha1";
		hFind = FindFirstFile(checksumfilename_.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);
			hashtype_ = SHA1;
		}
		else {
			checksumfilename_ = baseFilename + L".md5";
			hFind = FindFirstFile(checksumfilename_.c_str(), &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE) {
				FindClose(hFind);
				hashtype_ = MD5;
			}
			else {
				checksumfilename_ = baseFilename + L".crc32";
				hFind = FindFirstFile(checksumfilename_.c_str(), &FindFileData);
				if (hFind != INVALID_HANDLE_VALUE) {
					FindClose(hFind);
					hashtype_ = CRC32;
				}
				else {
					checksumfilename_ = baseFilename + L".sha1";
					hashtype_ = SHA1;
				}
			}
		}
	}

	hFind = FindFirstFile(checksumfilename_.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			checking_ = !updating_;
		}
		else {
			if (!silent_) {
				std::wstring msg = L"Error: Can't create checksum file. Delete '" + checksumfilename_ + L"' folder.";
				MessageBox(NULL, msg.c_str(), L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
			}
			else
			{
				// ...
			}
			ExitProcess(0);
		}
	}
	else {
		updating_ = false;
	}

}

int HashCheck::Process()
{

	if (checking_) {

		// Verify files

		WCHAR TempFileName[MAX_PATH];
		WCHAR TempFolder[MAX_PATH];

		GetTempPath(MAX_PATH, TempFolder);
		GetTempFileName(TempFolder, L"HashCheck", 0, TempFileName);

		int Result = Verify(TempFileName);

		if (Result != 0) {
			if (!silent_) {
				switch (Result) {
				case -1:
					MessageBox(NULL,L"Error: Could not create report file.",
							L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
					break;
				case -2:
					MessageBox(NULL,L"Error: Could not open checksum file.",
							L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
					break;
				default:
					break;
				}
			}
			return Result;
		}

		HANDLE hFile = CreateFile(TempFileName, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE) {
			if (!IsFileEmpty(hFile)) {
				if (!ViewReport(TempFileName)) {
					CloseHandle(hFile);
					DeleteFile(TempFileName);
					if (!silent_)
						MessageBox(NULL, L"Error: Could not open report.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
					return -1;
				}
			}
			else if (!silent_)
				MessageBox(NULL, L"All files OK.", L"HashCheck", MB_ICONINFORMATION |
						MB_SYSTEMMODAL);
			CloseHandle(hFile);
			DeleteFile(TempFileName);
		}

	}
	else {

		if (updating_) {

			// Update checksum file

			WCHAR TempFileName[MAX_PATH];
			WCHAR TempFolder[MAX_PATH];

			GetTempPath(MAX_PATH, TempFolder);
			GetTempFileName(TempFolder, L"SHA1", 0, TempFileName);

			int Result = Update(TempFileName);

			if (Result != 0) {
				if (!silent_) {
					switch (Result) {
					case -1:
						MessageBox(NULL, L"Error: Could not create new checksum file.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -2:
						MessageBox(NULL, L"Error: Could not open existing checksum file.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -3:
						MessageBox(NULL, L"Error: Nothing to update.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -4:
						MessageBox(NULL, L"Error: Files are missing, can't update.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -5:
						MessageBox(NULL, L"Error: Can't replace existing checksum file.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					default:
						break;
					}

					HANDLE hFile = CreateFile(TempFileName, GENERIC_READ, FILE_SHARE_READ,
							NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

					if (hFile != INVALID_HANDLE_VALUE) {
						if (!IsFileEmpty(hFile)) {
							if (!ViewReport(TempFileName)) {
								CloseHandle(hFile);
								DeleteFile(TempFileName);
								if (!silent_)
									MessageBox(NULL, L"Error: Could not open report.",
											L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
							}
						}
					}
				}

				return Result;

			}
			else if (!silent_)
				MessageBox(NULL, L"Checksum file was updated successfully.",
						L"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);

		}
		else {

			// Create checksum file

			int Result = Create();

			if (Result != 0) {
				if (!silent_)
					switch (Result) {
					case -1:
						MessageBox(NULL, L"Error: Could not create checksum file.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -2:
						MessageBox(NULL, L"Error: No file to process.",
								L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					default:
						break;
					}
				return Result;
			}
			else if (!silent_)
				MessageBox(NULL, L"Checksum file was created successfully.",
						L"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);

		}

	}

	return 0;
}

int HashCheck::Verify(LPCWSTR tempFileName) {

	int retval = 0;

	FileStream report(tempFileName, FileStream::Truncate);
	StreamLineWriter reportwriter(report);
	FileStream checksumfile(checksumfilename_.c_str(), FileStream::Open);
	StreamLineReader checksumfilereader(checksumfile);

	WCHAR FileBuf[512];
	std::wstring RecBuffer, FileKey, Temp;
	std::wstring::size_type xPos, yPos;
	FileProperties* fileproperties;

	do {
		RecBuffer = checksumfilereader.ReadLine();
		if (IsValidChecksumLine(RecBuffer)) {
			xPos = RecBuffer.find('|', 0);
			Temp = RecBuffer.substr(0, xPos);
			fileproperties = new FileProperties(Temp);
			lstrcpy(FileBuf, Temp.c_str());
			CharUpper(FileBuf);
			FileKey = FileBuf;
			yPos = RecBuffer.find('|', xPos + 1);
			Temp = RecBuffer.substr(xPos + 1, yPos - (xPos + 1));
			LARGE_INTEGER li;
			swscanf(Temp.c_str(), L"%I64d", &li.QuadPart);
			fileproperties->set_size(li);
			xPos = RecBuffer.find('|', yPos + 1);
			fileproperties->set_digest(RecBuffer.substr(yPos + 1, RecBuffer.size() - yPos));
			files_.insert(std::pair<std::wstring, FileProperties*>(FileKey, fileproperties));
		}
	} while (!checksumfilereader.EndOfStream());

	ProcessTree(basepath_, reportwriter);

	if (files_.size() != 0) {
		for (auto& it : files_) {
			reportwriter.WriteLine(L"Missing             : " + it.second->filename());
			delete it.second;
		}
		files_.clear();
	}

	return retval;

}

int HashCheck::Create() {

	int retval = 0;

	FileStream checksumfile(checksumfilename_.c_str(), FileStream::Create);
	StreamLineWriter checksumfilewriter(checksumfile);

	ProcessTree(basepath_, checksumfilewriter);

	checksumfilewriter.Close();

	HANDLE hFile = CreateFile(checksumfilename_.c_str(), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		if (IsFileEmpty(hFile)) {
			CloseHandle(hFile);
			DeleteFile(checksumfilename_.c_str());
			retval = -2;
		}
		else
		{
			CloseHandle(hFile);
		}
	}

	return retval;

}

int HashCheck::Update(LPCWSTR tempFileName) {

	int retval = 0;

	FileStream oldchecksumfile(checksumfilename_.c_str(), FileStream::Open);
	StreamLineReader oldchecksumfilereader(oldchecksumfile);
	FileStream newchecksumfile(tempFileName, FileStream::Truncate);
	StreamLineWriter newchecksumfilewriter(newchecksumfile);

	LARGE_INTEGER oldChecksumSize, newChecksumSize;

	HANDLE hFile = CreateFile(checksumfilename_.c_str(), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		if (!GetFileSizeEx(hFile, &oldChecksumSize))
			return -2;
		CloseHandle(hFile);
	}
	else
		return -2;

	WCHAR FileBuf[512];
	std::wstring RecBuffer, FileKey, Temp;
	std::wstring::size_type xPos, yPos;
	FileProperties* fileproperties;

	do {
		RecBuffer = oldchecksumfilereader.ReadLine();
		if (IsValidChecksumLine(RecBuffer)) {
			xPos = RecBuffer.find('|', 0);
			Temp = RecBuffer.substr(0, xPos);
			fileproperties = new FileProperties(Temp);
			lstrcpy(FileBuf, Temp.c_str());
			CharUpper(FileBuf);
			FileKey = FileBuf;
			yPos = RecBuffer.find('|', xPos + 1);
			Temp = RecBuffer.substr(xPos + 1, yPos - (xPos + 1));
			LARGE_INTEGER li;
			swscanf(Temp.c_str(), L"%I64d", &li.QuadPart);
			fileproperties->set_size(li);
			xPos = RecBuffer.find('|', yPos + 1);
			fileproperties->set_digest(RecBuffer.substr(yPos + 1, RecBuffer.size() - yPos));
			files_.insert(std::pair<std::wstring, FileProperties*>(FileKey, fileproperties));
			newchecksumfilewriter.WriteLine(RecBuffer);
		}
	} while (!oldchecksumfilereader.EndOfStream());

	oldchecksumfilereader.Close();

	ProcessTree(basepath_, newchecksumfilewriter);

	newchecksumfilewriter.Close();

	if (files_.size() != 0) {
		// some files are missing: can't update
		FileStream report(tempFileName, FileStream::Truncate);
		StreamLineWriter reportwriter(report);
		for (auto& it : files_) {
			reportwriter.WriteLine(L"Missing             : " + it.second->filename());
			delete it.second;
		}
		files_.clear();
		retval = -4;
	}
	else {

		hFile = CreateFile(tempFileName, GENERIC_READ,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE) {
			if (!GetFileSizeEx(hFile, &newChecksumSize))
				retval = -2;
			else {
				CloseHandle(hFile);
				if (newChecksumSize.QuadPart == oldChecksumSize.QuadPart) {
					retval = -3;
				}
				else {
					// Replace checksum file
					if (CopyFile(tempFileName, checksumfilename_.c_str(), FALSE) == 0) {
						retval = -5;
					}
				}
			}
		}
		else
			retval = -1;

		DeleteFile(tempFileName);

	}

	return retval;

}

std::wstring HashCheck::GetAppFileName(LPCWSTR AppTitle) {

	std::wstring Temp = AppTitle;

	int sPos = Temp.rfind(L"\\") + 1;
	int ePos = Temp.rfind(L".") + 4 - sPos;

	return Temp.substr(sPos, ePos);

}

void HashCheck::ProcessTree(std::wstring sPath, StreamLineWriter& out) {

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind, hFile;
	std::wstring Pattern, CurFile, BaseFile;
	WCHAR FileBuf[512];
	BOOL AddFileInfo;

	FileProperties fileproperties(L"");

	Pattern = sPath + L"*";

	hFind = FindFirstFile(Pattern.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) {

		std::map<std::wstring, FileProperties*, std::less<std::wstring>>::iterator i;
		do {

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

				if ((lstrcmp(FindFileData.cFileName, L".") != 0) &&
						(lstrcmp(FindFileData.cFileName, L"..") != 0)) {

					Pattern = sPath + FindFileData.cFileName + L"\\";
					ProcessTree(Pattern, out);

				}

			}
			else {

				if (!((sPath.size() == 0) && (0 == lstrcmpi(checksumfilename_.c_str(), FindFileData.cFileName) ||
						(0 == lstrcmpi(appfilename_.c_str(), FindFileData.cFileName))))) {

					CurFile = sPath + FindFileData.cFileName;

					LARGE_INTEGER li;
					li.QuadPart = 0;
					fileproperties.set_size(li);
					fileproperties.set_digest(L"");

					BaseFile = CurFile.substr(basepath_.length(), CurFile.length());

					AddFileInfo = FALSE;

					if (checking_) {

						lstrcpy(FileBuf, BaseFile.c_str());
						CharUpper(FileBuf);
						i = files_.find(FileBuf);
						if (i != files_.end()) {

							hFile = CreateFile(CurFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

							if (hFile != INVALID_HANDLE_VALUE) {

								if (!skipcheck_)
									CalculateChecksum(hFile, fileproperties, CurFile);
								if (!GetFileSizeEx(hFile, &li)) {
									out.WriteLine(L"Error getting file size : " + BaseFile);
								}
								else {
									if (li.QuadPart != (*i).second->size().QuadPart)
										out.WriteLine(L"Incorrect file size : " + BaseFile);
									else if (ChecksumCompare(fileproperties, *(*i).second)) {
										if (!skipcheck_)
											out.WriteLine(L"Incorrect checksum  : " + BaseFile);
									}
								}
								CloseHandle(hFile);

							}
							else
								out.WriteLine(L"Error opening file  : " + BaseFile);

							delete (*i).second;
							files_.erase(i);

						}
						else
							out.WriteLine(L"Unknown             : " + BaseFile);

					}
					else if (updating_) {

						lstrcpy(FileBuf, BaseFile.c_str());
						CharUpper(FileBuf);
						i = files_.find(FileBuf);
						if (i != files_.end()) {

							delete (*i).second;
							files_.erase(i);

						}
						else
							AddFileInfo = TRUE;

					}
					else
						AddFileInfo = TRUE;

					if (AddFileInfo) {

						hFile = CreateFile(CurFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

						if (hFile != INVALID_HANDLE_VALUE) {
							CalculateChecksum(hFile, fileproperties, CurFile);
							GetFileSizeEx(hFile, &li);
							CloseHandle(hFile);
						}
						out.WriteLine(BaseFile + L"|" + LargeIntToString(li) + L"|" + fileproperties.digest());

					}

				}

			}

		} while (FindNextFile(hFind, &FindFileData));

		FindClose(hFind);

	}

}

BOOL HashCheck::IsValidChecksumLine(const std::wstring& checksumLine) {

	if (checksumLine.size() == 0) return FALSE;

	if (checksumLine.size() > 512) return FALSE;

	// ...

	return TRUE;

}

BOOL HashCheck::IsFileEmpty(HANDLE hFile) {

	BOOL FileEmpty = TRUE;
	LARGE_INTEGER Size;

	if (GetFileSizeEx(hFile, &Size))
	{
		FileEmpty = Size.QuadPart == 0;
	}
	return FileEmpty;

}

std::wstring HashCheck::LargeIntToString(const LARGE_INTEGER& li) {
	WCHAR tmp[255];
#ifdef _MSC_VER
	swprintf(tmp, 255, L"%I64d", li.QuadPart);
#else
	_snwprintf(tmp, 255, L"%I64d", li.QuadPart);
#endif
	std::wstring result(tmp);
	return result;
}

BOOL HashCheck::ChecksumCompare(const FileProperties& fileprop1, const FileProperties& fileprop2) {

	return fileprop1.digest() != fileprop2.digest();

}

void HashCheck::CalculateChecksum(HANDLE hFile, FileProperties& fileprop, std::wstring filePath) {

	FileHash* fileHash = NULL;
	if (hashtype_ == SHA1)
	{
		fileHash = new SHA1FileHash(filePath);
	}
	else if (hashtype_ == MD5)
	{
		fileHash = new MD5FileHash(filePath);
	}
	else if (hashtype_ == CRC32)
	{
		fileHash = new CRC32FileHash(filePath);
	}
	if (fileHash != NULL)
	{
		fileHash->Compute();
		fileprop.set_digest(fileHash->digest());
		delete fileHash;
	}

}

BOOL HashCheck::ViewReport(LPWSTR TempFile) {

	WCHAR CmdLine[255];

	lstrcpy(CmdLine, L"notepad.exe ");
	lstrcat(CmdLine, TempFile);

	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess(NULL, CmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {

		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return TRUE;

	}

	return FALSE;

}

