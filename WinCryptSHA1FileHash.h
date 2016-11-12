#pragma once

#include "WinCryptFileHash.h"

class WinCryptSHA1FileHash : public WinCryptFileHash
{
public:
	static const DWORD kSHA1Length = 20;
public:
	WinCryptSHA1FileHash(std::wstring filepath) : WinCryptFileHash(filepath, CALG_SHA1)
	{
	}
private:
	DWORD HashByteLength() { return WinCryptSHA1FileHash::kSHA1Length; }
	PBYTE buffer() { return buffer_; };
	PBYTE hash() { return hash_; }
	BYTE hash_[kSHA1Length];
};
