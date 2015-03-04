/* Author: macote */

#include "HashCheck.h"

LPCWSTR HashCheck::kHashFileBaseName = L"checksum";

void HashCheck::Initialize()
{
	silent_ = checking_ = updating_ = skipcheck_ = FALSE;
	hashtype_ = UndefinedHashType;
	appfilename_ = GetAppFileName(args_[0].c_str());
	WIN32_FIND_DATAW findfiledata;
	HANDLE hFind;

	args_.erase(args_.begin());
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
		hFind = FindFirstFileW(tmp.c_str(), &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);
			basepath_ = args_[0] + L'\\';
		}
		else
		{
			silent_ = true;
		}
	}

	std::wstring baseFilename = kHashFileBaseName;
	if (hashtype_ == SHA1)
		hashfilename_ = baseFilename + L".sha1";
	else if (hashtype_ == MD5)
		hashfilename_ = baseFilename + L".md5";
	else if (hashtype_ == CRC32)
		hashfilename_ = baseFilename + L".crc32";
	else
	{
		hashfilename_ = baseFilename + L".sha1";
		hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			hashtype_ = SHA1;
		}
		else
		{
			hashfilename_ = baseFilename + L".md5";
			hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);
				hashtype_ = MD5;
			}
			else
			{
				hashfilename_ = baseFilename + L".crc32";
				hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					FindClose(hFind);
					hashtype_ = CRC32;
				}
				else
				{
					hashfilename_ = baseFilename + L".sha1";
					hashtype_ = SHA1;
				}
			}
		}
	}

	hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			checking_ = !updating_;
		}
		else
		{
			if (!silent_)
			{
				std::wstring msg = L"Error: Can't create hash file. Delete '" + hashfilename_ + L"' folder.";
				MessageBoxW(NULL, msg.c_str(), L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
			}
			else
			{
				// ...
			}
			ExitProcess(0);
		}
	}
	else
	{
		updating_ = false;
	}

}

int HashCheck::Process()
{
	auto mode = HashFileProcessor::Mode::Create;
	if (checking_)
	{
		mode = HashFileProcessor::Mode::Verify;
	}
	else if (updating_)
	{
		mode = HashFileProcessor::Mode::Update;
	}

	auto hashtype = HashFileProcessor::HashType::UndefinedHashType;
	switch (hashtype_)
	{
	case HashCheck::CRC32:
		hashtype = HashFileProcessor::HashType::CRC32;
		break;
	case HashCheck::MD5:
		hashtype = HashFileProcessor::HashType::MD5;
		break;
	case HashCheck::SHA1:
		hashtype = HashFileProcessor::HashType::SHA1;
		break;
	default:
		break;
	}

	HashFileProcessor hashfileprocessor(mode, hashtype, hashfilename_, appfilename_, basepath_);
	auto result = hashfileprocessor.ProcessTree();
	BOOL viewreport = FALSE;
	switch (result)
	{
	case HashFileProcessor::ProcessResult::FilesAreMissing:
		if (updating_)
		{
			MessageBoxW(NULL, L"Error: Can't update because files are missing.", L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
		}
		viewreport = TRUE;
		break;
	case HashFileProcessor::ProcessResult::ErrorsOccurredWhileProcessing:
		viewreport = TRUE;
		break;
	case HashFileProcessor::ProcessResult::CouldNotOpenHashFile:
		MessageBoxW(NULL, L"Error: Could not open hash file.", L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
		break;
	case HashFileProcessor::ProcessResult::NoFileToProcess:
		MessageBoxW(NULL, L"Error: No file to process.", L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
		break;
	case HashFileProcessor::ProcessResult::NothingToUpdate:
		MessageBoxW(NULL, L"Error: Nothing to update.", L"HashCheck", MB_ICONERROR | MB_SYSTEMMODAL);
		break;
	case HashFileProcessor::ProcessResult::Success:
		if (checking_)
		{
			MessageBoxW(NULL, L"All files OK.", L"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else if (updating_)
		{
			MessageBoxW(NULL, L"Hash file was updated successfully.", L"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else
		{
			MessageBoxW(NULL, L"Hash file was created successfully.", L"HashCheck", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		break;
	default:
		break;
	}

	if (viewreport)
	{
		WCHAR tempfile[MAX_PATH];
		WCHAR tempfolder[MAX_PATH];
		GetTempPathW(MAX_PATH, tempfolder);
		GetTempFileNameW(tempfolder, L"HashCheck", 0, tempfile);
		hashfileprocessor.report().Save(tempfile);
		ViewReport(tempfile);
	}

	return 0;
}

std::wstring HashCheck::GetAppFileName(LPCWSTR apptitle)
{
	std::wstring temp = apptitle;
	auto pos1 = temp.rfind(L"\\") + 1;
	auto pos2 = temp.rfind(L".") + 4 - pos1;
	return temp.substr(pos1, pos2);
}

BOOL HashCheck::ViewReport(LPWSTR filepath)
{
	WCHAR cmdline[255];
	lstrcpyW(cmdline, L"notepad.exe ");
	lstrcatW(cmdline, filepath);
	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
	}
	return FALSE;
}

