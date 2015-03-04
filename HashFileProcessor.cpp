/* Author: macote */

#include "HashFileProcessor.h"

HashFileProcessor::ProcessResult HashFileProcessor::ProcessTree()
{
	auto result = Success;
	newfilesupdated_ = FALSE;
	if (mode_ == HashFileProcessor::Mode::Verify || mode_ == HashFileProcessor::Mode::Update)
	{
		try
		{
			hashfile_.Load(hashfilename_);
		}
		catch (...)
		{
			result = CouldNotOpenHashFile;
			return result;
		}
	}
	FileTree filetree(basepath_, *this);
	filetree.Process();
	if (mode_ == HashFileProcessor::Mode::Create)
	{
		if (hashfile_.IsEmpty())
		{
			result = NoFileToProcess;
		}
		else if (!report_.IsEmpty())
		{
			result = ErrorsOccurredWhileProcessing;
		}
		else
		{
			hashfile_.Save(hashfilename_);
		}
	}
	else if (mode_ == HashFileProcessor::Mode::Verify || mode_ == HashFileProcessor::Mode::Update)
	{
		if (!hashfile_.IsEmpty())
		{
			for (auto& relativefilepath : hashfile_.GetFilePaths())
			{
				report_.AddLine(L"Missing             : " + relativefilepath);
			}
			result = FilesAreMissing;
		}
		else if (!report_.IsEmpty())
		{
			result = ErrorsOccurredWhileProcessing;
		}
		else if (mode_ == HashFileProcessor::Mode::Update)
		{
			if (!newfilesupdated_)
			{
				result = NothingToUpdate;
			}
			else
			{
				// replace old hash file
				DeleteFileW(hashfilename_.c_str());
				newhashfile_.Save(hashfilename_);
			}
		}
	}
	return result;
}

void HashFileProcessor::ProcessFile(const std::wstring& filepath)
{
	if (lstrcmpiW(appfilepath_.c_str(), filepath.c_str()) == 0 || lstrcmpiW(hashfilename_.c_str(), filepath.c_str()) == 0)
	{
		// skip self
		return;
	}
	auto relativefilepath = filepath.substr(basepath_.length(), filepath.length());
	auto fileentry = hashfile_.GetFileEntry(relativefilepath);
	if (mode_ == HashFileProcessor::Mode::Verify)
	{
		if (fileentry == NULL)
		{
			report_.AddLine(L"Unknown             : " + relativefilepath);
			return;
		}
	}
	else if (mode_ == HashFileProcessor::Mode::Update)
	{
		if (fileentry != NULL)
		{
			newhashfile_.AddFileEntry(fileentry->filepath(), fileentry->size(), fileentry->digest());
			hashfile_.RemoveFileEntry(relativefilepath);
			return;
		}
	}
	LARGE_INTEGER size;
	size.QuadPart = 0;
	auto file = CreateFileW(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		GetFileSizeEx(file, &size);
		CloseHandle(file);
	}
	else
	{
		report_.AddLine(L"Error opening file  : " + relativefilepath);
		if (mode_ == HashFileProcessor::Mode::Verify)
		{
			hashfile_.RemoveFileEntry(relativefilepath);
		}
		return;
	}
	if (mode_ == HashFileProcessor::Mode::Verify)
	{
		if (size.QuadPart != fileentry->size().QuadPart)
		{
			report_.AddLine(L"Incorrect file size : " + relativefilepath);
			hashfile_.RemoveFileEntry(relativefilepath);
			return;
		}
	}
	std::wstring digest;
	CalculateHash(filepath, digest);
	if (mode_ == HashFileProcessor::Mode::Create)
	{
		hashfile_.AddFileEntry(relativefilepath, size, digest);
	}
	else if (mode_ == HashFileProcessor::Mode::Update)
	{
		newhashfile_.AddFileEntry(relativefilepath, size, digest);
		newfilesupdated_ = TRUE;
	}
	else if (mode_ == HashFileProcessor::Mode::Verify)
	{
		if (size.QuadPart != fileentry->size().QuadPart)
		{
			report_.AddLine(L"Incorrect file size : " + relativefilepath);
		}
		else if (digest != fileentry->digest())
		{
			report_.AddLine(L"Incorrect hash      : " + relativefilepath);
		}
		hashfile_.RemoveFileEntry(relativefilepath);
	}
}

void HashFileProcessor::CalculateHash(const std::wstring& filepath, std::wstring& digest)
{
	FileHash* filehash = NULL;
	if (hashtype_ == SHA1)
	{
		filehash = new SHA1FileHash(filepath);
	}
	else if (hashtype_ == MD5)
	{
		filehash = new MD5FileHash(filepath);
	}
	else if (hashtype_ == CRC32)
	{
		filehash = new CRC32FileHash(filepath);
	}
	else
	{
		throw std::runtime_error("HashFileProcessor.CalculateHash(): hash type is undefined.");
	}
	if (filehash != NULL)
	{
		filehash->Compute();
		digest = filehash->digest();
		delete filehash;
	}
}
