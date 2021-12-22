#pragma once
#include <string>
#include <vector>
#include <iostream>

class Logger
{
public:
	std::vector<std::string> Errors = {};
	
	std::vector<std::string> Warnings = {};

	std::vector<std::string> Info = {};

	void Log()
	{
		for (std::string error : Errors)
		{
			std::cout << "\033[32m" << "Error" << error.c_str() << "\033[0m" << std::endl;
		}
	}
};

