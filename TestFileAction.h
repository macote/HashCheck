
#ifndef TESTFILEACTION_H_
#define TESTFILEACTION_H_

#include "FileTree.h"
#include "windows.h"

class TestFileAction : public FileAction
{
public:
	void Process(const std::string filepath)
	{
		MessageBox(NULL, filepath.c_str(), "TEST", MB_ICONERROR | MB_SYSTEMMODAL);
	}
};

#endif /* TESTFILEACTION_H_ */
