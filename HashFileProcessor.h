/* Author: macote */

#include "HashFile.h"
#include "FileTree.h"
#include "HashType.h"
#include "HashFileProcessType.h"
#include "FileHashFactory.h"
#include "Report.h"

struct HashFileProcessorProgressEventArgs
{
	std::wstring relativefilepath;
	LARGE_INTEGER filesize;
	LARGE_INTEGER bytesprocessed;
};

class HashFileProcessor : public IFileTreeAction
{
public:
	static const DWORD kDefaultBytesProcessedNotificationBlockSize = 4194304;
public:
	enum class ProcessResult
	{
		FilesAreMissing,
		NothingToUpdate,
		CouldNotOpenHashFile,
		ErrorsOccurredWhileProcessing,
		NoFileToProcess,
		Success,
		Canceled
	};
	HashFileProcessor(HashFileProcessType hashFileProcessType, HashType hashtype, std::wstring hashfilename, std::wstring appfilename, std::wstring basepath, BOOL& cancellationflag)
		: hashFileProcessType_(hashFileProcessType), hashtype_(hashtype), hashfilename_(hashfilename), appfilename_(appfilename), basepath_(basepath), cancellationflag_(cancellationflag)
	{ 
	}
	ProcessResult ProcessTree();
	void ProcessFile(const std::wstring& filepath);
	void SaveReport(const std::wstring& reportpath) const 
	{ 
		report_.Save(reportpath); 
	}
	void SetProgressEventHandler(std::function<void(HashFileProcessorProgressEventArgs)> handler)
	{
		SetProgressEventHandler(handler, kDefaultBytesProcessedNotificationBlockSize);
	}
	void SetProgressEventHandler(std::function<void(HashFileProcessorProgressEventArgs)> handler,
		const DWORD bytesprocessednotificationblocksize)
	{
		progressevent_ = handler;
		bytesprocessednotificationblocksize_ = bytesprocessednotificationblocksize;
	}
	void SetCompleteEventHandler(std::function<void()> handler)
	{
		completeevent_ = handler;
	}
	void CancelProcess()
	{ 
		cancellationflag_ = TRUE;
	}
private:
	HashFileProcessType hashFileProcessType_{ HashFileProcessType::Undefined };
	HashType hashtype_{ HashType::Undefined };
	HashFile hashfile_;
	HashFile newhashfile_;
	std::wstring hashfilename_;
	std::wstring appfilename_;
	std::wstring basepath_;
	BOOL newfilesupdated_{};
	Report report_;
	HashFileProcessorProgressEventArgs hfppea_;
	DWORD bytesprocessednotificationblocksize_{};
	std::function<void(HashFileProcessorProgressEventArgs)> progressevent_{ nullptr };
	std::function<void()> completeevent_{ nullptr };
	BOOL& cancellationflag_;
};
