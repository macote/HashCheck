/* Author: macote */

#include "HashFile.h"
#include "FileTree.h"
#include "HashType.h"
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
	enum class Mode
	{
		Create,
		Update,
		Verify,
		Undefined
	};
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
	HashFileProcessor()
		: mode_(Mode::Undefined), hashtype_(HashType::Undefined), progressevent_(nullptr),
		  completeevent_(nullptr){ }
	HashFileProcessor(Mode mode, HashType hashtype, std::wstring hashfilename, std::wstring appfilepath, std::wstring basepath)
		: mode_(mode), hashtype_(hashtype),
		  hashfilename_(hashfilename), appfilepath_(appfilepath), basepath_(basepath),
		  progressevent_(nullptr), completeevent_(nullptr) { }
	ProcessResult ProcessTree();
	void ProcessFile(const std::wstring& filepath);
	void SaveReport(const std::wstring& reportpath) const { report_.Save(reportpath); }
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
	void set_mode(const Mode mode) { mode_ = mode; }
	void set_hashtype(const HashType hashtype) { hashtype_ = hashtype; }
	void set_hashfilename(const std::wstring hashfilename) { hashfilename_ = hashfilename; }
	void set_appfilepath(const std::wstring appfilepath) { appfilepath_ = appfilepath; }
	void set_basepath(const std::wstring basepath) { basepath_ = basepath; }
	void CancelProcess() { cancellationflag_ = TRUE; }
private:
	Mode mode_;
	HashType hashtype_;
	HashFile hashfile_;
	HashFile newhashfile_;
	std::wstring hashfilename_;
	std::wstring appfilepath_;
	std::wstring basepath_;
	BOOL newfilesupdated_ = FALSE;
	Report report_;
	HashFileProcessorProgressEventArgs hfppea_;
	DWORD bytesprocessednotificationblocksize_ = 0;
	std::function<void(HashFileProcessorProgressEventArgs)> progressevent_;
	std::function<void()> completeevent_;
	BOOL cancellationflag_ = FALSE;
};
