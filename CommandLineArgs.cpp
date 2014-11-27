/*
* CommandLineArgs.cpp
*
*  Created on: 2014-11-21
*      Author: MAC
*/

#include "CommandLineArgs.h"

void CommandLineArgs::ParseArguments()
{
	bool inQuote = false;
	int argStart = 0;
	bool isFollowingSpace = false, isSpace = false, isQuote = false;
	char currentChar, lastChar = '\x0';
	for (unsigned int i = 0; i < commandline_.length(); i++)
	{
		currentChar = commandline_[i];
		isQuote = currentChar == '"';
		isSpace = currentChar == ' ';
		isFollowingSpace = (lastChar == ' ');
		lastChar = currentChar;
		if (inQuote)
		{
			inQuote = !isQuote;
			continue;
		}
		if (isQuote)
		{
			if (argStart == 0)
			{
				argStart = i;
			}
			inQuote = true;
			continue;
		}
		if (isSpace)
		{
			if (!isFollowingSpace)
			{
				if (argStart == 0)
				{
					arguments_.push_back(commandline_.substr(argStart, i));
				}
				else if (argStart > 0)
				{
					arguments_.push_back(commandline_.substr(argStart, i - argStart));
					argStart = 0;
				}
			}
		}
		else if (isFollowingSpace)
		{
			if (!isSpace)
			{
				argStart = i;
			}
		}
	}
	if (argStart > 0 || (!isFollowingSpace && arguments_.size() == 0))
		arguments_.push_back(commandline_.substr(argStart, commandline_.length() - argStart));
}