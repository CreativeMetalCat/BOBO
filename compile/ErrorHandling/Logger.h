#pragma once
#include <iostream>

/*Define this to use default std::cout*/
#define LOG_STD_OUT


#ifdef LOG_STD_OUT
namespace Logger
{
	void PrintError(std::string text, int line = -1);

	void PrintInfo(std::string text, int line = -1);

	void PrintWarning(std::string text, int line = -1);

}
#endif //LOG_STD_OUT
