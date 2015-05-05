/* Author: macote */

#include "HashCheckWindow.h"

void HashCheckWindow::Initialize()
{
	NONCLIENTMETRICS nonclientmetrics;
#if (WINVER >= 0x0600)
	nonclientmetrics.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(nonclientmetrics.iPaddedBorderWidth);
#else
	nonclientmetrics.cbSize = sizeof(NONCLIENTMETRICS);
#endif
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &nonclientmetrics, 0))
	{
		captionfont_ = CreateFontIndirectW(&nonclientmetrics.lfMessageFont);
	}
}

LRESULT HashCheckWindow::OnCreate()
{
	Initialize();

	status_ = CreateWindowExW(0,
		L"static",
		L"",
		WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
		10, 10, 340, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	currentfile_ = CreateWindowExW(0,
		L"static",
		L"",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX,
		10, 35, 540, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);
	
	fileprogress_ = CreateWindowExW(0,
		L"static",
		L"",
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		10, 60, 60, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);
/*
	progressbar_ = CreateWindowEx(0,
		PROGRESS_CLASS,
		(LPTSTR)NULL,
		WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		10, 60, 400, 20,
		hwnd_,
		(HMENU)IDC_PROGRESS,
		hinst_,
		NULL);
*/
	action_ = CreateWindowEx(0,
		L"button",
		L"Cancel",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_NOTIFY,
		10, 85, 85, 25,
		hwnd_,
		//(HMENU)IDC_ACTION,
		NULL,
		hinst_,
		NULL);

	SendMessageW(status_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessageW(currentfile_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessageW(fileprogress_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessageW(action_, WM_SETFONT, (UINT)captionfont_, 1);

	//SendMessageW(progressbar_, PBM_SETPOS, 0, 0);

	std::wstring status = hashcheck_.checking() ? L"Checking..." : hashcheck_.updating() ? L"Updating..." : L"Creating...";
	SendMessageW(status_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)status.c_str());

	QueryPerformanceFrequency(&frequency_);

	return FALSE;
}

void HashCheckWindow::StartProcess()
{
	hashcheck_.SetProgressEventHandler([this](HashFileProcessorProgressEventArgs hfppea) {
		ProgressEventData* ped = new ProgressEventData();
		ped->relativefilepath = hfppea.relativefilepath;
		ped->filesize = hfppea.filesize;
		ped->bytesprocessed = hfppea.bytesprocessed;
		PostMessageW(this->GetHWND(), WM_PROGRESS_EVENT_DATA, reinterpret_cast<WPARAM>(ped), NULL);
	});
	threadid_ = hashcheck_.StartProcessAsync();
}

LRESULT HashCheckWindow::OnProgressEventData(WPARAM wParam)
{
	std::auto_ptr<ProgressEventData> ped(reinterpret_cast<ProgressEventData*>(wParam));
	if (lastfile_ != ped->relativefilepath)
	{
		lastfile_ = ped->relativefilepath;
		SendMessageW(currentfile_, WM_SETTEXT, NULL, (LPARAM)ped->relativefilepath.c_str());
		SendMessageW(fileprogress_, WM_SETTEXT, NULL, (LPARAM)L"");
		QueryPerformanceCounter(&filestartcounter_);
	}
	else if (ped->bytesprocessed.QuadPart > 0)
	{
		LARGE_INTEGER counter, elapsed;
		QueryPerformanceCounter(&counter);
		elapsed.QuadPart = counter.QuadPart - filestartcounter_.QuadPart;
		elapsed.QuadPart *= 1000000;
		elapsed.QuadPart /= frequency_.QuadPart;
		if (elapsed.QuadPart > 100000)
		{
			int value = static_cast<int>(ped->bytesprocessed.QuadPart * 100 / ped->filesize.QuadPart);
			std::wstringstream wss;
			wss << value;
			SendMessageW(fileprogress_, WM_SETTEXT, NULL, (LPARAM)wss.str().c_str());
		}
	}
	else
	{
		SendMessageW(fileprogress_, WM_SETTEXT, NULL, (LPARAM)L"");
	}
	//SendMessageW(progressbar_, PBM_SETPOS, (WPARAM)value, 0);
	return FALSE;
}

void HashCheckWindow::OnNCDestroy()
{
	//processingthread_.detach();
	DeleteObject(captionfont_);
	PostQuitMessage(0);
}

LRESULT HashCheckWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PROGRESS_EVENT_DATA:
		return OnProgressEventData(wParam);
	case WM_ACTIVATE:
		dlgcurrent_ = 0 == wParam ? NULL : hwnd_;
		return FALSE;
	case WM_CREATE:
		return OnCreate();
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (reinterpret_cast<HWND>(lParam) == action_)
			{
				SendMessageW(status_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)L"Canceling...");
				// TODO: cancel and cleanup
				// ...
			}
			break;
		}
		return FALSE;
	case WM_NCDESTROY:
		// Death of the root window ends the thread
		OnNCDestroy();
		break;
	case WM_SIZE:
		//if (hwndChild_)
		//{
		//	SetWindowPos(hwndChild_, NULL, 0, 0, GET_X_LPARAM(lParam), 
		//		GET_Y_LPARAM(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
		//}
		return FALSE;
	case WM_SETFOCUS:
		//if (hwndChild_)
		//{
		//	SetFocus(hwndChild_);
		//}
		return FALSE;
	}
	return Window::HandleMessage(uMsg, wParam, lParam);
}

HashCheckWindow* HashCheckWindow::Create(HINSTANCE hInst, std::vector<std::wstring> args)
{
	auto self = new HashCheckWindow(hInst, args);
	if (self != NULL)
	{
		if (self->WinCreateWindow(WS_EX_APPWINDOW, 
			L"HashCheck",
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, // ^ WS_THICKFRAME = disable resize
			CW_USEDEFAULT, CW_USEDEFAULT, 600, 200,
			NULL, NULL))
		{
			return self;
		}
		delete self;
	}
	return NULL;
}
