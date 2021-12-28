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
	std::vector<std::unique_ptr<Variable>> variables = {};

	size_t AddNew(std::string name, Variable::Type type, bool is_array = false, int32_t array_size = 1);

	std::unique_ptr<Variable>& Get(std::string name);

	bool Exists(std::string name);
};