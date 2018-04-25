//==============================================================
// Charlotte - A small Windows web server 
//
// Stuart Patterson
//==============================================================


#include "Charlotte.h"

#define MAX_NUM_AD		100
//--------------------------------------------------------------
// global vars
//--------------------------------------------------------------
char wwwroot[MAX_PATH];
char hostname[MAX_PATH];
char szAdFilename[MAX_NUM_AD][MAX_PATH];
int nAdCount = 0;
int nAdNextItem = 0;
CRITICAL_SECTION output_criticalsection;
CRITICAL_SECTION listCS;
time_t	lastAccessTime = 0;

MimeAssociation mimetypes[] = { 
	{ ".txt", "text/plain" },
	{ ".html","text/html" },
	{ ".htm", "text/html" },
	{ ".gif", "image/gif" },
	{ ".jpg", "image/jpeg" }
};

void Start(const TCHAR* root, unsigned int port)
{
	srand(time(NULL));	
	SOCKET server_socket;
	strcpy(wwwroot, root);

	// get name of webserver machine. needed for redirects
	// gethostname does not return a fully qualified host name
	DetermineHost(hostname);

	// create a critical section for mutual-exclusion synchronization on cout
	InitializeCriticalSection (&output_criticalsection);
	InitializeCriticalSection(&listCS);

	// init the webserver
	server_socket = StartWebServer(port);
	if ( server_socket )
	{
		WaitForClientConnections(server_socket);
		closesocket(server_socket);
	}
	else
		//cout << "Error in StartWebServer()" << endl;
		printf("Error in StartWebServer() \n");

	// delete and release resources for critical section
	DeleteCriticalSection (&output_criticalsection);
	DeleteCriticalSection(&listCS);
}

//--------------------------------------------------------------
// WaitForClientConnections()
//		Loops forever waiting for client connections. On connection
//		starts a thread to handling the http transaction
//--------------------------------------------------------------
int WaitForClientConnections(SOCKET server_socket)
{
	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int client_address_len;
	ClientInfo *ci;

	client_address_len = sizeof(SOCKADDR_IN);

	if ( listen(server_socket,SOMAXCONN) == SOCKET_ERROR )
	{
		OutputScreenError("Error in listen()");
		closesocket(server_socket);
		return(0);
	}

	LoadAdvertisementList();

   // loop forever accepting client connections. user ctrl-c to exit!
   for ( ;; )
	{
		client_socket = accept(server_socket,(struct sockaddr *)&client_address,&client_address_len);
		if ( client_socket == INVALID_SOCKET )
		{
			OutputScreenError("Error in accept()");
			closesocket(server_socket);
			return(0);
		}

      // copy client ip and socket so the HandleHTTPRequest thread
      // and process the request.
		ci = new ClientInfo;
		ci->client_socket = client_socket;
		memcpy(&(ci->client_ip),&client_address.sin_addr.s_addr,4);

		// for each request start a new thread!
		_beginthread(HandleHTTPRequest,0,(void *)ci);
	}

}

