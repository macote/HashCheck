/* Author: macote */

#ifndef STREAMLINEREADER_H_
#define STREAMLINEREADER_H_

#include "FileStream.h"
#include <string>
#include <Windows.h>

class StreamLineReader
{
private:
	static const DWORD kDefaultBufferSize = 32768;
public:
	enum class Encoding
	{
		UTF8
	};
	StreamLineReader(std::wstring filepath) : StreamLineReader(filepath, Encoding::UTF8)
	{
	}
	StreamLineReader(std::wstring filepath, Encoding encoding)
		: filestream_(FileStream(filepath, FileStream::Mode::Open)), encoding_(encoding), buffersize_(kDefaultBufferSize)
	{
		AllocateBuffer();
	}
	StreamLineReader(FileStream&& filestream) : StreamLineReader(std::forward<FileStream>(filestream), Encoding::UTF8)
	{
	}
	StreamLineReader(FileStream&& filestream, Encoding encoding) : filestream_(std::move(filestream)), encoding_(encoding), buffersize_(kDefaultBufferSize)
	{
		AllocateBuffer();
	}
	StreamLineReader(StreamLineReader&& other)
		: filestream_(std::move(other.filestream_)), encoding_(other.encoding_)
	{
		buffersize_ = other.buffersize_;
		readindex_ = other.readindex_;
		readlength_ = other.readlength_;
		buffer_ = other.buffer_;
		other.buffer_ = NULL;
	}
	StreamLineReader& operator=(StreamLineReader&& other)
	{
		if (this != &other)
		{
			filestream_ = std::move(other.filestream_);
			encoding_ = other.encoding_;
			buffersize_ = other.buffersize_;
			readindex_ = other.readindex_;
			readlength_ = other.readlength_;
			buffer_ = other.buffer_;
			other.buffer_ = NULL;
		}

		return *this;
	}
	~StreamLineReader() 
	{ 
		FreeBuffer(); 
	}
	std::wstring ReadLine();
	BOOL EndOfStream() 
	{ 
		return ReadBytes() == 0; 
	}
	void Close() 
	{ 
		filestream_.Close(); 
	}
private:
	void AllocateBuffer();
	void FreeBuffer();
	DWORD ReadBytes();
	FileStream filestream_;
	Encoding encoding_;
	PBYTE buffer_;
	DWORD buffersize_;
	DWORD readindex_{};
	DWORD readlength_{};
};

#endif /* STREAMLINEREADER_H_ */
