/*
* FileTree.h
*
*  Created on: 2014-11-17
*      Author: MAC
*/

#ifndef FILETREE_H_
#define FILETREE_H_

#include <string>
#include <Windows.h>

// file action interface for FileTree processor
class FileTreeAction
{
public:
	virtual ~FileTreeAction() { }
	virtual void Process(const std::wstring filepath) = 0;
};

// file tree processor
class FileTree
{
public:
	FileTree(const std::wstring basepath, FileTreeAction *fileaction) : basepath_(basepath), fileaction_(fileaction) { };
	~FileTree()
	{
		if (fileaction_ != NULL) delete fileaction_;
	}
	void Process()
	{
		ProcessTree(basepath_);
	}
private:
	void ProcessTree(const std::wstring path);
	const std::wstring basepath_;
	FileTreeAction *fileaction_;
};

#endif /* FILETREE_H_ */
