/*
PoolSocketMgr.h

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

/**	@file PoolSocketMgr.h
 *	This file declares the class for handling the socket pool.
 */

#ifndef _H_POOL_SOCKET_MGR_
#define _H_POOL_SOCKET_MGR_

#include <map>
#include <set>
#include "../Util/Common.h"
#include "../Util/SeqGenerator.h"
#include "../Util/Thread.h"
#include "../Util/InetAddress.h"
#include "../DHTSocket/VirtualPoolSocket.h"
#include "../DHTSocket/PoolSocket.h"
#include "../DHTSocket/ServerHandle.h"
#include "../DHTSocket/SocketSelector.h"
#include "../DHTSocket/MessageBoxSet.h"
#include "../DHTSocket/SessionReference.h"

namespace DHTSocket{
	/**	@class PoolSocketMgr
	 *	This class manages the connection pool.
	 * Note that this class follows singular pattern. That means there is only one instance of this class.
	 */
	class PoolSocketMgr : public Util::Thread{
	private:
		/** @var mgr
		 * Pointer to the singular manager instance.
		 */
		static PoolSocketMgr* mgr;

		/** @var isEmployed
		 * True if the manager has not been fired yet.
		 */
		bool isEmployed;

		/** @var mgrMutex
		 * Lock for exclusive access to critical region in this class.
		 */
		Util::Mutex mgrMutex;

		#ifndef DHT_WIN32
		/** @var waitSocketCondVar
		 * Condition variable for waiting the socket pool being filled.
		 */
		Util::ConditionVariable waitSocketCondVar;
		#endif

		/** @var localAddr
		 * A combination of IP and port. This address is used as part of the global identifer for sessions.
		 */
		Util::InetAddress localAddr;

		/** @var refGenerator
		 * Generate a sequence of reference number for pool sockets.
		 */
		Util::SeqGenerator refGenerator;

		/** @var sessionGenerator
		 * Generate a sequence of reference number for sessions.
		 */
		Util::SeqGenerator sessionGenerator;

		/** @var reqPoolMaxSize
		 * The maximum size of the outgoing connection pool.
		 * It is possible that the actual number of connection may exceed this number
		 * because if the maximum is reached and all connections are in used and a new socket is requested
		 * then we cannot only clean up sockets immediately
		 */
		unsigned int reqPoolMaxSize;

		/** @var srvPoolMaxSize
		 * The maximum size of the incoming connection pool
		 * It is possible that the actual number of connection may exceed this number
		 * because if the maximum is reached and all connections are in used and a new socket is requested
		 * then we cannot only clean up sockets immediately
		 */
		unsigned int srvPoolMaxSize;

		/** @var isNoSocket
		 * True if there is no socket in the pool
		 */
		bool isNoSocket;

		/** @var addrMapping
		 * The map from an address to a reference number of the outgoing socket
		 */
		map<Util::InetAddress, unsigned long> addrMapping;

		/** @var reqSocketPool
		 * The map from a reference number of the outgoing socket to instance of the socket
		 */
		map<unsigned long, PoolSocket*> reqSocketPool;

		/** @var srvSocketPool
		 * The map from a reference number of the incoming socket to instance of the socket
		 */
		map<unsigned long, PoolSocket*> srvSocketPool;

		/** @var listeningSocketPool
		 * The map of listening socket and corresponding server handles
		 */
		map<SocketServer*, ServerHandle*> listeningSocketPool;

		/** @var srvHandle
		 * The map from a reference number of the incoming socket to the corresponding server handle
		 */
		map<unsigned long, ServerHandle*> srvHandle;

		/** @var socketGarbage
		 * Store sockets that will be removed after no one will use it any more
		 */
		set<PoolSocket*> socketGarbage;

		/** @var listenerGarbage
		 * Store server sockets that will be removed after no one will use it any more
		 */
		set<SocketServer*> listenerGarbage;

		/** @var messenger
		 * Messenger to deliver messages from pool socket to virtual socket
		 */
		MessageBoxSet messenger;

		PoolSocketMgr(const char* _localIP, int _localPort);

		virtual ~PoolSocketMgr();

		/** @fn bool isReqSocketExist(unsigned long _ref);
		 * @param _ref: reference number to be checked
		 * @return true if the given reference number exists in the outgoing connection pool.
		 */
		 bool isReqSocketExist(unsigned long _ref);

		/** @fn bool isSrvSocketExist(unsigned long _ref);
		 * @param _ref: reference number to be checked
		 * @return true if the given reference number exists in the incoming connection pool.
		 */
		bool isSrvSocketExist(unsigned long _ref);

		/** @fn void removePoolSocket();
		 * @return Nil
		 *	Push th least recently used socket from each of the incoming and outgoing connection pool to garbage.
		 */
		void removePoolSocket();

		/** @fn void cleanupGarbage();
		 * @return Nil
		 *	Free the garbage.
		 */
		void cleanupGarbage();

