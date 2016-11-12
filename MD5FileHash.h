#pragma once

#include "FileHash.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>

struct MD5Context
{
	UINT32 state[4];	// state (ABCD)
	UINT32 count[2];	// number of bits, modulo 2^64 (lsb first)
	BYTE buffer[64];	// input buffer
};

class MD5FileHash : public FileHash
{
public:
	using FileHash::FileHash;
private:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize();
	void Transform(UINT32 state[4], PUINT32 buffer);
	void ConvertHashToDigestString();
	BYTE hash_[16];
	MD5Context context_;
};