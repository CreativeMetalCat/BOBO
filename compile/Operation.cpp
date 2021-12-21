#include "Operation.h"

uchar SwitchBasedOnRegistry(char regName, uchar base, uchar offset, bool returnARegValue)

{
	switch (regName)
	{
	case 'b':case 'B':
		return base;
		break;
	case 'c':case 'C':
		return base + (offset);
		break;
	case 'd':case 'D':
		return base + (offset * 2);
		break;
	case 'e':case 'E':
		return base + (offset * 3);
		break;
	case 'h':case 'H':
		return base + (offset * 4);
		break;
	case 'l':case 'L':
		return base + (offset * 5);
		break;
	case 'a':case 'A':
		return returnARegValue ? base + (offset * 7) : 0x00;
	default:
		return 0x00;
		break;
	}
}

std::vector<Operation*> ProcessOperation(std::vector<std::string>& operators, std::string& program, VariableManager*& manager, std::string resultRegistryName)

{
	std::vector<Operation*> res = {};
	//look for operators in the string
	size_t operatorStart = NPOS;
	//find any operator
	for (int i = 0; i < operators.size(); i++)
	{
		operatorStart = program.find_first_of(operators[i], 0);
		if (operatorStart != NPOS)
		{
			std::vector<std::string> args = {};
			//process left hand argument
			//note that for "=" left argument must be a variable
			std::string arg1 = program.substr(0, operatorStart);
			std::vector<Operation*> proc1 = ProcessOperation(operators, arg1, manager, "&a");
			if (proc1.empty())
			{
				args.push_back(arg1);
			}
			else
			{
				res.insert(res.end(), proc1.begin(), proc1.end());
				args.push_back("&a");
			}
			//if there is an operation then we return the operation

			//process right hand argument
			//seek till end of the line(maybe add support for divding via ";" in the future?) 
			std::string arg2 = program.substr(operatorStart + 1);
			//arg2 will have to go though the same operation as this 
			//so recursion
			std::vector<Operation*> proc2 = ProcessOperation(operators, arg2, manager, "&b");
			if (proc2.empty())
			{
				args.push_back(arg2);
			}
			else
			{
				res.insert(res.end(), proc2.begin(), proc2.end());
				args.push_back("&b");
			}
			args.push_back(resultRegistryName);
			res.push_back(new Operation(operators[i], args, manager));
			return res;
		}
	}

	//no operations found - the operation higher on the tree will just add the whole string as argument
	return {};
}


std::vector<uchar> Operation::Compile()

