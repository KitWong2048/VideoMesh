/*
SocketSelector.cpp

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

#include "../DHTSocket/SocketSelector.h"

using namespace Util;

namespace DHTSocket{

	SocketSelector::SocketSelector(){
		clearSocket();
	}

	SocketSelector::~SocketSelector(){

	}

	int SocketSelector::setSocket(Socket** _socket){
		int i = 0;
		if (_socket != NULL){
			while (_socket[i] != NULL){
				socketSet[_socket[i]->sockFD] = _socket[i];
				i++;
			}
		}
		return i;
	}

	int SocketSelector::setSocket(set<Socket*>& _socket){
		int i = 0;
		set<Socket*>::iterator l;
		for (l = _socket.begin(); l != _socket.end(); l++)
			if (*l != NULL){
				socketSet[(*l)->sockFD] = (*l);
				i++;
			}

		return i;
	}

	Socket** SocketSelector::selectSocket(unsigned int _timeout){
		int maxFD,selectedFDCount, i, errorCode;
		struct timeval tv;
		map<SOCKET, Socket*>::iterator l;
		Socket** selectedSocket = NULL;

		tv.tv_sec = _timeout / 1000;
		tv.tv_usec = 1000 * (_timeout % 1000);
		FD_ZERO(&(this->fdSet));
		maxFD = -1;

		if (socketSet.begin() != socketSet.end())
			for (l = socketSet.begin(); l != socketSet.end(); l++){
				if (!(l->first == -1)){
					l->second->setBlocking(false);
					FD_SET(l->first, &(this->fdSet));
					maxFD = max(maxFD, l->first);
				}
			}

		selectedFDCount = select(maxFD + 1, &fdSet, NULL, NULL, &tv);
		if (selectedFDCount > 0){
			i = 0;
			selectedSocket = new Socket*[selectedFDCount + 1];
			// check which sockets are set
			for (l = socketSet.begin(); l != socketSet.end(); l++){
				if (FD_ISSET(l->first, &fdSet)){
					l->second->setBlocking(true);
					selectedSocket[i] = l->second;
					i++;
				}
			}

			// set remaining entry of the array to NULL
			while (i <= selectedFDCount){
				selectedSocket[i] = NULL;
				i++;
			}

			// remove the selected socket from the set
			for (i = 0; i < selectedFDCount; i++){
				if (selectedSocket[i] != NULL)
					this->socketSet.erase(selectedSocket[i]->sockFD);
			}
		}else if (selectedFDCount < 0){
			selectedSocket = NULL;
			#ifdef DHT_WIN32
				errorCode = WSAGetLastError();
			#else
				errorCode = errno;
			#endif
		}

		return selectedSocket;
	}

	void SocketSelector::clearSocket(){
		this->socketSet.clear();
	}

}
