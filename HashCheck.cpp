/*
 * HashCheck.cpp
 *
 *  Created on: 2014-09-09
 *      Author: macote
 */

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include "FileTree.h"
#include "TestFileAction.h"
#include "HashCheck.h"
#include "CRC32FileHash.h"
#include "MD5FileHash.h"
#include "SHA1FileHash.h"
#include "HashCheckWindow.h"
#include "CommandLineArgs.h"

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

#include <Windows.h>
#include <Ole2.h>
#include <CommCtrl.h>

#ifndef _MSC_VER
// eclipse editor "Function {0} could not be resolved" error suppression workaround
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE,PLARGE_INTEGER);
#endif

std::string AppFileName;
std::string BasePath;
std::string ChecksumFilename;

MAPFILES Files;

HashTypeEnum HashType;

BOOL Silent;
BOOL Checking;
BOOL Updating;
BOOL SkipCheck;
BOOL NoBuffering;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
//	if (SUCCEEDED(CoInitialize(NULL)))
//	{
//		InitCommonControls();
//		HashCheckWindow *hcw = HashCheckWindow::Create(hInstance);
//		if (hcw)
//		{
//			ShowWindow(hcw->GetHWND(), nCmdShow);
//			MSG msg;
//			while (GetMessage(&msg, NULL, 0, 0))
//			{
//				TranslateMessage(&msg);
//				DispatchMessage(&msg);
//			}
//		}
//		CoUninitialize();
//	}
//	return 0;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	std::string TempPath;

	Silent = false;
	Checking = false;
	Updating = false;
	SkipCheck = false;
	NoBuffering = true;

	HashType = htUndefined;

	if (lstrlen(lpCmdLine) > 0) {

		std::string cmdLine(lpCmdLine);

		// checksum update?
		std::string::size_type idx = cmdLine.find("-u");
		if (idx != std::string::npos) {
			Updating = true;
			cmdLine = cmdLine.substr(0, idx) + cmdLine.substr(idx + 2);
		}

		// skip checking?
		idx = cmdLine.find("-sm");
		if (idx != std::string::npos) {
			SkipCheck = true;
			cmdLine = cmdLine.substr(0, idx) + cmdLine.substr(idx + 3);
		}

		idx = cmdLine.find("-sha1");
		if (idx != std::string::npos) {
			HashType = htSHA1;
			cmdLine = cmdLine.substr(0, idx) + cmdLine.substr(idx + 5);
		}

		idx = cmdLine.find("-md5");
		if (idx != std::string::npos) {
			HashType = htMD5;
			cmdLine = cmdLine.substr(0, idx) + cmdLine.substr(idx + 4);
		}

		idx = cmdLine.find("-crc32");
		if (idx != std::string::npos) {
			HashType = htCRC32;
			cmdLine = cmdLine.substr(0, idx) + cmdLine.substr(idx + 6);
		}

		if (cmdLine.size() > 0) {

			while (cmdLine.at(0) == ' ')
				cmdLine = cmdLine.substr(1);
			if (cmdLine.at(cmdLine.size() - 1) != '\\')
				cmdLine += "\\";

			TempPath = cmdLine + "*";
			hFind = FindFirstFile(TempPath.c_str(), &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE) {
				FindClose(hFind);
				BasePath = cmdLine;
			}
			else
				Silent = true;

		}

	}

	char ModuleFileName[512];

	GetModuleFileName(NULL, (LPTSTR)ModuleFileName, 512);
	AppFileName = GetAppFileName((LPSTR)ModuleFileName);

	std::string baseFilename = CHECKSUM_FILE_BASE;
	if (HashType == htSHA1)
		ChecksumFilename = baseFilename + ".sha1";
	else if (HashType == htMD5)
		ChecksumFilename = baseFilename + ".md5";
	else if (HashType == htCRC32)
		ChecksumFilename = baseFilename + ".crc32";
	else {
		ChecksumFilename = baseFilename + ".sha1";
		hFind = FindFirstFile(ChecksumFilename.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);
			HashType = htSHA1;
		}
		else {
			ChecksumFilename = baseFilename + ".md5";
			hFind = FindFirstFile(ChecksumFilename.c_str(), &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE) {
				FindClose(hFind);
				HashType = htMD5;
			}
			else {
				ChecksumFilename = baseFilename + ".crc32";
				hFind = FindFirstFile(ChecksumFilename.c_str(), &FindFileData);
				if (hFind != INVALID_HANDLE_VALUE) {
					FindClose(hFind);
					HashType = htCRC32;
				}
				else {
					ChecksumFilename = baseFilename + ".sha1";
					HashType = htSHA1;
				}
			}
		}
	}

	hFind = FindFirstFile(ChecksumFilename.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (!Updating)
				Checking = true;
		}
		else {
			if (!Silent) {
				std::string msg = "Error: Can't create checksum file. Delete '" + ChecksumFilename + "' folder.";
				MessageBox(NULL, msg.c_str(), "HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
			}
			return -1;
		}
	}
	else {
		Updating = false;
	}

	if (Checking) {

		// Verify files

		char TempFileName[MAX_PATH];
		TCHAR TempFolder[MAX_PATH];

		GetTempPath(MAX_PATH, TempFolder);
		GetTempFileName(TempFolder, "HashCheck", 0, TempFileName);

		int Result = Verify(TempFileName);

		if (Result != 0) {
			if (!Silent) {
				switch (Result) {
				case -1:
					MessageBox(NULL,
							"Error: Could not create report file.",
							"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
					break;
				case -2:
					MessageBox(NULL,
							"Error: Could not open checksum file.",
							"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
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
					if (!Silent)
						MessageBox(NULL, "Error: Could not open report.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
					return -1;
				}
			}
			else if (!Silent)
				MessageBox(NULL, "All files OK.", "HashCheck", MB_ICONINFORMATION |
						MB_SYSTEMMODAL);
			CloseHandle(hFile);
			DeleteFile(TempFileName);
		}

	}
	else {

		if (Updating) {

			// Update checksum file

			char TempFileName[MAX_PATH];
			TCHAR TempFolder[MAX_PATH];

			GetTempPath(MAX_PATH, TempFolder);
			GetTempFileName(TempFolder, "SHA1", 0, TempFileName);

			int Result = Update(TempFileName);

			if (Result != 0) {
				if (!Silent) {
					switch (Result) {
					case -1:
						MessageBox(NULL, "Error: Could not create new checksum file.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -2:
						MessageBox(NULL, "Error: Could not open existing checksum file.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -3:
						MessageBox(NULL, "Error: Nothing to update.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -4:
						MessageBox(NULL, "Error: Files are missing, can't update.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -5:
						MessageBox(NULL, "Error: Can't replace existing checksum file.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
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
								if (!Silent)
									MessageBox(NULL, "Error: Could not open report.",
											"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
							}
						}
					}
				}

				return Result;

			}
			else if (!Silent)
				MessageBox(NULL, "Checksum file was updated successfully.",
						"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);

		}
		else {

			// Create checksum file

			int Result = Create();

			if (Result != 0) {
				if (!Silent)
					switch (Result) {
					case -1:
						MessageBox(NULL, "Error: Could not create checksum file.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					case -2:
						MessageBox(NULL, "Error: No file to process.",
								"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
						break;
					default:
						break;
					}
				return Result;
			}
			else if (!Silent)
				MessageBox(NULL, "Checksum file was created successfully.",
						"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);

		}

	}

	return 0;
}

int Verify(LPCTSTR tempFileName) {

	std::ofstream report;
	std::ifstream checksumFile;

	report.open(tempFileName, std::ios::out);
	if (!report.is_open())
		return -1;

	checksumFile.open(ChecksumFilename.c_str(), std::ios::in);
	if (!checksumFile.is_open()) {
		report.close();
		DeleteFile(tempFileName);
		return -2;
	}

	// Cache the SHA1 file using the map

	char LineBuffer[1024], FileBuf[512];
	std::string RecBuffer, FileKey, Temp;
	std::string::size_type xPos, yPos;
	PFileInfo pFSI;

	do {
		checksumFile.getline(LineBuffer, 1024);
		RecBuffer = LineBuffer;
		if (IsValidChecksumLine(RecBuffer)) {
			// Fill the FileInfo struct
			pFSI = new FileInfo;
			xPos = RecBuffer.find('|', 0);
			Temp = RecBuffer.substr(0, xPos);
			pFSI->FileName = Temp;
			lstrcpy(FileBuf, Temp.c_str());
			CharUpper(FileBuf);
			FileKey = FileBuf;
			yPos = RecBuffer.find('|', xPos + 1);
			Temp = RecBuffer.substr(xPos + 1, yPos - (xPos + 1));
			sscanf(Temp.c_str(), "%I64d", &pFSI->Size.QuadPart);
			xPos = RecBuffer.find('|', yPos + 1);
			pFSI->Digest = RecBuffer.substr(yPos + 1, RecBuffer.size() - yPos);
			// Add the file to the map
			Files.insert(std::pair<std::string, PFileInfo>(FileKey, pFSI));
		}
	} while (!checksumFile.eof());

	checksumFile.close();

	ProcessTree(BasePath, report);

	if (Files.size() != 0) {

		// Report missing files

		for (MAPFILESITER i = Files.begin(); i != Files.end(); i++) {
			report << "Missing             : " << (*i).second->FileName << std::endl;
			delete (*i).second;
		}

		Files.clear();

	}

	report.close();

	return 0;

}

int Create(void) {

	std::ofstream checksumFile;

	checksumFile.open(ChecksumFilename.c_str(), std::ios::out);
	if (!checksumFile.is_open())
		return -1;

	ProcessTree(BasePath, checksumFile);

	checksumFile.close();

	HANDLE hFile = CreateFile(ChecksumFilename.c_str(), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		if (IsFileEmpty(hFile)) {
			CloseHandle(hFile);
			DeleteFile(ChecksumFilename.c_str());
			return -2;
		}
		CloseHandle(hFile);
	}

	return 0;

}

int Update(LPCTSTR tempFileName) {

	std::ifstream oldChecksumFile;
	std::ofstream newChecksumFile;

	LARGE_INTEGER oldChecksumSize, newChecksumSize;
	int retval = 0;

	HANDLE hFile = CreateFile(ChecksumFilename.c_str(), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		if (!GetFileSizeEx(hFile, &oldChecksumSize))
			return -2;
		CloseHandle(hFile);
	}
	else
		return -2;

	newChecksumFile.open(tempFileName, std::ios::out);
	if (!newChecksumFile.is_open())
		return -1;

	oldChecksumFile.open(ChecksumFilename.c_str(), std::ios::in);
	if (!oldChecksumFile.is_open()) {
		newChecksumFile.close();
		DeleteFile(tempFileName);
		return -2;
	}

	// Cache the SHA1 file using the map

	char LineBuffer[1024], FileBuf[512];
	std::string RecBuffer, FileKey, Temp;
	UINT32 xPos, yPos;
	PFileInfo pFSI;

	do {
		oldChecksumFile.getline(LineBuffer, 1024);
		RecBuffer = LineBuffer;
		if (IsValidChecksumLine(RecBuffer)) {
			// Fill the FileInfo struct
			pFSI = new FileInfo;
			xPos = RecBuffer.find('|', 0);
			Temp = RecBuffer.substr(0, xPos);
			pFSI->FileName = Temp;
			lstrcpy(FileBuf, Temp.c_str());
			CharUpper(FileBuf);
			FileKey = FileBuf;
			yPos = RecBuffer.find('|', xPos + 1);
			Temp = RecBuffer.substr(xPos + 1, yPos);
			sscanf(Temp.c_str(), "%I64d", &pFSI->Size.QuadPart);
			xPos = RecBuffer.find('|', yPos + 1);
			pFSI->Digest = RecBuffer.substr(yPos + 1, RecBuffer.size() - yPos);
			// Add the file to the map
			Files.insert(std::pair<std::string, PFileInfo>(FileKey, pFSI));
			newChecksumFile << LineBuffer << std::endl;
		}
	} while (!oldChecksumFile.eof());

	oldChecksumFile.close();

	ProcessTree(BasePath, newChecksumFile);

	newChecksumFile.close();

	if (Files.size() != 0) {
		// Files are missing - can't update
		std::ofstream report;
		report.open(tempFileName, std::ios::out);
		if (!report.is_open())
			return -4;
		for (MAPFILESITER i = Files.begin(); i != Files.end(); i++) {
			report << "Missing             : " << (*i).second->FileName << std::endl;
			delete (*i).second;
		}
		Files.clear();
		report.close();
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
					if (CopyFile(tempFileName, ChecksumFilename.c_str(), false) == 0) {
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

std::string GetAppFileName(LPSTR AppTitle) {

	std::string Temp = AppTitle;

	int sPos = Temp.rfind("\\") + 1;
	int ePos = Temp.rfind(".") + 4 - sPos;

	return Temp.substr(sPos, ePos);

}

void ProcessTree(std::string sPath, std::ostream& out) {

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind, hFile;
	std::string Pattern, CurFile, BaseFile;
	char FileBuf[512];
	BOOL AddFileInfo;

	FileInfo FileInfo;

	Pattern = sPath + "*";

	hFind = FindFirstFile(Pattern.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) {

		MAPFILESITER i;
		do {

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

				if ((lstrcmp(FindFileData.cFileName, ".") != 0) &&
						(lstrcmp(FindFileData.cFileName, "..") != 0)) {

					Pattern = sPath + FindFileData.cFileName + "\\";
					ProcessTree(Pattern, out);

				}

			}
			else {

				if (!((sPath.size() == 0) && (0 == lstrcmpi(ChecksumFilename.c_str(), FindFileData.cFileName) ||
						(0 == lstrcmpi(AppFileName.c_str(), FindFileData.cFileName))))) {

					CurFile = sPath + FindFileData.cFileName;

					FileInfo.Size.QuadPart = 0;
					FileInfo.Digest = "";

					BaseFile = CurFile.substr(BasePath.length(), CurFile.length());

					AddFileInfo = FALSE;

					if (Checking) {

						lstrcpy(FileBuf, BaseFile.c_str());
						CharUpper(FileBuf);
						i = Files.find(FileBuf);
						if (i != Files.end()) {

							hFile = CreateFile(CurFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

							if (hFile != INVALID_HANDLE_VALUE) {

								if (!SkipCheck)
									CalculateChecksum(hFile, &FileInfo, CurFile);
								if (!GetFileSizeEx(hFile, &FileInfo.Size)) {
									out << "Error getting file size : " << BaseFile << std::endl;
								}
								else {
									if (FileInfo.Size.QuadPart != (*i).second->Size.QuadPart)
										out << "Incorrect file size : " << BaseFile << std::endl;
									else if (ChecksumCompare(&FileInfo, (*i).second)) {
										if (!SkipCheck)
											out << "Incorrect checksum  : " << BaseFile << std::endl;
									}
								}
								CloseHandle(hFile);

							}
							else
								out << "Error opening file  : " << BaseFile << std::endl;

							delete (*i).second;
							Files.erase(i);

						}
						else
							out << "Unknown             : " << BaseFile << std::endl;

					}
					else if (Updating) {

						lstrcpy(FileBuf, BaseFile.c_str());
						CharUpper(FileBuf);
						i = Files.find(FileBuf);
						if (i != Files.end()) {

							delete (*i).second;
							Files.erase(i);

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
							CalculateChecksum(hFile, &FileInfo, CurFile);
							GetFileSizeEx(hFile, &FileInfo.Size);
							CloseHandle(hFile);
						}

						out << BaseFile << "|";
						out << LargeIntToString(FileInfo.Size) << "|" << FileInfo.Digest << std::endl;

					}

				}

			}

		} while (FindNextFile(hFind, &FindFileData));

		FindClose(hFind);

	}

}

BOOL IsValidChecksumLine(std::string checksumLine) {

	if (checksumLine.size() == 0) return false;

	if (checksumLine.size() > 512) return false;

	// ...

	return true;

}

BOOL IsFileEmpty(HANDLE hFile) {

	BOOL FileEmpty = TRUE;
	LARGE_INTEGER Size;

	if (GetFileSizeEx(hFile, &Size))
	{
		FileEmpty = Size.QuadPart == 0;
	}
	return FileEmpty;

}

std::string LargeIntToString(const LARGE_INTEGER& li) {
	char tmp[255];
	sprintf(tmp, "%I64d", li.QuadPart);
	std::string result(tmp);
	return result;
}

BOOL ChecksumCompare(PFileInfo ChecksumInfo1, PFileInfo ChecksumInfo2) {

	return (ChecksumInfo1->Digest != ChecksumInfo2->Digest) ? true : false;

}

void CalculateChecksum(HANDLE hFile, PFileInfo pFileInfo, std::string filePath) {

	FileHash* fileHash = NULL;
	if (HashType == htSHA1)
	{
		fileHash = new SHA1FileHash(filePath);
	}
	else if (HashType == htMD5)
	{
		fileHash = new MD5FileHash(filePath);
	}
	else if (HashType == htCRC32)
	{
		fileHash = new CRC32FileHash(filePath);
	}
	if (fileHash != NULL)
	{
		fileHash->Compute();
		pFileInfo->Digest.append(fileHash->digest());
		delete fileHash;
	}

}

BOOL ViewReport(LPTSTR TempFile) {

	char CmdLine[255];

	lstrcpy(CmdLine, "notepad.exe ");
	lstrcat(CmdLine, TempFile);

	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess(NULL, CmdLine, NULL, NULL, false, 0, NULL, NULL,
			&si, &pi)) {

		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return true;

	}

	return false;

}

