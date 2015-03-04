/* Author: macote */

#include "HashFile.h"
#include "FileTree.h"
#include "FileHash.h"
#include "SHA1FileHash.h"
#include "MD5FileHash.h"
#include "CRC32FileHash.h"
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
	enum class HashType {
		CRC32,
		MD5,
		SHA1,
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
	HashFileProcessor(Mode mode, HashType hashtype, const std::wstring hashfilename, const std::wstring appfilepath, const std::wstring basepath)
		: mode_(mode), hashtype_(hashtype), hashfilename_(hashfilename), appfilepath_(appfilepath), basepath_(basepath) { };
	ProcessResult ProcessTree();
	void ProcessFile(const std::wstring& filepath);
	Report report() const { return report_; }
private:
	void CalculateHash(const std::wstring& filepath, std::wstring& digest);
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
