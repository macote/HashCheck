/*
 * SHA1FileHash.h
 *
 *  Created on: 2014-10-23
 *      Author: MAC
 */

#ifndef SHA1FILEHASH_H_
#define SHA1FILEHASH_H_

#include "windows.h"
#include "FileHash.h"

struct SHA1Context
{
	UINT32 state[5];	// state (ABCDE)
	UINT32 count[2];	// bits
	BYTE buffer[64];	// input buffer
};

class SHA1FileHash : public FileHash
{
public:
#if defined(_MSC_VER) && _MSC_VER < 1900
	SHA1FileHash(const std::string &filepath, const DWORD buffersize) : FileHash(filepath, buffersize) {};
	SHA1FileHash(const std::string &filepath) : FileHash(filepath) {};
#else
	using FileHash::FileHash;
#endif
protected:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize();
private:
	void Transform(UINT32 state[5], PUINT32 buffer);
	void ConvertHashToDigestString();
	BYTE hash_[20];
	SHA1Context context_;
};

#endif /* SHA1FILEHASH_H_ */
