
#ifndef TESTFILEACTION_H_
#define TESTFILEACTION_H_

#include "FileTree.h"
#include "windows.h"

#include "String.h"

class TestFileAction : public FileTreeAction
{
public:
	void Process(const std::wstring filepath)
	{
		MessageBox(NULL, filepath.c_str(), L"TEST", MB_ICONERROR | MB_SYSTEMMODAL);
	}
};

#endif /* TESTFILEACTION_H_ */
