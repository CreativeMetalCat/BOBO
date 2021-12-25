#include "VariableManager.h"

size_t VariableManager::AddNew(std::string name, Variable::Type type, bool is_array, int32_t array_size)
{
	variables.push_back(new Variable(name, type, _current_offset, is_array, array_size));
	_current_offset += (char)type > 2 ? 2 : 1;
	return _current_offset;
}

Variable* VariableManager::Get(std::string name)
{
	std::vector<Variable*>::iterator it = std::find_if(variables.begin(), variables.end(), [name](Variable* var) {return var->name == name; });
	return it == variables.end() ? nullptr : *it;
}
