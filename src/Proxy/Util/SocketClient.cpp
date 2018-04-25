#include "../Util/SocketClient.h"

using std::string;

_VMESH_NAMESPACE_HEADER

SocketClient::SocketClient(SOCKET sock, const char* ip, int port)
: Socket(), myLock("SocketClientLock")
{
	sockFD = sock;
	strcpy(remoteIP, ip);
	remotePort = port;
}

SocketClient::SocketClient(const char* ip, int port)
: Socket(ip, port), myLock("SocketClientLock")
{
	sockFD = -1;
}

SocketClient::~SocketClient()
{
	this->close();
}

void SocketClient::close()
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

// connect to a remote SocketServer, return if connect succeeds
bool SocketClient::connect(const char* destIP, int destPort, int timeout)
{
#ifdef SOCKET_DEBUG
	printf("connect(): My IP is %s(%d)\n", inet_ntoa(myAddr.sin_addr), ntohs(myAddr.sin_port));
#endif

	if (sockFD != -1)
		return 0;

	struct sockaddr_in remoteAddr;

	/*create a socket*/
	#ifdef _VMESH_WIN32_
		initWinSocket();
		sockFD = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	#else
		sockFD = ::socket(AF_INET, SOCK_STREAM, 0);
	#endif

	if(sockFD == -1){
#ifdef SOCKET_DEBUG
		printf("Cannot create socket\n");
#endif
		return 0;
	}

	if (!Socket::bind(sockFD))
		return 0;

	/*create the server address structure*/
	bzero(&remoteAddr, sizeof(remoteAddr));
	remoteAddr.sin_family=AF_INET;

	// send to myself if ip is null
	if (destIP==0)
		remoteAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	else
		remoteAddr.sin_addr.s_addr=inet_addr(destIP);

	remoteAddr.sin_port=htons(destPort);
	memset(&(remoteAddr.sin_zero), '\0', 8);

	// set non-blocking mode
	u_long iMode = 1;
	ioctlsocket(sockFD, FIONBIO, &iMode);
	/*connect to server*/
	::connect(sockFD, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));

	fd_set writeSet;
	struct timeval wait;
	wait.tv_sec  = timeout / 1000;
	wait.tv_usec = ( timeout % 1000 ) * 1000;

	FD_ZERO( &writeSet );
	FD_SET( sockFD, &writeSet );
	int selectResult = select( FD_SETSIZE, NULL, &writeSet, NULL, &wait );

	if ( selectResult == 0 )
	{
#ifdef SOCKET_DEBUG
		printf ("Errno: %d\n", errno);
		perror("Cannot connect to the server.");
#endif
		return 0;
	}

	//To set the socket back to blocking mode, an application must first disable
	//WSAAsyncSelect by calling WSAAsyncSelect with the lEvent parameter equal to
	//zero, or disable WSAEventSelect by calling WSAEventSelect with the lNetworkEvents
	//parameter equal to zero.
	//int evtret = WSAEventSelect(sockFD, NULL, 0);
	iMode = 0;
	ioctlsocket(sockFD, FIONBIO, &iMode);

	return 1;
}

// Total message length = 102400[BUF_LEN]
bool SocketClient::send(const char* msg, int len)
{
	myLock.lock();

	// determine the length
	if (len==-1)
		len = (int)strlen(msg)+1;

	// send message length
	char mymsg[BUF_LEN];
	mymsg[0] = mymsg[1] = mymsg[2] = mymsg[3] = 0;
	int length = len;
	if ( len < 256 )
	{
		mymsg[0] = (char) len;
	}
	else
	{
		mymsg[0] = char(length & 0xff);
		length >>= 8;
		mymsg[1] = char(length & 0xff);
		length >>= 8;
		mymsg[2] = char(length & 0xff);
		length >>= 8;
		mymsg[3] = char(length);
	}
	memcpy(mymsg+4, msg, len);

	bool success = Socket::send(sockFD, mymsg, len+4);

#ifdef SOCKET_DEBUG
	printf("Send: Packet length %d bytes\n", len);
#endif
	myLock.release();
	return success;
}

