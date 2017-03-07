#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include "HashCheck.h"
#include "HashCheckWindow.h"
#include <string>
#include <vector>
#include <Windows.h>
#include <Ole2.h>
#include <CommCtrl.h>
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argscount;
	auto args = CommandLineToArgvW(GetCommandLine(), &argscount);
	std::vector<std::wstring> argsvector;
	for (int i = 0; i < argscount; ++i)
	{
		argsvector.push_back(args[i]);
	}

	LocalFree(args);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		InitCommonControls();
		HashCheckWindow* hashcheckwindow = HashCheckWindow::Create(hInstance, argsvector);
		if (hashcheckwindow)
		{
			ShowWindow(hashcheckwindow->hwnd(), nShowCmd);
			hashcheckwindow->StartProcess();
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0))
			{
				if (hashcheckwindow->dlgcurrent() == NULL || !IsDialogMessage(hashcheckwindow->dlgcurrent(), &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		CoUninitialize();
	}

	return 0;
}
