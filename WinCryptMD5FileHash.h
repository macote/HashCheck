#pragma once

#include "WinCryptFileHash.h"

class WinCryptMD5FileHash : public WinCryptFileHash
{
public:
	static const DWORD kMD5Length = 16;
public:
	WinCryptMD5FileHash(std::wstring filepath) : WinCryptFileHash(filepath, CALG_MD5)
	{
	}
private:
	DWORD HashByteLength() { return kMD5Length; }
	PBYTE hash() { return hash_; }
	BYTE hash_[kMD5Length];
};
