#include <iostream>
#include <fstream>
#include <stdio.h>

#include "Common.h"
#include "SegmentDB.h"
#include "LogServerConfig.h"
using namespace std;

//--------------------------------------------------------------
// global vars
//--------------------------------------------------------------
char hostname[MAX_PATH];
SegmentDB* segmentdb;
CRITICAL_SECTION output_criticalsection;
boost::asio::io_service io_service;
LogServerConfig myLogServerConfig("svc.conf");

//--------------------------------------------------------------
//  main()
//		Start of application.  Initializes winsock and starts
//		server
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
	segmentdb = new SegmentDB();

	InitializeCriticalSection(&output_criticalsection);
	StartServer();
	DeleteCriticalSection(&output_criticalsection);
	delete segmentdb;

	return(0);
}

//--------------------------------------------------------------
// WaitForClientConnections()
//		Loops forever waiting for client connections. On connection
//		starts a thread to handling the http transaction
//--------------------------------------------------------------
void StartServer(void)
{
	tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), DEFAULT_PORT));
	for (;;)
	{
		socket_ptr socket_(new tcp::socket(io_service));
		acceptor_.accept(*socket_);
		boost::thread t(boost::bind(HandleHTTPRequest, socket_));
	}
}

//--------------------------------------------------------------
//	HandleHTTPRequest()
//		Executed in its own thread to handling http transaction
//--------------------------------------------------------------
void HandleHTTPRequest( socket_ptr socket_ )
{
	LookupRequestHeader requestheader;
	char income_data[COMM_BUFFER_SIZE];
	char outgoing_data[COMM_BUFFER_SIZE];
	char databuffer[COMM_BUFFER_SIZE];
	boost::system::error_code error;

	boost::asio::streambuf request;
	boost::asio::streambuf response;

	std::ostream request_stream(&request);

	std::stringstream income_stream;
	size_t reponse_length;
	int new_position = 0;
	string retQuery;
	try
	{
		do
		{
			requestheader.client_ip = socket_->remote_endpoint();
		
			boost::asio::read_until(*socket_, request, "\r\n\r\n");
			income_stream << &request;
	
			strcpy(income_data, income_stream.str().c_str());
			
			if ( !ParseLookupHeader(income_data, requestheader, outgoing_data) )
			{
				// handle bad header!
				OutputHTTPRespond(socket_, 400);   // 400 - bad request
				return;
			}

			strcpy(outgoing_data, "HTTP/1.0 200 OK\r\n");
			new_position = strlen("HTTP/1.0 200 OK\r\n");	
			
			if(strcmp(requestheader.method, "GET") == 0)
			{
				if ( NULL == strstr(requestheader.url, "ReqLogServer"))
				{
					printf("Request Client IP address\n");
					sprintf(outgoing_data + new_position, "Content-Length: %ld\r\n\r\n", requestheader.client_ip.address().to_string().length());
					strcat(outgoing_data, requestheader.client_ip.address().to_string().c_str());
					reponse_length = strlen( outgoing_data );
				}
				else
				{
					printf("Request Log Server Info\n");
					char serverInfo[LOG_SERVER_INFO_LENGTH];
					GetLogServerInfo(serverInfo);
					int i;
					sprintf( outgoing_data + new_position, "Content-Length: %ld\r\n\r\n", LOG_SERVER_INFO_LENGTH);
					new_position = strlen(outgoing_data);
					memcpy(outgoing_data + new_position, serverInfo, LOG_SERVER_INFO_LENGTH);
					reponse_length = new_position + LOG_SERVER_INFO_LENGTH;
				}
			}
			else
			{
				//for debug
				//int identity=0;
				//identity=atoi(requestheader.identity);
				//printf("########################\n");
				//printf("identity parsed as %d\n", identity);
				//printf("########################\n");
				//end for debug

				retQuery = segmentdb->Query(requestheader.method, requestheader.url, databuffer,atoi(requestheader.identity));
				sprintf(outgoing_data + new_position, "Content-Length: %ld\r\n\r\n", retQuery.length());
				new_position = strlen(outgoing_data);
				strcat(outgoing_data + new_position, retQuery.c_str());
				reponse_length = strlen( outgoing_data );
			}
 
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.
			
			// Jack's comment::send back the response base on different request [eq: nodeList(Node Lookup)]
			boost::asio::write(*socket_, boost::asio::buffer(outgoing_data, reponse_length));

		} while (0);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}

	socket_->close();
}


