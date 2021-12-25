#pragma once
#include "Types.h"
#include <vector>
#include "Variable.h"

class VariableManager
{
private:
	size_t _current_offset;
public:
	size_t	program_lenght = 0u;
	std::vector<Variable*> variables = {};

	size_t AddNew(std::string name, Variable::Type type, bool is_array = false, int32_t array_size = 1);

	Variable* Get(std::string name);
};