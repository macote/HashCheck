#pragma once

#include "FileStream.h"
#include "StreamLineWriter.h"
#include <string>
#include <list>
#include <Windows.h>

class Report
{
public:
	void AddLine(const std::wstring& line)
	{ 
		report_.push_back(line); 
	}
	void Save(const std::wstring& reportpath) const
	{
		StreamLineWriter reportfilewriter(reportpath, false);
		for (auto& item : report_)
		{
			reportfilewriter.WriteLine(item);
		}
	}
	BOOL IsEmpty()
	{ 
		return report_.size() == 0; 
	}
private:
	std::list<std::wstring> report_;
};