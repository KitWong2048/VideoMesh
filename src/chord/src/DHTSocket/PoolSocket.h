/*
PoolSocket.h

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

/**	@file PoolSocket.h
 *	This file declares the class for a socket in the socket pool.
 */

#ifndef _H_POOLSOCKET_
#define _H_POOLSOCKET_

#include "../Util/Common.h"
#include "../Util/ConditionVariable.h"
#include "../DHTSocket/SocketClient.h"
#include "../DHTSocket/VirtualPoolSocket.h"

#define POOL_SOCKET_TIMEOUT 1000

namespace DHTSocket{
	/**	@class PoolSocket
	 *	This class encapuslates a socket in the connection pool.
	 */
	class PoolSocket : public SocketClient{
	private:
		/** @var poolMutex
		 * The mutex for accessing the object.
		 */
		Util::Mutex poolMutex;

		/** @var poolCondVar
		 * The conditional varaible for accessing the object.
		 */
		Util::ConditionVariable poolCondVar;

		/** @var count
		 * This counts how many virtual pool socket is referencing this pool socket.
		 */
		int count;

		/** @var countMutex
		 * The mutex for accessing the member count.
		 */
		Util::Mutex countMutex;

		/** @var isBusy
		 * True when this socket is sending or receiving messages.
		 */
		bool isBusy;

		/** @var isSocketDead
		 * True when this socket has been closed by someone.
		 */
		bool isSocketDead;

		/** @var socketTimeout
		 * The timeout value for connecting and receiving.
		 */
		int socketTimeout;

		/** @var lastAccessedTime
		 * The time that this object is accessed last time.
		 */
		time_t lastAccessedTime;

		/** @var poolRef
		 * A local unique number for identifying this socket.
		 */
		unsigned long poolRef;

		/** @var userNum
		 * The number of users sending or receiving or waiting to do so.
		 */
		int userNum;

	public:
		PoolSocket(const char* _ip, int _port, int _socketTimeout, unsigned long _poolRef);

		PoolSocket(const SocketClient& _socket, int _socketTimeout, unsigned long _poolRef);

		~PoolSocket();

		/** @fn void preclose()
		 * @return Nil
		 * Declare the socket is about to close to prepare new virtual socket from using it.
		 */
		void preclose();

		/** @fn virtual void close()
		 * @return Nil
		 * Close the socket
		 */
		virtual void close();

		/** @fn virtual int send(const char* _msg, int _len)
		 * @param _msg: the message to be sent
		 * @param _len: the length of the message
		 * @return the number of bytes sent
		 * Send a message using the pool socket.
		 */
		virtual int send(const char* _msg, int _len);

		/** @fn virtual int receive(char* &_msg, int _timeout = -1)
		 * @param _msg: output param, the message to be received
		 * @param _timeout: the time to wait for a message
		 * @return the number of bytes received; -1 or 0 if failure
		 * Receive a message using the pool socket.
		 */
		virtual int receive(char* &_msg, int _timeout = -1);

		/** @fn void operator++(int)
		 * @return Nil
		 * Increment the member count.
		 */
		void operator++(int);

		/** @fn void operator--(int)
		 * @return Nil
		 * Decrement the member count.
		 */
		void operator--(int);

		/** @fn unsigned long getPoolRef() const
		 * @return the local unique number for identifying this socket.
		 */
		unsigned long getPoolRef() const;

		/** @fn bool isProxyOf(const VirtualPoolSocket& _virtualSocket)
		 * @param _virtualSocket
		 * @return true if the pool socket is connected to the host that the virutal socket communicates with.
		 *
		 */
		bool isProxyOf(const VirtualPoolSocket& _virtualSocket);

		/** @fn bool isInUse()
		 * @return true if someone is sending or receiving throught socket.
		 */
		bool isInUse();

		/** @fn time_t getLastAccesstedTime() const
		 * @return the time that this object is accessed last time.
		 */
		time_t getLastAccesstedTime() const;

		/** @fn int getCount()
		 * @return the count how many virtual pool socket is referencing this pool socket.
		 */
		int getCount();

		/** @fn int getUserNum()
		 * @return the number of users sending or receiving or waiting to do so.
		 */
		int getUserNum();
	};
}
#endif
