/*
 * FileHash.h
 *
 *  Created on: 2014-09-12
 *      Author: macote
 */

#ifndef FILEHASH_H_
#define FILEHASH_H_

#include "FileStream.h"

#include <stdexcept>
#include <string>
#include <Windows.h>

class FileHash : public FileStream
{
private:
	static const DWORD kDefaultBufferSize = 32768;
public:
	FileHash(const std::wstring &filepath) : FileHash(filepath, kDefaultBufferSize) { };
	FileHash(const std::wstring &filepath, const DWORD buffersize) : FileStream(filepath, FileStream::Mode::OpenNoBuffering, buffersize)
	{
		buffersize_ = buffersize;
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
	virtual void Update(UINT32 bytes) = 0;
	virtual void Finalize() = 0;
	virtual void ConvertHashToDigestString() = 0;
	PBYTE buffer_ = NULL;
	std::wstring digest_;
private:
	void AllocateBuffer();
	void FreeBuffer();
	DWORD buffersize_;
};

#endif /* FILEHASH_H_ */
