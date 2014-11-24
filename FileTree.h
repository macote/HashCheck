/*
* FileTree.h
*
*  Created on: 2014-11-17
*      Author: MAC
*/

#ifndef FILETREE_H_
#define FILETREE_H_

#include "windows.h"

#include <string>

// file action interface for FileTree processor
class FileAction
{
public:
	virtual ~FileAction() {}
	virtual void Process(const std::string filepath) = 0;
};

// file tree processor
class FileTree
{
public:
	FileTree(const std::string &basepath, FileAction *fileaction) : basepath_(basepath), fileaction_(fileaction) {};
	~FileTree()
	{
		if (fileaction_ != nullptr) delete fileaction_;
	}
	void Process()
	{
		ProcessTree(basepath_);
	}
private:
	void ProcessTree(const std::string path);
	const std::string basepath_;
	FileAction *fileaction_;
};

#endif /* FILETREE_H_ */
