#pragma once
#include "Types.h"
#include <vector>
#include "VariableManager.h"
#include "ErrorHandling/Exceptions.h"

uchar SwitchBasedOnRegistry(char regName, uchar base, uchar offset = 0x01, bool returnARegValue = false);

class Operation
{
public:
	std::string name;
	std::vector<std::string> arguments;

	VariableManager* varManager = nullptr;

	Operation(std::string name, std::vector<std::string> arguments, VariableManager* varManager)
		:name(name), arguments(arguments), varManager(varManager) {}

	std::vector<uchar> Compile();
};

std::vector<Operation*> ProcessOperation(std::vector<std::string>& operators, std::string& program, VariableManager*& manager, std::string resultRegistryName);