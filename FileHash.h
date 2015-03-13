/* Author: macote */

#ifndef FILEHASH_H_
#define FILEHASH_H_

#include "FileStream.h"
#include <string>
#include <Windows.h>

class FileHash
{
private:
	static const DWORD kDefaultBufferSize = 32768;
public:
	FileHash(const std::wstring& filepath) : FileHash(filepath, kDefaultBufferSize) { };
	FileHash(const std::wstring& filepath, const DWORD buffersize) 
		: buffersize_(buffersize), filestream_(FileStream(filepath, FileStream::Mode::OpenNoBuffering, buffersize))
	{
		AllocateBuffer();
	}
	virtual ~FileHash()
	{
		FreeBuffer();
	};
	void Compute();
	std::wstring digest() const { return digest_; }
protected:
	virtual void Initialize() = 0;
	virtual void Update(const UINT32 bytes) = 0;
	virtual void Finalize() = 0;
	virtual void ConvertHashToDigestString() = 0;
	PBYTE buffer_ = NULL;
	std::wstring digest_;
private:
	void AllocateBuffer();
	void FreeBuffer();
	DWORD buffersize_;
	FileStream filestream_;
};

#endif /* FILEHASH_H_ */
