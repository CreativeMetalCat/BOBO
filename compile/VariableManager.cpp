#include "VariableManager.h"
#include "ErrorHandling/Exceptions.h"

size_t VariableManager::AddNew(std::string name, Variable::Type type, bool is_array, int32_t array_size)
{
	variables.push_back(std::make_unique<Variable>(name, type, _current_offset, is_array, array_size));
	_current_offset += (char)type > 2 ? 2 : 1;
	return _current_offset;
}

std::unique_ptr<Variable>& VariableManager::Get(std::string name)
{
	std::vector<std::unique_ptr<Variable>>::iterator it = std::find_if(variables.begin(), variables.end(), [name](std::unique_ptr<Variable>& var) {return var->name == name; });
	if (it == variables.end())
	{
		throw VariableNotFoundException();
	}
	return *it;
}

bool VariableManager::Exists(std::string name)
{
	return std::find_if(variables.begin(), variables.end(), [name](std::unique_ptr<Variable>& var) {return var->name == name; }) != variables.end();
}
