#include "../Util/SocketServer.h"
_VMESH_NAMESPACE_HEADER

SocketServer::SocketServer(const char* ip, int port)
: Socket(ip, port)
{
	/*create a socket*/
	sockFD=socket(AF_INET, SOCK_STREAM,0);
	if(sockFD<0){
		perror("Cannot create socket\n");
		exit(1);
	}
}

SocketServer::~SocketServer()
{
	this->close();
}

void SocketServer::close()
{

	if(sockFD != -1)
	{
		#ifndef _VMESH_WIN32_
			::close(sockFD);
		#else
			::closesocket(sockFD);
		#endif
		sockFD = -1;
	}
}

bool SocketServer::listen()
{
	//list on the port
	if(::listen(sockFD, 10)<0){
		perror("listen");
		return 0;
	}
	return 1;
}

SocketClient* SocketServer::accept(int timeout)
{
	SOCKET remoteSock = -1;
	socklen_t remoteAddrLen;
	struct sockaddr_in remoteAddr;
	char remoteIP[IP_LEN];

#ifdef SOCKET_DEBUG
//	printf("Waiting client to connect.\n");
#endif
	bzero(&remoteAddr,sizeof(remoteAddr));
	remoteAddrLen=sizeof(remoteAddr);

	if (timeout<0)
	{
		//accept the connection from client
		remoteSock = ::accept(sockFD, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
	}
	else
	{
		
		// Set sockFD non-blocking

		#ifndef _VMESH_WIN32_	// non-Win32
			long arg;
			arg = fcntl(sockFD, F_GETFL, NULL);
			arg |= O_NONBLOCK;
			fcntl(sockFD, F_SETFL, arg);
		#else // Win32
			u_long blockingMode = 1; // non-zero is non-blocking mode
			ioctlsocket(sockFD, FIONBIO, &blockingMode);
		#endif
		// wait for timeout
		fd_set myset;
		struct timeval tv;

		tv.tv_sec = timeout/1000;
		tv.tv_usec = 1000*(timeout%1000);
		FD_ZERO(&myset);
		FD_SET(sockFD, &myset);
		int state = select((int)sockFD+1, &myset, NULL, NULL, &tv);
		if (state > 0) {
			//accept the connection from client
			remoteSock = ::accept(sockFD, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
		}
		else if (state==0){
			//printf("accept() timeout\n");
			return 0;
		}
		else if (state==-1){
			//printf("accept() fails\n");
			return 0;
		}

		// Set sockFD blocking again

		#ifndef _VMESH_WIN32_	// non-Win32
			arg = fcntl(sockFD, F_GETFL, NULL);
			arg &= (~O_NONBLOCK);
			fcntl(sockFD, F_SETFL, arg);
		#else	// Win32
			blockingMode = 0; // zero is blocking mode
			ioctlsocket(sockFD, FIONBIO, &blockingMode);
		#endif
	}

	if(remoteSock < 0)
	{
#ifdef SOCKET_DEBUG
//		printf("Cannot accept client connection.\n");
#endif
		return 0;
	}

	bzero(remoteIP,IP_LEN);

#ifdef SOCKET_DEBUG
	//printf the ip address of the client
//	printf("Client (%s:%d) connected\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
#endif

	SocketClient* client = new SocketClient(remoteSock, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));

	return client;
}

bool SocketServer::ready()
{
	if (!bind(sockFD))
		return 0;
	if (!listen())
		return 0;
	return 1;
}

_VMESH_NAMESPACE_FOOTER
