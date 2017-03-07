#pragma once

#include <string>

enum class HashType
{
	CRC32,
	MD5,
	SHA1,
	SHA256,
	SHA512,
	Undefined
};

class HashTypeStrings
{
public:
	static std::wstring GetHashTypeString(HashType hastype);
};

inline std::wstring HashTypeStrings::GetHashTypeString(HashType hashtype)
{
	switch (hashtype)
	{
	case HashType::CRC32:
		return std::wstring(L"CRC32");
		break;
	case HashType::MD5:
		return std::wstring(L"MD5");
		break;
	case HashType::SHA1:
		return std::wstring(L"SHA1");
		break;
	case HashType::SHA256:
		return std::wstring(L"SHA256");
		break;
	case HashType::SHA512:
		return std::wstring(L"SHA512");
		break;
	case HashType::Undefined:
		return std::wstring(L"Undefined");
		break;
	default:
		return std::wstring(L"Unknown");
		break;
	}
}