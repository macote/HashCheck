#pragma once

#include "WinCryptFileHash.h"

class WinCryptSHA512FileHash : public WinCryptFileHash
{
public:
	static const DWORD kSHA512Length = 64;
public:
	WinCryptSHA512FileHash(std::wstring filepath) : WinCryptFileHash(filepath, CALG_SHA_512)
	{
	}
private:
	DWORD HashByteLength() { return kSHA512Length; }
	PBYTE hash() { return hash_; }
	BYTE hash_[kSHA512Length];
};
