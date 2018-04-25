/*
Socket.cpp

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

#include "../DHTSocket/Socket.h"
#include "../Util/Win32Support.h"
#include <iostream>
using namespace std;
using namespace Util;

namespace DHTSocket{

	Socket::Socket(){
		sockFD = -1;
	}

	Socket::Socket(const char* _localIP, int _localPort){
		sockFD = -1;
		if (_localIP == 0)
			strcpy(localIP, "0.0.0.0");
		else
			strcpy(localIP, _localIP);
		if (_localPort >= 1024)
			localPort = _localPort;
		else localPort = 0;

		bzero(&localAddr, sizeof(localAddr));
		localAddr.sin_family = AF_INET;

		if (!strcmp(localIP, "0.0.0.0"))
			localAddr.sin_addr.s_addr = INADDR_ANY;
		else
			inet_aton(localIP, &(localAddr.sin_addr));

		localAddr.sin_port = htons(localPort);
		memset(&(localAddr.sin_zero), '\0', 8);

		#ifdef DHT_WIN32
		isWinSocketInited = false;
		#endif
	}

	Socket::~Socket(){
	}

	bool Socket::bind(){
		if(::bind(sockFD, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0){
			return false;
		}else return true;
	}

	void Socket::close(){
		if (sockFD != -1){
			#ifndef DHT_WIN32
				::close(sockFD);
			#else
				::closesocket(sockFD);
			#endif
			sockFD = -1;
		}
	}

	int Socket::setBlocking(bool _isBlocking){
		#ifndef DHT_WIN32

			long arg = fcntl(sockFD, F_GETFL, 0);
			if (_isBlocking)
				arg &= (~O_NONBLOCK);
			else
				arg |= O_NONBLOCK;
			arg = fcntl(sockFD, F_SETFL, arg);

		#else

			unsigned long blocking = 0;
			unsigned long nonBlocking = 1;
			if (_isBlocking)
				ioctlsocket(sockFD, FIONBIO, &blocking);
			else
				ioctlsocket(sockFD, FIONBIO, &nonBlocking);

		#endif

		return 0;
	}

	int Socket::getSocketErrorCode(){
		#ifdef DHT_WIN32
			return WSAGetLastError();
		#else
			return errno;
		#endif
	}

	void Socket::printSocketError(){		
		int errorCode = getSocketErrorCode();
		if (errorCode != 0)
			printf("Socket Error %d: %s\n", errorCode, strerror(errorCode));		
	}

	#ifdef DHT_WIN32
	void Socket::initWinSocket(){
		WORD sockVersion;
		WSADATA wsaData;
		if (!isWinSocketInited){
			sockVersion = MAKEWORD(2, 0);
			WSAStartup(sockVersion, &wsaData);
			isWinSocketInited = true;
		}

	}
	#endif


}
