#ifndef _SOCKET_SERVER_H_IN_VMESH_
#define _SOCKET_SERVER_H_IN_VMESH_

#include "../Util/Socket.h"
#include "../Util/SocketClient.h"
#include "../Util/Win32Support.h"

/**	@file SocketServer.h
 *	This file contains the definition of class SocketServer
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

#define ESTABLISH_DELAY 5000

/**	@class SocketServer
 *	This class provides server functionality of TCP Socket
 */
class SocketServer : public VMesh::Socket
{
public:
	/**	@fn SocketServer(const char* ip, int port)
	 *	specified ip and port, use 0.0.0.0 if ip is null pointer
	 *	@param ip IP address
	 *	@param port port number
	 */
	SocketServer(const char* ip, int port);
	
	/**	@fn ~SocketClient()
	 *	default destructor
	 */
	~SocketServer();

	virtual void close();

	/**	@fn ready()
	 *	bind and listen port
	 *	@return true if the SocketServer is ready
	 */
	bool ready();

	/**	@fn accept(int timeout=-1)
	 *	blocking and wait to accept connection
	 *	@param timeout timeout in ms, set to -1 for blocking forever
	 *	@return the SocketClient that have been accepted
	 *	@return NULL if no SocketClient is accepted
     */
	SocketClient* accept(int timeout=-1);

private:
	/**	@fn listen()
	 *	listen port
	 *	@return true if the SocketServer is listening
	 */
	bool listen();
};
_VMESH_NAMESPACE_FOOTER
#endif