//--------------------------------------------------------------
//	OutputScreenError()
//		Writes an error message to the screen displays the socket
//		error code, clearing the error before exiting.
//--------------------------------------------------------------
void OutputScreenError(const char *errmsg)
{
	EnterCriticalSection (&output_criticalsection);
   cout << errmsg << " - " << WSAGetLastError() << endl;
   WSASetLastError(0);
	LeaveCriticalSection (&output_criticalsection);
}


//--------------------------------------------------------------
//	OutputHTTPRespond()
//		Sends an http header and html body to the client with
//		error information.
//--------------------------------------------------------------
void OutputHTTPRespond(socket_ptr client_socket, int statuscode)
{
	char headerbuffer[COMM_BUFFER_SIZE];
	char htmlbuffer[COMM_BUFFER_SIZE];

	sprintf(htmlbuffer,"<html><body><h2>Return Code: %d</h2></body></html>",statuscode);
	sprintf(headerbuffer,"VMesh/1.0 %d\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n",statuscode,strlen(htmlbuffer));
	
	boost::asio::write(*client_socket, boost::asio::buffer(headerbuffer, strlen(headerbuffer)));
	boost::asio::write(*client_socket, boost::asio::buffer(htmlbuffer, strlen(htmlbuffer)));

	client_socket->close();
}


//--------------------------------------------------------------
//	ParseLookupHeader()
//		Fills a LookupRequestHeader with method, url, http version
//		and file system path information.
//--------------------------------------------------------------
BOOL ParseLookupHeader(char *receivebuffer, LookupRequestHeader &requestheader, char* data)
{
	char *pos;
	// http request header format
	// method uri httpversion

	//debugging
	EnterCriticalSection (&output_criticalsection);
	//cout << "==================================" << endl;
	//cout << receivebuffer << endl;
	//cout << "==================================" << endl;
	LeaveCriticalSection (&output_criticalsection);
	// end debuggine	


	pos = strtok(receivebuffer," ");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.method,pos,SMALL_BUFFER_SIZE);
	//printf("%s checking the request header method!!!!!!", pos);
	
	pos = strtok(NULL," ");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.url,pos,MAX_PATH);
	//printf("%s checking the request header URL!!!!!!", pos);

	pos = strtok(NULL," ");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.httpversion,pos,SMALL_BUFFER_SIZE);
	//printf("%s checking the request HTTP version!!!!!!", pos);

	pos = strtok(NULL,"\r");
	if ( pos == NULL )
		return(FALSE);
	strncpy(requestheader.identity,pos,SMALL_BUFFER_SIZE);
	//printf("%s checking the identity!!!!!!\n\n", pos);

	pos = strtok(NULL,"\r");
	if ( pos == NULL )
		return(FALSE);
	//pos = strtok(NULL,"\n");
	/*if ( pos == NULL )
		return(TRUE);*/
	pos = strtok(NULL,"");
	strcpy(data,pos);

	// based on the url lets figure out the filename + path
	strncpy(requestheader.filepathname,requestheader.url,MAX_PATH);

	// because the filepathname can have relative references  ../ ./ 
	// call _fullpath to get the absolute 'real' filepath
	// _fullpath seems to handle '/' and '\' 
   _fullpath(requestheader.filepathname,requestheader.filepathname,MAX_PATH);

	return(TRUE);
}



void GetLogServerInfo(char * serverInfo)
{
	NetworkAddress addr = myLogServerConfig.GetLogServerAddress();
	int nInterval = myLogServerConfig.GetNumericConfigData("Interval");
	unsigned int ip[4];
	if ( sscanf(addr.ip_address.c_str(), "%u.%u.%u.%u", ip, ip+1, ip+2, ip+3) < 4 )
		return;
	int i;
	for ( i = 0; i < 4; i ++ )
		serverInfo[i] = ip[i];
	int port = addr.port;
	serverInfo[4] = unsigned char (port>>8 & 0xff);
	serverInfo[5] = unsigned char (port & 0xff);

	serverInfo[6] = unsigned char (nInterval>>8 & 0xff);
	serverInfo[7] = unsigned char (nInterval & 0xff);
}
