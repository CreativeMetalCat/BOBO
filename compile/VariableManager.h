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

	size_t AddNew(std::string name, Variable::Type type,bool is_array = false,int32_t array_size = 1)
	{
		variables.push_back(new Variable(name, type, _current_offset,is_array,array_size));
		_current_offset += (char)type > 2 ? 2 : 1;
		return _current_offset;
	}

	Variable* Get(std::string name)
	{
		std::vector<Variable*>::iterator it = std::find_if(variables.begin(), variables.end(), [name](Variable* var) {return var->name == name; });
		return it == variables.end() ? nullptr : *it;
	}
};