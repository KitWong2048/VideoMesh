/*
ServerHandle.h

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

/**	@file ServerHandle.h
 *	This file declares the class which an application has to inherit from.
 */

#ifndef _H_SERVER_HANDLE_
#define _H_SERVER_HANDLE_

#include "../Util/Common.h"
#include "../DHTSocket/VirtualPoolSocket.h"
#include "../DHTSocket/SocketServer.h"

namespace DHTSocket{
	/**	@class ServerHandle
	 * This class declares the interface for classes which wish to use the connection pool.
	 * It stipulates an implementation for the function response. It is called when there is a new request arriving at the communication pool socket on the server side.
	 */
	class ServerHandle{
	public:
		virtual ~ServerHandle();

		/** @fn virtual int response(VirtualPoolSocket*  _socket);
		 * @param _socket: the socket to communicate withe the connected host which makes a new request
		 * @return 0 if there is no error
		 *	A class may wish to make use of the connection pool.
		 * When a communication socket is accepted on the server side, it is put into the socket pool.
		 * Later requests from the client will arrive at this socket.
		 * Upon receiving the request, a new virtual pool socket is created. The server has to provide a way to handle this request.
		 * The server class has to overwrite this method to implement the desired behaviour.
		 * This function is supposed to close and delete the socket.
		 */
		virtual	int response(VirtualPoolSocket*  _socket);

		/** @fn virtual int response(SocketServer*  _socket);
		 * @param _socket: the socket to accept client
		 * @return 0 if there is no error
		 * Similar to the above function but this function is defined for server type socket
		 * The server class has to overwrite this method to implement the desired behaviour.
		 * This function is NOT supposed to close and delete the socket.
		 */
		virtual	int response(SocketServer*  _socket);
	};
}
#endif
