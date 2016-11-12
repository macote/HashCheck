#pragma once

#include <string>
#include <Windows.h>

class IFileTreeAction
{
public:
	virtual ~IFileTreeAction()
	{ 
	}
	virtual void ProcessFile(const std::wstring& filepath) = 0;
};

class FileTree
{
public:
	FileTree(const std::wstring& basepath, IFileTreeAction& fileaction) : basepath_(basepath), fileaction_(fileaction)
	{
	}
	void Process(BOOL& cancellationflag) const
	{ 
		ProcessTree(basepath_, cancellationflag); 
	}
private:
	void ProcessTree(const std::wstring& path, BOOL& cancellationflag) const;
	std::wstring basepath_;
	IFileTreeAction& fileaction_;
};

inline void FileTree::ProcessTree(const std::wstring& path, BOOL& cancellationflag) const
{
	WIN32_FIND_DATA findfiledata;
	HANDLE hFind;
	std::wstring pattern = path + L"*";
	hFind = FindFirstFileW(pattern.c_str(), &findfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (lstrcmpW(findfiledata.cFileName, L".") != 0 && lstrcmpW(findfiledata.cFileName, L"..") != 0)
				{
					std::wstring currentpath(path + findfiledata.cFileName + L"\\");
					ProcessTree(currentpath, cancellationflag);
				}
			}
			else
			{
				std::wstring currentfile(path + findfiledata.cFileName);
				fileaction_.ProcessFile(currentfile);
			}
		} while (FindNextFileW(hFind, &findfiledata) && !cancellationflag);

		FindClose(hFind);
	}
}