/*
* StreamLineWriter.h
*
*  Created on: 2014-12-17
*      Author: MAC
*/

#ifndef STREAMLINEWRITER_H_
#define STREAMLINEWRITER_H_

#include "FileStream.h"

class StreamLineWriter
{
public:
	enum Encoding
	{
		UTF8
	};
	StreamLineWriter(FileStream& filestream, Encoding encoding) : filestream_(filestream), encoding_(encoding) { }
	StreamLineWriter(FileStream& filestream) : StreamLineWriter(filestream, Encoding::UTF8) { }
	~StreamLineWriter()
	{
		Close();
	}
	void Write(std::wstring line);
	void WriteLine(std::wstring line);
	void Close();
private:
	void WriteEOL();
	FileStream& filestream_;
	Encoding encoding_;
};

#endif /* STREAMLINEWRITER_H_ */