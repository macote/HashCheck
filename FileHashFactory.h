#pragma once

#include "FileHash.h"
#include "HashType.h"
#include "CRC32FileHash.h"
#include "WinCryptSHA512FileHash.h"
#include "WinCryptSHA256FileHash.h"
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
		switch (hashtype)
		{
		case HashType::CRC32:
			return std::make_unique<CRC32FileHash>(filepath);
			break;
		case HashType::MD5:
			return std::make_unique<WinCryptMD5FileHash>(filepath);
			break;
		case HashType::SHA1:
			return std::make_unique<WinCryptSHA1FileHash>(filepath);
			break;
		case HashType::SHA256:
			return std::make_unique<WinCryptSHA256FileHash>(filepath);
			break;
		case HashType::SHA512:
			return std::make_unique<WinCryptSHA512FileHash>(filepath);
			break;
		default:
			throw std::runtime_error("FileHashFactory::Create(): the specified hash type is not supported.");
		}
	}
};