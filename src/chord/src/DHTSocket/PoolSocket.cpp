/*
PoolSocket.cpp

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

#include "../DHTSocket/PoolSocket.h"

using namespace Util;

namespace DHTSocket{

	PoolSocket::PoolSocket(const char* _ip, int _port, int _socketTimeout, unsigned long _poolRef) : SocketClient(_ip, _port){
		this->isBusy = false;
		this->isSocketDead = false;
		this->lastAccessedTime = 0;
		this->count = 0;
		this->socketTimeout = _socketTimeout;
		this->poolRef = _poolRef;
		this->userNum = 0;
	}

	PoolSocket::PoolSocket(const SocketClient& _socket, int _socketTimeout, unsigned long _poolRef) : SocketClient(_socket){
		this->isBusy = false;
		this->isSocketDead = false;
		this->lastAccessedTime = 0;
		this->count = 0;
		this->socketTimeout = _socketTimeout;
		this->poolRef = _poolRef;
		this->userNum = 0;
	}

	PoolSocket::~PoolSocket(){
	}

	void PoolSocket::preclose(){
		isSocketDead = true;
	}

	void PoolSocket::close(){
		isSocketDead = true;
		poolMutex.lock();
		#ifdef DHT_WIN32
			poolMutex.release();
			while (isBusy){			
				sleep(5);		
			}
			poolMutex.lock();
		#else
			while (isBusy){			
				sleep(5);		
			}
		#endif
		SocketClient::close();
		#ifndef DHT_WIN32
			poolCondVar.boardcast();
		#endif
		poolMutex.release();
	}

	int PoolSocket::send(const char* _msg, int _len){
		int returnValue;

		if (isSocketDead)
			return -1;

		poolMutex.lock();
		userNum++;

		#ifdef DHT_WIN32
			int sleptTime = 0;				
			poolMutex.release();
			while (isBusy){				
				sleep(1);
				sleptTime += 1;
				if (sleptTime * 1000 > socketTimeout)
					break;
			}
			poolMutex.lock();
		#else
			while (isBusy){
				if (poolCondVar.timedWait(poolMutex, socketTimeout) < 0 )
					break;
			}
		#endif
		
		if (!isBusy){
			isBusy = true;
			returnValue = SocketClient::send(_msg, _len);
			lastAccessedTime = time(0);
			isBusy = false;
			#ifndef DHT_WIN32
				poolCondVar.boardcast();
			#endif
		}else returnValue = 0;

		userNum--;
		poolMutex.release();

		return returnValue;
	}

	int PoolSocket::receive(char* &_msg, int _timeout){
		int returnValue;

		_msg = NULL;
		if (isSocketDead)
			return -1;

		poolMutex.lock();
		userNum++;

		#ifdef DHT_WIN32
			int sleptTime = 0;
			poolMutex.release();
			while (isBusy){				
				sleep(1);
				sleptTime += 1;				
				if (sleptTime * 1000 > socketTimeout)
					break;
			}
			poolMutex.lock();
		#else
			while (isBusy){
				if (poolCondVar.timedWait(poolMutex, socketTimeout) < 0 )
					break;
			}
		#endif

		if (!isBusy){
			isBusy = true;
			returnValue = SocketClient::receive(_msg, _timeout);
			lastAccessedTime = time(0);
			isBusy = false;
			#ifndef DHT_WIN32
				poolCondVar.boardcast();
			#endif
		}else returnValue = 0;

		userNum--;
		poolMutex.release();

		return returnValue;
	}

	time_t PoolSocket::getLastAccesstedTime() const{
		return lastAccessedTime;
	}

	unsigned long PoolSocket::getPoolRef() const{
		return poolRef;
	}

	bool PoolSocket::isProxyOf(const VirtualPoolSocket& _virtualSocket){
		if (_virtualSocket.getPoolRef() == this->poolRef)
			return true;
		else return false;
	}

	void PoolSocket::operator++(int){
		countMutex.lock();
		count++;
		countMutex.release();
	}

	void PoolSocket::operator--(int){
		countMutex.lock();
		count--;
		countMutex.release();
	}

	int PoolSocket::getCount(){
		int returnValue;

		countMutex.lock();
		returnValue = count;
		countMutex.release();

		return returnValue;
	}

	bool PoolSocket::isInUse(){
		return isBusy;
	}

	int PoolSocket::getUserNum(){
		int value;
		poolMutex.lock();
		value = userNum;
		poolMutex.release();
		return value;
	}

}
