/*
 * FileHash.h
 *
 *  Created on: 2014-09-12
 *      Author: MAC
 */

#ifndef FILEHASH_H_
#define FILEHASH_H_

#include <string>

// file hash computer
class FileHash
{
public:
	FileHash(const std::string &filepath, const DWORD buffersize) : filepath_(filepath), buffersize_(buffersize)
	{
		AllocateBuffer();
	};
	FileHash(const std::string &filepath) : FileHash(filepath, kDefaultBufferSize) {};
	virtual ~FileHash()
	{
		FreeBuffer();
	};
	void Compute();
	std::string digest() const { return digest_; }
	DWORD lasterror() const { return lasterror_; }
protected:
	virtual void Initialize() = 0;
	void Open();
	DWORD Read();
	virtual void Update(UINT32 bytes) = 0;
	void Close();
	virtual void Finalize() = 0;
	virtual void ConvertHashToDigestString() = 0;
	const std::string filepath_;
	const DWORD buffersize_;
	PBYTE buffer_ = NULL;
	std::string digest_;
private:
	void AllocateBuffer();
	void FreeBuffer();
	HANDLE filehandle_ = NULL;
	DWORD lasterror_ = 0;
	static const DWORD kDefaultBufferSize = 32768;
};

#endif /* FILEHASH_H_ */
