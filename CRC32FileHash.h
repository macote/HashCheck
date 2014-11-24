/*
 * CRC32FileHash.h
 *
 *  Created on: 2014-10-20
 *      Author: MAC
 */

#ifndef CRC32FILEHASH_H_
#define CRC32FILEHASH_H_

#include "windows.h"
#include "FileHash.h"

class CRC32FileHash : public FileHash
{
public:
#if _MSC_VER < 1900
	CRC32FileHash(const std::string &filepath, const DWORD buffersize) : FileHash(filepath, buffersize) {};
	CRC32FileHash(const std::string &filepath) : FileHash(filepath) {};
#else
	using FileHash::FileHash;
#endif
protected:
	void Initialize();
	void Update(UINT32 bytecount);
	void Finalize();
private:
	void ConvertHashToDigestString();
	const static UINT32 kCRC32Table[];
	UINT32 hash_;
};

#endif /* CRC32FILEHASH_H_ */
