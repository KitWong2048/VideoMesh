/*
SocketSelector.h

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

/**	@file SocketSelector.h
 *	This file declares the class of selecting from a list the sockets ready that are for reading.
 */

#ifndef _H_SOCKET_SELECTOR_
#define _H_SOCKET_SELECTOR_

#include "../Util/Common.h"
#include "../DHTSocket/Socket.h"
#include <map>
#include <set>

using namespace std;

namespace DHTSocket{
	/**	@class SocketSelector
	 *	This class is the data structure for selecting from a list of sockets those that are ready for reading.
	 */
	class SocketSelector{
		private:
			 /** @var fdSet
			* The object for holding file descriptor.
			*/
			fd_set fdSet;

			 /** @var socketSet
			* Placeholder for the list of sockets.
			*/
			map<SOCKET, Socket*> socketSet;

		public:
			SocketSelector();

			~SocketSelector();

			/** @fn int setSocket(Socket** _socket)
			*	@param _socket: The array of sockets to be put into the selector, each entry is a pointer to Socket object, the last element should be NULL to indicate end of array.
			 * @return 0 if it is successful;
			 *	This function puts a set of sockets into the selector so that application does not need to be blocked by a particular socket.
			 */
			int setSocket(Socket** _socket);

			int setSocket(set<Socket*>& _socket);

			/** @fn void clearSocket()
			 *	This function removes all the sockets in the selector.
			 */
			void clearSocket();

			/** @fn Socket** selectSocket(unsigned int _timeout)
			*	@param _timeout: The maximum amount of waiting time for any one of the input.
			 * @return an array of pointers to Socket which have their input available;
			 *	This function waits for input to come to anyone of the sockets in the selector.
			 */
			Socket** selectSocket(unsigned int _timeout);
	};
}


#endif
