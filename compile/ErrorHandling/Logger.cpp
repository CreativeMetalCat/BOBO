#include "Logger.h"

void Logger::PrintError(std::string text, int line)
{
	std::cout << "\033[31m" << "Error: " << "\033[0m"  << text << " Line: " << line  << std::endl;
}

void Logger::PrintInfo(std::string text, int line)
{
	std::cout << "Note: " << text << " Line: " << line << std::endl;
}

void Logger::Print(std::string text)
{
	std::cout << text << std::endl;
}

void Logger::PrintWarning(std::string text, int line)
{
	std::cout << "\033[33m" << "Warning: " << "\033[0m"  << text << " Line: " << line  << std::endl;
}
