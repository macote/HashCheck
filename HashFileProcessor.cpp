#include "HashFileProcessor.h"

std::wstring HashFileProcessor::FileName() const
{
	size_t lastslashposition = basepath_.find_last_of(L'\\');
	if (lastslashposition == std::wstring::npos)
	{
		return basepath_;
	}

	return basepath_.substr(lastslashposition + 1, basepath_.size());
}

HashFileProcessor::ProcessResult HashFileProcessor::Process()
{
	auto result = ProcessResult::Success;
	newfilesupdated_ = FALSE;
	if (hashFileProcessType_ == HashFileProcessType::Verify || hashFileProcessType_ == HashFileProcessType::Update)
	{
		try
		{
			hashfile_.Load(hashfilename_);
		}
		catch (...)
		{
			return ProcessResult::CouldNotOpenHashFile;
		}
	}
	else if (hashFileProcessType_ == HashFileProcessType::Single)
	{
		ProcessFile(basepath_);
	}
	else if (hashFileProcessType_ != HashFileProcessType::Create)
	{
		return ProcessResult::UnsupportedProcessType;
	}

	FileTree filetree(basepath_, *this);
	filetree.Process(cancellationflag_);
	if (cancellationflag_)
	{
		result = ProcessResult::Canceled;
	}
	else if (hashFileProcessType_ == HashFileProcessType::Create)
	{
		if (hashfile_.IsEmpty())
		{
			result = ProcessResult::NoFileToProcess;
		}
		else if (!report_.IsEmpty())
		{
			result = ProcessResult::ErrorsOccurredWhileProcessing;
		}
		else
		{
			hashfile_.Save(hashfilename_);
		}
	}
	else if (hashFileProcessType_ == HashFileProcessType::Verify || hashFileProcessType_ == HashFileProcessType::Update)
	{
		if (!hashfile_.IsEmpty())
		{
			for (auto& relativefilepath : hashfile_.GetFilePaths())
			{
				// TODO: replace hardcoded text
				report_.AddLine(L"Missing             : " + relativefilepath);
			}

			result = ProcessResult::FilesAreMissing;
		}
		else if (!report_.IsEmpty())
		{
			result = ProcessResult::ErrorsOccurredWhileProcessing;
		}
		else if (hashFileProcessType_ == HashFileProcessType::Update)
		{
			if (!newfilesupdated_)
			{
				result = ProcessResult::NothingToUpdate;
			}
			else
			{
				// replace old hash file
				DeleteFile(hashfilename_.c_str());
				newhashfile_.Save(hashfilename_);
			}
		}
	}

	if (progressevent_ != nullptr)
	{
		hfppea_.bytesprocessed.QuadPart = 0;
		hfppea_.filesize.QuadPart = 0;
		hfppea_.relativefilepath = L"";
		progressevent_(hfppea_);
	}

	if (completeevent_ != nullptr)
	{
		completeevent_();
	}

	return result;
}

void HashFileProcessor::ProcessFile(const std::wstring& filepath)
{
	if (lstrcmpi(appfilename_.c_str(), filepath.c_str()) == 0 || lstrcmpi(hashfilename_.c_str(), filepath.c_str()) == 0)
	{
		// skip self and current hash file
		return;
	}

	std::wstring relativefilepath;
	if (basepath_.length() == filepath.length())
	{
		relativefilepath = FileName();
	}
	else
	{
		relativefilepath = filepath.substr(basepath_.length(), filepath.length());
	}

	const FileEntry& fileentry = hashfile_.GetFileEntry(relativefilepath);
	if (hashFileProcessType_ == HashFileProcessType::Verify)
	{
		if (&fileentry == &HashFile::kFileEntryNull)
		{
			report_.AddLine(L"Unknown             : " + relativefilepath);
			return;
		}
	}
	else if (hashFileProcessType_ == HashFileProcessType::Update)
	{
		if (&fileentry == &HashFile::kFileEntryNull)
		{
			newhashfile_.AddFileEntry(fileentry.filepath(), fileentry.size(), fileentry.digest());
			hashfile_.RemoveFileEntry(relativefilepath);
			return;
		}
	}

	LARGE_INTEGER filesize;
	filesize.QuadPart = 0;
	auto file = CreateFile(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		GetFileSizeEx(file, &filesize);
		CloseHandle(file);
	}
	else
	{
		report_.AddLine(L"Error opening file  : " + relativefilepath);
		if (hashFileProcessType_ == HashFileProcessType::Verify)
		{
			hashfile_.RemoveFileEntry(relativefilepath);
		}

		return;
	}

	if (hashFileProcessType_ == HashFileProcessType::Verify)
	{
		if (filesize.QuadPart != fileentry.size().QuadPart)
		{
			report_.AddLine(L"Incorrect file size : " + relativefilepath);	
			hashfile_.RemoveFileEntry(relativefilepath);
			return;
		}
	}

	auto filehash = FileHashFactory::Create(hashtype_, filepath);
	if (progressevent_ != nullptr)
	{
		hfppea_.bytesprocessed.QuadPart = 0;
		hfppea_.relativefilepath = relativefilepath;
		progressevent_(hfppea_);
		filehash->SetBytesProcessedEventHandler([this, filesize](FileHashBytesProcessedEventArgs fhbea) {
			this->hfppea_.bytesprocessed = fhbea.bytesprocessed;
			this->hfppea_.filesize = filesize;
			this->progressevent_(hfppea_);
		}, bytesprocessednotificationblocksize_);
	}

	filehash->Compute(cancellationflag_);
	currentdigest_ = filehash->digest();
	if (hashFileProcessType_ == HashFileProcessType::Create)
	{
		hashfile_.AddFileEntry(relativefilepath, filesize, currentdigest_);
	}
	else if (hashFileProcessType_ == HashFileProcessType::Update)
	{
		newhashfile_.AddFileEntry(relativefilepath, filesize, currentdigest_);
		newfilesupdated_ = TRUE;
	}
	else if (hashFileProcessType_ == HashFileProcessType::Verify)
	{
		if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, currentdigest_.c_str(), -1, fileentry.digest().c_str(), -1) != CSTR_EQUAL)
		{
			report_.AddLine(L"Incorrect hash      : " + relativefilepath);
		}

		hashfile_.RemoveFileEntry(relativefilepath);
	}
}