{
	std::vector<uchar> res;

	//wr is a special function that writes directly to the registry
	//why does it need to exist?
	//idk
	if (name == "wr")
	{
		if (arguments.size() != 2)
		{
			throw CompilationErrorException("Invalid number of arguments passed for function WR!");
		}


		if (Variable* var = varManager->Get(arguments[1]))
		{
			unsigned short addr = (var->promisedOffset + 0x800 + varManager->program_lenght);
			res.push_back(0x3a);

			res.push_back(addr & 0x00ff);
			res.push_back((addr & 0xff00) >> 8);
		}
		else
		{
			//b,c,d,e,h,l,m,a
			uchar base = 0;
			if (arguments[0] == "a")
			{
				//base = 0x78;
				base = 0x3e;
			}
			else if (arguments[0] == "b")
			{
				//base = 0x40;
				base = 0x06;
			}
			else if (arguments[0] == "c")
			{
				//base = 0x48;
				base = 0x0e;
			}
			else if (arguments[0] == "d")
			{
				//base = 0x50;
				base = 0x16;
			}
			else if (arguments[0] == "e")
			{
				//base = 0x58;
				base = 0x1e;
			}
			else if (arguments[0] == "h")
			{
				//base = 0x60;
				base = 0x26;
			}
			else if (arguments[0] == "l")
			{
				//base = 0x68;
				base = 0x2e;
			}
			else if (arguments[0] == "m")
			{
				//base = 0x70;
				base = 0x36;
			}
			else
			{
				throw CompilationErrorException("Error function WR expects a,b,c,d,e,h,l as first argument,but got " + arguments[0]);
			}
			res.push_back(base);

			res.push_back((uchar)atoi(arguments[1].c_str()));
		}


	}
	else if (name == "+")
	{
		/*
		* lda var1
		* mov a,b
		* lxi loc(arg2)
		* add m
		* mov a,_result_registry_name
		*/

		//if any argmuent is prefixed with & means it's refering to registry name
		//otherwise assume it is a variable or a number. 
		// ISSUE: both variabels and numbers lack & in them
		//TODO:Add note somewhere to not allow language users to use &a cause accumulator is used by calculations

		//are both arguments temp values?
		size_t arg1_name_start = arguments[0].find('&');
		size_t arg2_name_start = arguments[1].find('&');

		bool is_arg1_number = arguments[0].find_first_not_of("1234567890") == NPOS;
		bool is_arg2_number = arguments[1].find_first_not_of("1234567890") == NPOS;

		//both are registers(or numbers)
		if (arg1_name_start != NPOS && arg2_name_start != NPOS)
		{
			/*
			* mov a,_arg1_reg_name_
			* add _arg2_reg_name
			* mov _result_registry_name,a
			*/
			//move one of the values to Accumulator
			if (uchar result = SwitchBasedOnRegistry(arguments[0][arg1_name_start + 1], 0x4f, 0x08) != 0x00)
			{
				res.push_back(result);
			}
			//chose correct addition
			res.push_back(SwitchBasedOnRegistry(arguments[1][arg1_name_start + 1], 0x80, 0x01));
			//move result to desired registry for futher use
			res.push_back(SwitchBasedOnRegistry(arguments[2][arg1_name_start + 1], 0x78, 0x01));
		}
		//is the second argument variable?
		else if (arg1_name_start != NPOS && arg2_name_start == NPOS)
		{
			/*
			* mov a,_arg1_reg_name_
			* lxi arg2addr
			* add m
			* mov _result_registry_name,a
			*/
			//or if number
			/*
			* mov a,_arg1_reg_name_
			* adi number
			* mov _result_registry_name,a
			*/

			//move one of the values to Accumulator

			uchar result = 0x00;
			if ((result = SwitchBasedOnRegistry(arguments[0][arg1_name_start + 1], 0x4f, 0x08)) != 0x00)
			{
				res.push_back(result);
			}
			if (is_arg2_number)
			{
				res.push_back(0xc6);
				res.push_back((uchar)std::stoi(arguments[1]));
			}
			else if (Variable* var = varManager->Get(arguments[1]))
			{
				unsigned short addr = (var->promisedOffset +  + 0x800 + varManager->program_lenght);
				//lxi h,
				res.push_back(0x21);

				//addr
				res.push_back(addr & 0x00ff);
				res.push_back((addr & 0xff00) >> 8);

				//add m
				res.push_back(0x86);
			}
			else
			{
				throw CompilationErrorException("Error! Attempted to use undefined variable: " + arguments[1]);
			}

			//move result to desired registry for futher use
			res.push_back(SwitchBasedOnRegistry(arguments[2][arg1_name_start + 1], 0x78, 0x01));
		}
		//is the first argument variable
		else if (arg1_name_start == NPOS && arg2_name_start != NPOS)
		{
			/*
			* mov a,_arg1_reg_name_
			* lxi arg1addr
			* add m
			* mov _result_registry_name,a
			*/
			//or if number
			/*
			* mov a,_arg1_reg_name_
			* adi number
			* mov _result_registry_name,a
			*/

			//move one of the values to Accumulator
			uchar result = 0x00;
			if ((result = SwitchBasedOnRegistry(arguments[1][arg2_name_start + 1], 0x47, 0x08)) != 0x00)
			{
				res.push_back(result);
			}

			if (is_arg1_number)
			{
				res.push_back(0xc6);
				res.push_back((uchar)std::stoi(arguments[0]));
			}
			else if (Variable* var = varManager->Get(arguments[0]))
			{
				unsigned short addr = (var->promisedOffset +  + 0x800 + varManager->program_lenght);
				//lxi h,
				res.push_back(0x21);

				//addr
				res.push_back(addr & 0x00ff);
				res.push_back((addr & 0xff00) >> 8);

				//add m
				res.push_back(0x86);
			}
			else
			{
				throw CompilationErrorException("Error! Attempted to use undefined variable: " + arguments[0]);
			}
			//move result to desired registry for futher use
			res.push_back(SwitchBasedOnRegistry(arguments[2][arg1_name_start + 1], 0x78, 0x01));
		}
		//are both variables(or could be just numbers)
		else
		{
			/*
			lda varaddr
			mov b,a
			lda var2addr
			add b
			mov _result_registry_name,a
			*/
			//or
			/*
				mvi a, value1
				adi value2
				mov _result_registry_name,a
			*/

			//if both are numbers, just calculate before hand, who cares
			if (is_arg1_number && is_arg2_number)
			{
				//mvi a,
				res.push_back(0x3e);
				res.push_back((uchar)std::stoi(arguments[1]));
			}
			else
			{
				if (is_arg1_number)
				{
					//mvi a,
					res.push_back(0x3e);
					res.push_back((uchar)std::stoi(arguments[0]));
				}
				else if (Variable* var = varManager->Get(arguments[0]))
				{
					unsigned short addr = (var->promisedOffset +  + 0x800 + varManager->program_lenght);

					//lda
					res.push_back(0x3a);
					//addr
					res.push_back(addr & 0x00ff);
					res.push_back((addr & 0xff00) >> 8);
				}
				else
				{
					throw CompilationErrorException("Error! Attempted to use undefined variable: " + arguments[0]);
				}

				if (is_arg2_number)
				{
					res.push_back(0xc6);
					res.push_back((uchar)std::stoi(arguments[1]));
				}
				else if (Variable* var = varManager->Get(arguments[1]))
				{
					unsigned short addr = (var->promisedOffset +  + 0x800 + varManager->program_lenght);
					//lxi h,
					res.push_back(0x21);

					//addr
					res.push_back(addr & 0x00ff);
					res.push_back((addr & 0xff00) >> 8);

					//add m
					res.push_back(0x86);
				}
				else
				{
					throw CompilationErrorException("Error! Attempted to use undefined variable: " + arguments[1]);
				}

			}
		}
	}
	else if (name == "=")
	{
		//unlike math operations this one is very simple
		//sta and that's it
		//sta

		//if only numbers, then prepare them
		if (arguments[1].find_first_not_of("1234567890") == NPOS)
		{
			res.push_back(0x3e);
			res.push_back((uchar)std::stoi(arguments[1]));
		}
		//if result of something, prepare that too
		else if (arguments[1].find("&") != NPOS)
		{
			//move to a
			res.push_back(SwitchBasedOnRegistry(arguments[0][arguments[1].find("&") + 1], 0x78, 0x01, true));
		}
		if (Variable* var = varManager->Get(arguments[0]))
		{
			unsigned short addr = (var->promisedOffset +  + 0x800 + varManager->program_lenght);
			res.push_back(0x32);

			res.push_back(addr & 0x00ff);
			res.push_back((addr & 0xff00) >> 8);
		}
		//if it's variable defenition + initialization 
		else if (arguments[0].find("var") != NPOS)
		{
			/*
			* for offset we would need to know program lenght before compilation
			*/

			//declare variable and then use it
			varManager->AddNew(arguments[0].substr(arguments[0].find("var"), 3), Variable::Type::Byte);
			unsigned short addr = (varManager->variables[varManager->variables.size() - 1]->promisedOffset +  + 0x800 + varManager->program_lenght);
			res.push_back(0x32);

			res.push_back(addr & 0x00ff);
			res.push_back((addr & 0xff00) >> 8);
		}
	}
	return res;
}