int SocketClient::receive(char* &msg, int timeout)
{
	myLock.lock();

	msg = NULL;

	int recv_state = Socket::receive(sockFD, buf, sizeof(int), timeout);
	
	if ( recv_state < 4 )
	{
		myLock.release();
		return recv_state;
	}

	int len;
	len = (unsigned char)buf[3];
	len <<= 8; //len *= 256;
	len += (unsigned char)buf[2];
	len <<= 8; //len *= 256;
	len += (unsigned char)buf[1];
	len <<= 8; //len *= 256;
	len += (unsigned char)buf[0];

//#ifdef SOCKET_DEBUG
	
//#endif

	if ( len > 0 )
		recv_state = Socket::receive(sockFD, buf, len, timeout);

	if ( len == 0 || recv_state < 0 )
	{
		myLock.release();
		return recv_state;
	}

	int total_len = 0;

	msg = new char[len];
	if ( msg == NULL )
	{
		myLock.release();
		return 0;
	}

	if ( recv_state <= len )
		memcpy(msg, buf, recv_state );

	total_len = recv_state;

	while (total_len<len)
	{
		recv_state = Socket::receive(sockFD, buf, (len - total_len) , timeout); // (len - total_len) = remaining data required
		if ( recv_state <= 0 )
		{
			delete [] msg;
			msg = NULL;
			myLock.release();
			return recv_state;
		}
		if (total_len + recv_state > len)
		{
			len = 0;
			break;
		}
		memcpy(msg + total_len, buf, recv_state);
		total_len += recv_state;
		#ifdef SOCKET_DEBUG
			printf ("recv = %d, total recv = %d len = %d\n", recv_state, total_len, len);
		#endif

	}
	
	if (total_len == len)
	{
		myLock.release();
		return len;
	}
	else
	{
		delete [] msg;
		msg = NULL;
		myLock.release();
		return 0;
	}
}

// for rate control
int SocketClient::receive(char* &msg, int timeout, int max_byte_per_sec)
{
	int state;
	int time_taken_in_msec;
	struct timeval start, end, diff;
	
#ifdef _VMESH_WIN32_
	LARGE_INTEGER freq;
	LARGE_INTEGER start_time, end_time;
	if ( QueryPerformanceFrequency(&freq) && max_byte_per_sec > 0  )
	{
		QueryPerformanceCounter(&start_time);
		state = receive(msg, timeout);
		QueryPerformanceCounter(&end_time);
		time_taken_in_msec = (end_time.QuadPart - start_time.QuadPart) * 1000/freq.QuadPart;

	}
	else
	{
		bzero(&start, sizeof(start));
		bzero(&end, sizeof(end));
		bzero(&diff, sizeof(diff));

		gettimeofday(&start, NULL);

		state = receive(msg, timeout);

		gettimeofday(&end, NULL);
		
		if (end.tv_usec < start.tv_usec)
		{
			diff.tv_usec = 1000000 + end.tv_usec - start.tv_usec;
			diff.tv_sec = end.tv_sec - start.tv_sec - 1;
		}
		else
		{
			diff.tv_usec = end.tv_usec - start.tv_usec;
			diff.tv_sec = end.tv_sec - start.tv_sec;
		}
		
		time_taken_in_msec = diff.tv_sec * 1000 + diff.tv_usec / 1000 ;
	}

#else

	bzero(&start, sizeof(start));
	bzero(&end, sizeof(end));
	bzero(&diff, sizeof(diff));

	gettimeofday(&start, NULL);

	state = receive(msg, timeout);

	gettimeofday(&end, NULL);

	if (end.tv_usec < start.tv_usec)
	{
		diff.tv_usec = 1000000 + end.tv_usec - start.tv_usec;
		diff.tv_sec = end.tv_sec - start.tv_sec - 1;
	}
	else
	{
		diff.tv_usec = end.tv_usec - start.tv_usec;
		diff.tv_sec = end.tv_sec - start.tv_sec;
	}

	time_taken_in_msec = diff.tv_sec * 1000 + diff.tv_usec / 1000 ;

#endif

	if ( state > 0 )
	{
		int max_time_allowed = state * 1000 / max_byte_per_sec;	// in ms
		int sleep_time = max_time_allowed - time_taken_in_msec;
		if ( sleep_time  > 0 )
			usleep( sleep_time * 1000 );
		else
			sleep_time = 0;

		//printf("Received: %d Bytes\tRate: %d\tMin time: %d ms\tTime taken: %d ms\tSleep: %d ms\n",state, max_byte_per_sec, max_time_allowed, time_taken_in_msec, sleep_time);
	}

	return state;
}

char* SocketClient::getRemoteIP()
{
	return remoteIP;
}

int SocketClient::getRemotePort()
{
	return remotePort;
}
_VMESH_NAMESPACE_FOOTER

