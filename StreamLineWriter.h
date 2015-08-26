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
	StreamLineWriter(FileStream& filestream, Encoding encoding) : filestream_(filestream), encoding_(encoding) { }
	StreamLineWriter(FileStream& filestream) : StreamLineWriter(filestream, Encoding::UTF8) { }
	~StreamLineWriter() { Close(); }
	void Write(std::wstring line);
	void WriteLine(std::wstring line)
	{
		Write(line);
		WriteEOL();
	}
	void Close() { filestream_.Close(); }
private:
	void WriteEOL() { filestream_.Write((PBYTE)"\r\n", 2); }
	FileStream& filestream_;
	Encoding encoding_;
};

#endif /* STREAMLINEWRITER_H_ */
