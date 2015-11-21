/* Author: macote */

#include "HashCheckWindow.h"

HashCheckWindow* HashCheckWindow::Create(HINSTANCE hInst, std::vector<std::wstring> args)
{
	auto self = new HashCheckWindow(hInst, args);
	if (self != NULL)
	{
		if (self->WinCreateWindow(WS_EX_APPWINDOW, 
			L"HashCheck",
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, // ^ WS_THICKFRAME = disable resize
#ifdef _MSC_VER
			CW_USEDEFAULT, CW_USEDEFAULT, 600, 125,
#else
			CW_USEDEFAULT, CW_USEDEFAULT, 588, 116,
#endif
			NULL, NULL))
		{
			return self;
		}

		delete self;
	}

	return NULL;
}

LRESULT HashCheckWindow::OnCreate()
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

	int rowpixels = 6, leftmargin = 6, fullwidthcontrolpixels = 570;
	currentfile_ = CreateWindowExW(0,
		L"static",
		L"",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX,
		leftmargin, rowpixels, fullwidthcontrolpixels, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	rowpixels += 25;
	progressbar_ = CreateWindowExW(0,
		PROGRESS_CLASS,
		(LPCWSTR)NULL,
		WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		leftmargin, rowpixels, fullwidthcontrolpixels, 20,
		hwnd_,
		(HMENU)IDC_PROGRESS,
		hinst_,
		NULL);

	rowpixels += 25;
	action_ = CreateWindowExW(0,
		L"button",
		L"Cancel",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_NOTIFY,
		leftmargin, rowpixels, 85, 25,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	SetWindowTheme(progressbar_, L" ", L" ");
	SendMessageW(progressbar_, PBM_SETBARCOLOR, 0, 0x00888888);
	SendMessageW(progressbar_, PBM_SETPOS, 0, 0);
	SendMessageW(currentfile_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessageW(action_, WM_SETFONT, (UINT)captionfont_, 1);

	std::wstring status = hashcheck_.checking() ? L"Checking" : hashcheck_.updating() ? L"Updating" : L"Creating";
	UpdateTitleStatus(status);

	QueryPerformanceFrequency(&frequency_);

	return FALSE;
}

void HashCheckWindow::StartProcess()
{
	hashcheck_.SetProgressEventHandler([this](HashFileProcessorProgressEventArgs hfppea)
	{
		ProgressEventData* ped = new ProgressEventData();
		ped->relativefilepath = hfppea.relativefilepath;
		ped->filesize = hfppea.filesize;
		ped->bytesprocessed = hfppea.bytesprocessed;
		PostMessageW(this->GetHWND(), WM_PROGRESS_EVENT_DATA, reinterpret_cast<WPARAM>(ped), (LPARAM)NULL);
	});

	hashcheck_.SetCompleteEventHandler([this]()
	{
		PostMessageW(this->GetHWND(), WM_COMPLETE_EVENT, (WPARAM)NULL, (LPARAM)NULL);
	});

	hashcheck_.set_silent(TRUE);
	hashcheckthread_ = hashcheck_.StartProcessAsync();
}

void HashCheckWindow::UpdateTitleStatus(std::wstring status)
{
	std::wstring title = L"HashCheck";
	if (status.length() > 0)
	{
		title += L" - " + status;
	}

	SetWindowTextW(hwnd_, title.c_str());
}

void HashCheckWindow::CancelProcess()
{
	cancellationflag_ = TRUE;
	SendMessageW(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)L"Canceling...");
	hashcheck_.CancelProcess();
	WaitForSingleObject(hashcheckthread_, INFINITE);
	hashcheckthread_ = INVALID_HANDLE_VALUE;
}

LRESULT HashCheckWindow::OnProgressEventData(WPARAM wParam)
{
	std::auto_ptr<ProgressEventData> ped(reinterpret_cast<ProgressEventData*>(wParam));
	if (lastfile_ != ped->relativefilepath)
	{
		lastfile_ = ped->relativefilepath;
		SendMessageW(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)ped->relativefilepath.c_str());
		SendMessageW(progressbar_, PBM_SETPOS, 0, 0);
		QueryPerformanceCounter(&filestartcounter_);
	}
	else if (ped->bytesprocessed.QuadPart > 0)
	{
		LARGE_INTEGER counter, elapsed;
		QueryPerformanceCounter(&counter);
		elapsed.QuadPart = counter.QuadPart - filestartcounter_.QuadPart;
		elapsed.QuadPart *= 1000000;
		elapsed.QuadPart /= frequency_.QuadPart;
		if (elapsed.QuadPart > 166666)
		{
			int value = static_cast<int>(ped->bytesprocessed.QuadPart * 100 / ped->filesize.QuadPart);
			std::wstringstream wss;
			wss << value;
			SendMessageW(progressbar_, PBM_SETPOS, (WPARAM)value, 0);
		}
	}
	else
	{
		SendMessageW(progressbar_, PBM_SETPOS, (WPARAM)0, 0);
	}

	return FALSE;
}

LRESULT HashCheckWindow::OnCompleteEvent()
{
	if (!cancellationflag_)
	{
		WaitForSingleObject(hashcheckthread_, INFINITE);
		hashcheckthread_ = INVALID_HANDLE_VALUE;
	}

	DWORD exitcode;
	GetExitCodeThread(hashcheckthread_, &exitcode);
	std::wstring status = hashcheck_.lastmessage();
	SendMessageW(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)status.c_str());
	SetWindowTextW(action_, L"Exit");

	return FALSE;
}

void HashCheckWindow::OnNCDestroy()
{
	if (hashcheckthread_ != INVALID_HANDLE_VALUE)
	{
		CancelProcess();
	}

	DeleteObject(captionfont_);
	PostQuitMessage(0);
}

LRESULT HashCheckWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PROGRESS_EVENT_DATA:
		return OnProgressEventData(wParam);
	case WM_COMPLETE_EVENT:
		return OnCompleteEvent();
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
				if (hashcheckthread_ != INVALID_HANDLE_VALUE)
				{
					CancelProcess();
					SetWindowTextW(action_, L"Exit");
				}
				else
				{
					PostQuitMessage(0);
				}
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
