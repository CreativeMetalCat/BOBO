#pragma once

#include <exception>
#include <string>

/*-Current goal: At least be able to proccess strings into tokens
+Current goal: figure out basic coversion of programmes
figure out how to add the proc functionality
+Current goal: figure out how to do variables,while still allowing for memory
+better logging system, instead of just trhowing exceptions everywhere*/
class ParsingErrorException : public std::exception
{
public:
	ParsingErrorException(std::string msg) : std::exception(msg.c_str()) {}
};


class CompilationErrorException : public std::exception
{
public:
	CompilationErrorException(std::string msg) : std::exception(msg.c_str()) {}
};

class VariableNotFoundException : public std::exception
{
public:
	VariableNotFoundException(): std::exception() {}
};