		/** @fn void freeSocketPool()
		 * @return Nil
		 *	Push all sockets in the connection pool to garbage.
		 */
		void freeSocketPool();

		void waitSocket();
		
		void pushGarbage(PoolSocket* _socket);

	protected:
		/** @fn void* execute()
		 * @return NULL;
		 *	This is the starting point of running the manager.
		 * Override the execute() method in Util::Thread class.
		 */
		void* execute();

	public:
		/** @fn static int employ(unsigned int _reqPoolMaxSize, unsigned int _srvPoolMaxSize, const char* _localIP, int _localPort)
		 * @param _reqPoolMaxSize: the number of request socket kept in the pool
		 * @param _srvPoolMaxSize: the number of service socket kept in the pool
		 * @param _localIP: the local IP address which the manager will use it as part of the key
		 * @param _localPort: the local Port address which the manager will use it as part of the key
		 * @return 0 if success
		 *	Create an instance of socket pool manager. Note that there is only one single instance for one process.
		 *	There should not be more than one process using this class with same _localIP and _localPort.
		 */
		static int employ(unsigned int _reqPoolMaxSize, unsigned int _srvPoolMaxSize, const char* _localIP, int _localPort);

		/** @fn static int dismiss()
		 * @return 0 if success
		 *	Dismiss the socket pool manager. Once the function is called, the application cannot use the connection pool unless employ() is called
		 */
		static int dismiss();

		/** @fn static PoolSocketMgr* invokePoolSocketMgr()
		 * @return an instance of socket pool manager if one exists; NULL if not
		 *	Get a reference to the socket pool manager.
		 */
		static PoolSocketMgr* invokePoolSocketMgr();

		/** @fn VirtualPoolSocket* connect(const char* _remoteIP, int _remotePort, int _timeout, ServerHandle& _server)
		 * @param _remoteIP: the IP of the host to be connected
		 * @param _remotePort: the port of the host to be connected
		 * @param _timeout: the amount of time to wait for establishing the connection, measured in ms
		 * @param _server: the server that provides callback function for handling new request which will arrive at the socket because client may use the connection to be established to make request
		 * @return a socket for I/O with the remote host
		 *	Connect to a remote host.
		 */
		VirtualPoolSocket* connect(const char* _remoteIP, int _remotePort, int _timeout, ServerHandle& _server);

		/** @fn int delegate(SocketClient& _socket, ServerHandle& _server)
		 * @param _socket: the socket to be monitored by the manager
		 * @param _server: the server that provides callback function for handling new request which will arrive at the socket
		 * @return 0 if success
		 *	Put a communication socket under the monitoring of the pool manager.
		 *	When new request arrives at this socket, the response(VirtualPoolSocket* _socket) function in the _server will be called.
		 *	Application should inherit the class ServerHandle and override the response function with non-blocking I/O.
		 */
		int delegate(SocketClient& _socket, ServerHandle& _server);

		/** @fn int delegate(SocketServer& _socket, ServerHandle& _server)
		 * @param _socket: the socket to be monitored by the manager
		 * @param _server: the server that provides callback function for handling new request which will arrive at the socket
		 * @return 0 if success
		 *	Put a communication socket under the monitoring of the pool manager.
		 *	When new request arrives at this socket, the response(SocketServer* _socket) function in the _server will be called.
		 *	Application should inherit the class ServerHandle and override the response function with non-blocking I/O.
		 */
		int delegate(SocketServer& _socket, ServerHandle& _server);

		/** @fn void undelegate(const ServerHandle* _server)
		 * @param _server: the server that has previously delegated sockets to the manager
		 * @return Nil
		 *	Remove all the server handle from the manager so that previously delegated sockets, except those are sending or receiving, will be ignored .
		 */
		void undelegate(const ServerHandle* _server);

		/** @fn int send(const VirtualPoolSocket& _socket, const char* _msg, int _len);
		 * @param _socket: the socket that is sending the message
		 * @param _msg: the message to be sent
		 * @param _len: the length of the message
		 * @return the length of the message that is sent
		 *	Send a message through a socket.
		 */
		int send(const VirtualPoolSocket& _socket, const char* _msg, int _len);

		/** @fn int receive(const VirtualPoolSocket& _socket, char* & _msg, int _timeout = -1)
		 * @param _socket: the socket that is receiving the message
		 * @param _msg: the place to the store the message
		 * @param _timeout: amount of time waiting for the message, measured in millisecond
		 * @return the length of the message
		 *	Receive a message from the socket.
		 */
		int receive(const VirtualPoolSocket& _socket, char* & _msg, int _timeout = -1);

		/** @fn void close(const VirtualPoolSocket& _socket)
		 * @param _socket: the socket that is being closed
		 * @return Nil
		 *	Close a socket.
		 */
		void close(const VirtualPoolSocket& _socket);
	};
}
#endif
