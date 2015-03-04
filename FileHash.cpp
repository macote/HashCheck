/* Author: macote */

#include "FileHash.h"

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
	DWORD bytesread = 0;
	do
	{
		bytesread = Read(buffer_, buffersize_);
		if (bytesread > 0)
		{
			Update(bytesread);
		}
	}
	while (bytesread > 0);
	Finalize();
	ConvertHashToDigestString();
}
