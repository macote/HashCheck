/*
* FileTree.cc
*
*  Created on: 2014-11-17
*      Author: MAC
*/

#include "FileTree.h"
#include "windows.h"

void FileTree::ProcessTree(const std::string path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	std::string pattern = path + "*";
	hFind = FindFirstFile(pattern.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
				if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0) 
				{
					std::string currentpath;
					currentpath = path + FindFileData.cFileName + "\\";
					ProcessTree(currentpath);
				}
			}
			else
			{
				std::string currentfile(path + FindFileData.cFileName);
				fileaction_->Process(currentfile);
			}
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}
