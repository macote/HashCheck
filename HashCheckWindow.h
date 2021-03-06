#pragma once

#include "HashCheck.h"
#include "resource.h"
#include "Window.h"
#include <Windows.h>
#include <commctrl.h>
#include <Uxtheme.h>

struct ProgressEventData
{
	std::wstring relativefilepath;
	LARGE_INTEGER filesize{};
	LARGE_INTEGER bytesprocessed{};
};

class HashCheckWindow : public Window
{
private:
	static LPCWSTR kClassName;
public:
	HashCheckWindow(HINSTANCE hinst, std::vector<std::wstring> args) : Window(hinst), hashcheck_(HashCheck(args)) 
	{ 
	}
	virtual LPCWSTR ClassName() { return kClassName; }
	static HashCheckWindow* Create(HINSTANCE hInst, std::vector<std::wstring> args);
	HWND dlgcurrent() const { return dlgcurrent_; }
	void StartProcess();
protected:
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void UpdateTitle();
	void UpdateTitle(LONGLONG bytespersecond);
	void UpdateTitle(std::wstring status);
	void UpdateTitle(std::wstring status, LONGLONG bytespersecond);
	LRESULT OnCreate();
	void OnNCDestroy();
	LRESULT OnProgressEventData(WPARAM wParam);
	LRESULT OnCompleteEvent();
	void CancelProcess();
	void CopyTextToClipboard(const std::wstring text);
private:
	HashCheck hashcheck_;
	HANDLE hashcheckthread_{ INVALID_HANDLE_VALUE };
	HANDLE completeevent_{ INVALID_HANDLE_VALUE };
	//HWND hwndChild_;
	std::wstring lastfile_;
	std::wstring status_;
	LARGE_INTEGER frequency_{};
	LARGE_INTEGER filestartcounter_{};
	BOOL cancellationflag_{};
	HWND currentfile_{ NULL };
	HWND resultfile_{ NULL };
	HWND progressbar_{ NULL };
	HWND action_{ NULL };
	HWND copy_{ NULL };
	HWND dlgcurrent_{ NULL };
	HFONT captionfont_{ NULL };
};