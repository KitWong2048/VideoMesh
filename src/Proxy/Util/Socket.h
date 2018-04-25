#ifndef _SOCKET_H_IN_VMESH_
#define _SOCKET_H_IN_VMESH_

#include "../Common/VMeshCommon.h"

/**	@file Socket.h
 *	This file contains the definition of class Socket
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/*! @brief show debug message for Socket */
//#define SOCKET_DEBUG

#ifndef IP_LEN
#define IP_LEN 16
#endif

#ifndef BUF_LEN
#define BUF_LEN 102400
#endif

#ifndef _VMESH_WIN32_
#define SOCKET int
#endif

/**	@class Socket
 *	This class provides abstract of TCP Socket functions
 */
class Socket
{
public:
	/**	@fn close()
	 *	close the socket
	 */
	virtual void close() = 0;
protected:
	/**	@fn Socket()
	 *	default blank constructor
	 */
	Socket();

	/**	@fn Socket(const char* ip, int port)
	 *	specified ip and port, use 0.0.0.0 if ip is null pointer
	 *	@param ip IP address
	 *	@param port port number
	 */
	Socket(const char* ip, int port);

	/**	@fn ~Socket()
	 *	default destructor
	 */
	virtual ~Socket();

	/**	@fn send(SOCKET sock, const char* msg, int len)
	 *	send msg of length len to a socket
	 *	@param sock socket to be used
	 *	@param msg character array of message
	 *	@param len length of the array
	 *	@return true if send succeeds, false otherwise
     */
	bool send(SOCKET sock, const char* msg, int len);

	/**	@fn receive(SOCKET sock, char* &msg, int timeout)
	 *	blocking with timeout(in ms), wait to receive data;
	 *	@param msg location to be allocated memory to receive message
	 *	@param sock socket to be used
	 *	@param buf character array of message
	  *	@param bufferSize maximum number of bytes to be read
	 *	@param timeout timeout in ms, set to -1 for blocking forever
	 *	@return -1 if timeout
	 *	@return 0 if socket error
	 *	@return >0 if succeed (number of bytes received)
     */
	int receive(SOCKET sock, char* buf, int bufferSize, int timeout);

	/**	@fn bind(SOCKET sock)
	 *	bind the socket
	 *	@param sock socket to be used
	 *	@return true if bind succeeds, false otherwise
	 */
	bool bind(SOCKET sock);

private:
	/**	@fn initialize()
	 *	initialize all parameters for a socket to be used
	 */
	void initialize();

protected:
	char myIP[IP_LEN];
	int myPort;
	struct sockaddr_in myAddr;

	
	SOCKET sockFD;

	#ifdef _VMESH_WIN32_
		bool isWinSocketInited;
		/**	@fn initWinSocket()
		 *	initialize Socket for Windows
		 */
		void initWinSocket();
	#endif
	friend class SocketSelector;

};
_VMESH_NAMESPACE_FOOTER
#endif