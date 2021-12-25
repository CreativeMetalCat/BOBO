#include "Variable.h"
#include "ErrorHandling/Logger.h"

unsigned short Variable::GetElementAddress(uchar id)
{
    if (id >= ArraySize || id < 0)
    {
        Logger::PrintError("Array index out of bounds!");
        return NPOS;
    }
    return promisedOffset + 0x0800 + (uchar)id;
    
}
