#pragma once

#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <ctime>

using namespace std;

//--------------------------------------------------------------
// manifest constants
//--------------------------------------------------------------
#define COMM_BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 10

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

struct ClientInfo
{
	SOCKET client_socket;
	IN_ADDR client_ip;
};

struct MimeAssociation
{
	char *file_ext;
	char *mime;
};

//--------------------------------------------------------------
// prototypes
//--------------------------------------------------------------
void Start(const TCHAR* root, unsigned int port);
SOCKET StartWebServer(unsigned int port);
int WaitForClientConnections(SOCKET server_socket);
void HandleHTTPRequest( void *data );
int findMimeType(const char *extension);
BOOL ParseHTTPHeader(char *receivebuffer, HTTPRequestHeader &requestheader);
void OutputHTTPError(SOCKET client_socket, int statuscode );
void OutputHTTPRedirect(SOCKET client_socket, const char *defaulturl);
void OutputScreenError(const char *errmsg);
int SocketRead(SOCKET client_socket, char *receivebuffer, int buffersize);
void DetermineHost( char *hostname );
void LoadAdvertisementList();
