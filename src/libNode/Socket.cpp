#include <winsock2.h>

#include "Socket.h"

using namespace std;



Socket::Socket(std::string ip, int port): myIP(ip), myPort(port)
{
}

Socket::~Socket(void)
{
	SocketClose();
}

int Socket::SocketConnect(void)
{
	sock_ = socket(AF_INET,SOCK_STREAM,0);
	if ( sock_ == INVALID_SOCKET )
	{
		OutputScreenError("Error creating sock()");
		return(-1);
	}
	
	SOCKADDR_IN si;

	si.sin_family = AF_INET;
	si.sin_port = htons(myPort);		// port
	si.sin_addr.s_addr = inet_addr(myIP.c_str());

	if ( connect(sock_,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		OutputScreenError("Error in connect()");
		closesocket(sock_);
		return(-1);
	}

	return(0);
}

int Socket::SocketClose()
{
	return closesocket(sock_);
}

std::string Socket::SocketSend(const std::string command, const std::string hash, const std::string content)
{
	if ( sock_ )
	{
		int size;
		char receivebuffer[COMM_BUFFER_SIZE];
		char receivebuffer2[COMM_BUFFER_SIZE];
		char sendbuffer[COMM_BUFFER_SIZE];


		// send the http header and the file contents to the browser
		//strcpy(sendbuffer,"GET f791c625fffe8f592767161c HTTP/1.0\r\n");
		strcpy(sendbuffer, command.c_str());
		strcat(sendbuffer," ");
		strcat(sendbuffer, hash.c_str());
		strcat(sendbuffer," VMesh/1.0\r\n");
		strncat(sendbuffer,"\r\n",COMM_BUFFER_SIZE);
		strcat(sendbuffer, content.c_str());
		send(sock_,sendbuffer,strlen(sendbuffer),0);

		size = SocketRead(receivebuffer,COMM_BUFFER_SIZE, receivebuffer2,COMM_BUFFER_SIZE);
		if ( size == SOCKET_ERROR || size == 0 )
		{
			OutputScreenError("Error calling recv()");
			closesocket(sock_);
			return "";
		}
		receivebuffer2[size] = NULL;

		std::string str = receivebuffer2;
		return str;

	}
	else
		cout << "Error in StartLookupClient()" << endl;
	return "";

}


//--------------------------------------------------------------
//	SocketRead()
//		Reads data from the client socket until it gets a valid http
//		header or the client disconnects.
//--------------------------------------------------------------
int Socket::SocketRead(char *receivebuffer, int buffersize, char *receivebuffer2, int buffersize2)
{
	int size = 0, totalsize = 0;

	do
	{
		size = recv(sock_,receivebuffer+totalsize,buffersize-totalsize,0);

		if ( strstr(receivebuffer,"\r\n\r\n") )
			break;
		
	} while ( size != 0 && size != SOCKET_ERROR );

	size = 0;
	totalsize = 0;
	do
	{
		size = recv(sock_,receivebuffer2+totalsize,buffersize2-totalsize,0);
		if ( size != 0 && size != SOCKET_ERROR )
		{
			totalsize += size;
		}
		
	} while ( size != 0 && size != SOCKET_ERROR );

	return(totalsize);
}


//--------------------------------------------------------------
//	OutputScreenError()
//		Writes an error message to the screen displays the socket
//		error code, clearing the error before exiting.
//--------------------------------------------------------------
void Socket::OutputScreenError(const char *errmsg)
{
   cout << errmsg << " - " << WSAGetLastError() << endl;
   WSASetLastError(0);
}