void string_replace(string & strBig, const string & strsrc, const string &strdst) 
{
        string::size_type pos=0;
        string::size_type srclen=strsrc.size();
        string::size_type dstlen=strdst.size();
        while( (pos=strBig.find(strsrc, pos)) != string::npos)
		{
                strBig.replace(pos, srclen, strdst);
                pos += dstlen;
        }
}
//--------------------------------------------------------------
//	HandleHTTPRequest()
//		Executed in its own thread to handling http transaction
//--------------------------------------------------------------
void HandleHTTPRequest( void *data )
{

	SOCKET client_socket;
	HTTPRequestHeader requestheader;
	int size;
	char receivebuffer[COMM_BUFFER_SIZE];
	char sendbuffer[COMM_BUFFER_SIZE];

	client_socket = ((ClientInfo *)data)->client_socket;
	requestheader.client_ip = ((ClientInfo *)data)->client_ip;
	
	delete data;

	size = SocketRead(client_socket,receivebuffer,COMM_BUFFER_SIZE);
	if ( size == SOCKET_ERROR || size == 0 )
	{
		OutputScreenError("Error calling recv()");
		closesocket(client_socket);
		return;
	}
	receivebuffer[size] = NULL;

	if ( !ParseHTTPHeader(receivebuffer,requestheader) )
	{
		// handle bad header!
		OutputHTTPError(client_socket, 400);   // 400 - bad request
		return;
	}

	if ( strstr(requestheader.method,"GET") )
	{
		if ( strstr(requestheader.filepathname, "GetAdvertisement") != NULL )  // for advertisement
		{
			FILE *in;	
			char *filebuffer;
			long filesize;
			DWORD fileattrib;

			if ( ( time(0) - lastAccessTime ) > 300 || nAdCount == 0 )	// 300 seconds
				LoadAdvertisementList();

			if ( nAdCount == 0 )
			{
				OutputHTTPError(client_socket, 404);   // 404 - not found
				return;
			}					

			EnterCriticalSection(&listCS);
			char filepath[MAX_PATH];
			sprintf(filepath, "%s\\ad\\%s", wwwroot, szAdFilename[nAdNextItem]);
			nAdNextItem++;
			if ( nAdNextItem >= nAdCount )
				nAdNextItem = 0;
			LeaveCriticalSection(&listCS);
			
			in = fopen(filepath,"rb");  // read binary
			if ( !in )
			{
				LoadAdvertisementList();

				if ( nAdCount == 0 )
				{
					OutputHTTPError(client_socket, 404);   // 404 - not found
					return;
				}

				EnterCriticalSection(&listCS);
				sprintf(filepath, "%s\\ad\\%s", wwwroot, szAdFilename[nAdNextItem]);
				nAdNextItem++;
				if ( nAdNextItem >= nAdCount )
					nAdNextItem = 0;
				LeaveCriticalSection(&listCS);
				in = fopen(filepath,"rb");
				if ( !in )
				{
					OutputHTTPError(client_socket, 404);   // 404 - not found
					return;
				}
			}

			// determine file size
			fseek(in,0,SEEK_END);
			filesize = ftell(in);
			fseek(in,0,SEEK_SET);

			// allocate buffer and read in file contents
			filebuffer = new char[filesize];
			fread(filebuffer,sizeof(char),filesize,in);
			fclose(in);

			// send the http header and the file contents to the browser
			strcpy(sendbuffer,"HTTP/1.0 200 OK\r\n");
			strncat(sendbuffer,"Content-Type: ",COMM_BUFFER_SIZE);
			strncat(sendbuffer,mimetypes[findMimeType(requestheader.filepathname)].mime,COMM_BUFFER_SIZE);
			sprintf(sendbuffer+strlen(sendbuffer),"\r\nContent-Length: %ld\r\n",filesize);
			strncat(sendbuffer,"\r\n",COMM_BUFFER_SIZE);

			send(client_socket,sendbuffer,strlen(sendbuffer),0);
			send(client_socket,filebuffer,filesize,0);

			// log line
			EnterCriticalSection (&output_criticalsection);
			//  cout << inet_ntoa(requestheader.client_ip) << " - " << requestheader.method << " " << requestheader.url << endl;
			printf("%s - %s %s \n", inet_ntoa(requestheader.client_ip), requestheader.method, requestheader.url);
			LeaveCriticalSection (&output_criticalsection);

			delete [] filebuffer;
		}
		else if ( strnicmp(requestheader.filepathname,wwwroot,strlen(wwwroot)) == 0 )  // for getting server playlist
		{
			FILE *in;	
			char *filebuffer;
			long filesize;
			DWORD fileattrib;

			fileattrib = GetFileAttributes(requestheader.filepathname);

			if ( fileattrib != -1 && fileattrib & FILE_ATTRIBUTE_DIRECTORY )
			{
				OutputHTTPRedirect(client_socket, requestheader.url);
				return;
			}
			
			string filepath = requestheader.filepathname;
			filepath.insert(filepath.find_last_of("\\"), "\\Playlist");
			string_replace(filepath, "%20", " ");
			printf("%s\n", filepath.c_str());
			in = fopen(filepath.c_str(),"rb");  // read binary
			if ( !in )
			{
				// file error, not found?
				OutputHTTPError(client_socket, 404);   // 404 - not found
				return;
			}

			// determine file size
			fseek(in,0,SEEK_END);
			filesize = ftell(in);
			fseek(in,0,SEEK_SET);

			// allocate buffer and read in file contents
			filebuffer = new char[filesize];
			fread(filebuffer,sizeof(char),filesize,in);
			fclose(in);

			// send the http header and the file contents to the browser
			strcpy(sendbuffer,"HTTP/1.0 200 OK\r\n");
			strncat(sendbuffer,"Content-Type: ",COMM_BUFFER_SIZE);
			strncat(sendbuffer,mimetypes[findMimeType(requestheader.filepathname)].mime,COMM_BUFFER_SIZE);
			sprintf(sendbuffer+strlen(sendbuffer),"\r\nContent-Length: %ld\r\n",filesize);
			strncat(sendbuffer,"\r\n",COMM_BUFFER_SIZE);

			send(client_socket,sendbuffer,strlen(sendbuffer),0);
			send(client_socket,filebuffer,filesize,0);

			// log line
			EnterCriticalSection (&output_criticalsection);
			//  cout << inet_ntoa(requestheader.client_ip) << " - " << requestheader.method << " " << requestheader.url << endl;
			printf("%s - %s %s \n", inet_ntoa(requestheader.client_ip), requestheader.method, requestheader.url);
			LeaveCriticalSection (&output_criticalsection);

			delete [] filebuffer;
		} else // security violation
		{
			OutputHTTPError(client_socket, 403);	// 403 - forbidden
			return;
		}
	}
	else
	{
		OutputHTTPError(client_socket, 501);   // 501 not implemented
		return;
	}

	closesocket(client_socket);
}


