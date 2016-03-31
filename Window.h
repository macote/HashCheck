/* Author: macote */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <Windows.h>

class Window
{
public:
	Window(HINSTANCE hinst) : hinst_(hinst)
	{
	}
	HWND hwnd() const { return hwnd_; }
protected:
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void PaintContent(PAINTSTRUCT* pps)
	{ 
	}
	virtual LPCWSTR ClassName() = 0;
	virtual BOOL WinRegisterClass(WNDCLASSEX* pwc) 
	{ 
		return RegisterClassExW(pwc); 
	}
	virtual ~Window() 
	{ 
	}
	HWND WinCreateWindow(DWORD dwExStyle, LPCWSTR pszName, DWORD dwStyle, 
		int x, int y, int cx, int cy, HWND hwndParent, HMENU hmenu)
	{
		Register();
		return CreateWindowExW(dwExStyle, ClassName(), pszName, dwStyle,
			x, y, cx, cy, hwndParent, hmenu, hinst_, this);
	}
	HWND hwnd_{ NULL };
	HINSTANCE hinst_;
private:
	void Register();
	void OnPaint();
	void OnPrintClient(HDC hdc);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif /* WINDOW_H_ */
