/*
 * MD5FileHash.h
 *
 *  Created on: 2014-10-22
 *      Author: MAC
 */

#ifndef MD5FILEHASH_H_
#define MD5FILEHASH_H_

#include "windows.h"
#include "FileHash.h"

struct MD5Context
{
	UINT32 state[4];	// state (ABCD)
	UINT32 count[2];	// number of bits, modulo 2^64 (lsb first)
	BYTE buffer[64];	// input buffer
};

class MD5FileHash : public FileHash
{
public:
#if _MSC_VER < 1900
	MD5FileHash(const std::string &filepath, const DWORD buffersize) : FileHash(filepath, buffersize) {};
	MD5FileHash(const std::string &filepath) : FileHash(filepath) {};
#else
	using FileHash::FileHash;
#endif
protected:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize();
private:
	void Transform(UINT32 state[4], PUINT32 buffer);
	void ConvertHashToDigestString();
	BYTE hash_[16];
	MD5Context context_;
};

#endif /* MD5FILEHASH_H_ */
