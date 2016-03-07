/* Author: macote */

#ifndef HASHCHECKWINDOW_H_
#define HASHCHECKWINDOW_H_

#include "HashCheck.h"
#include "resource.h"
#include "Window.h"
#include <Windows.h>
#include <commctrl.h>
#include <Uxtheme.h>

struct ProgressEventData
{
	std::wstring relativefilepath;
	LARGE_INTEGER filesize;
	LARGE_INTEGER bytesprocessed;
};

class HashCheckWindow : public Window
{
public:
	HashCheckWindow(HINSTANCE hinst, std::vector<std::wstring> args) : Window(hinst), 
		hashcheck_(HashCheck(args)) { }
	virtual LPCWSTR ClassName() { return L"HashCheckWindow"; }
	static HashCheckWindow* Create(HINSTANCE hInst, std::vector<std::wstring> args);
	HWND dlgcurrent() const { return dlgcurrent_; }
	void StartProcess();
protected:
	//virtual void PaintContent(PAINTSTRUCT* pps);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void UpdateTitle();
	void UpdateTitle(LONGLONG bytespersecond);
	LRESULT OnCreate();
	void OnNCDestroy();
	LRESULT OnProgressEventData(WPARAM wParam);
	LRESULT OnCompleteEvent();
	void CancelProcess();
private:
	HashCheck hashcheck_;
	HANDLE hashcheckthread_ = INVALID_HANDLE_VALUE;
	HANDLE completeevent_ = INVALID_HANDLE_VALUE;
	//HWND hwndChild_;
	std::wstring lastfile_;
	std::wstring status_;
	LARGE_INTEGER frequency_;
	LARGE_INTEGER filestartcounter_;
	BOOL cancellationflag_ = FALSE;
	HWND currentfile_ = NULL;
	HWND progressbar_ = NULL;
	HWND action_ = NULL;
	HWND dlgcurrent_= NULL;
	HFONT captionfont_ = NULL;
};

#endif /* HASHCHECKWINDOW_H_ */
