#ifndef VMESH_CONTROLLER_H
#define VMESH_CONTROLLER_H
//===========================================================================
// VMesh Controller - A VMesh client engine to receive packet and send as a stream
//					  Based on
//					  Charlotte
//
// This is the core part of the mesh based system which handles 
//		1. Overlay Management
//		2. Data Request Management
//
// Charlotte is a small Windows web server 
//
// Stuart Patterson
//===========================================================================
//#include <windows.h>
//#include <process.h>
#include "winsock2.h"
#include "Mstcpip.h"
#include "..\Common\natupnp.h"
#include <AtlBase.h>
#include <AtlConv.h>
#include <urlmon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "..\Common\Common.h"
#include "../../VMesh/Interface/VMeshClient.h"

//Add by jack		[05-08-2010]
//For modify the VMeshController to VMeshControllerThread using Thread.h
#include "../../VMesh/Util/Thread.h"

using std::string;

_USING_VMESH_NAMESPACE
//--------------------------------------------------------------
// manifest constants
//--------------------------------------------------------------
//#define COMM_BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 10
#define STOP_COMMAND	1000
#define RESTART_COMMAND 1001
#define TIME_ZONE +8
#define LOG_SERVER_CHECKING_INTERVAL	300		// 300 seconds = 5 minutes
//#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)
//--------------------------------------------------------------
// structures
//--------------------------------------------------------------

	struct HTTPRequestHeader
	{
		char method[SMALL_BUFFER_SIZE];
		char url[MAX_PATH];
		char filepathname[MAX_PATH];
		char httpversion[SMALL_BUFFER_SIZE];
	   IN_ADDR client_ip;
	};


	struct __TCP_KEEPALIVE {
		u_long onoff;
		u_long keepalivetime;
		u_long keepaliveinterval;
	};

	struct ControllerInfo{
		 char *hash;
		 char *port;
		 int dataport;
		 char *server;
		 int numSegment;
	};

	struct ClientInfo
	{
		SOCKET server_socket;
		SOCKET client_socket;
		IN_ADDR client_ip;
		int identity;		// Proxy=0, Client=1 [add by Jack 14/12/2010]
	};


//--------------------------------------------------------------
	// prototypes
	//--------------------------------------------------------------
	// start the server
	SOCKET StartWebServer();
	unsigned __stdcall StartVMeshContoller(void *data);
	// wait for connections
	int WaitForClientConnections(SOCKET server_socket, int num_of_segments = 0);

	unsigned __stdcall UpdateLogServerInfo( void *data );
	unsigned __stdcall HandleHTTPRequest( void *data );
	int findMimeType(const char *extension);
	BOOL ParseHTTPHeader(char *receivebuffer, HTTPRequestHeader &requestheader);

	// error handling
	void OutputHTTPError(SOCKET client_socket, int statuscode );
	void OutputHTTPRedirect(SOCKET client_socket, const char *defaulturl);
	

	// send and recv msg
	int SocketWrite(SOCKET client_socket, char *sendbuffer, int buffersize);
	int SocketRead(SOCKET client_socket, char *receivebuffer, int buffersize);
	void DetermineHost( char *hostname );

	// command handling
	int ParseSetTime(string str);
	string ParseCommand(string str);
	string ParseMovieHash(string str);
	bool ParseStopCommand(string str);
	bool ParseRestartCommand(string str);
	string ParseServerAddress(string str);
	int ParsePortNumber(string str);
	
	// logging
	void ThreadLog(int thread_no, const char* s);
	void donothing(void *data);

	void GetLoggingInfo(const char* filepath, string& server, int& port, int& interval);
	void OutputScreenError(const char *errmsg);
	bool CheckIsPortAvailable(int port);
	void readConfigFile();



class VMeshControllerThread : public Thread
{
public:
	VMeshControllerThread(void* receiveData);
	~VMeshControllerThread();

	void* execute();
	void stop();

private:
	void* data;

};
#endif