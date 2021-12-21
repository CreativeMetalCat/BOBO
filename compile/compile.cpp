// compile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>

#include "ErrorHandling/Exceptions.h"
#include "Variable.h"
#include "Operation.h"


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

//should instead return result memory addr or argument itself


int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << argv[i] << std::endl;
	}

	std::vector<std::string> program_full = { "var c = 2","var b = c + 89 + c ","var a = b + b + 20 + 56" };
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
	
	/*try
	{*/
		
		std::vector<Operation*>operations = {};
		VariableManager* manager = new VariableManager();
		std::vector<uchar> result_code = {};

		for (std::string program : program_full)
		{
			//prepare string for processing
			program.erase(std::remove_if(program.begin(), program.end(), ::isspace), program.end());

			//turn all of them to lowercase
			std::transform(program.begin(), program.end(), program.begin(),
				[](uchar c) { return std::tolower(c); }
			);

			

			/*Because of different syntax for math operations and function execution*/

			//explicitly seek write to registry/memory operations
			//first find opening expression
			if (program.find(tokens[0]) != NPOS)
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
			//else process math and variable defenition(because they use = as base of operation)
			else
			{
				std::vector<Operation*> proc1 = ProcessOperation(operators, program, manager, "&a");
				operations.insert(operations.end(), proc1.begin(), proc1.end());
			}
			
		}
		for (Operation* op : operations)
		{
			manager->program_lenght += op->GetLenght();
		}
		manager->program_lenght += 1u;
		//first get program lenght(for variables),because name of the thing doesn't matter we can just return lenght
		for (Operation* op : operations)
		{
			std::vector<uchar> code = op->Compile();
			result_code.insert(result_code.end(), code.begin(), code.end());
		}
		std::ofstream result_file;
		result_file.open("./out.cod");
		for (uchar byte : result_code)
		{
			result_file << byte;
		}
		result_file.close();
		
	/* }
	catch(ParsingErrorException e)
	{
		std::cout << e.what();
		exit(1);
	}
	catch (CompilationErrorException e)
	{
		std::cout << e.what();
		exit(2);
	}*/

	//now to compile
	
}