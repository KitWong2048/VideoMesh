/*
SocketClient.cpp

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

#include "../DHTSocket/SocketClient.h"

using namespace Util;

namespace DHTSocket{

	SocketClient::SocketClient(SOCKET _sockFD, const char* _remoteIP, int _remotePort) : Socket(){
		sockFD = _sockFD;
		strcpy(remoteIP, _remoteIP);
		remotePort = _remotePort;
	}

	SocketClient::SocketClient(const char* _localIP, int _localPort) : Socket(_localIP, _localPort){
	}

	SocketClient::SocketClient(const SocketClient& _socket) : Socket(){
		this->sockFD = _socket.sockFD;
		strcpy(this->remoteIP, _socket.remoteIP);
		this->remotePort = _socket.remotePort;
	}

	SocketClient::~SocketClient(){
		// don't close upon deletion because there may be some other sockets handling the same FD
	}

	bool SocketClient::connect(const char* _remoteIP, int _remotePort, int _timeout){
		struct sockaddr_in remoteAddr;

		if ((sockFD != -1) || (_remoteIP == NULL) || (_remotePort <= 0))
			return false;

		#ifdef DHT_WIN32
			initWinSocket();
			sockFD = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		#else
			sockFD = ::socket(AF_INET, SOCK_STREAM, 0);
		#endif

		if(sockFD == -1){
			return false;
		}

		if (!Socket::bind())
			return false;

		// create the server address structure
		bzero(&remoteAddr, sizeof(remoteAddr));
		remoteAddr.sin_family = AF_INET;
		remoteAddr.sin_addr.s_addr = inet_addr(_remoteIP);
		remoteAddr.sin_port = htons(_remotePort);
		memset(&(remoteAddr.sin_zero), '\0', 8);

		// connect to server
		
		if(::connect(sockFD, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){									
			return false;
		}else{
			strcpy(remoteIP, _remoteIP);
			remotePort = _remotePort;
			return true;
		}
	}

	int SocketClient::send(const char* _msg, int _len){
		if (_len == -1)
			_len = (int) strlen(_msg) + 1;

		// check sock if valid
		if (sockFD == (long)stdin || sockFD == (long)stdout || sockFD == (long)stderr)
			return 0;

		//send a line of message to a server
		#ifdef DHT_WIN32
			return ::send(sockFD, _msg, _len, 0);
		#else
			return ::send(sockFD, _msg, _len, MSG_NOSIGNAL);
		#endif
	}

	int SocketClient::receive(char* &_msg, int _timeout){
		char buf[MSG_LEN];
		int len = 0, state;
		fd_set socketSet;
		struct timeval tv;

		bzero(buf, MSG_LEN);

		// check sockFD if valid
		if (sockFD == (long)stdin || sockFD == (long)stdout || sockFD == (long)stderr)
			return 0;

		if (_timeout < 0){
			len = recv(sockFD, buf, MSG_LEN, 0);
		}else{
			// set socket to non-blocking
			setBlocking(false);

			tv.tv_sec = _timeout / 1000;
			tv.tv_usec = 1000 * (_timeout % 1000);
			FD_ZERO(&socketSet);
			FD_SET(sockFD, &socketSet);
			state = select((int) sockFD + 1, &socketSet, NULL, NULL, &tv);
			if (state > 0){
				len = recv(sockFD, buf, MSG_LEN, 0);
			}else if (state == 0){
				len = -1;
			}else if (state == -1){
				len = 0;
			}
			// set socket blocking again
			setBlocking(true);
		}

		if (len > 0){
			_msg = new char[len];
			memcpy(_msg, buf, len);
		}
		return len;
	}

	char* SocketClient::getRemoteIP(){
		return remoteIP;
	}

	int SocketClient::getRemotePort(){
		return remotePort;
	}

}
