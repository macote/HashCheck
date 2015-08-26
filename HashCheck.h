/* Author: macote */

#ifndef HASHCHECK_H_
#define HASHCHECK_H_

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
		progressevent_ = nullptr;
		Initialize();
	}
	int Process();
	void CancelProcess() { hashfileprocessor_.CancelProcess(); }
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
	std::wstring GetAppFileName(LPCWSTR apptitle) const;
	BOOL ViewReport(LPCWSTR filepath) const;
	static DWORD WINAPI StaticThreadStart(void* hashcheckinstance);
	void DisplayMessage(std::wstring message, int mbconstant);
private:
	std::vector<std::wstring> args_;
	std::wstring hashfilename_;
	std::wstring basepath_;
	std::wstring appfilename_;
	std::wstring lastmessage_;
	HashType hashtype_;
	BOOL silent_;
	BOOL checking_;
	BOOL updating_;
	BOOL skipcheck_;
	HashFileProcessor hashfileprocessor_;
	std::function<void(HashFileProcessorProgressEventArgs)> progressevent_;
	std::function<void()> completeevent_;
};

#endif /* HASHCHECK_H_ */
