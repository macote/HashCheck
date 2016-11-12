#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "HashType.h"
#include "HashFileProcessor.h"
#include <vector>
#include <string>
#include <algorithm>
#include <Windows.h>

class HashCheck
{
private:
	static LPCWSTR kHashFileBaseName;
	static LPCWSTR kHashCheckTitle;
public:
	HashCheck(std::vector<std::wstring> args) : args_(args)
	{
		Initialize();
	}
	DWORD Process();
	void CancelProcess()
	{ 
		cancellationflag_ = TRUE;
	}
	void SetProgressEventHandler(std::function<void(HashFileProcessorProgressEventArgs)> handler)
	{
		progressevent_ = handler;
	}
	void SetCompleteEventHandler(std::function<void()> handler)
	{
		completeevent_ = handler;
	}
	HANDLE StartProcessAsync();
	BOOL silent() const { return silent_; }
	BOOL checking() const { return checking_; }
	BOOL updating() const { return updating_; }
	BOOL skipcheck() const { return skipcheck_; }
	std::wstring lastmessage() const { return lastmessage_; }
	void set_silent(const BOOL silent) { silent_ = silent; }
private:
	void Initialize();
	std::wstring GetAppFileName(LPCWSTR command) const;
	BOOL ViewReport(LPCWSTR filepath) const;
	static DWORD WINAPI StaticThreadStart(void* hashcheckinstance);
	void DisplayMessage(std::wstring message, UINT mbconstant);
private:
	std::vector<std::wstring> args_;
	std::wstring hashfilename_;
	std::wstring basepath_;
	std::wstring appfilename_;
	std::wstring lastmessage_;
	HashFileProcessType hashFileProcessType_{ HashFileProcessType::Create };
	HashType hashtype_{ HashType::Undefined };
	BOOL silent_{};
	BOOL checking_{};
	BOOL updating_{};
	BOOL skipcheck_{};
	BOOL cancellationflag_{};
	std::function<void(HashFileProcessorProgressEventArgs)> progressevent_{ nullptr };
	std::function<void()> completeevent_{ nullptr };
};