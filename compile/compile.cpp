// compile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>

#define NPOS std::string::npos

//#define OLD_PROC_OPER

typedef unsigned char uchar;


uchar SwitchBasedOnRegistry(char regName,uchar base, uchar offset = 0x01,bool returnARegValue = false)
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
		return returnARegValue  ? base + (offset * 7)  : 0x00;
	default:
		return 0x00;
		break;
	}
}


/*-Current goal: At least be able to proccess strings into tokens
+Current goal: figure out basic coversion of programmes
figure out how to add the proc functionality
+Current goal: figure out how to do variables,while still allowing for memory
+better logging system, instead of just trhowing exceptions everywhere*/
class ParsingErrorException : public std::exception
{
public:
	ParsingErrorException(std::string msg) : std::exception(msg.c_str()) {}
};


class CompilationErrorException : public std::exception
{
public:
	CompilationErrorException(std::string msg) : std::exception(msg.c_str()) {}
};



/*:(*/
class Variable
{
public:
	enum Type : uchar
	{
		Byte,
		LongByte,
		UnsignedByte,
		LongUnsignedByte
	};
	std::string name;
	Type type;
	/*Because variables will just use memory located right after programm we need to keep track of what cells are promised to it*/
	size_t promisedOffset = 0;

	Variable(std::string name, Type type, size_t promisedOffset) :
		name(name), type(type), promisedOffset(promisedOffset) {}
};

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

class Operation
{
public:
	std::string name;
	std::vector<std::string> arguments;

	VariableManager* varManager = nullptr;

	Operation(std::string name, std::vector<std::string> arguments, VariableManager* varManager)
		:name(name), arguments(arguments),varManager(varManager) {}

	std::vector<uchar> Compile()
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
			

			if(Variable*var = varManager->Get(arguments[1]))
			{
				unsigned short addr = (var->promisedOffset + 0x850);
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
			if (arg1_name_start  != NPOS && arg2_name_start != NPOS)
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
					unsigned short addr = (var->promisedOffset + 0x850);
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
				if (( result = SwitchBasedOnRegistry(arguments[1][arg2_name_start + 1], 0x47, 0x08)) != 0x00)
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
					unsigned short addr = (var->promisedOffset + 0x850);
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
						unsigned short addr = (var->promisedOffset + 0x850);

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
						unsigned short addr = (var->promisedOffset + 0x850);
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
			else if(arguments[1].find("&") != NPOS)
			{
				//move to a
				res.push_back(SwitchBasedOnRegistry(arguments[0][arguments[1].find("&") + 1], 0x78, 0x01,true));
			}
			if (Variable* var = varManager->Get(arguments[0]))
			{
				unsigned short addr = (var->promisedOffset + 0x850);
				res.push_back(0x32);

				res.push_back(addr & 0x00ff);
				res.push_back((addr & 0xff00) >> 8);
			}
		}
		else if (name == "eq_sum")
		{
			/*
			lda varaddr
			mov b,a
			lda var2addr
			add b
			sta var3 addr*/
			//addr1 = a
			//addr2 = b
			//addr 3 = c
			// c = a+b
			if (Variable* var1 = varManager->Get(arguments[0]))
			{
				unsigned short addr1 = var1->promisedOffset + 0x850;

				res.push_back(0x3a);
				res.push_back(addr1 & 0x00ff);
				res.push_back((addr1 & 0xff00) >> 1);

				if (Variable* var2 = varManager->Get(arguments[1]))
				{
					unsigned short addr2 = var2->promisedOffset + 0x850;
					res.push_back(0x47);

					res.push_back(0x3a);
					res.push_back(addr2 & 0x00ff);
					res.push_back((addr2 & 0xff00) >> 1);

					res.push_back(0x80);
					if (Variable* var3 = varManager->Get(arguments[2]))
					{
						unsigned short addr3 = var3->promisedOffset + 0x850;
						res.push_back(0x32);
						res.push_back(addr3 & 0x00ff);
						res.push_back((addr3 & 0xff00) >> 1);
					}
					else
					{
						throw CompilationErrorException("Error! Refenced variable: " + arguments[2] + " does not exist in code");
					}
				}
				else
				{
					throw CompilationErrorException("Error! Refenced variable: " + arguments[1] + " does not exist in code");
				}
			}
			else
			{
				throw CompilationErrorException("Error! Refenced variable: " + arguments[0] + " does not exist in code");
			}


		}
		return res;
	}
};




std::vector<std::string> Split(std::string& str, char separator)
{
	//reslt vector 
	std::vector<std::string>res;
	//string stream for std::getline
	std::istringstream iss_str(str);
	//temp thing
	std::string item;
	while (std::getline(iss_str, item, separator))
	{
		res.push_back(item);
	}
	return res;
}

std::vector<std::string> ProcessOperationS(std::vector<std::string>& operators, std::string& program, bool& finished)
{
	finished = false;
	std::vector<std::string> res = {};
	//look for operators in the string
	size_t operatorStart = NPOS;
	//find any operator
	for (int i = 0; i < operators.size(); i++)
	{
		operatorStart = program.find_first_of(operators[i], 0);
		if (operatorStart != NPOS)
		{
			//found operation

			//process left hand argument
			//note that for "=" left argument must be a variable
			std::string arg1 = program.substr(0, operatorStart);
			std::vector<std::string> proc1 = ProcessOperationS(operators, arg1, finished);
			if (proc1.empty())
			{
				res.push_back(arg1);
			}
			else
			{
				res.insert(res.end(), proc1.begin(), proc1.end());
			}
			//process right hand argument
			//seek till end of the line(maybe add support for divding via ";" in the future?) 
			//Equal But Against Less(EBAL)
			std::string arg2 = program.substr(operatorStart + 1);
			//arg2 will have to go though the same operation as this 
			//so recursion
			std::vector<std::string> proc2 = ProcessOperationS(operators, arg2, finished);
			if (proc2.empty())
			{
				res.push_back(arg2);
			}
			else
			{
				res.insert(res.end(), proc2.begin(), proc2.end());
			}
			int i = 0;
		}
	}
	finished = true;
	return res;
}

