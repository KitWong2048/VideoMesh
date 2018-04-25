#include "../Util/Socket.h"
#include "../Util/Win32Support.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

/*
blank constructor
*/
Socket::Socket()
{
}

/*
constructor that takes specified ip and port
use 0.0.0.0 if ip is null pointer
*/
Socket::Socket(const char* ip, int port)
{
	if (ip==0)
		strcpy(myIP, "0.0.0.0");
	else
		strcpy(myIP, ip);
	myPort=port;

	initialize();
}


void Socket::initialize()
{
	/*create the local address structure*/	
	bzero(&myAddr,sizeof(myAddr));
	myAddr.sin_family=AF_INET;
	
	if (strcmp(myIP, "0.0.0.0")==0)
		myAddr.sin_addr.s_addr = INADDR_ANY;
	else
		inet_aton(myIP, &(myAddr.sin_addr));

	myAddr.sin_port = htons(myPort);
	memset(&(myAddr.sin_zero), '\0', 8);
	#ifdef _VMESH_WIN32_
		isWinSocketInited = false;
	#endif
}

Socket::~Socket()
{
}

bool Socket::bind(SOCKET sock)
{		
	int yes=1;
	// lose the pesky "Address already in use" error message
	#ifndef _VMESH_WIN32_
		if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) { // non-Win32
			#ifdef SOCKET_DEBUG
					perror("setsockopt");
			#endif
			exit(1);
		}
	#else
		if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(int)) == -1) { // Win32
			exit(1);
		}
	#endif

	//bind the socket to the port
	myAddr.sin_addr.s_addr = INADDR_ANY;	// added to bind to non-local ip (public/external ip while in NAT)
	if(::bind(sock,(struct sockaddr*)&myAddr,sizeof(myAddr))<0){
		perror("Cannot bind port.");
		return 0;
	}
	return 1;
}

bool Socket::send(SOCKET sock, const char* msg, int len)
{
	if (len<=0)
		return false;

	#ifndef _VMESH_WIN32_
		// check sock if valid
		if (sock==(long)stdin || sock==(long)stdout || sock==(long)stderr)
			return 0;
	#endif

	int sendlen;
	#ifdef _VMESH_WIN32_
		sendlen = ::send(sock, msg, len, 0);
	#else
		sendlen = ::send(sock, msg, len, MSG_NOSIGNAL);
	#endif
			
	if ( sendlen != len )
		printf("data len: %d\tsent len = %d\n", len, sendlen);
	return (sendlen==len);
}


int Socket::receive(SOCKET sock, char* buf, int bufferSize, int timeout)
{
	//char buf[BUF_LEN];
	int len = 0, state;
	#ifndef _VMESH_WIN32_
		long arg;
	#endif

	fd_set socketSet;
	struct timeval tv;

	bzero(buf, bufferSize);

	#ifndef _VMESH_WIN32_
		// check sockFD if valid
		if (sockFD == (long)stdin || sockFD == (long)stdout || sockFD == (long)stderr)
			return 0;
	#endif
	if (timeout < 0){
		len = recv(sockFD, buf, bufferSize, 0);
	}else{
		// Set mySock non-blocking
		#ifndef _VMESH_WIN32_	// non-Win32
			arg = fcntl(sockFD, F_GETFL, NULL);
			arg |= O_NONBLOCK;
			fcntl(sockFD, F_SETFL, arg);
		#else	// Win32
			u_long blockingMode = 1; // non-zero is non-blocking mode
			ioctlsocket(sockFD, FIONBIO, &blockingMode);
		#endif

		tv.tv_sec = timeout / 1000;
		tv.tv_usec = 1000 * (timeout % 1000);
		FD_ZERO(&socketSet);
		FD_SET(sockFD, &socketSet);
		state = select((int)sockFD + 1, &socketSet, NULL, NULL, &tv);
		if (state > 0){
			len = recv(sockFD, buf, bufferSize, 0);
		}else if (state == 0){
			len = -1;
		}else if (state == -1){
			len = 0;
		}
		// set socket blocking again
		#ifndef _VMESH_WIN32_	// non-Win32
			arg = fcntl(sockFD, F_GETFL, NULL);
			arg &= (~O_NONBLOCK);
			fcntl(sockFD, F_SETFL, arg);
		#else	// Win32
			blockingMode = 0; // zero is blocking mode
			ioctlsocket(sockFD, FIONBIO, &blockingMode);
		#endif
	}

	/*if (len > 0){
		msg = new char[len];
		memcpy(msg, buf, len);
	}*/
	return len;
}

#ifdef _VMESH_WIN32_
void Socket::initWinSocket(){
	WORD sockVersion;
	WSADATA wsaData;
	if (!isWinSocketInited){
		sockVersion = MAKEWORD(2, 0);
		WSAStartup(sockVersion, &wsaData);
		isWinSocketInited = true;
	}

}
#endif
_VMESH_NAMESPACE_FOOTER

