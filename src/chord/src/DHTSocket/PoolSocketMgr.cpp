/*
PoolSocketMgr.cpp

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

#include "../DHTSocket/PoolSocketMgr.h"

using namespace Util;

namespace DHTSocket{

	PoolSocketMgr* PoolSocketMgr::mgr = NULL;

	PoolSocketMgr::PoolSocketMgr(const char* _localIP, int _localPort){
		localAddr.setIP(_localIP);
		localAddr.setPort(_localPort);
		isNoSocket = true;
	}

	PoolSocketMgr::~PoolSocketMgr(){
	}

	PoolSocketMgr* PoolSocketMgr::invokePoolSocketMgr(){
		return mgr;
	}

	int PoolSocketMgr::employ(unsigned int _reqPoolMaxSize, unsigned int _srvPoolMaxSize, const char* _localIP, int _localPort){
		if (mgr == NULL){
			mgr = new PoolSocketMgr(_localIP, _localPort);
			mgr->reqPoolMaxSize = _reqPoolMaxSize;
			mgr->srvPoolMaxSize = _srvPoolMaxSize;
			mgr->isEmployed = true;
			mgr->run();
			return 0;
		}else return -1;
	}

	int PoolSocketMgr::dismiss(){
		if(mgr != NULL){
			mgr->isEmployed = false;
			if (mgr->getJoinableStatus())
				mgr->join();
			mgr->freeSocketPool();
			mgr->cleanupGarbage();
			delete mgr;
			mgr = NULL;
		}		
		return 0;
	}

	VirtualPoolSocket* PoolSocketMgr::connect(const char* _remoteIP, int _remotePort, int _timeout, ServerHandle& _server){
		PoolSocket* aPoolSocket = NULL;
		VirtualPoolSocket* aVirtualPoolSocket = NULL;
		unsigned long ref;
		SessionReference sessionRef;
		InetAddress remoteAddr(_remoteIP, _remotePort);

		// get the request socket by address key
		mgrMutex.lock();
		if (addrMapping.find(remoteAddr) != addrMapping.end()){
            ref = addrMapping[remoteAddr];
            if (this->isReqSocketExist(ref)){
                aPoolSocket = reqSocketPool[ref];
                if (aPoolSocket == NULL)
                    reqSocketPool.erase(ref);
            }else if (this->isSrvSocketExist(ref)){
                aPoolSocket = srvSocketPool[ref];
                if (aPoolSocket == NULL)
                    srvSocketPool.erase(ref);
			}else aPoolSocket = NULL;
		}
		mgrMutex.release();

		// if the socket does not exist, create it and connect to the remote host
		if (aPoolSocket == NULL){
			ref = refGenerator.getNext();
			aPoolSocket = new PoolSocket(NULL, 0, POOL_SOCKET_TIMEOUT, ref);
			if (aPoolSocket->connect(_remoteIP, _remotePort, _timeout)){
				mgrMutex.lock();
				reqSocketPool[ref] = aPoolSocket;
				addrMapping[remoteAddr] = ref;
				srvHandle.insert(make_pair(ref, &_server));
				// signal the manager thread that now there is a socket ready for mangament
				#ifndef DHT_WIN32
					waitSocketCondVar.signal();
				#endif
				mgrMutex.release();
			}else{
				delete aPoolSocket;
				aPoolSocket = NULL;
			}
		}

		// open a message box for this session
		if (aPoolSocket != NULL){
			sessionRef.setAddr(localAddr);
			sessionRef.setSeqNo(sessionGenerator.getNext());
			aVirtualPoolSocket = new VirtualPoolSocket(ref, sessionRef, VirtualPoolReqSocket, this);
			messenger.createBox(sessionRef);
			(*aPoolSocket)++;
		}

		return aVirtualPoolSocket;
	}

	int PoolSocketMgr::delegate(SocketClient& _socket, ServerHandle& _server){
		PoolSocket* aPoolSocket;
		unsigned long ref;

		ref = refGenerator.getNext();
		aPoolSocket = new PoolSocket(_socket, POOL_SOCKET_TIMEOUT, ref);
		srvSocketPool.insert(make_pair(ref, aPoolSocket));
		srvHandle.insert(make_pair(ref, &_server));
		// signal the manager thread that now there is a socket ready for mangament
		#ifndef DHT_WIN32
			waitSocketCondVar.signal();
		#endif

		return 0;
	}

	int PoolSocketMgr::delegate(SocketServer& _socket, ServerHandle& _server){
		listeningSocketPool.insert(make_pair(&_socket, &_server));
		// signal the manager thread that now there is a socket ready for mangament
		#ifndef DHT_WIN32
			waitSocketCondVar.signal();
		#endif

		return 0;
	}

	void PoolSocketMgr::removePoolSocket(){
		time_t minAccessedTime, socketLastAccessedTime;
		map<unsigned long, PoolSocket*>::iterator l, k;
		map<unsigned long, PoolSocket*>* socketPool;
		unsigned int socketPoolSize;


		for (int i = 0; i < 2; i++){
			if (i == 0){
				socketPool = &reqSocketPool;
				socketPoolSize = reqPoolMaxSize;
			}else{
				socketPool = & srvSocketPool;
				socketPoolSize = srvPoolMaxSize;
			}
			minAccessedTime = 0;
			socketLastAccessedTime = time(0) - MIN_POOL_SOCKET_LIFE_TIME;

			if (socketPool->size() >= socketPoolSize){
				mgrMutex.lock();
				k = socketPool->end();
				for (l = socketPool->begin(); l != socketPool->end(); l++)
					if (l->second == NULL)
						k = l;
					else if (l->second->getCount() == 0){ // determine whether there is virtual socket referencing the pool socket
								// get the pool socket that is least used
								socketLastAccessedTime = l->second->getLastAccesstedTime();
								if (minAccessedTime > socketLastAccessedTime){
									k = l;
									minAccessedTime = socketLastAccessedTime ;
								}
							}

				if (k != socketPool->end()){					
					if (k->second != NULL){
						pushGarbage(k->second);
						srvHandle.erase(k->second->getPoolRef());
					}
					socketPool->erase(k);
				}
				mgrMutex.release();
			}

		}


	}

	int PoolSocketMgr::send(const VirtualPoolSocket& _socket, const char* _msg, int _len){
		int status;
		PoolSocket* aPoolSocket;
		char* sessionMsg;
		int sessionMsgLen;
		char* tmp;

		if (isReqSocketExist(_socket.poolRef))
			aPoolSocket = reqSocketPool[_socket.poolRef];
		else if (isSrvSocketExist(_socket.poolRef))
			aPoolSocket = srvSocketPool[_socket.poolRef];
		else aPoolSocket = NULL;

		if (aPoolSocket == NULL){
			status = -1;
		}else if (!(aPoolSocket->isProxyOf(_socket))){
			status = -1;
		}else{
			sessionMsg = new char[SESSION_REF_LENGTH + _len];
			_socket.sessionRef.toBin(tmp);
			memmove(sessionMsg, tmp, SESSION_REF_LENGTH);
			sessionMsgLen = SESSION_REF_LENGTH;
			delete []tmp;
			memmove(sessionMsg + sessionMsgLen, _msg, _len);
			status = aPoolSocket->send(sessionMsg, sessionMsgLen + _len);
			if (status > 0)
				status -= sessionMsgLen;
			delete []sessionMsg;
		}

		return status;
	}

	int PoolSocketMgr::receive(const VirtualPoolSocket& _socket, char* & _msg, int _timeout){
		return messenger.take(_socket.sessionRef, _msg, _timeout);
	}

	void PoolSocketMgr::close(const VirtualPoolSocket& _socket){
		PoolSocket* aPoolSocket;

		if (isReqSocketExist(_socket.poolRef)){
			aPoolSocket = reqSocketPool[_socket.poolRef];
		}else if (isSrvSocketExist(_socket.poolRef)){
			aPoolSocket = srvSocketPool[_socket.poolRef];
		}else aPoolSocket = NULL;

		if (aPoolSocket != NULL){
			if (aPoolSocket->isProxyOf(_socket))
				(*aPoolSocket)--;
			messenger.destroyBox(_socket.sessionRef);
		}
	}

	bool PoolSocketMgr::isReqSocketExist(unsigned long _ref){
		if (reqSocketPool.find(_ref) != reqSocketPool.end())
			return true;
		else return false;
	}

	bool PoolSocketMgr::isSrvSocketExist(unsigned long _ref){
		if (srvSocketPool.find(_ref) != srvSocketPool.end())
			return true;
		else return false;
	}

	void PoolSocketMgr::cleanupGarbage(){
		set<PoolSocket*>::iterator k, l;
		k = socketGarbage.begin();
		l = k;
		if (l != socketGarbage.end())
			do{
				l++;
				if (!(*k)->isInUse())
					if ((*k)->getUserNum() == 0){
						(*k)->close();
						if (srvHandle.find((*k)->getPoolRef()) != srvHandle.end())
							srvHandle.erase((*k)->getPoolRef());
						delete (*k);
						socketGarbage.erase(k);
					}
				k = l;
			}while (l != socketGarbage.end());

		set<SocketServer*>::iterator i, j;		
		i = listenerGarbage.begin();
		j = i;
		if (j != listenerGarbage.end())
			do{
				i++;							
				if (*j != NULL){
					(*j)->close();
					delete *j;
				}
				listenerGarbage.erase(j);
				j = i;
			}while (i != listenerGarbage.end());
	}

	void* PoolSocketMgr::execute(){
		SocketSelector selector;
		Socket** readySocket;
		set<Socket*> pendingSocket;
		PoolSocket* aPoolSocket;
		map<unsigned long, PoolSocket*>::iterator commIterator;
		map<SocketServer*, ServerHandle*>::iterator listenIterator;
		int i, msgLen;
		VirtualPoolSocket* unhandledSocket;
		ServerHandle* server;
		char *buf, *index;
		unsigned long poolRef;
		SessionReference sessionRef;
		InetAddress sessionAddr;

		while (isEmployed){
			// wait for non-empty pool
			waitSocket();
			if (!isEmployed)
				break;

			// prepare selector
			i = 0;
			pendingSocket.clear();
			for (commIterator = reqSocketPool.begin(); commIterator != reqSocketPool.end(); commIterator++)
				if (commIterator->second != NULL){
					pendingSocket.insert(commIterator->second);
					i++;
				}
			for (commIterator = srvSocketPool.begin(); commIterator != srvSocketPool.end(); commIterator++)
				if (commIterator->second != NULL){
					pendingSocket.insert(commIterator->second);
					i++;
				}
			for (listenIterator = listeningSocketPool.begin(); listenIterator != listeningSocketPool.end(); listenIterator++)
				if (listenIterator->first != NULL){
					pendingSocket.insert(listenIterator->first);
					i++;
				}
			
			if (i == 0)
				this->isNoSocket = true;
			selector.clearSocket();
			selector.setSocket(pendingSocket);

			// wait for any socket ready or timeout
			readySocket = selector.selectSocket(MANAGER_CHECKING_INTERVAL);
			if (readySocket != NULL){
				// an array of socket is ready for reading
				i = 0;
				while (readySocket[i] != NULL){
					
					if (listeningSocketPool.find((SocketServer*) readySocket[i]) != listeningSocketPool.end()){
						server = listeningSocketPool[(SocketServer*) readySocket[i]];
						if (server != NULL)
							server->response((SocketServer*) readySocket[i]);
					}else{
						aPoolSocket = (PoolSocket*)readySocket[i];
						// read the message from each socket and deliver them to message boxes according to session reference
						msgLen = aPoolSocket->receive(buf);
						if ((msgLen < 0) || (buf == NULL)){
							// socket error, remove it from the pool
							pushGarbage(aPoolSocket);							
							if (isSrvSocketExist(aPoolSocket->getPoolRef()))
								srvSocketPool.erase(aPoolSocket->getPoolRef());
							else reqSocketPool.erase(aPoolSocket->getPoolRef());
						}else if (buf != NULL){
							if (msgLen > SESSION_REF_LENGTH){
								sessionRef.fromBin(buf);
								index = buf + SESSION_REF_LENGTH;
								if (messenger.isBoxExist(sessionRef)){
									messenger.deliver(sessionRef, index, msgLen - SESSION_REF_LENGTH);
								}else{
									sessionAddr = sessionRef.getAddr();
									// check whether the received message is a request or an answer to our previously sent request
									// if the address in the session header is equal to the local address
									// this is an answer to our previously sent request but the virtual socket has been already closed so we ignore it
									if ( !(localAddr == sessionAddr) ){
										// this is a request from the remote side
										poolRef = aPoolSocket->getPoolRef();
										mgrMutex.lock();
										if (srvHandle.find(poolRef) != srvHandle.end()){
											server = srvHandle[poolRef];
											messenger.createBox(sessionRef);
											unhandledSocket = new VirtualPoolSocket(poolRef, sessionRef, VirtualPoolSrvSocket, this);
											messenger.deliver(sessionRef, index, msgLen - SESSION_REF_LENGTH);
											(*aPoolSocket)++;
											server->response(unhandledSocket);
											// also put down this address for connecting to this host in the future
											if (addrMapping.find(sessionAddr) == addrMapping.end())
												addrMapping[sessionAddr] = poolRef;
										}
										mgrMutex.release();
									}
									// finish handling a message which does not have a message box for it
								}
								// finish handling message which is longer than the minimal
							}
							// finish handling a non-empty buffer
							delete []buf;
						}
					}

					i++;
				}
				delete []readySocket;
			}

			// remove some sockets in excess to garbage
			this->removePoolSocket();
			// clean up the garbage
			this->cleanupGarbage();
		}

		return NULL;
	}

	void PoolSocketMgr::freeSocketPool(){
		map<unsigned long, PoolSocket*>* pool;
		map<unsigned long, PoolSocket*>::iterator k, l;

		for (int n = 0; n < 2; n++){
			if (n == 0)
				pool = &reqSocketPool;
			else pool = &srvSocketPool;
			k = pool->begin();
			if (k != pool->end()){
				l = k;
				do{
					l++;
					pushGarbage(k->second);
					pool->erase(k);
					k = l;
				}while (l != pool->end());
			}
		}

		map<SocketServer*, ServerHandle*>::iterator i, j;
		// remove listening socket pool handles
		i = listeningSocketPool.begin();
		j = i;
		if (j != listeningSocketPool.end())
			do{
				i++;							
				if (j->first != NULL){
					listenerGarbage.insert(j->first);
				}
				listeningSocketPool.erase(j);
				j = i;
			}while (i != listeningSocketPool.end());
	}

	void PoolSocketMgr::undelegate(const ServerHandle* _server){
		map<unsigned long, ServerHandle*>::iterator k, l;
		map<SocketServer*, ServerHandle*>::iterator i, j;
		map<unsigned long, PoolSocket*>::iterator socketPointer;
		mgrMutex.lock();
		// remove server socket pool handles
		k = srvHandle.begin();
		l = k;
		if (l != srvHandle.end())
			do{
				l++;
				if (k->second == _server){
					socketPointer = srvSocketPool.find(k->first);
					if (socketPointer != srvSocketPool.end())
						pushGarbage(socketPointer->second);					
					srvSocketPool.erase(k->first);
					srvHandle.erase(k);
				}
				k = l;
			}while (l != srvHandle.end());

		// remove listening socket pool handles
		i = listeningSocketPool.begin();
		j = i;
		if (j != listeningSocketPool.end())
			do{
				i++;
				if (j->second == _server){					
					if (j->first != NULL){
						listenerGarbage.insert(j->first);
					}
					listeningSocketPool.erase(j);
				}
				j = i;
			}while (i != listeningSocketPool.end());
		mgrMutex.release();
	}

	void PoolSocketMgr::waitSocket(){
		// in WIN32 version, we use busy sleeping to wait for the condition;
		// in Linux, we use conition variable to wait
		#ifdef DHT_WIN32
		int i = 0;
			// modified by Kelvin: add a counter to break the sleep
			while ((i++<2) && (reqSocketPool.size() + srvSocketPool.size() == 0) && (isEmployed)){
				sleep(1);
			}
		#else
			if (isNoSocket){
				mgrMutex.lock();
				if (reqSocketPool.size() + srvSocketPool.size() == 0)
					waitSocketCondVar.timedWait(mgrMutex, 5000);
				isNoSocket = false;
				mgrMutex.release();
			}
		#endif
	}
	
	void PoolSocketMgr::pushGarbage(PoolSocket* _socket){
		if (_socket != NULL){
			_socket->preclose();
			socketGarbage.insert(_socket);
		}
	}
}
