/*
* CommandLineArgs.h
*
*  Created on: 2014-11-21
*      Author: MAC
*/

#ifndef COMMANDLINEARGS_H_
#define COMMANDLINEARGS_H_

#include <string>
#include <map>
#include <vector>

#include <windows.h>

class CommandLineArgs
{
public:
	CommandLineArgs(LPSTR commandline) : commandline_(commandline)
	{ 
		ParseArguments();
	}
	std::string commandline() const { return commandline_; }
	const std::vector<std::string>& arguments() const { return arguments_; }
private:
	void ParseArguments();
	std::vector<std::string> arguments_;
	std::string commandline_;
};

#endif /* COMMANDLINEARGS_H_ */