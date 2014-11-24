/*
 * FileHash.cc
 *
 *  Created on: 2014-10-20
 *      Author: MAC
 */

#include <stdexcept>

#include "CRC32FileHash.h"
#include "windows.h"

void FileHash::AllocateBuffer()
{
	buffer_ = (PBYTE)VirtualAlloc(NULL, buffersize_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void FileHash::FreeBuffer()
{
	if (buffer_ != NULL)
	{
		VirtualFree(buffer_, 0, MEM_RELEASE);
	}
}

void FileHash::Compute()
{
	Initialize();
	Open();
	DWORD bytesread = 0;
	do
	{
		bytesread = Read();
		if (bytesread > 0)
		{
			Update(bytesread);
		}
	}
	while (bytesread > 0);
	Close();
	Finalize();
	ConvertHashToDigestString();
}

void FileHash::Open()
{
	filehandle_ = CreateFile(filepath_.c_str(), GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
	if (filehandle_ == INVALID_HANDLE_VALUE)
	{
		// try again without FILE_FLAG_NO_BUFFERING
		filehandle_ = CreateFile(filepath_.c_str(), GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (filehandle_ == INVALID_HANDLE_VALUE)
	{
		lasterror_ = GetLastError();
		throw std::runtime_error("CreateFile() failed.");
	}
}

DWORD FileHash::Read()
{
	DWORD bytesread = 0;
	ReadFile(filehandle_, buffer_, buffersize_, &bytesread, NULL);
	return bytesread;
}

void FileHash::Close()
{
	if (filehandle_ != INVALID_HANDLE_VALUE)
	{
		CloseHandle(filehandle_);
	}
}