size_t Operation::GetLenght()
{

	size_t res = 0u;
	if (name == "wr")
	{
		if (Variable* var = varManager->Get(arguments[1]))
		{
			res += 3u;
		}
		else
		{
			res += 2u;
		}
	}
	else if (name == "+")
	{
		size_t arg1_name_start = arguments[0].find('&');
		size_t arg2_name_start = arguments[1].find('&');

		bool is_arg1_number = arguments[0].find_first_not_of("1234567890") == NPOS;
		bool is_arg2_number = arguments[1].find_first_not_of("1234567890") == NPOS;

		//both are registers(or numbers)
		if (arg1_name_start != NPOS && arg2_name_start != NPOS)
		{
			res += 3u;
		}
		//is the second argument variable?
		else if (arg1_name_start != NPOS && arg2_name_start == NPOS)
		{
			res += 2u;
			if (is_arg2_number)
			{
				res += 2u;
			}
			else 
			{
				res += 3u;
			}		
		}
		//is the first argument variable
		else if (arg1_name_start == NPOS && arg2_name_start != NPOS)
		{
			res += 2u;
			if (is_arg1_number)
			{
				res += 2u;
			}
			else
			{
				res += 3u;
			}
		}
		//are both variables(or could be just numbers)
		else
		{
			//if both are numbers, just calculate before hand, who cares
			if (is_arg1_number && is_arg2_number)
			{
				res += 2u;
			}
			else
			{
				if (is_arg1_number)
				{
					res += 2u;
				}
				else 
				{
					res += 3u;
				}

				if (is_arg2_number)
				{
					res += 2u;
				}
				else
				{
					res += 4u;
				}

			}
		}
	}
	else if (name == "=")
	{
		//unlike math operations this one is very simple
		//sta and that's it
		//sta

		//if only numbers, then prepare them
		if (arguments[1].find_first_not_of("1234567890") == NPOS)
		{
			res += 2u;
		}
		//if result of something, prepare that too
		else if (arguments[1].find("&") != NPOS)
		{
			res += 1u;
		}
		if (Variable* var = varManager->Get(arguments[0]))
		{
			res += 3u;
		}
		//if it's variable defenition + initialization 
		else if (arguments[0].find("var") != NPOS)
		{
			res += 3u;
		}
	}
	return res;
}
