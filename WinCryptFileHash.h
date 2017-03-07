#pragma once

#include "FileHash.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>
#include <wincrypt.h>

class WinCryptFileHash : public FileHash
{
public:
	WinCryptFileHash(std::wstring filepath, ALG_ID alg_id) : FileHash(filepath), alg_id_(alg_id)
	{
	}
protected:
	virtual DWORD HashByteLength() = 0;
	virtual PBYTE hash() = 0;
	ALG_ID alg_id_{};
private:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize();
	void ConvertHashToDigestString();
	HCRYPTPROV cryptprov_{};
	HCRYPTHASH crypthash_{};
};

inline void WinCryptFileHash::Initialize()
{
	if (!CryptAcquireContext(&cryptprov_, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		throw std::runtime_error("WinCryptFileHash::Initialize(): CryptAcquireContext() failed.");
	}

	if (!CryptCreateHash(cryptprov_, alg_id_, 0, 0, &crypthash_))
	{
		throw std::runtime_error("WinCryptFileHash::Initialize(): CryptCreateHash() failed.");
	}
}

inline void WinCryptFileHash::Update(UINT32 bytecount)
{
	if (!CryptHashData(crypthash_, buffer_, bytecount, 0))
	{
		throw std::runtime_error("WinCryptFileHash::Update(): CryptHashData() failed.");
	}
}

inline void WinCryptFileHash::Finalize()
{
	auto hashbytelength = HashByteLength();
	if (!CryptGetHashParam(crypthash_, HP_HASHVAL, hash(), &hashbytelength, 0))
	{
		throw std::runtime_error("WinCryptFileHash::Finalize(): CryptGetHashParam() failed.");
	}

	if (crypthash_ != 0)
	{
		CryptDestroyHash(crypthash_);
	}

	if (cryptprov_ != 0)
	{
		CryptReleaseContext(cryptprov_, 0);
	}
}

inline void WinCryptFileHash::ConvertHashToDigestString()
{
	digest_ = ConvertByteArrayToHexString(hash(), HashByteLength());
}