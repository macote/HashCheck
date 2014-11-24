/*
* HashCheckWindow.h
*
*  Created on: 2014-11-19
*      Author: MAC
*/

#ifndef HASHCHECKWINDOW_H_
#define HASHCHECKWINDOW_H_

#include "Window.h"

#include <windows.h>

class HashCheckWindow : public Window
{
public:
#if _MSC_VER < 1900
	HashCheckWindow(HINSTANCE hinst) : Window(hinst) { };
#else
	using Window::Window;
#endif	
	virtual LPCTSTR ClassName() { return TEXT("HashCheckWindow"); }
	static HashCheckWindow *Create(HINSTANCE hinst);
protected:
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate();
private:
	HWND hwndChild_;
};

#endif /* HASHCHECKWINDOW_H_ */