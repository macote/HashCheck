/* Author: macote */

#include "HashFile.h"
#include "FileTree.h"
#include "HashType.h"
#include "FileHashFactory.h"
#include "Report.h"

class HashFileProcessor : public IFileTreeAction
{
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
		Success
	};
	HashFileProcessor(Mode mode, HashType hashtype, std::wstring hashfilename, std::wstring appfilepath, std::wstring basepath)
		: mode_(mode), hashtype_(hashtype), hashfilename_(hashfilename), appfilepath_(appfilepath), basepath_(basepath) { };
	ProcessResult ProcessTree();
	void ProcessFile(const std::wstring& filepath);
	void SaveReport(const std::wstring& reportpath) const { report_.Save(reportpath); }
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
};
