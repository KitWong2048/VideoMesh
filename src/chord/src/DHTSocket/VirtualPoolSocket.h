/*
VirtualPoolSocket.h

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

#ifndef _H_VIRTUAL_POOL_SOCKET_
#define _H_VIRTUAL_POOL_SOCKET_

#include "../Util/Common.h"
#include "../DHTSocket/SocketClient.h"
#include "../DHTSocket/SessionReference.h"

using namespace std;

namespace DHTSocket{

	/**	Forward class definition the class PoolSocketMgr.
	 *	@See PoolSocketMgr
	 */
	class PoolSocketMgr;

	enum VirtualPoolSocketType{
		VirtualPoolReqSocket,
		VirtualPoolSrvSocket
	};

	/**	@class VirtualPoolSocket
	 * This class represents a virtual pool socket. By virtual, it means this is in fact not a socket.
	 * It provides a socket like interface for connecting, sending, receiving and closing.
	 * The actual pool socket is the class PoolSocket. Interfacing the actual pool socket is done through PoolSocketMgr.
	 */
	class VirtualPoolSocket{
	private:
		/** @var sessionRef
		 * The globally unique number that identify the session. It is in the format [Local IP][Local Port][Local Session #]
		 */
		SessionReference sessionRef;

		/** @var poolRef
		 * A local unique number for identifying the pool socket which this virtual socket associates with.
		 */
		unsigned long poolRef;

		/** @var isSocketDead
		 * True if this socket cannot be used anymore.
		 */
		 bool isSocketDead;

		/** @var socketType
		 * The type of the virtual socket.
		 */
		VirtualPoolSocketType socketType;

		/** @var mgr
		 * The reference to the socket pool manager
		 */
		PoolSocketMgr* mgr;

		/** @fn VirtualPoolSocket(unsigned long _poolRef, const SessionReference& _sessionRef, VirtualPoolSocketType _socketType, PoolSocketMgr* _mgr)
		 * @param _poolRef: the local unique number of the pool socket to associate with
		 * @param _sessionRef: the global unique session number
		 * @param _socketType: the type of the socket
		 * @param _mgr: a pointer to the manager
		 * A private constructor because only the class PoolSocketMgr can create instances of this class
		 */
		VirtualPoolSocket(unsigned long _poolRef, const SessionReference& _sessionRef, VirtualPoolSocketType _socketType, PoolSocketMgr* _mgr);

	public:
		/** @fn ~VirtualPoolSocket();
		 * A public destructor means that anyone can destroy instances of this class
		 */
		 ~VirtualPoolSocket();

		/** @fn void close();
		 * @return Nil
		 *	Close the virtual socket.
		 */
		void close();

		/** @fn int send(const char* _msg, int _len) const;
		 * @param _msg: the message to be sent
		 * @param _len: the length of the message
		 * @return the length of the message that is sent
		 *	Send a message through a socket.
		 */
		int send(const char* _msg, int _len) const;

		/** @fn int receive(char* &_msg, int _timeout = -1) const;
		 * @param _msg: the place to the store the message
		 * @param _timeout: amount of time waiting for the message, measured in millisecond
		 * @return the length of the message
		 *	Receive a message from the socket.
		 */
		int receive(char* &_msg, int _timeout = -1) const;

		/** @fn unsigned long getPoolRef() const;
		 * @return the local unique number for identifying the pool socket which this virtual socket associates with.
		 */
		unsigned long getPoolRef() const;

	friend class PoolSocketMgr;
	};
}
#endif
