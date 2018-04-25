/*
Common.h

Copyright (c) 2007 Hong Kong University of Science and Technology ("HKUST")
This source code is an intellectual property owned by HKUST and funded by
Innovation and Technology Fund (Ref No. GHP/045/05)

Permission is hereby granted, to any person and party obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the
Software with the rights to use, copy, modify and merge, subject to the
following conditions:

1. Commercial use of this source code or any derivatives works from this source
 code in any form is not allowed

2. Redistribution of this source code or any derivatives works from this source
 code in any form is not allowed

3. Any of these conditions can be waived if you get permission from the
copyright holder

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**	@file Common.h
 *	This file contains common include directives and define marros.
 */

#define DHT_WIN32

#ifndef _H_COMMON_
#define _H_COMMON_

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DHT_WIN32
	#pragma comment(lib, "ws2_32.lib")
	//#include <windows.h>
	#include <process.h>
	#include <winsock2.h>
#else
	#include <pthread.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif

namespace Util{
	#define SHA1_LENGTH 20
	//#define ID_LENGTH 12 // bytes
	#define ID_LENGTH 13 // bytes
	#define ID_BIT_LENGTH	(ID_LENGTH * 8) // bits

	#define IP_LENGTH 4 // bytes
	#define PORT_LENGHTH 2 // bytes
	#define ADDRESS_LENGTH 6 // bytes = IP_LENGTH + PORT_LENGHTH
	#define APP_VALUE_LEN 6 // bytes
	#define NODE_LEN (ID_LENGTH + ADDRESS_LENGTH + APP_VALUE_LEN) // bytes

	#define SEQ_NO_LENGTH 4 // bytes = sizeof(unsigned long)
	#define SESSION_REF_LENGTH (ADDRESS_LENGTH + SEQ_NO_LENGTH) // bytes

	#define STABILIZING_INTERVAL 30 // s
	#define FIXING_INTERVAL STABILIZING_INTERVAL * 1 // s
	#define NEIGHBOR_VISIT_INTERVAL 5 * 60 // s
	#define MIN_STABILIZER_INTERVAL 10 // s
	#define DHT_CONNECTION_TIMEOUT (30 * 1000) // ms
	#define DHT_MIN_CONNECTION_TIMEOUT (5 * 1000) // ms

	#define MAX_SEARCH_TIME 60
	#define MAX_FINGER_RETURN 16
	#define MAX_NEIGHBOR_RETURN 8

	#define MAX_DHT_CONNECTION 25
	#define DEFAULT_NEIGHBOR_SIZE 8
	#define NODE_BUCKET_MAX_SIZE 8

	#define NODE_FAIL_THRESHOLD 2
	#define CRLF "\r\n"
	
	#define MIN_POOL_SOCKET_LIFE_TIME (5 * 60) // s
	#define MANAGER_CHECKING_INTERVAL 2000 // ms
	

	#define MSG_REQ_PING	"REQ PING"
	#define MSG_REQ_PRED	"REQ PRED"
	#define MSG_REQ_SUCC	"REQ SUCC"
	#define MSG_REQ_CLOSEST_FINGER	"REQ CLOSEST FINGER"
	#define MSG_REQ_NOTIFY	"REQ NOTIFY"
	#define MSG_REQ_ALL_FINGER	"REQ ALL FINGER"
	#define MSG_REQ_ALL_NEIGHBOR "REQ ALL NEIGHBOR"
	#define MSG_REQ_SELF "REQ SELF"
	#define MSG_SRV_PING	"SRV PING"
	#define MSG_SRV_PRED	"SRV PRED"
	#define MSG_SRV_SUCC	"SRV SUCC"
	#define MSG_SRV_CLOSEST_FINGER	"SRV CLOSEST FINGER"
	#define MSG_SRV_NOTIFY	"SRV NOTIFY"
	#define MSG_SRV_ALL_FINGER	"SRV ALL FINGER"
	#define MSG_SRV_ALL_NEIGHBOR "SRV ALL NEIGHBOR"
	#define MSG_SRV_SELF "SRV SELF"
	#define MSG_ERROR "ERROR"

	#define DHT_MSG_TYPE_LENGTH 2 // bytes
	#define DHT_MSG_TYPE_MIN 1
	#define DHT_MSG_TYPE_MAX 16

	enum DHTMsgType{
		DHTReqPred = 1,
		DHTReqSucc = 2,
		DHTReqClosestFinger = 3,
		DHTReqNotify = 4,
		DHTReqPing = 5,
		DHTReqAllFingers = 6,
		DHTReqAllNeighbor = 7,
		DHTSrvPred = 8,
		DHTSrvSucc = 9,
		DHTSrvClosestFinger = 10,
		DHTSrvNotify = 11,
		DHTSrvPing = 12,
		DHTSrvAllFingers = 13,
		DHTSrvAllNeighbor = 14,
		DHTReqSelf = 15,
		DHTSrvSelf = 16,
		DHTError = 0
	};
	
	enum LogLevel{
		LogNone = 0,
		LogDebug = 1,
		LogNormal = 2,
		LogError = 4,
		LogRequest = 8,
		LogService = 16,
		LogStabilizer = 32,
		LogDeepDebug = 64,
		LogCommon = LogNormal | LogError,
		LogAll =  LogCommon | LogRequest | LogService | LogStabilizer| LogDeepDebug
	};	
}

#endif
