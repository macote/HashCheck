#pragma once

#include "FileStream.h"
#include <string>
#include <functional>
#include <Windows.h>

struct FileHashBytesProcessedEventArgs
{
	LARGE_INTEGER bytesprocessed;
};

class FileHash
{
public:
	static const DWORD kDefaultBufferSize = 2097152;
	static const DWORD kDefaultBytesProcessedNotificationBlockSize = 1048576;
public:
	FileHash(std::wstring filepath) : FileHash(filepath, kDefaultBufferSize)
	{ 
	}
	FileHash(std::wstring filepath, DWORD buffersize) 
		: buffersize_(buffersize), filestream_(FileStream(filepath, FileStream::Mode::OpenWithoutBuffering, buffersize))
	{
		AllocateBuffer();
	}
	virtual ~FileHash() 
	{ 
		FreeBuffer(); 
	}
	void Compute(BOOL& cancellationflag);
	std::wstring digest() const { return digest_; }
	void SetBytesProcessedEventHandler(std::function<void(FileHashBytesProcessedEventArgs)> handler)
	{
		SetBytesProcessedEventHandler(handler, kDefaultBytesProcessedNotificationBlockSize);
	}
	void SetBytesProcessedEventHandler(std::function<void(FileHashBytesProcessedEventArgs)> handler, 
		const DWORD bytesprocessednotificationblocksize)
	{
		bytesprocessedevent_ = handler;
		bytesprocessednotificationblocksize_ = bytesprocessednotificationblocksize;
	}
protected:
	virtual void Initialize() = 0;
	virtual void Update(UINT32 bytecount) = 0;
	virtual void Finalize() = 0;
	virtual void ConvertHashToDigestString() = 0;
	std::wstring ConvertByteArrayToHexString(BYTE arr[], UINT length);
	std::wstring ConvertUInt32ToHexString(UINT32 value);
	PBYTE buffer_{ NULL };
	std::wstring digest_;
private:
	void AllocateBuffer();
	void FreeBuffer();
	DWORD buffersize_;
	FileStream filestream_;
	DWORD bytesprocessednotificationblocksize_{};
	std::function<void(FileHashBytesProcessedEventArgs)> bytesprocessedevent_{ nullptr };
};