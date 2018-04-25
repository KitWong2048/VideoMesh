#ifndef _SOCKET_CLIENT_H_IN_VMESH_
#define _SOCKET_CLIENT_H_IN_VMESH_

#include "string"
#include "../Util/Socket.h"
#include "../Util/Mutex.h"
#include "../Util/Win32Support.h"

/**	@file SocketClient.h
 *	This file contains the definition of class SocketClient
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class SocketClient
 *	This class provides communication of TCP Socket client
 */
class SocketClient : public Socket 
{
public:
	/**	@fn SocketClient(const char* ip, int port)
	 *	specified ip and port, use 0.0.0.0 if ip is null pointer
	 *	@param ip IP address
	 *	@param port port number
	 */
	SocketClient(const char* ip, int port);

	/**	@fn SocketClient(SOCKET sock, const char* ip, int port)
	 *	constructor with the underlying Socket;
	 *	specified ip and port, use 0.0.0.0 if ip is null pointer;
	 *	@param sock explicitly created SOCKET
	 *	@param ip IP address
	 *	@param port port number
	 */
	SocketClient(SOCKET sock, const char* ip, int port);
	
	/**	@fn ~SocketClient()
	 *	default destructor
	 */
	virtual ~SocketClient();

	virtual void close();
	
	/**	@fn connect(const char* destIP, int destPort, int timeout)
	 *	establish a connection to a SocketServer
	 *	@param destIP destination IP
	 *	@param destPort destination port
	 *	@param timeout timeout in ms
	 *	@return true if connect succeeds, false otherwise
	 */
	virtual bool connect(const char* destIP, int destPort, int timeout);

	
	/**	@fn send(const char* msg, int len=-1)
	 *	send msg of length len to a socket;
	 *	if msg is null-termainted, then len could be set to strlen(msg)+1, or leave it, otherwise len must be specified
	 *	@param msg character array of message
	 *	@param len length of the array
	 *	@return true if send succeeds, false otherwise
     */
	virtual bool send(const char* msg, int len=-1);
	
	/**	@fn receive(char* &msg, int timeout=-1)
	 *	blocking with timeout(in ms), wait to receive data;
	 *	@param msg location to be allocated memory to receive message
	 *	@param timeout timeout in ms, set to -1 for blocking forever
	 *	@return -1 if timeout
	 *	@return 0 if socket error
	 *	@return >0 if succeed (number of bytes received)
     */
	virtual int receive(char* &msg, int timeout=-1);

	// restrict receiving rate
	int receive(char* &msg, int timeout, int max_byte_per_sec);

	/**	@fn getRemoteIP()
	 *	get the IP address, shallow copy is returned
	 *	@return the direct pointer of IP address of the Socket
     */
	char* getRemoteIP();
	
	/**	@fn getRemotePort()
	 *	get the port number
	 *	@return the port number
     */	
	int getRemotePort();

private:

	char remoteIP[IP_LEN];	
	int remotePort;
	char buf[BUF_LEN];	
	Mutex myLock;

};
_VMESH_NAMESPACE_FOOTER
#endif