//should instead return result memory addr or argument itself
std::vector<Operation*> ProcessOperation(std::vector<std::string>& operators, std::string& program, VariableManager*& manager,std::string resultRegistryName)
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
			std::vector<Operation*> proc1 = ProcessOperation(operators, arg1, manager,"&a");
			if (proc1.empty())
			{
				args.push_back(arg1);
			}
			else
			{
				res.insert(res.end(), proc1.begin(),proc1.end());
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

#ifdef OLD_PROC_OPER
std::vector<Operation*> ProcessOperationA(std::vector<std::string>& operators, std::string& program, VariableManager*& manager)
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
			//found operation
			std::vector<std::string> args = {};
			//process left hand argument
			//note that for "=" left argument must be a variable
			std::string arg1 = program.substr(0, operatorStart);
			std::vector<Operation*> proc1 = ProcessOperation(operators, arg1, manager);
			if (proc1.empty())
			{
				args.push_back(arg1);
			}
			else
			{
				res.insert(res.end(), proc1.begin(), proc1.end());
			}
			//process right hand argument
			//seek till end of the line(maybe add support for divding via ";" in the future?) 
			//Equal But Against Less(EBAL)
			std::string arg2 = program.substr(operatorStart + 1);
			//arg2 will have to go though the same operation as this 
			//so recursion
			std::vector<Operation*> proc2 = ProcessOperation(operators, arg2, manager);
			if (proc2.empty())
			{
				args.push_back(arg2);
			}
			else
			{
				res.insert(res.end(), proc2.begin(), proc2.end());
			}

			res.push_back(new Operation(operators[i], args, manager));
		}
	}
	return res;
}
#endif // OLD_PROC_OPER

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << argv[i] << std::endl;
	}

	std::vector<std::string> program_full = { "c = 2","a = c + 50" };
	//std::string program = "c = 2";

	/*List of allowed tokens*/
	std::vector<std::string> tokens = { "wr" };
	std::vector<std::string> operators = { "=","+","-" };

	/*
	* op:wr
	* opening:(
	* arg1:a
	* arg2 : 10
	* closing : )
	*/

	/*
	op: =
	arg1: c
	arg2: 100
	*/

	/*
	c = a + b
	op: =
	arg1: c
	arg2:  a + b
	op1: +
	arg1.1: a
	arg1.2: b
	*/

	//first catch the operand name
	//how?
	//for that we would need a table of allowed operands
	//then we would simply split based on that
	
	try
	{
		VariableManager* manager = new VariableManager();
		std::vector<Operation*>operations = {};
		std::vector<uchar> result_code = {};

		for (std::string program : program_full)
		{//prepare string for processing
			program.erase(std::remove_if(program.begin(), program.end(), std::isspace), program.end());

			//turn all of them to lowercase
			std::transform(program.begin(), program.end(), program.begin(),
				[](uchar c) { return std::tolower(c); }
			);

			//PURELY FOR TESTING, REMOVE THIS BEFORE RELEASE YOU MORON!
			manager->AddNew("a", Variable::Type::Byte);
			manager->AddNew("b", Variable::Type::Byte);
			manager->AddNew("c", Variable::Type::Byte);

			//explicitly seek write to registry/memory operations
			//first find opening expression
			if (program.find_first_of(tokens[0]) != NPOS)
			{
				std::cout << "found func Name: " << tokens[0] << std::endl;
				//it's a function
				//next token MUST be an opening bracket then
				size_t ind_opening = std::string::npos;
				size_t ind_closing = std::string::npos;

				//find the range of workable space
				if ((ind_opening = program.find_first_of('(')) == std::string::npos)
				{
					throw ParsingErrorException("Error! Missing \'(\' after operand " + tokens[0] + " at line 1");
				}
				//we have an opening 
				if ((ind_closing = program.find_first_of(')')) == std::string::npos)
				{
					throw ParsingErrorException("Error! Missing closing \')\' after operand " + tokens[0] + " at line 1");
				}

				//parse the args
				std::string arg_str = program.substr(ind_opening + 1, ind_closing - ind_opening - 1);
				auto res = Split(arg_str, ',');
				std::for_each(res.begin(), res.end(), [](std::string r) {std::cout << r << " "; });
				operations.push_back(new Operation(tokens[0], res, manager));

				//we've split this operand
				std::for_each(operations.begin(), operations.end(), [&result_code](Operation* op)
					{
						auto vec = op->Compile();
						result_code.insert(result_code.end(), vec.begin(), vec.end());
					});
			}
			else
			{
				bool b = 0;

				std::vector<Operation*> proc1 = ProcessOperation(operators, program, manager, "&a");
				operations.insert(operations.end(), proc1.begin(), proc1.end());
				int bd = 0;
			}
			for (Operation* op : operations)
			{
				std::vector<uchar> code = op->Compile();
				result_code.insert(result_code.end(), code.begin(), code.end());
			}
		}
		std::ofstream result_file;
		result_file.open("./out.cod");
		for (uchar byte : result_code)
		{
			result_file << byte;
		}
		result_file.close();
		
	}
	catch(ParsingErrorException e)
	{
		std::cout << e.what();
		exit(1);
	}
	catch (CompilationErrorException e)
	{
		std::cout << e.what();
		exit(2);
	}

	//now to compile
	
}