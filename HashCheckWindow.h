/* Author: macote */

#ifndef HASHCHECKWINDOW_H_
#define HASHCHECKWINDOW_H_


#include "HashCheck.h"
#include "resource.h"
#include <thread>
#include "Window.h"
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>

struct ProgressEventData
{
	std::wstring relativefilepath;
	LARGE_INTEGER filesize;
	LARGE_INTEGER bytesprocessed;
};

class HashCheckWindow : public Window
{
public:
//#if _MSC_VER < 1900
	HashCheckWindow(HINSTANCE hinst, std::vector<std::wstring> args) : Window(hinst), 
		hashcheck_(HashCheck(args)), dlgcurrent_(NULL) { };
//#else
//	using Window::Window;
//#endif	
	virtual LPCWSTR ClassName() { return L"HashCheckWindow"; }
	static HashCheckWindow* Create(HINSTANCE hInst, std::vector<std::wstring> args);
	HWND dlgcurrent() const { return dlgcurrent_; };
	void StartProcess();
protected:
	//virtual void PaintContent(PAINTSTRUCT* pps);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Initialize();
	LRESULT OnCreate();
	void OnNCDestroy();
	LRESULT OnProgressEventData(WPARAM wParam);
private:
	HashCheck hashcheck_;
	//HWND hwndChild_;
	HWND status_;
	HWND currentfile_;
	std::wstring lastfile_;
	HWND fileprogress_;
	BOOL showfileprogress_;
	LARGE_INTEGER frequency_;
	LARGE_INTEGER filestartcounter_;
	//HWND size_;
	//HWND bytes_;
	//HWND progressbar_;
	HWND action_;
	HWND dlgcurrent_;
	HFONT captionfont_;
	//HANDLE thread_;
	DWORD threadid_;
	//int test_;
};

#endif /* HASHCHECKWINDOW_H_ */
