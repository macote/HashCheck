/*
 * HashCheck.h
 *
 *  Created on: 2014-09-09
 *      Author: macote
 */

#ifndef HASHCHECK_H_
#define HASHCHECK_H_

#include <string>
#include <map>

typedef struct tagFileInfo {
	std::string FileName;
	LARGE_INTEGER Size;
	std::string Digest;
} FileInfo, *PFileInfo;

typedef std::map<std::string, PFileInfo, std::less<std::string>> MAPFILES;
typedef MAPFILES::iterator MAPFILESITER;

const char * CHECKSUM_FILE_BASE = "checksum";

enum HashTypeEnum {
	htCRC32,
	htMD5,
	htSHA1,
	htUndefined
};

// Prototypes

std::string GetAppFileName(LPSTR AppTitle);
int Verify(LPCTSTR tempFileName);
int Create(void);
int Update(LPCTSTR tempFileName);
BOOL IsValidChecksumLine(std::string SHA1Line);
BOOL IsFileEmpty(HANDLE hFile);
std::string LargeIntToString(const LARGE_INTEGER& li);
BOOL ChecksumCompare(PFileInfo ChecksumInfo1, PFileInfo ChecksumInfo2);
void CalculateChecksum(HANDLE hFile, PFileInfo pFileInfo, std::string filePath);
void ProcessTree(std::string sPath, std::ostream& out);
BOOL ViewReport(LPSTR TempFile);

#endif /* HASHCHECK_H_ */
