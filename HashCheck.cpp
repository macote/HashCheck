#include "HashCheck.h"

LPCWSTR HashCheck::kHashFileBaseName = L"checksum";

LPCWSTR HashCheck::kHashCheckTitle = L"HashCheck";

std::map<HashType, std::wstring> HashCheck::kHashTypeExtensionMap = 
{
	{ HashType::CRC32, L".crc32" },
	{ HashType::MD5, L".md5" },
	{ HashType::SHA1, L".sha1" },
	{ HashType::SHA256, L".sha256" },
	{ HashType::SHA512, L".sha512" }
};

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

		it = std::find(args_.begin(), args_.end(), L"-sha512");
		if (it != args_.end())
		{
			hashtype_ = HashType::SHA512;
			args_.erase(it);
		}

		it = std::find(args_.begin(), args_.end(), L"-sha256");
		if (it != args_.end())
		{
			hashtype_ = HashType::SHA256;
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

	InitializeHashType();

	if (args_.size() > 0)
	{
		InitializeTarget(args_[0]);
	}

	if (hashfileprocesstype_ != HashFileProcessType::Single)
	{
		InitializeHashFile();
	}
}

void HashCheck::InitializeHashType()
{
	WIN32_FIND_DATA findfiledata;
	HANDLE find;

	std::wstring baseFilename = kHashFileBaseName;
	if (hashtype_ == HashType::Undefined)
	{
		for (const auto &pair : kHashTypeExtensionMap)
		{
			auto hashfilename = baseFilename + pair.second;
			find = FindFirstFile(hashfilename.c_str(), &findfiledata);
			if (find != INVALID_HANDLE_VALUE && !(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FindClose(find);
				hashtype_ = pair.first;
			}
		}

		if (hashtype_ == HashType::Undefined)
		{
			hashtype_ = kDefaultHashType;
		}
	}

	hashfilename_ = baseFilename + kHashTypeExtensionMap[hashtype_];
}

void HashCheck::InitializeTarget(std::wstring target)
{
	WIN32_FIND_DATA findfiledata;
	HANDLE find;

	std::wstring fileorpath(args_[0]);
	if (*(fileorpath.end() - 1) == L'\\' || *(fileorpath.end() - 1) == L'\"')
	{
		fileorpath.pop_back();
	}

	find = FindFirstFile(fileorpath.c_str(), &findfiledata);
	if (find != INVALID_HANDLE_VALUE)
	{
		FindClose(find);
		if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			hashfileprocesstype_ = HashFileProcessType::Single;
			basepath_ = fileorpath;
		}
		else
		{
			std::wstring path = fileorpath + L"\\*";
			find = FindFirstFile(path.c_str(), &findfiledata);
			if (find != INVALID_HANDLE_VALUE)
			{
				FindClose(find);
				basepath_ = fileorpath + L'\\';
			}
			else
			{ 
				auto message = L"Error: There's nothing to process in the target folder.";
				MessageBox(NULL, message, kHashCheckTitle, MB_ICONERROR | MB_SYSTEMMODAL);
				ExitProcess(0);
			}
		}
	}
	else
	{
		auto message = L"Error: The specified file or folder argument was not found.";
		MessageBox(NULL, message, kHashCheckTitle, MB_ICONERROR | MB_SYSTEMMODAL);
		ExitProcess(0);
	}
}

void HashCheck::InitializeHashFile()
{
	WIN32_FIND_DATA findfiledata;
	HANDLE find;

	find = FindFirstFile(hashfilename_.c_str(), &findfiledata);
	if (find != INVALID_HANDLE_VALUE)
	{
		FindClose(find);
		if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			checking_ = !updating_;
		}
		else
		{
			auto message = L"Error: Can't create hash file. Delete '" + hashfilename_ + L"' folder.";
			MessageBox(NULL, message.c_str(), kHashCheckTitle, MB_ICONERROR | MB_SYSTEMMODAL);
			ExitProcess(0);
		}
	}
	else
	{
		updating_ = FALSE;
	}

	if (checking_)
	{
		hashfileprocesstype_ = HashFileProcessType::Verify;
	}
	else if (updating_)
	{
		hashfileprocesstype_ = HashFileProcessType::Update;
	}
}

DWORD HashCheck::Process()
{
	HashFileProcessor hashFileProcessor(hashfileprocesstype_, hashtype_, hashfilename_, appfilename_, basepath_, cancellationflag_);

	if (progressevent_ != nullptr)
	{
		hashFileProcessor.SetProgressEventHandler(progressevent_, kBytesProcessedNotificationBlockSize);
	}

	if (completeevent_ != nullptr)
	{
		hashFileProcessor.SetCompleteEventHandler(completeevent_);
	}

	auto result = hashFileProcessor.Process();
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
		if (hashfileprocesstype_ == HashFileProcessType::Update)
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
		if (hashfileprocesstype_ == HashFileProcessType::Single)
		{
			DisplayMessage(hashFileProcessor.currentdigest().c_str() , MB_ICONINFORMATION);
		}
		else if (hashfileprocesstype_ == HashFileProcessType::Verify)
		{
			DisplayMessage(L"All files OK.", MB_ICONINFORMATION);
		}
		else if (hashfileprocesstype_ == HashFileProcessType::Update)
		{
			DisplayMessage(L"Hash file was updated successfully.", MB_ICONINFORMATION);
		}
		else  if (hashfileprocesstype_ == HashFileProcessType::Create)
		{
			DisplayMessage(L"Hash file was created successfully.", MB_ICONINFORMATION);
		}

		break;
	default:
		DisplayMessage(L"Error: Unhandled error.", MB_ICONERROR);
		exitcode = 10;
		break;
	}

	if (viewreport)
	{
		WCHAR tempfile[MAX_PATH];
		WCHAR tempfolder[MAX_PATH];
		GetTempPath(MAX_PATH, tempfolder);
		GetTempFileName(tempfolder, kHashCheckTitle, 0, tempfile);
		hashFileProcessor.SaveReport(tempfile);
		ViewReport(tempfile);
	}

	return exitcode;
}

std::wstring HashCheck::GetAppFileName(LPCWSTR command) const
{
	std::wstring temp = command;
	auto pos1 = temp.rfind(L"\\") + 1;
	auto pos2 = temp.rfind(L".") + 4 - pos1;
	return temp.substr(pos1, pos2);
}

BOOL HashCheck::ViewReport(LPCWSTR filepath) const
{
	WCHAR cmdline[255];
	lstrcpy(cmdline, L"notepad.exe ");
	lstrcat(cmdline, filepath);
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) && !silent_)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
	}

	return FALSE;
}

void HashCheck::DisplayMessage(std::wstring message, UINT mbconstant)
{
	lastmessage_ = message;
	if (silent_)
	{
		return;
	}

	MessageBox(NULL, message.c_str(), kHashCheckTitle, mbconstant | MB_SYSTEMMODAL);
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
