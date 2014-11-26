/*
* HashCheckWindow.cc
*
*  Created on: 2014-11-19
*      Author: MAC
*/

#include "HashCheckWindow.h"

#include <windows.h>
#include <windowsx.h>

LRESULT HashCheckWindow::OnCreate()
{
	return 0;
}

LRESULT HashCheckWindow::HandleMessage(
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		return OnCreate();

	case WM_NCDESTROY:
		// Death of the root window ends the thread
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		if (hwndChild_) {
			SetWindowPos(hwndChild_, NULL, 0, 0,
				GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;

	case WM_SETFOCUS:
		if (hwndChild_) {
			SetFocus(hwndChild_);
		}
		return 0;
	}

	return Window::HandleMessage(uMsg, wParam, lParam);
}

HashCheckWindow *HashCheckWindow::Create(HINSTANCE hinst)
{
	HashCheckWindow *self = new HashCheckWindow(hinst);
	if (self && self->WinCreateWindow(0,
		TEXT("Scratch"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL)) {
		return self;
	}
	delete self;
	return NULL;
}