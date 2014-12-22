/*
* FileStream.cpp
*
*  Created on: 2014-12-05
*      Author: MAC
*/

#include "FileStream.h"

void FileStream::AllocateBuffer()
{
	buffer_ = (PBYTE)VirtualAlloc(NULL, buffersize_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void FileStream::FreeBuffer()
{
	if (buffer_ != NULL)
	{
		VirtualFree(buffer_, 0, MEM_RELEASE);
	}
}

void FileStream::OpenFile()
{
	DWORD desiredaccess = (mode_ >= Create) ? GENERIC_WRITE : GENERIC_READ;
	DWORD createdisposition;
	switch (mode_)
	{
	case FileStream::Create:
		createdisposition = CREATE_NEW;
		break;
	case FileStream::Truncate:
		createdisposition = CREATE_ALWAYS;
		break;
	default:
		createdisposition = OPEN_EXISTING;
		break;
	}
	DWORD flagsandattributes = FILE_ATTRIBUTE_NORMAL;
	if (mode_ == OpenNoBuffering)
	{
		flagsandattributes |= FILE_FLAG_NO_BUFFERING;
	}
	filehandle_ = CreateFile(filepath_.c_str(), desiredaccess, FILE_SHARE_READ,
		NULL, createdisposition, flagsandattributes, NULL);
	if (filehandle_ == INVALID_HANDLE_VALUE)
	{
		lasterror_ = GetLastError();
		std::string msg = "FileStream.Open(CreateFile()) failed with error ";
		char errnum[24];
#ifdef _MSC_VER
		sprintf_s(errnum, 24, "0x%08X.", lasterror_);
#else
		_snprintf(errnum, 24, "0x%08X.", lasterror_);
#endif
		msg.append(errnum);
		throw std::runtime_error(msg.c_str());
	}
}

DWORD FileStream::Read(PBYTE buffer, DWORD offset, DWORD count)
{
	DWORD bytesread = 0;
	ReadFile(filehandle_, buffer + offset, count, &bytesread, NULL);
	return bytesread;
}

DWORD FileStream::Write(PBYTE buffer, DWORD offset, DWORD count)
{
	DWORD byteswritten = 0;
	WriteFile(filehandle_, buffer + offset, count, &byteswritten, NULL);
	return byteswritten;
}

void FileStream::FlushWrite()
{
	Write(buffer_, 0, writeindex_);
	writeindex_ = 0;
}

void FileStream::CloseFile()
{
	if (filehandle_ != INVALID_HANDLE_VALUE)
	{
		CloseHandle(filehandle_);
		filehandle_ = INVALID_HANDLE_VALUE;
	}
}

DWORD FileStream::Read(PBYTE buffer, DWORD count)
{
	DWORD bufferbytes = readlength_ - readindex_;
	BOOL eof = FALSE;
	if (bufferbytes == 0)
	{
		DWORD bytesread;
		if (count >= buffersize_)
		{
			bytesread = Read(buffer, 0, count);
			readindex_ = readlength_ = 0;
			return bytesread;
		}
		bytesread = Read(buffer_, 0, buffersize_);
		if (bytesread == 0) return 0;
		readindex_ = 0;
		readlength_ = bufferbytes = bytesread;
		eof = bytesread < buffersize_;
	}
	if (bufferbytes > count)
	{
		bufferbytes = count;
	}
	CopyMemory(buffer, buffer_ + readindex_, bufferbytes);
	readindex_ += bufferbytes;
	if (bufferbytes < count && !eof)
	{
		DWORD bytesread = Read(buffer, bufferbytes, count - bufferbytes);
		bufferbytes += bytesread;
		readindex_ = readlength_ = 0;
	}
	return bufferbytes;
}

void FileStream::Write(PBYTE buffer, DWORD count)
{
	DWORD bufferindex = 0;
	if (writeindex_ > 0)
	{
		DWORD bufferbytes = buffersize_ - writeindex_;
		if (bufferbytes > 0)
		{
			if (bufferbytes > count)
			{
				bufferbytes = count;
			}
			CopyMemory(buffer_ + writeindex_, buffer, bufferbytes);
			writeindex_ += bufferbytes;
			if (bufferbytes == count) return;
			bufferindex = bufferbytes;
			count -= bufferbytes;
		}
		Write(buffer_, 0, writeindex_);
		writeindex_ = 0;
	}
	if (count >= buffersize_)
	{
		Write(buffer, 0, count);
	}
	else if (count > 0)
	{
		CopyMemory(buffer_ + writeindex_, buffer + bufferindex, count);
		writeindex_ = count;
	}
}

void FileStream::Flush()
{
	if (writeindex_ > 0)
	{
		FlushWrite();
	}
}

void FileStream::Close()
{
	Flush();
	CloseFile();
}
