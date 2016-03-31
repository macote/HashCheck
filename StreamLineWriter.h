/* Author: macote */

#ifndef STREAMLINEWRITER_H_
#define STREAMLINEWRITER_H_

#include "FileStream.h"
#include <string>
#include <Windows.h>

class StreamLineWriter
{
public:
	enum class Encoding
	{
		UTF8
	};
	StreamLineWriter(std::wstring filepath, bool append) : StreamLineWriter(filepath, Encoding::UTF8, append)
	{
	}
	StreamLineWriter(std::wstring filepath, Encoding encoding, bool append)
		: filestream_(FileStream(filepath, append ? FileStream::Mode::Append : FileStream::Mode::Truncate)), encoding_(encoding)
	{
	}
	StreamLineWriter(FileStream&& filestream) : StreamLineWriter(std::forward<FileStream>(filestream), Encoding::UTF8)
	{
	}
	StreamLineWriter(FileStream&& filestream, Encoding encoding) : filestream_(std::move(filestream)), encoding_(encoding)
	{
	}
	StreamLineWriter(StreamLineWriter&& other)
		: filestream_(std::move(other.filestream_)), encoding_(other.encoding_)
	{
	}
	StreamLineWriter& operator=(StreamLineWriter&& other)
	{
		if (this != &other)
		{
			filestream_ = std::move(other.filestream_);
			encoding_ = other.encoding_;
		}

		return *this;
	}
	~StreamLineWriter()
	{ 
		Close(); 
	}
	void Write(std::wstring line);
	void WriteLine(std::wstring line)
	{
		Write(line);
		WriteEOL();
	}
	void Close() 
	{ 
		filestream_.Close(); 
	}
private:
	void WriteEOL() 
	{ 
		filestream_.Write((PBYTE)"\r\n", 2); 
	}
	FileStream filestream_;
	Encoding encoding_;
};

#endif /* STREAMLINEWRITER_H_ */
