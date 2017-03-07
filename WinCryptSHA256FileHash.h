#pragma once

#include "WinCryptFileHash.h"

class WinCryptSHA256FileHash : public WinCryptFileHash
{
public:
	static const DWORD kSHA256Length = 32;
public:
	WinCryptSHA256FileHash(std::wstring filepath) : WinCryptFileHash(filepath, CALG_SHA_256)
	{
	}
private:
	DWORD HashByteLength() { return kSHA256Length; }
	PBYTE hash() { return hash_; }
	BYTE hash_[kSHA256Length];
};
