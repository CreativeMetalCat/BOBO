#pragma once
#include "Types.h"

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