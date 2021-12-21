#pragma once
#include "Types.h"
#include <vector>
#include "Variable.h"

class VariableManager
{
private:
	size_t _current_offset;
public:
	std::vector<Variable*> variables = {};

	size_t AddNew(std::string name, Variable::Type type)
	{
		variables.push_back(new Variable(name, type, _current_offset));
		_current_offset += (char)type > 2 ? 2 : 1;
		return _current_offset;
	}

	Variable* Get(std::string name)
	{
		std::vector<Variable*>::iterator it = std::find_if(variables.begin(), variables.end(), [name](Variable* var) {return var->name == name; });
		return it == variables.end() ? nullptr : *it;
	}
};