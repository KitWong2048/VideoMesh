/*
SocketServer.cpp

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

#include "../DHTSocket/SocketServer.h"

using namespace Util;

namespace DHTSocket{

	SocketServer::SocketServer(const char* _localIP, int _localPort) : Socket(_localIP, _localPort){
	}

	SocketServer::~SocketServer(){
		this->close();
	}

	SocketClient* SocketServer::accept(int timeout){
		SOCKET remoteSock = -1;
		socklen_t remoteAddrLen;
		struct sockaddr_in remoteAddr;
		char remoteIP[IP_LEN];
		SocketClient* client;

		bzero(&remoteAddr,sizeof(remoteAddr));
		remoteAddrLen = sizeof(remoteAddr);

		if (timeout < 0){
			//accept the connection from client
			remoteSock = ::accept(sockFD, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
		}else{
			// Set sockFD non-blocking
			setBlocking(false);

			// wait for timeout
			fd_set myset;
			struct timeval tv;

			tv.tv_sec = timeout / 1000;
			tv.tv_usec = 1000 * (timeout % 1000);
			FD_ZERO(&myset);
			FD_SET(sockFD, &myset);
			int state = select((int) sockFD + 1, &myset, NULL, NULL, &tv);
			if (state > 0) 				
				remoteSock = ::accept(sockFD, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
			else if (state <= 0)
				remoteSock = -1;			

			// Set sockFD blocking again
			setBlocking(true);
		}

		if(remoteSock == -1){
			client = NULL;
		}else{
			bzero(remoteIP, IP_LEN);
			client = new SocketClient(remoteSock, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
		}

		return client;
	}

	bool SocketServer::ready(){
		// if the socket has been ready, don't do anyting
		if (sockFD != -1)
			return false;


		#ifdef DHT_WIN32
			initWinSocket();
			sockFD = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		#else
			sockFD = ::socket(AF_INET, SOCK_STREAM, 0);
		#endif

		if(sockFD <= 0){
			sockFD = -1;
			return false;
		}

		if (!bind()){
			sockFD = -1;
			return false;
		}

		if (::listen(sockFD, 10) < 0){
			sockFD = -1;
			return false;
		}

		return true;
	}

}
