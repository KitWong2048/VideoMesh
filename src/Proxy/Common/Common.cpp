#include "Common.h"

void DebugLog(const char* msg)
{
//#ifdef _DEBUG
	printf(msg);
	DEBUG_MSG(msg)
//#endif
}