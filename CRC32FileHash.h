#pragma once

#include "FileHash.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>

class CRC32FileHash : public FileHash
{
public:
	const static UINT32 kCRC32Table[];
public:
	using FileHash::FileHash;
private:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize()
	{
		hash_ = ~hash_;
	}
	void ConvertHashToDigestString()
	{
		digest_ = ConvertUInt32ToHexString(hash_);
	}
	UINT32 hash_{};
};
