/*
* FileTree.cpp
*
*  Created on: 2014-11-17
*      Author: MAC
*/

#include "FileTree.h"

void FileTree::ProcessTree(const std::wstring path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	std::wstring pattern = path + L"*";
	hFind = FindFirstFile(pattern.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
				if (lstrcmp(FindFileData.cFileName, L".") != 0 && lstrcmp(FindFileData.cFileName, L"..") != 0) 
				{
					std::wstring currentpath;
					currentpath = path + FindFileData.cFileName + L"\\";
					ProcessTree(currentpath);
				}
			}
			else
			{
				std::wstring currentfile(path + FindFileData.cFileName);
				fileaction_->Process(currentfile);
			}
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}
