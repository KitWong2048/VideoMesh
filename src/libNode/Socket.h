#pragma once

#include "LookUpCommon.h"
#include <string>
#include <iostream>

#ifndef BUF_LEN
#define BUF_LEN 1024
#endif


/**	@class Socket
 *	This class provides abstract of TCP Socket functions
 */
class Socket
{
public:
	Socket(std::string ip = DEFAULT_SERVER_IP, int port = DEFAULT_LOOKUP_PORT);
	~Socket();
	int SocketConnect(void);
	std::string SocketSend(const std::string command, const std::string hash, const std::string content = "");
	int SocketClose();

private:
	int SocketRead(char *receivebuffer, int buffersize, char *receivebuffer2, int buffersize2);
	void OutputScreenError(const char *errmsg);

	std::string myIP;
	int myPort;
	SOCKET sock_;
};
