#include "HashCheckWindow.h"

LPCWSTR HashCheckWindow::kClassName = L"HashCheckWindow";

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
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &nonclientmetrics, 0))
	{
		captionfont_ = CreateFontIndirect(&nonclientmetrics.lfMessageFont);
	}

	int rowpixels = 6, leftmargin = 6, fullwidthcontrolpixels = 570;
	currentfile_ = CreateWindowEx(0,
		L"static",
		(LPCWSTR)NULL,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX,
		leftmargin, rowpixels, fullwidthcontrolpixels, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	rowpixels += 25;
	progressbar_ = CreateWindowEx(0,
		PROGRESS_CLASS,
		(LPCWSTR)NULL,
		WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		leftmargin, rowpixels, fullwidthcontrolpixels, 20,
		hwnd_,
		(HMENU)IDC_PROGRESS,
		hinst_,
		NULL);

	resultfile_ = CreateWindowEx(0,
		L"static",
		(LPCWSTR)NULL,
		WS_CHILD | SS_NOPREFIX | SS_WORDELLIPSIS,
		leftmargin, rowpixels, fullwidthcontrolpixels, 20,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	rowpixels += 25;
	action_ = CreateWindowEx(0,
		L"button",
		L"Cancel",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_NOTIFY,
		leftmargin, rowpixels, 85, 25,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	copy_ = CreateWindowEx(0,
		L"button",
		L"Copy",
		WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_NOTIFY,
		leftmargin + 85 + 5, rowpixels, 85, 25,
		hwnd_,
		NULL,
		hinst_,
		NULL);

	SetWindowTheme(progressbar_, L" ", L" ");
	SendMessage(progressbar_, PBM_SETBARCOLOR, 0, 0x00888888);
	SendMessage(progressbar_, PBM_SETPOS, 0, 0);
	SendMessage(currentfile_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessage(resultfile_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessage(action_, WM_SETFONT, (UINT)captionfont_, 1);
	SendMessage(copy_, WM_SETFONT, (UINT)captionfont_, 1);

	QueryPerformanceFrequency(&frequency_);

	switch (hashcheck_.fileprocesstype())
	{
	case HashFileProcessType::Create:
		status_ = L"Creating";
		break;
	case HashFileProcessType::Update:
		status_ = L"Updating";
		break;
	case HashFileProcessType::Verify:
		status_ = L"Checking";
		break;
	case HashFileProcessType::Single:
		break;
	default:
		break;
	}

	UpdateTitle();

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
		PostMessage(this->hwnd(), WM_PROGRESS_EVENT_DATA, reinterpret_cast<WPARAM>(ped), (LPARAM)NULL);
	});

	hashcheck_.SetCompleteEventHandler([this]()
	{
		PostMessage(this->hwnd(), WM_COMPLETE_EVENT, (WPARAM)NULL, (LPARAM)NULL);
	});

	hashcheck_.set_silent(TRUE);
	hashcheckthread_ = hashcheck_.StartProcessAsync();
}

void HashCheckWindow::UpdateTitle()
{
	UpdateTitle(status_, 0);
}

void HashCheckWindow::UpdateTitle(LONGLONG bytespersecond)
{
	UpdateTitle(status_, bytespersecond);
}

void HashCheckWindow::UpdateTitle(std::wstring status)
{
	UpdateTitle(status, 0);
}

void HashCheckWindow::UpdateTitle(std::wstring status, LONGLONG bytespersecond)
{
	std::wstring title = L"HashCheck - " + HashTypeStrings::GetHashTypeString(hashcheck_.hashtype());
	if (status.length() > 0)
	{
		title += L" - " + status;
	}

	if (bytespersecond > 0)
	{
		auto bytesperseconddouble = static_cast<double>(bytespersecond);
		std::wstringstream wss;
		std::wstring uom;
		if (bytespersecond > 1073741824)
		{
			uom = L"GiB/s";
			wss << std::setprecision(1) << std::fixed << bytesperseconddouble / 1073741824;
		}
		else if (bytespersecond > 1048576)
		{
			uom = L"MiB/s";
			wss << std::setprecision(1) << std::fixed << bytesperseconddouble / 1048576;
		}
		else if (bytespersecond > 1024)
		{
			uom = L"KiB/s";
			wss << std::setprecision(1) << std::fixed << bytesperseconddouble / 1024;
		}
		else
		{
			uom = L"B/s";
			wss << bytespersecond;
		}

		title += L" - " + wss.str() + L" " + uom;
	}

	SetWindowText(hwnd_, title.c_str());
}

void HashCheckWindow::CancelProcess()
{
	cancellationflag_ = TRUE;
	hashcheck_.CancelProcess();
	WaitForSingleObject(hashcheckthread_, INFINITE);
	hashcheckthread_ = INVALID_HANDLE_VALUE;
}

LRESULT HashCheckWindow::OnProgressEventData(WPARAM wParam)
{
	std::unique_ptr<ProgressEventData> ped(reinterpret_cast<ProgressEventData*>(wParam));
	if (lastfile_ != ped->relativefilepath)
	{
		lastfile_ = ped->relativefilepath;
		if (ped->relativefilepath.size() > 0)
		{
			SendMessage(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)ped->relativefilepath.c_str());
		}

		SendMessage(progressbar_, PBM_SETPOS, 0, 0);
		QueryPerformanceCounter(&filestartcounter_);
		UpdateTitle(0);
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
			UINT value = static_cast<UINT>(ped->bytesprocessed.QuadPart * 100 / ped->filesize.QuadPart);
			SendMessage(progressbar_, PBM_SETPOS, (WPARAM)value, 0);
			UpdateTitle((ped->bytesprocessed.QuadPart * 1000000) / elapsed.QuadPart);
		}
	}
	else
	{
		SendMessage(progressbar_, PBM_SETPOS, (WPARAM)0, 0);
		UpdateTitle(0);
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

	status_ = L"";
	UpdateTitle();
	SetWindowText(action_, L"Exit");
	ShowWindow(progressbar_, SW_HIDE);
	ShowWindow(resultfile_, SW_SHOW);
	SendMessage(resultfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)hashcheck_.lastmessage().c_str());
	if (hashcheck_.fileprocesstype() == HashFileProcessType::Single)
	{
		if (!cancellationflag_)
		{
			ShowWindow(copy_, SW_SHOW);
		}
	}
	else
	{
		if (hashcheck_.basepath().size() > 0)
		{
			SendMessage(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)hashcheck_.basepath().c_str());
		}
		else
		{
			TCHAR currentdirectory[2048];
			GetCurrentDirectory(2048, currentdirectory);
			SendMessage(currentfile_, WM_SETTEXT, (WPARAM)NULL, (LPARAM)currentdirectory);
		}
	}

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
			auto button = reinterpret_cast<HWND>(lParam);
			if (button == action_)
			{
				if (hashcheckthread_ != INVALID_HANDLE_VALUE)
				{
					CancelProcess();
					SetWindowText(action_, L"Exit");
				}
				else
				{
					PostQuitMessage(0);
				}
			}
			else if (button == copy_)
			{
				CopyTextToClipboard(hashcheck_.lastmessage());
			}

			break;
		}

		return FALSE;
	case WM_NCDESTROY:
		OnNCDestroy();
		break;
	case WM_SIZE:
		return FALSE;
	case WM_SETFOCUS:
		return FALSE;
	}

	return Window::HandleMessage(uMsg, wParam, lParam);
}

void HashCheckWindow::CopyTextToClipboard(const std::wstring text)
{
	OpenClipboard(NULL);
	EmptyClipboard();
	auto hashstringsize = text.size() * sizeof(TCHAR);
	auto bufferhandle = GlobalAlloc(GHND, hashstringsize + sizeof(TCHAR));
	CopyMemory(GlobalLock(bufferhandle), hashcheck_.lastmessage().c_str(), hashstringsize);
	GlobalUnlock(bufferhandle);
	SetClipboardData(CF_UNICODETEXT, bufferhandle);
	GlobalFree(bufferhandle);
	CloseClipboard();
}