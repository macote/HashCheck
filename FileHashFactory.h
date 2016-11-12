#pragma once

#include "FileHash.h"
#include "HashType.h"
#include "CRC32FileHash.h"
#include "WinCryptSHA1FileHash.h"
#include "WinCryptMD5FileHash.h"
#include <string>
#include <memory>
#include <Windows.h>

class FileHashFactory
{
public:
	static std::unique_ptr<FileHash> Create(HashType hashtype, std::wstring filepath)
	{ 
		if (hashtype == HashType::SHA1)
		{
			return std::make_unique<WinCryptSHA1FileHash>(filepath);
		}
		else if (hashtype == HashType::MD5)
		{
			return std::make_unique<WinCryptMD5FileHash>(filepath);
		}
		else if (hashtype == HashType::CRC32)
		{
			return std::make_unique<CRC32FileHash>(filepath);
		}
		else
		{
			throw std::runtime_error("FileHashFactory::Create(): the specified hash type is not supported.");
		}
	}
};