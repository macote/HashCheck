/* Author: macote */

#include "HashCheck.h"

LPCWSTR HashCheck::kHashFileBaseName = L"checksum";

LPCWSTR HashCheck::kHashCheckTitle = L"HashCheck";

void HashCheck::Initialize()
{
	silent_ = checking_ = updating_ = skipcheck_ = FALSE;
	hashtype_ = HashType::Undefined;
	appfilename_ = GetAppFileName(args_[0].c_str());

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
			hashtype_ = HashType::SHA1;
			args_.erase(it);
		}

		it = std::find(args_.begin(), args_.end(), L"-md5");
		if (it != args_.end())
		{
			hashtype_ = HashType::MD5;
			args_.erase(it);
		}

		it = std::find(args_.begin(), args_.end(), L"-crc32");
		if (it != args_.end())
		{
			hashtype_ = HashType::CRC32;
			args_.erase(it);
		}
	}

	WIN32_FIND_DATA findfiledata;
	HANDLE hFind;

	if (args_.size() > 0)
	{
		std::wstring tmp(args_[0]);
		if (*(tmp.end() - 1) != L'\\')
		{
			tmp += L'\\';
		}

		tmp += L"*";
		hFind = FindFirstFileW(tmp.c_str(), &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			basepath_ = args_[0] + L'\\';
		}
	}

	std::wstring baseFilename = kHashFileBaseName;
	if (hashtype_ == HashType::SHA1)
		hashfilename_ = baseFilename + L".sha1";
	else if (hashtype_ == HashType::MD5)
		hashfilename_ = baseFilename + L".md5";
	else if (hashtype_ == HashType::CRC32)
		hashfilename_ = baseFilename + L".crc32";
	else
	{
		hashfilename_ = baseFilename + L".sha1";
		hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			hashtype_ = HashType::SHA1;
		}
		else
		{
			hashfilename_ = baseFilename + L".md5";
			hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);
				hashtype_ = HashType::MD5;
			}
			else
			{
				hashfilename_ = baseFilename + L".crc32";
				hFind = FindFirstFileW(hashfilename_.c_str(), &findfiledata);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					FindClose(hFind);
					hashtype_ = HashType::CRC32;
				}
				else
				{
					hashfilename_ = baseFilename + L".sha1";
					hashtype_ = HashType::SHA1;
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
			auto message = L"Error: Can't create hash file. Delete '" + hashfilename_ + L"' folder.";
			MessageBoxW(NULL, message.c_str(), kHashCheckTitle, MB_ICONERROR | MB_SYSTEMMODAL);
			ExitProcess(0);
		}
	}
	else
	{
		updating_ = FALSE;
	}

	auto mode = HashFileProcessor::Mode::Create;
	if (checking_)
	{
		mode = HashFileProcessor::Mode::Verify;
	}
	else if (updating_)
	{
		mode = HashFileProcessor::Mode::Update;
	}

	hashfileprocessor_.set_mode(mode);

	hashfileprocessor_.set_hashtype(hashtype_);
	hashfileprocessor_.set_appfilepath(appfilename_);
	hashfileprocessor_.set_hashfilename(hashfilename_);
	hashfileprocessor_.set_basepath(basepath_);
}

int HashCheck::Process()
{
	if (progressevent_ != nullptr)
	{
		hashfileprocessor_.SetProgressEventHandler(progressevent_, 2097152);
	}

	if (completeevent_ != nullptr)
	{
		hashfileprocessor_.SetCompleteEventHandler(completeevent_);
	}

	auto result = hashfileprocessor_.ProcessTree();
	if (result == HashFileProcessor::ProcessResult::Canceled)
	{
		DisplayMessage(L"Canceled.", MB_ICONERROR);
		return 99;
	}

	BOOL viewreport = FALSE;
	int exitcode = 0;
	switch (result)
	{
	case HashFileProcessor::ProcessResult::FilesAreMissing:
		if (updating_)
		{
			DisplayMessage(L"Error: Can't update because files are missing.", MB_ICONERROR);
		}
		else
		{
			DisplayMessage(L"Errors were detected while processing.  Consult the report for details.", MB_ICONEXCLAMATION);
		}

		viewreport = TRUE;
		exitcode = 1;
		break;
	case HashFileProcessor::ProcessResult::ErrorsOccurredWhileProcessing:
		DisplayMessage(L"Errors were detected while processing.  Consult the report for details.", MB_ICONEXCLAMATION);
		viewreport = TRUE;
		exitcode = 2;
		break;
	case HashFileProcessor::ProcessResult::CouldNotOpenHashFile:
		DisplayMessage(L"Error: Could not open hash file.", MB_ICONERROR);
		exitcode = 3;
		break;
	case HashFileProcessor::ProcessResult::NoFileToProcess:
		DisplayMessage(L"Error: No file to process.", MB_ICONERROR);
		exitcode = 4;
		break;
	case HashFileProcessor::ProcessResult::NothingToUpdate:
		DisplayMessage(L"Error: Nothing to update.", MB_ICONERROR);
		exitcode = 5;
		break;
	case HashFileProcessor::ProcessResult::Success:
		if (checking_)
		{
			DisplayMessage(L"All files OK.", MB_ICONINFORMATION);
		}
		else if (updating_)
		{
			DisplayMessage(L"Hash file was updated successfully.", MB_ICONINFORMATION);
		}
		else
		{
			DisplayMessage(L"Hash file was created successfully.", MB_ICONINFORMATION);
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
		GetTempFileNameW(tempfolder, kHashCheckTitle, 0, tempfile);
		hashfileprocessor_.SaveReport(tempfile);
		ViewReport(tempfile);
	}

	return exitcode;
}

std::wstring HashCheck::GetAppFileName(LPCWSTR apptitle) const
{
	std::wstring temp = apptitle;
	auto pos1 = temp.rfind(L"\\") + 1;
	auto pos2 = temp.rfind(L".") + 4 - pos1;
	return temp.substr(pos1, pos2);
}

BOOL HashCheck::ViewReport(LPCWSTR filepath) const
{
	WCHAR cmdline[255];
	lstrcpyW(cmdline, L"notepad.exe ");
	lstrcatW(cmdline, filepath);
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) && !silent_)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
	}

	return FALSE;
}

void HashCheck::DisplayMessage(std::wstring message, int mbconstant)
{
	lastmessage_ = message;
	if (silent_)
	{
		return;
	}

	MessageBoxW(NULL, message.c_str(), kHashCheckTitle, mbconstant | MB_SYSTEMMODAL);
}

HANDLE HashCheck::StartProcessAsync()
{
	return CreateThread(NULL, 0, StaticThreadStart, (void*)this, 0, NULL);
}

DWORD WINAPI HashCheck::StaticThreadStart(void* hashcheckinstance)
{
	HashCheck* hashcheck = (HashCheck*)hashcheckinstance;
	return hashcheck->Process();
}
