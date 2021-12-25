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
#include "ErrorHandling/Logger.h"


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

	//#endif
	std::vector<std::string> program_full = {};//{ "var c = 2","var b = d + 89 + c ","var a = b + b + 20 + 56" };
	//std::string program = "c = 2";

	/*List of allowed tokens*/
	std::vector<std::string> tokens = { "wr" };
	std::vector<std::string> operators = { "=","+","-" };

	//for linux use differnt style flags
//#ifndef __linux__
	for (int i = 0; i < argc; i++)
	{
#ifdef _DEBUG
		std::cout << argv[i] << std::endl;
#endif // _DEBUG
		if (std::string(argv[i]) == "/f" && i + 1 < argc)
		{
			//next argument should be file name
			std::ifstream input;
			input.open(argv[i + 1]);
			if (input.is_open())
			{
				while (!input.eof())
				{
					program_full.push_back("");
					std::getline(input, program_full[program_full.size() - 1]);
					Logger::Print(program_full[program_full.size() - 1]);
				}
				input.close();
			}
		}
		if (std::string(argv[i]) == "/h" || std::string(argv[i]) == "/?")
		{
			Logger::Print("BOBO compiler.");
			Logger::Print("Usage: compiler [OPTIONS]");
			Logger::Print("/f [filename]		Name of the file that needs to be compiled");
			Logger::Print("/o [filename]		Name of the output file");
			Logger::Print("/O [offsetSize]		Where should program offset start(Only affects variable and function declarations)");
			exit(0);
		}
	}
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

	/*
	* how does compiler search?
	* first find line where main is defined
	* Then where main ends
	* Until functions will be introduced that will be the only part of code that will be processed
	* After addition of functions -> first process functions then main
	*/

	//first catch the operand name
	//how?
	//for that we would need a table of allowed operands
	//then we would simply split based on that

	size_t main_proc_start = NPOS;
	size_t main_proc_end = NPOS;

	for (int i = 0; i < program_full.size(); i++)
	{
		//turn all of them to lowercase
		std::transform(program_full[i].begin(), program_full[i].end(), program_full[i].begin(),
			[](uchar c) { return std::tolower(c); }
		);

		if (program_full[i].find("def") != NPOS && program_full[i].find("main") != NPOS)
		{
			//we found beggining of main
			main_proc_start = i;
		}
		if (program_full[i].find("end") != NPOS && main_proc_start != NPOS)
		{
			main_proc_end = i;
		}
	}
	if (main_proc_start == NPOS)
	{
		Logger::PrintError("Unable to find MAIN function(Entry point)");
		exit(2);
	}
	else if (main_proc_end == NPOS)
	{
		Logger::PrintError("Unable to find end of END function");
		exit(2);
	}

	std::vector<Operation*>operations = {};
	VariableManager* manager = new VariableManager();
	std::vector<uchar> result_code = {};


	for (std::vector<std::string>::iterator it = program_full.begin() + main_proc_start; it != program_full.begin() + main_proc_end; it++)
	{
		//prepare string for processing
		(*it).erase(std::remove_if((*it).begin(), (*it).end(), ::isspace), (*it).end());

		//turn all of them to lowercase
		std::transform((*it).begin(), (*it).end(), (*it).begin(),
			[](uchar c) { return std::tolower(c); }
		);

		//for now any string that has comment symbol(#) is treated like comment
		//in future make it stop processing the string if # is found
		if ((*it).find('#') != NPOS)
		{
			continue;
		}


		/*Because of different syntax for math operations and function execution*/

		//explicitly seek write to registry/memory operations
		//first find opening expression
		if ((*it).find(tokens[0]) != NPOS)
		{
			std::cout << "found func Name: " << tokens[0] << std::endl;
			//it's a function
			//next token MUST be an opening bracket then
			size_t ind_opening = std::string::npos;
			size_t ind_closing = std::string::npos;

			//find the range of workable space
			if ((ind_opening = (*it).find_first_of('(')) == std::string::npos)
			{
				throw ParsingErrorException("Error! Missing \'(\' after operand " + tokens[0] + " at line 1");
			}
			//we have an opening 
			if ((ind_closing = (*it).find_first_of(')')) == std::string::npos)
			{
				throw ParsingErrorException("Error! Missing closing \')\' after operand " + tokens[0] + " at line 1");
			}

			//parse the args
			std::string arg_str = (*it).substr(ind_opening + 1, ind_closing - ind_opening - 1);
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
			std::vector<Operation*> proc1 = ProcessOperation(operators, (*it), manager, "&a");
			operations.insert(operations.end(), proc1.begin(), proc1.end());
		}

	}
	operations.push_back(new Operation("main_end", {}, nullptr));
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


}