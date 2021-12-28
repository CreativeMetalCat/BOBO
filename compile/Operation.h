#pragma once
#include "Types.h"
#include <vector>
#include "VariableManager.h"
#include "ErrorHandling/Exceptions.h"

uchar SwitchBasedOnRegistry(char regName, uchar base, uchar offset = 0x01, bool returnARegValue = false);

class Operation
{
private:
	/**Adds bytes for address element based on array index value
	*@param: operationByte What byte  to put in front of adress bytes*/
	 bool read_array_variable_addr(std::string arg, std::vector<uchar>& res,uchar operationByte = 0x3a);

	 bool read_variable_addr(std::string arg, std::vector<uchar>& res, uchar operationByte);
public:
	std::string name;
	std::vector<std::string> arguments;

	//what line is currently being compiled
	int32_t lineId = 0;

	VariableManager* varManager = nullptr;

	Operation(std::string name, std::vector<std::string> arguments, VariableManager* varManager)
		:name(name), arguments(arguments), varManager(varManager) {}

	std::vector<uchar> Compile(size_t currentProgramLenght = 0);

	size_t GetLenght();
};

std::vector<Operation*> ProcessOperation(std::vector<std::string>& operators, std::string& program, VariableManager*& manager, std::string resultRegistryName);