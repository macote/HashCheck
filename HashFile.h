/* Author: macote */

#ifndef HASHFILE_H_
#define HASHFILE_H_

#include "FileStream.h"
#include "StreamLineReader.h"
#include "StreamLineWriter.h"
#include <sstream>
#include <list>
#include <map>
#include <string>
#include <Windows.h>

class FileEntry
{
public:
	FileEntry(const std::wstring filepath, const LARGE_INTEGER size, const std::wstring digest) 
		: filepath_(filepath), size_(size), digest_(digest) { };
	std::wstring filepath() const { return filepath_; }
	LARGE_INTEGER size() const { return size_; }
	std::wstring digest() const { return digest_; }
private:
	const std::wstring filepath_;
	const LARGE_INTEGER size_;
	const std::wstring digest_;
};

class HashFile
{
public:
	HashFile() { };
	~HashFile()
	{
		Clear();
	}
	void Save(const std::wstring& hashfilepath);
	void Load(const std::wstring& hashfilepath);
	void AddFileEntry(const std::wstring filepath, const LARGE_INTEGER li, const std::wstring digest);
	void RemoveFileEntry(const std::wstring& filepath);
	bool IsEmpty() const { return files_.size() == 0; }
	bool ContainsFileEntry(const std::wstring& filepath);
	FileEntry* GetFileEntry(const std::wstring& filepath);
	std::list<std::wstring> GetFilePaths();
private:
	std::map<std::wstring, FileEntry*, std::less<std::wstring>>::iterator FindEntry(const std::wstring& filepath);
	BOOL IsValidHashLine(const std::wstring& fileentryline);
	std::wstring LargeIntToString(const LARGE_INTEGER& li);
	void Clear();
private:
	std::map<std::wstring, FileEntry*, std::less<std::wstring>> files_;
};

#endif /* HASHFILE_H_ */
