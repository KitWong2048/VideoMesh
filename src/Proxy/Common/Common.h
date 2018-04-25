#pragma once
#include <stdio.h>

#define DATA_PORT		10100
#define SERVER_PORT		8088
#define DEBUG


// For disable Logging
//#define LOGGING_OFF

#include "../Common/Logging.h"

void DebugLog(const char* msg);
