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
#include <set>

class CommandLineArgs
{
public:
	CommandLineArgs();
	std::string commandLineArguments() const { return commandLineArguments_; }
private:
	std::set<std::string> arguments_;
	std::map<std::string, std::string> parameters_;
	std::string commandLineArguments_;
};

#endif /* COMMANDLINEARGS_H_ */