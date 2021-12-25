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

	bool IsArray = false;
	//if array ->
	int32_t ArraySize = 1;
	//if array end

	std::string name;
	Type type;
	/*Because variables will just use memory located right after programm we need to keep track of what cells are promised to it*/
	size_t promisedOffset = 0;

	unsigned short GetElementAddress(uchar id);

	Variable(std::string name, Type type, size_t promisedOffset,bool isArray,int32_t size) :
		name(name), type(type), promisedOffset(promisedOffset),IsArray(IsArray),ArraySize(size) {}
};