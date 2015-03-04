/* Author: macote */

#include "HashFile.h"

void HashFile::Load(const std::wstring& hashfilepath)
{
	FileStream hashfile(hashfilepath.c_str(), FileStream::Open);
	StreamLineReader hashfilereader(hashfile);
	WCHAR buffer[2048];
	std::wstring line, key, sizetemp, filepath, digest;
	std::wstring::size_type pos1, pos2;
	LARGE_INTEGER size;
	// hash line format: <filepath>|<size>|<digest> 
	do 
	{
		line = hashfilereader.ReadLine();
		if (IsValidHashLine(line))
		{
			pos1 = line.find('|', 0);
			filepath = line.substr(0, pos1);
			lstrcpyW(buffer, filepath.c_str());
			key = CharUpperW(buffer);
			pos2 = line.find('|', pos1 + 1);
			sizetemp = line.substr(pos1 + 1, pos2 - (pos1 + 1));
			std::wstringstream wss(sizetemp);
			wss >> size.QuadPart;
			digest = line.substr(pos2 + 1, line.size() - pos2);
			files_.insert(std::pair<std::wstring, FileEntry*>(key, new FileEntry(filepath, size, digest)));
		}
	} while (!hashfilereader.EndOfStream());
}

void HashFile::Save(const std::wstring& hashfilepath)
{
	FileStream hashfile(hashfilepath, FileStream::Mode::Create);
	StreamLineWriter hashfilewriter(hashfile);
	for (auto& item : files_) 
	{
		FileEntry* fileentry = item.second;
		hashfilewriter.WriteLine(fileentry->filepath() + L"|" + LargeIntToString(fileentry->size()) + L"|" + fileentry->digest());
	}
}

void HashFile::AddFileEntry(const std::wstring filepath, const LARGE_INTEGER size, const std::wstring digest)
{
	files_.insert(std::pair<std::wstring, FileEntry*>(filepath, new FileEntry(filepath, size, digest)));
}

std::map<std::wstring, FileEntry*, std::less<std::wstring>>::iterator HashFile::FindEntry(const std::wstring& filepath)
{
	WCHAR key[2048];
	lstrcpyW(key, filepath.c_str());
	CharUpperW(key);
	return files_.find(key);
}

void HashFile::RemoveFileEntry(const std::wstring& filepath)
{
	std::map<std::wstring, FileEntry*, std::less<std::wstring>>::iterator i = FindEntry(filepath);
	if (i != files_.end())
	{
		delete (*i).second;
		files_.erase(i);
	}
}

bool HashFile::ContainsFileEntry(const std::wstring& filepath)
{
	std::map<std::wstring, FileEntry*, std::less<std::wstring>>::iterator i = FindEntry(filepath);
	return i != files_.end();
}

FileEntry* HashFile::GetFileEntry(const std::wstring& filepath)
{
	FileEntry* fileentry = NULL;
	std::map<std::wstring, FileEntry*, std::less<std::wstring>>::iterator i = FindEntry(filepath);
	if (i != files_.end())
	{
		fileentry = (*i).second;
	}
	return fileentry;
}

std::list<std::wstring> HashFile::GetFilePaths()
{
	std::list<std::wstring> filepaths;
	for (auto& item : files_)
	{
		filepaths.push_back(item.second->filepath());
	}
	return filepaths;
}

BOOL HashFile::IsValidHashLine(const std::wstring& fileentryline)
{
	if (fileentryline.size() == 0) return FALSE;
	if (fileentryline.size() > 2176) return FALSE;
	// TODO: implement proper hash line validation
	return TRUE;
}

std::wstring HashFile::LargeIntToString(const LARGE_INTEGER& li) {
	std::wstringstream wss;
	wss << li.QuadPart;
	return wss.str();
}

void HashFile::Clear()
{
	for (auto& item : files_)
	{
		delete item.second;
	}
	files_.clear();
}