//--------------------------------------------------------------
//	SocketRead()
//		Reads data from the client socket until it gets a valid http
//		header or the client disconnects.
//--------------------------------------------------------------
int SocketRead(SOCKET client_socket, char *receivebuffer, int buffersize)
{
	int size = 0, totalsize = 0;

	do
	{
		size = recv(client_socket,receivebuffer+totalsize,buffersize-totalsize,0);
		if ( size != 0 && size != SOCKET_ERROR )
		{
			totalsize += size;

			// are we done reading the http header?
			if ( strstr(receivebuffer,"\r\n\r\n") )
				break;
		}
		else
			totalsize = size;			// remember error state for return
		
	} while ( size != 0 && size != SOCKET_ERROR );

	return(totalsize);
}


//--------------------------------------------------------------
//	OutputScreenError()
//		Writes an error message to the screen displays the socket
//		error code, clearing the error before exiting.
//--------------------------------------------------------------
void OutputScreenError(const char *errmsg)
{
	EnterCriticalSection (&output_criticalsection);
	//cout << errmsg << " - " << WSAGetLastError() << endl;
	printf("%s - %d \n", errmsg, WSAGetLastError());
	WSASetLastError(0);
	LeaveCriticalSection (&output_criticalsection);
}


//--------------------------------------------------------------
//	OutputHTTPError()
//		Sends an http header and html body to the client with
//		error information.
//--------------------------------------------------------------
void OutputHTTPError(SOCKET client_socket, int statuscode)
{
	char headerbuffer[COMM_BUFFER_SIZE];
	char htmlbuffer[COMM_BUFFER_SIZE];

	sprintf(htmlbuffer,"<html><body><h2>Error: %d</h2></body></html>",statuscode);
	sprintf(headerbuffer,"HTTP/1.0 %d\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n",statuscode,strlen(htmlbuffer));
	
	send(client_socket,headerbuffer,strlen(headerbuffer),0);
	send(client_socket,htmlbuffer,strlen(htmlbuffer),0);

	closesocket(client_socket);
}


//--------------------------------------------------------------
//	OutputHTTPRedirect()
//		Writes an HTTP redirect header and body to the client.
//		Called if the user requests a directory causing the redirect
//		to directory/index.html 
//--------------------------------------------------------------
void OutputHTTPRedirect(SOCKET client_socket, const char *defaulturl)
{
	char headerbuffer[COMM_BUFFER_SIZE];
	char htmlbuffer[COMM_BUFFER_SIZE];
	char hosturl[MAX_PATH];

	sprintf(hosturl,"http://%s",hostname);
	strncat(hosturl,defaulturl,COMM_BUFFER_SIZE);

	if ( hosturl[strlen(hosturl)-1] != '/' )
		strncat(hosturl,"/",MAX_PATH);										
	strncat(hosturl,"index.html",MAX_PATH);

	sprintf(htmlbuffer,"<html><body><a href=\"%s\">%s</a></body></html>",hosturl,hosturl);
	sprintf(headerbuffer,"HTTP/1.0 301\r\nContent-Type: text/html\r\nContent-Length: %ld\r\nLocation: %s\r\n\r\n",strlen(htmlbuffer),hosturl);
	
	send(client_socket,headerbuffer,strlen(headerbuffer),0);
	send(client_socket,htmlbuffer,strlen(htmlbuffer),0);

	closesocket(client_socket);
}

