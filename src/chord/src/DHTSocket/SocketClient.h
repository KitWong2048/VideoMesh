/*
SocketClient.h

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

/**	@file SocketClient.h
 *	This file declares the class of a communication socket.
 */

#ifndef _H_SOCKET_CLIENT_
#define _H_SOCKET_CLIENT_

#include "../DHTSocket/Socket.h"
#include "../Util/Win32Support.h"

namespace DHTSocket{
	/**	@class SocketClient
	 *	This class declares communication socket.
	 */
	class SocketClient : public Socket{
	public:
		SocketClient(const char* _remoteIP, int _remotePort);

		SocketClient(SOCKET _sockFD, const char* _localIP, int _localPort);

		SocketClient(const SocketClient& _socket);

		virtual ~SocketClient();

		/** @fn virtual bool connect(const char* _remoteIP, int _remotePort, int _timeout)
		 * @param _remoteIP: the remote host IP
		 * @param _remotePort: the remote host port
		 * @param _timeout: the amount of time to wait for establishing the connection
		 * @return true if connecton succeeds
		 *	Connect to a remote host
		 */
		virtual bool connect(const char* _remoteIP, int _remotePort, int _timeout);

		/** @fn int send(const char* _msg, int _len);
		 * @param _msg: the message to be sent
		 * @param _len: the length of the message
		 * @return the number of bytes sent
		 *	Sent a message.
		 */
		virtual int send(const char* _msg, int _len = -1);

		/** @fn int receive(char* &_msg, int _timeout);
		 * @param _msg: output param, the message received
		 * @param _timeout: the time to wait for receiving a message; -1 means waiting forever
		 * @return -1 if timeout; 0 if socket error; otherwise return the length of the message received
		 *	Receive a message.
		 */
		virtual int receive(char* &_msg, int _timeout = -1);

		/** @fn char* getRemoteIP();
		 * @return the IP of the remote host which this socket has connected to
		 */
		char* getRemoteIP();

		/** @fn char* getRemotePort();
		 * @return the port of the remote host which this socket has connected to
		 */
		int getRemotePort();

	private:
		/** @var remoteIP
		 * The IP of the remote host which this socket has connected to
		 */
		char remoteIP[IP_LEN];

		/** @var remotePort
		 * The port of the remote host which this socket has connected to
		 */
		int remotePort;

	};
}
#endif
