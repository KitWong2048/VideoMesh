/*
SocketServer.h

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

/**	@file SocketServer.h
 *	This file declares the class of a listening socket.
 */

#ifndef _SOCKET_SERVER_H
#define _SOCKET_SERVER_H

#include "../DHTSocket/Socket.h"
#include "../DHTSocket/SocketClient.h"
#include "../Util/Win32Support.h"

/*
Derived class of Socket for server
*/
namespace DHTSocket{
	/**	@class SocketServer
	 *	This class represents a listening socket.
	 */
	class SocketServer : public Socket{
	public:
		SocketServer(const char* _localIP, int _localPort);

		~SocketServer();

		/** @fn bool ready()
		 * @return true if the socket can be binded and start listening
		 */
		bool ready();

		/** @fn SocketClient* accept(int timeout = -1)
		 * @param timeout: the amount of time to wait for a client to connect
		 * @return a SocketClient object if a client is connected; otherwise NULL
		 */
		SocketClient* accept(int timeout = -1);
	};
}
#endif