//--------------------------------------------------------------
//	ParseHTTPHeader()
//		Fills a HTTPRequestHeader with method, url, http version
//		and file system path information.
//--------------------------------------------------------------
BOOL ParseHTTPHeader(char *receivebuffer, HTTPRequestHeader &requestheader)
{
	char *pos;
	// http request header format
	// method uri httpversion

	//debugging
	EnterCriticalSection (&output_criticalsection);
	// cout << receivebuffer << endl;
	printf("%s \n", receivebuffer);
	LeaveCriticalSection (&output_criticalsection);
	// end debugging	

	pos = strtok(receivebuffer," ");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.method,pos,SMALL_BUFFER_SIZE);
	
	pos = strtok(NULL," ");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.url,pos,MAX_PATH);

	pos = strtok(NULL,"\r");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.httpversion,pos,SMALL_BUFFER_SIZE);

	// based on the url lets figure out the filename + path
	strncpy(requestheader.filepathname,wwwroot,MAX_PATH);
	strncat(requestheader.filepathname,requestheader.url,MAX_PATH);

	// because the filepathname can have relative references  ../ ./ 
	// call _fullpath to get the absolute 'real' filepath
	// _fullpath seems to handle '/' and '\' 
	_fullpath(requestheader.filepathname,requestheader.filepathname,MAX_PATH);

	return(TRUE);
}


//--------------------------------------------------------------
//	StartWebServer()
//		Creates server sock and binds to ip address and port
//--------------------------------------------------------------
SOCKET StartWebServer(unsigned int port)
{
	SOCKET s;

	s = socket(AF_INET,SOCK_STREAM,0);
	if ( s == INVALID_SOCKET )
	{
		OutputScreenError("Error creating sock()");
		return(0);
	}
	
	SOCKADDR_IN si;

	si.sin_family = AF_INET;
	si.sin_port = htons(port);		// port
	si.sin_addr.s_addr = htonl(INADDR_ANY);

	if ( bind(s,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		OutputScreenError("Error in bind()");
		closesocket(s);
		return(0);
	}

	return(s);
}

//--------------------------------------------------------------
//	findMimeType()
//		Performs linear search through mimetypes array looking for
//		matching file extension returning index of mime type 
//--------------------------------------------------------------
int findMimeType(const char *filename)
{
	char *pos;
	int numofelements;
	
	pos = strrchr((char *)filename,'.');

	if ( pos )
	{
		numofelements = sizeof(mimetypes) / sizeof(MimeAssociation);

		for ( int x = 0; x < numofelements; ++x )
		{
			if ( stricmp(mimetypes[x].file_ext,pos) == 0 )
				return(x);
		}
	}

	return(0);  // return default mimetype  'text/plain' 
}

//--------------------------------------------------------------
//	DetermineHost()
//		If webserver needs to redirect user from directory to 
//		default html file the server builds a full url and hence
//		needs it's full domain name for http address.
//			http://mymachine.rollins.brevard.edu/index.html
//--------------------------------------------------------------
void DetermineHost( char *hostname )
{
	IN_ADDR in;
	hostent *h;

	gethostname(hostname, MAX_PATH);
	h = gethostbyname(hostname);
	memcpy(&in,h->h_addr, 4);
	h = gethostbyaddr((char *)&in, 4, PF_INET);
	strcpy(hostname,h->h_name);
}

// deal with advertisment
void LoadAdvertisementList()
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char szPath[MAX_PATH];

	sprintf(szPath, "%s\\ad\\*.wmv", wwwroot);
	EnterCriticalSection(&listCS);
	hFind = FindFirstFile(szPath, &FindFileData);
	nAdCount = 0;

	if (INVALID_HANDLE_VALUE == hFind)
	{
		LeaveCriticalSection(&listCS);
		return;
	}

	strcpy(szAdFilename[0], FindFileData.cFileName);

	printf("Begin of advertisement search\n File %02d: %s \n", nAdCount + 1, szAdFilename[nAdCount]);

	nAdCount++;

	while ( FindNextFile(hFind, &FindFileData) != 0 && nAdCount < MAX_NUM_AD )
	{
		strcpy(szAdFilename[nAdCount], FindFileData.cFileName);
		printf(" File %02d: %s \n", nAdCount + 1, szAdFilename[nAdCount]);
		nAdCount++;
	}

	printf("End of advertisement search\n");
	nAdNextItem = rand() % nAdCount;

	LeaveCriticalSection(&listCS);

	lastAccessTime = time(0);

	FindClose(hFind);
}
