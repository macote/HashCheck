/*
 * HashCheck.h
 *
 *  Created on: 2014-09-09
 *      Author: macote
 */

#ifndef HASHCHECK_H_
#define HASHCHECK_H_

#define _CRT_SECURE_NO_WARNINGS

#include "FileTree.h"
#include "StreamLineReader.h"
#include "StreamLineWriter.h"
#include "CRC32FileHash.h"
#include "MD5FileHash.h"
#include "SHA1FileHash.h"


#include <map>
#include <vector>
#include <algorithm>
#include <string>

#include <Windows.h>

#ifndef _MSC_VER
// eclipse editor "Function {0} could not be resolved" error suppression workaround
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE, PLARGE_INTEGER);
#endif


class FileProperties
{
public:
	FileProperties(const std::wstring &filename) : filename_(filename) { };
	std::wstring filename() const { return filename_; }
	LARGE_INTEGER size() const { return size_; }
	void set_size(LARGE_INTEGER size) { size_ = size; }
	std::wstring digest() const { return digest_; }
	void set_digest(const std::wstring digest) { digest_ = digest; }
private:
	const std::wstring filename_;
	LARGE_INTEGER size_;
	std::wstring digest_;
};

class HashCheck
{
private:
	static LPCWSTR kChecksumBaseFileName;
public:
	HashCheck(std::vector<std::wstring> args) : args_(args)
	{
		Initialize();
	};
	enum HashType {
		CRC32,
		MD5,
		SHA1,
		Undefined
	};
	int Process();
private:
	std::vector<std::wstring> args_;
	std::map<std::wstring, FileProperties*, std::less<std::wstring>> files_;
	std::wstring checksumfilename_;
	std::wstring basepath_;
	std::wstring appfilename_;
	HashType hashtype_;
	BOOL silent_;
	BOOL checking_;
	BOOL updating_;
	BOOL skipcheck_;
	void Initialize();
	std::wstring GetAppFileName(LPCWSTR AppTitle);
	int Verify(LPCWSTR tempFileName);
	int Create();
	int Update(LPCWSTR tempFileName);
	BOOL IsValidChecksumLine(const std::wstring& checksumLine);
	BOOL IsFileEmpty(HANDLE hFile);
	std::wstring LargeIntToString(const LARGE_INTEGER& li);
	BOOL ChecksumCompare(const FileProperties& fileprop1, const FileProperties& fileprop2);
	void CalculateChecksum(HANDLE hFile, FileProperties& fileprop, std::wstring filePath);
	void ProcessTree(std::wstring sPath, StreamLineWriter& out);
	BOOL ViewReport(LPWSTR TempFile);
};

#endif /* HASHCHECK_H_ */
