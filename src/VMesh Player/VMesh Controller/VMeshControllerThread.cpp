#include "VMeshControllerThread.h"
#include "VMeshControllerGlobal.h"


	struct MimeAssociation
	{
		char *file_ext;
		char *mime;
	};

	// html association
	MimeAssociation mimetypes[] = { 
		{ ".txt", "text/plain" },
		{ ".html","text/html" },
		{ ".htm", "text/html" },
		{ ".gif", "image/gif" },
		{ ".jpg", "image/jpeg" }
	};

	// Jack's comment:: temp code -- get number of proxy exchange neighbor and client exchange neighbor
	//					from config file
	int clientExchangeNeighbor;

VMeshControllerThread::VMeshControllerThread(void* receiveData)
{
	data = receiveData;
}

VMeshControllerThread::~VMeshControllerThread()
{
}

void VMeshControllerThread::stop()
{
	this->stop();
	this->join();
}

void GetLoggingInfo(const char* filepath, string& server, int& port, int& interval)
{
	server = "127.0.0.1";
	port = 0;
	interval = 60;
	FILE* file = fopen(filepath, "r");
	if ( file == NULL )
		return;

	char data[8];
	char ipstring[256];
	fread(data, 8, 1, file);

	sprintf(ipstring, "%u.%u.%u.%u", unsigned char(data[0]) & 0xff, unsigned char(data[1]) & 0xff, unsigned char(data[2]) & 0xff, unsigned char(data[3]) & 0xff);
	server = ipstring;
	port =  ((unsigned char(data[4]) & 0xff ) << 8 )  | (unsigned char(data[5]) & 0xff);
	interval = ((unsigned char(data[6]) & 0xff ) << 8 )  | (unsigned char(data[7]) & 0xff);

	fclose(file);
}

void readConfigFile()
{
	char* temp_result;
	string string_result;
	char* option , *value;

	ifstream config_file;
	config_file.open("vmesh.conf");
	getline(config_file, string_result, ' ');
	cout << string_result << endl;
	
	temp_result = new char[string_result.size()+1];

	strcpy(temp_result, string_result.c_str());
	
	// read hash from config
	option = strtok(temp_result, "=");

	while(option != NULL)
	{
		if (strcmp(option, "hash") == 0)
		{
			value = strtok(NULL, "\r\n");
		}
		else if (strcmp(option, "port") == 0)
		{
			value = strtok(NULL, "\r\n");
		}
		else if (strcmp(option, "listen_port") == 0)
		{
			value = strtok(NULL, "\r\n");
		}
		else if (strcmp(option, "server") == 0)
		{
			value = strtok(NULL, "\r\n");
		}
		else if (strcmp(option, "ProxyExchangeNeighbor") == 0)
		{
			value = strtok(NULL, "\r\n");
		}
		else if (strcmp(option, "ClientExchangeNeighbor") == 0)
		{
			value = strtok(NULL, "\r\n");
			if (value != NULL)
				clientExchangeNeighbor = atoi(value);
		}

		option = strtok(NULL, "=");
	}

	config_file.close();

	cout<<"ClientExchangeNeighbor is"<<clientExchangeNeighbor<<endl;
}

//--------------------------------------------------------------
//  main()
//		Start of application.  Initializes winsock and starts
//		server
//--------------------------------------------------------------
//unsigned __stdcall StartVMeshContoller(void *data)
void* VMeshControllerThread::execute()
{
	// Jack's comment:: temp code -- read number of proxy exchange neighor and client exchange neighor from config file
	readConfigFile();

	int numOfSegments;
#ifdef CENTRAL_LOOKUP
	printf("VMesh Controller Started (Central lookup manager option) \n");
#endif

#ifdef CHORD_LOOKUP
	printf("VMesh Controller Started (DHT chord option) \n");
#endif

#ifdef TABLE_EXCH_LOOKUP
	printf("VMesh Controller Started (Decentralized Table Exchange Lookup Option) \n");
#endif

	cout << "VMeshControllerThread" << endl;

	srand(time(NULL));
	WSADATA wd;
	SOCKET server_socket;
	packetSize = 0;
	m_Thread = NULL; 
	
	listen_port = DATA_PORT;
	char strTemp[100];
	
	//hash = argv[0+startingParameter];
	hash = ((ControllerInfo *)data)->hash;

	std::stringstream ss(((ControllerInfo *)data)->port);
	ss >> port;

	//std::stringstream ss2(argv[2+startingParameter]);
	//ss2 >> listen_port;
	listen_port = ((ControllerInfo *)data)->dataport;

	server = ((ControllerInfo *)data)->server;

	std::stringstream ss1(((ControllerInfo *)data)->dataport);

	 string string_temp =  ss1.str();

	/////////////////////// Getting log server info from central lookup server ///////////////////////
	/////////////////////// End of getting log server info  //////////////////////////////////////////////

	//hUpdateLogServerInfoThread = (HANDLE)_beginthreadex(NULL,0,&UpdateLogServerInfo, (void *)&server,0,&threadLogger);

	Logging::getInstance("VMesh_Controller")->write_msg(8, "VMesh Controller Started");
	strcpy(strTemp, "Ready");
	strcat(strTemp, string_temp.c_str());
	hReady = CreateEvent(NULL, TRUE, false, strTemp);

	hFailed = CreateEvent(NULL, TRUE, false, "VMesh Failed");

	strcpy(strTemp, "No Packet");
	strcat(strTemp, string_temp.c_str());
	hNoPacket = CreateEvent(NULL, TRUE, false, strTemp);

	strcpy(strTemp, "Release Mutex");
	strcat(strTemp, string_temp.c_str());
	hReleaseMutex = CreateEvent(NULL, TRUE, false, strTemp);

	if ( WSAStartup(MAKEWORD(1,1), &wd) != 0 )
	{
		DebugLog("Unable to initialize WinSock 1.1 \n");

		_endthreadex(0);
		return NULL;
	}

	// get name of webserver machine. needed for redirects
    // gethostname does not return a fully qualified host name
	DetermineHost(hostname);

	// create a critical section for mutual-exclusion synchronization on cout
	InitializeCriticalSection (&output_criticalsection);
	InitializeCriticalSection (&vmclient_criticalsection);
	InitializeCriticalSection (&thread_criticalsection);
	
	// init the webserver
	firstTime = true;
	server_socket = StartWebServer();

	numOfSegments = ((ControllerInfo *)data)->numSegment;
	cout<<"hash is"<<hash<<endl;
	cout<<"port is"<<port<<endl;
	cout<<"m_DataPort"<<listen_port<<endl;
	cout<<"server is"<<server<<endl;
	cout<<"numSeg is"<<numOfSegments<<endl;
	int status = 0;
	int i = 0;

	if ( server_socket )
	{
		status = WaitForClientConnections(server_socket, numOfSegments);
		closesocket(server_socket);
	}
	else
		DebugLog("Error in StartWebServer() \n");
	is_ControllerRun = 0;
	// delete and release resources for critical section
	DeleteCriticalSection (&output_criticalsection);
	DeleteCriticalSection (&vmclient_criticalsection);
	DeleteCriticalSection (&thread_criticalsection);
	WSACleanup();

	if (status < 0)
		SetEvent(hFailed);
	
	_endthreadex( 0 );

	return NULL;
}

//--------------------------------------------------------------
// WaitForClientConnections()
//		Loops forever waiting for client connections. On connection
//		starts a thread to handling the http transaction
//--------------------------------------------------------------
int WaitForClientConnections(SOCKET server_socket, int num_of_segments)
{
	cout<<"WaitForClientConnections";
	USES_CONVERSION;
	
	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int client_address_len;
	ClientInfo *ci;
	char msg[100];
	client_address_len = sizeof(SOCKADDR_IN);
	if ( listen(server_socket,SOMAXCONN) == SOCKET_ERROR )
	{
		OutputScreenError("Error in listen()");
		closesocket(server_socket);
		return(0);
	}
	
	if ( ! SetEvent(hReleaseMutex) )
		OutputScreenError("Cannot Set Event: Release Mutex");
	hCommand = CreateEvent(0, false, false, 0);
	command = 0;
   
	// loop forever accepting client connections. user ctrl-c to exit!
	// this instance do it only once...
	do
	{
		int available_port = 5000 + (rand() % 1000);
		while ( available_port < 65535 && CheckIsPortAvailable(available_port) == false )
			available_port++;
		if ( available_port == 65535 )
		{
			available_port = 1;
			while ( available_port < 5000 && CheckIsPortAvailable(available_port) == false )
				available_port++;
			if ( available_port == 5000 )
			{
				printf("No available port\n");
				system("pause");
				exit(0);
			}
		}

		string szExternalIPAddress = getHostIP();
		bool bTryAlternativelyMethod = false;
		bool bVMeshPortMappingSuccess = true;
		bool bDHTPortMappingSuccess = true;

		//////////////////////////// Start of UPnP code /////////////////////////////////////////////////////////
		// Get external IP
		// 1st: Try UPnP service
		CoInitialize(NULL);
		IUPnPNAT* pUPnPNAT;
		IStaticPortMappingCollection * pSPMC = NULL;
		IStaticPortMapping * pSPM = NULL;
		char msg[100];
		HRESULT hr = CoCreateInstance (__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void**)&pUPnPNAT);

		if(SUCCEEDED(hr))
		{
			hr = pUPnPNAT->get_StaticPortMappingCollection (&pSPMC);

			if(SUCCEEDED(hr) && pSPMC)
			{
				hr = pSPMC->Add(available_port, A2BSTR("TCP"), available_port, A2BSTR(getHostIP().c_str()), VARIANT_TRUE, A2BSTR("VMesh Port"), &pSPM );
				if( SUCCEEDED(hr) && pSPM) 
					DebugLog("VMesh port mapping successes\n");
				else
				{
					sprintf(msg, "VMesh port mapping fails, error code: %x\n", hr);
					DebugLog(msg);
					bVMeshPortMappingSuccess = false;
					bTryAlternativelyMethod = true;
				}
				
				#ifdef CHORD_LOOKUP
					hr = pSPMC->Add(available_port+1, A2BSTR("TCP"), available_port+1, A2BSTR(getHostIP().c_str()), VARIANT_TRUE, A2BSTR("DHT Port"), &pSPM );
					if( SUCCEEDED(hr) && pSPM) 
						DebugLog("DHT port mapping successes\n");
					else
					{
						sprintf(msg, "DHT port mapping fails, error code: %x\n", hr);
						DebugLog(msg);
						bDHTPortMappingSuccess = false;
						bTryAlternativelyMethod = true;
					}
				#endif

				if( pSPM && !bTryAlternativelyMethod )
				{
					BSTR bstrExternalIP;
					hr = pSPM->get_ExternalIPAddress(&bstrExternalIP);
					DebugLog("Finished getting external IP from UPnP-enabled device\n");
					if ( SUCCEEDED(hr) )
						szExternalIPAddress = OLE2A(bstrExternalIP);

					SysFreeString(bstrExternalIP);
				}
				else
					bTryAlternativelyMethod = true;
			}

			// 2nd: retrieved from server
			else
			{
				bTryAlternativelyMethod = true;
				DebugLog("No UPnP device detected\n");
			}
			if ( pSPMC)
				pSPMC->Release();
			if ( pSPM )
				pSPM->Release();
			pUPnPNAT->Release();
		}
		else
			bTryAlternativelyMethod = true;
		
		if ( bTryAlternativelyMethod )
		{
			char pathToFile[MAX_PATH];
			char ip[MAX_PATH];
			char url[MAX_PATH];
			sprintf(url, "http://%s:10080/", server.c_str());
			URLDownloadToCacheFile(NULL, _T(url), pathToFile, sizeof(pathToFile), 0, 0);
			FILE* cacheFile;
			cacheFile = fopen(pathToFile, "r");
			if ( cacheFile != NULL )
			{
				fscanf(cacheFile, "%s", ip);
				if ( ip[0] <= '9' && ip[0] >= '0' )
				{
					szExternalIPAddress = ip;
					DebugLog("Finished getting external IP from server\n");
				}
			}
		}
		//////////////////////////////// End of UPnP code //////////////////////////////////////////////////////////
		is_ControllerRun = 1;  //Add by Juan

		//VMeshClient vmeshClient(szExternalIPAddress, available_port, server, port, num_of_segments, 2);
		VMeshClient vmeshClient(szExternalIPAddress, available_port, server, port, num_of_segments, clientExchangeNeighbor);
		vmClient = &vmeshClient;
				
		#ifdef CENTRAL_LOOKUP 
			sprintf(msg, "External IP Address: %s \nData Port to Player:\t %d\nVMesh ports:\t\t %d\n",szExternalIPAddress.c_str(), listen_port, available_port);
		#endif
		#ifdef CHORD_LOOKUP
			sprintf(msg, "External IP Address: %s \nData Port to Player:\t %d\nVMesh ports:\t\t %d & %d\n",szExternalIPAddress.c_str(), listen_port, available_port, available_port+1);
		#endif
		#ifdef TABLE_EXCH_LOOKUP
			sprintf(msg, "External IP Address: %s \nData Port to Player:\t %d\nVMesh ports:\t\t %d & %d\n",szExternalIPAddress.c_str(), listen_port, available_port, available_port+1);
		#endif

		DebugLog(msg);

		if (vmClient->movieJoin(hash.c_str()))
			SetEvent(hReady);
		else
		{
			vmeshClient.stop();

			hr = CoCreateInstance (__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void**)&pUPnPNAT);

			if(SUCCEEDED(hr))
			{
				hr = pUPnPNAT->get_StaticPortMappingCollection (&pSPMC);

				if(SUCCEEDED(hr) && pSPMC)
				{
					if ( bVMeshPortMappingSuccess )
						pSPMC->Remove(available_port, CComBSTR("TCP"));
					if ( bDHTPortMappingSuccess )
						pSPMC->Remove(available_port+1, CComBSTR("TCP"));
				}
				if ( pSPMC )
					pSPMC->Release();
				pUPnPNAT->Release();
			}
			CoUninitialize();
			return -1;
		}

		CoUninitialize();
		
		cout<<"handle incoming request";
		// handle incoming request
		is_ControllerRun = 2; //Add by Juan
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
			ci->server_socket = server_socket;
			ci->client_socket = client_socket;
			memcpy(&(ci->client_ip),&client_address.sin_addr.s_addr,4);
			ci->identity = 1;		// Client indentity

			// for each request start a new thread!
			cout<<"handlehttprequest"<<endl;
			m_Thread = (HANDLE)_beginthreadex(NULL,0,&HandleHTTPRequest,(void *)ci,0,&threadHandle);
		}
		closesocket(client_socket);
		vmeshClient.stop();
	} while (false);
	
}

unsigned __stdcall UpdateLogServerInfo( void *data)
{
	cout<<"start UpdateLogServerInfo"<<endl;
	string server = *((string *)data);
	char pathToFile[256];
	char urlToLookupServer[256];
	string logServerIPAddress;
	int logServerPort;
	int interval;

	sprintf(urlToLookupServer, "http://%s:10080/?ReqLogServer", server.c_str());

	HRESULT hr;
	do 
	{
		hr = URLDownloadToCacheFile(NULL, urlToLookupServer, pathToFile, sizeof(pathToFile), 0, 0);

		if ( hr == S_OK )
		{
			GetLoggingInfo(pathToFile, logServerIPAddress, logServerPort, interval);
			remove(pathToFile);

			if ( interval > 0 && logServerPort > 0 )
			{
				Logging::setLoggingInterval(interval);
				Logging::getInstance("VMesh_Controller")->setLoggingServer(logServerIPAddress, logServerPort);
				printf("Log server info: %s on port %d \t Interval: %d\n", logServerIPAddress.c_str(), logServerPort, interval);
			}
		}
		else
			Sleep( 1000 );	
	} while ( hr != S_OK );
	

	while ( 1 )
	{
		Sleep( LOG_SERVER_CHECKING_INTERVAL * 1000 );		// 5 minutes
		hr = URLDownloadToCacheFile(NULL, urlToLookupServer, pathToFile, sizeof(pathToFile), 0, 0);

		if ( hr == S_OK )
		{
			GetLoggingInfo(pathToFile, logServerIPAddress, logServerPort, interval);
			remove(pathToFile);

			if ( logServerPort > 0 )
			{
				Logging::getInstance("VMesh_Controller")->setLoggingServer(logServerIPAddress, logServerPort);
				printf("Updated log server info: %s on port %d \n", logServerIPAddress.c_str(), logServerPort, interval);
			}
		}
	}
	_endthreadex(0);
	return 0;

}

//--------------------------------------------------------------
//	HandleHTTPRequest()
//		Executed in its own thread to handling http transaction
//--------------------------------------------------------------
unsigned __stdcall HandleHTTPRequest( void *data )
{
	EnterCriticalSection(&thread_criticalsection);
	thread_count++;
	if (thread_count > 99)
		thread_count = 0;
	int thread_no = thread_count;
	ThreadLog(thread_no, "Connection Accepted\n");
	LeaveCriticalSection(&thread_criticalsection);
	
	//int request_identity;
	SOCKET server_socket;
	SOCKET client_socket;
	HTTPRequestHeader requestheader;
	int size;
	char receivebuffer[COMM_BUFFER_SIZE];
	char sendbuffer[COMM_BUFFER_SIZE];
	char msg[100];
	int packetSeq = -1;
	do
	{	
		server_socket = ((ClientInfo *)data)->server_socket;
		client_socket = ((ClientInfo *)data)->client_socket;
		requestheader.client_ip = ((ClientInfo *)data)->client_ip;
		//request_identity = ((ClientInfo *)data)->identity; // For checking the identity of request  node

		size = SocketRead(client_socket,receivebuffer,COMM_BUFFER_SIZE);  
		if ( size == SOCKET_ERROR || size == 0 )
		{
			sprintf(msg, "Thread %02d: Error calling recv()", thread_no);
			OutputScreenError(msg);
			delete data;
			closesocket(client_socket);
			_endthreadex(0);
			return 0;
		}
		receivebuffer[size] = NULL;
		//ThreadLog(thread_no, "HTTP Request Message:\n");
		if ( !ParseHTTPHeader(receivebuffer,requestheader) )
		{
			// handle bad header!
			OutputHTTPError(client_socket, 400);   // 400 - bad request
			sprintf(msg, "Thread %02d: Error 400: %s\n", thread_no, receivebuffer);
			ThreadLog(thread_no, msg);
			delete data;
			_endthreadex(0);
			return 0;
		}
		if ( strstr(requestheader.method,"GET") )
		{
			
			string url(requestheader.url);	

			sprintf(msg, "Content-Length: %d\r\n", vmClient->getMediaInfo(hash.c_str())->getFileSize());

			char httpOutput[1000];
			strcpy(httpOutput, "HTTP/1.1 200 OK\r\n");
			
//////////////////////////////////////////////////////////////////////////////////////////

			strcat(httpOutput, "Pragma: packet-pair-experiment=1, no-cache, pipeline-experiment=1, client-id=1415253077, xResetStrm=1, features=\"seekable,stridable\", timeout=60000\r\n");
			strcat(httpOutput, "Content-Type: application/vnd.ms.wms-hdr.asfv1\r\n");
			//strcat(httpOutput, "Content-Type: application/x-mms-framed\r\n");
			//strcat(httpOutput, "Content-Type: application/octet-stream\r\n");
			//strcat(httpOutput, "Server: Cougar/9.00.00.3380\r\n");
			//strcat(httpOutput, msg);

			//strcat(httpOutput, "Date: Thu, 27 Apr 2006 07:41:00 GMT\r\n");
			strcat(httpOutput, "Cache-Control: no-cache\r\n");
			//strcat(httpOutput, "Last-Modified: Tue, 18 Apr 2006 17:28:03 GMT\r\n");
			//strcat(httpOutput, "Transfer-Encoding: chunked\r\n");
			strcat(httpOutput, "Supported: com.microsoft.wm.srvppair, com.microsoft.wm.sswitch, com.microsoft.wm.predstrm, com.microsoft.wm.fastcache\r\n");

//////////////////////////////////////////////////////////////////////////////////////////
			strcat(httpOutput, "\r\n");

			//DebugLog(httpOutput);
			
			if ( -1 == SocketWrite(client_socket, httpOutput, strlen(httpOutput)))
				ThreadLog(thread_no, "Send http response error");

			command = 0;

			int settime = ParseSetTime(url);
			string clientCommand = ParseCommand(url);

			if (clientCommand == "terminate")
			{
				printf("About to terminate now\n");
				OutputHTTPError(client_socket, 200);
				// Call to clean up the Resources created in VMesh
				vmClient->stop();
				delete data;
				closesocket(client_socket);
				closesocket(server_socket);
				printf("Terminated successfully\n");
				exit(0);
				//break;
			}

			if (vmClient->getMediaInfo(hash.c_str())->getMediaLength(AUDIO_STREAM)/1000 <= settime)
			{
				ThreadLog(thread_no, "Set time error\n");
				break;
			}
			
			EnterCriticalSection (&vmclient_criticalsection);
			ThreadLog(thread_no, "VMesh Client Locked\n");

			time_t curTime;
			time(&curTime);
			int hh = (curTime / 3600 + TIME_ZONE) % 24;
			int mm =  (curTime / 60) % 60;
			int ss = curTime % 60;
			sprintf(msg, "Set Time = %d \t Current Time = %02d:%02d:%02d \n", settime, hh, mm, ss);
			ThreadLog(thread_no, msg);

			vmClient->setTime(settime);

			char* appData;
			int appLen = vmClient->getMediaInfo(hash.c_str())->getAppData((const char*&)appData);
			appLen = appLen - 8;
			time(&curTime);
			hh = (curTime / 3600 + TIME_ZONE) % 24;
			mm =  (curTime / 60) % 60;
			ss = curTime % 60;
			//sprintf(msg, "Before send header \t Current Time = %02d:%02d:%02d \n", hh, mm, ss);
			//ThreadLog(thread_no, msg);

			if ( -1 == SocketWrite(client_socket , appData +8 , appLen) )
			{
				ThreadLog(thread_no, "Send header error\n");
				
				/*int retry = 0;
				while ( retry < 5 )
				{
					Sleep(1000); // 1s
					sprintf(msg, "Send header retry %d\n", retry + 1);
					ThreadLog(thread_no, msg);
					if ( -1 == SocketWrite(client_socket , appData +8 , appLen) )
						retry++;
					else
						break;

				}

				if ( retry >= 5 )
				{
					ThreadLog(thread_no, "After 5 retries, still cannot send header\n");*/
					ThreadLog(thread_no, "VMesh Client Released\n");
					closesocket(client_socket);
					LeaveCriticalSection (&vmclient_criticalsection);
					break;
			//	}
				
			}

			time(&curTime);
			hh = (curTime / 3600  + TIME_ZONE) % 24;
			mm =  (curTime / 60) % 60;
			ss = curTime % 60;
			//sprintf(msg, "After send header \t Current Time = %02d:%02d:%02d \n", hh, mm, ss);
			//ThreadLog(thread_no, msg);
			
			int pid = -1;
			Packet* pkt;
			int pktLength;
			char *pktData;
			int cntPkt = 0;
			Sleep(100);
			DWORD start, end, elapsed;

			//MediaInfo* info = vmClient->getMediaInfo(hash.c_str());
			//// estimate the number of packets in 1 second
			//Comment by Joe, Mar. 31
			//int avgNumPkt = 1000*(unsigned int)(info->getNumSegments() * info->getNumPacketsinSegment(0)) / info->getMediaLength(0);
			//int boostRecvThreshold = 20 * avgNumPkt;
			//int interval = 0.1*1000.0/avgNumPkt; // interval between feeding packets (10 times of packet rate)
			//End by Joe
			////printf( "avgNumPkt: %d\n", avgNumPkt);
			////printf( "interval: %d\n", interval);
			////printf( "boostRecvThreshold: %d\n", boostRecvThreshold);
						
			while (1)
			{
				//Sleep((cntPkt<boostRecvThreshold?1:interval)); // use a smaller delay for first few seconds
				Sleep(5);
				
				// ==========================================================
				// Detect socket connection status (closed by peer or not)
				// ==========================================================

				// Controls the I/O mode of a socket [non-blocking mode]
				u_long blockingMode = 1;
				ioctlsocket(client_socket, FIONBIO, &blockingMode);
				char a[100];
				int result;
				if ( (result = recv(client_socket, a, 100, 0) ) <= 0 )
				{

					if ( WSAGetLastError() != WSAEWOULDBLOCK/* || result == 0 */)
					{
						ThreadLog(thread_no, "VMesh Client Released (recv error)\n");

						LeaveCriticalSection (&vmclient_criticalsection);
						if ( pid > -1 )
							sprintf(a, "After packet %u \trecv Error: %d\n", pid, WSAGetLastError());
						else
							sprintf(a, "Recv Error: %d\n", WSAGetLastError());
						ThreadLog(thread_no, a);
						delete data;
						closesocket(client_socket);
						ThreadLog(thread_no, "Thread Termination (Connection closed)\n");
						_endthreadex(0);
						return 0;
					}
				}

				// Controls the I/O mode of a socket [blocking mode]
				blockingMode = 0;
				ioctlsocket(client_socket, FIONBIO, &blockingMode);

				// ==========================================================
				// End of socket status detection
				// ==========================================================
				
				int cnt = 0;
				if (!vmClient->existPacket(AUDIO_STREAM))
					if (vmClient->getMediaInfo(hash.c_str())->getIntervalbyPID(pid + 1, AUDIO_STREAM) == -1)
					{
						ThreadLog(thread_no, "Set Time = 0 (At the end of movie)\n");
						vmClient->setTime(0);
						Sleep(200);
						packetSeq = 0;
						continue;
		
					}

				while ((++cnt < 300) && !vmClient->existPacket( AUDIO_STREAM))
					Sleep(100);

				if (!vmClient->existPacket(AUDIO_STREAM))
				{		
					char msg[100];
					sprintf(msg, "No packet %d (time: %ds), waited 30s \n", packetSeq, vmClient->getMediaInfo(0)->getIntervalbyPID(packetSeq, 0));
					ThreadLog(thread_no, msg);
					
					ThreadLog(thread_no, "VMesh Client Released ( no packet )\n");
					LeaveCriticalSection (&vmclient_criticalsection);

					continue;
					//closesocket(client_socket);
					//SetEvent(hNoPacket);
					
					break;
				}

				//start = GetTickCount();
				pkt = vmClient->getPacket(AUDIO_STREAM);
				if (pkt==NULL)
				{
					ThreadLog(thread_no, "No packet, getPacket is NULL\n");
					continue;
				}

				/*elapsed = GetTickCount() - start;
				{
					sprintf(msg, "getPacket() takes %d.%03ds seconds\n", elapsed / 1000, elapsed % 1000);
					ThreadLog(thread_no, msg);
				}
				*/
				if ( cntPkt==0 ) // first packet
				{
					curTime = time(NULL);
					hh = (curTime / 3600 + TIME_ZONE) % 24;
					mm =  (curTime / 60) % 60;
					ss = curTime % 60;
					sprintf(msg, "After getting first packet\tCurrent Time = %02d:%02d:%02d \n", hh, mm, ss);
					ThreadLog(thread_no, msg);
					packetSeq = pkt->getPacketID();

				}
				++cntPkt;
				
				pktLength = pkt->getLength();
				pktData = pkt->getData();					
				if ( packetSeq != pkt->getPacketID() )
				{
					sprintf(msg, "Missing packet %u (time: %ds) \t The packet received is %u \n", packetSeq
						, vmClient->getMediaInfo(0)->getIntervalbyPID(packetSeq, 0), pkt->getPacketID() );
					ThreadLog(thread_no, msg );
					packetSeq = pkt->getPacketID();

					// ====================================
		

					// ====================================
				}
				packetSeq++;
				LeaveCriticalSection (&vmclient_criticalsection);

				if ( pkt->validatePacket() )
				{
					if( -1 == SocketWrite(client_socket , pktData, pktLength ) )
					{
						sprintf(msg, "Packet %u cannot be sent to the client\n", pkt->getPacketID());
						ThreadLog(thread_no, msg);
						delete pkt;
						delete data;
						closesocket(client_socket);
						ThreadLog(thread_no, "Thread Termination\n");
						_endthreadex(0);
						return 0;
					}
					else
					{}	
				}
				else
				{
					sprintf(msg, "Packet corruption detected: ID = %d, it will be skipped\n", pkt->getPacketID());
					ThreadLog(thread_no, msg);
				}

				if ( packetSize == 0 )
					packetSize = pkt->getLength();
				if (pkt->getLength() != packetSize )
				{	
					sprintf(msg, "Incorrect packet size: %d \n", pkt->getLength());
					ThreadLog(thread_no, msg);
				}

				if (pkt->getPacketID() % 100 == 0)
				{
					sprintf(msg, "Packet %u is retrieved and sent to the client\n", pkt->getPacketID());
					ThreadLog(thread_no, msg);
				}
				pid = pkt->getPacketID();
				pktData = NULL;
				delete pkt;
				pkt = NULL;
				EnterCriticalSection (&vmclient_criticalsection);
			}
		}
		else
		{
			OutputHTTPError(client_socket, 501);   // 501 not implemented
		}
			
	} while (0);
	
	delete data;
	//closesocket(client_socket);
	ThreadLog(thread_no, "Thread Termination\n");
	_endthreadex(0);
	return 0;
}

//--------------------------------------------------------------
//	SocketWrite()
//		Sends data from the client socket until it gets a valid http
//		header or the client disconnects.
//--------------------------------------------------------------
int SocketWrite(SOCKET client_socket, char *sendbuffer, int buffersize)
{
	int length = buffersize;
	int bytesent;
	char* s = sendbuffer;
	char errorMsg[100];

	while ( length > 0 )
	{
		bytesent = send(client_socket, s, length, 0); 
		//cout << "bytesent: " << bytesent << " length: " << length << "\n";
		// Handle send buffer is full situation, poll and re-send until success
		if ( bytesent == SOCKET_ERROR)
			if ( WSAGetLastError() == WSAEWOULDBLOCK )
			{
				DebugLog("Send Buffer is full, packet will be re-send\n");
				while ( bytesent == SOCKET_ERROR )
				{
					Sleep(10);
					bytesent = send(client_socket, s, length, 0);
					if ( bytesent == SOCKET_ERROR )
						if ( WSAGetLastError() != WSAEWOULDBLOCK )
						{
							sprintf(errorMsg, " WSAGetLastError() = %d\n", WSAGetLastError());
							DebugLog(errorMsg);
							return -1;
						}
				}
			}
			else
			{
				sprintf(errorMsg, "WSAGetLastError() = %d\n", WSAGetLastError());
				DebugLog(errorMsg);
				return -1;
			}

		length = length - bytesent;
		s += bytesent;
	}
	return buffersize;
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
	char msg[100];
	EnterCriticalSection (&output_criticalsection);
   //cout << errmsg << " - " << WSAGetLastError() << endl;
   
	sprintf(msg, "%s - %d \n", errmsg, WSAGetLastError());
	DebugLog(msg);
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
	//DebugLog(receivebuffer);
	//DebugLog("\n");
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
SOCKET StartWebServer()
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
	si.sin_port = htons(listen_port);		// port
	si.sin_addr.s_addr = htonl(INADDR_ANY);

	// Set send timeout
	// int timeout = 5000;
	// setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

	// Enable socket reuse
	int l = 1;
    //setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&l, sizeof(l));
    setsockopt(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));

	// Enable TCP keep-alive 
	BOOL bKeepAlive = TRUE;
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive)); 
	
	tcp_keepalive inKeepAlive = {0};
	unsigned long ulInLen = sizeof(__TCP_KEEPALIVE);    

	tcp_keepalive outKeepAlive = {0};
	unsigned long ulOutLen = sizeof(__TCP_KEEPALIVE);    

	unsigned long ulBytesReturn = 0;

	inKeepAlive.onoff = 1;  
	inKeepAlive.keepaliveinterval = 500;
	inKeepAlive.keepalivetime = 1000;

	WSAIoctl(s, SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, ulInLen, (LPVOID)&outKeepAlive, ulOutLen, &ulBytesReturn, NULL, NULL); 

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

   gethostname(hostname,MAX_PATH);
   h = gethostbyname(hostname);
	memcpy(&in,h->h_addr,4);
	h = gethostbyaddr((char *)&in,4,PF_INET);
	strcpy(hostname,h->h_name);
}

//--------------------------------------------------------------
//	A list of url parsing functions			
//--------------------------------------------------------------
int ParseSetTime(string str){
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	int start = str.find("set_time=");
	if (start == string::npos)
		return 0;
	start += strlen("set_time=");
	int end = start;
	
	while ( '0' <= str[end] && str[end] <= '9' )
		end++;
	
	string num = str.substr(start, end-start);
	
	int time = atoi(num.c_str());
	return time;
}

string ParseCommand(string str){
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	size_t start = 0;
	start = str.find("command=");

	if (start == string::npos)
		return "";
	start += strlen("command=");

	size_t end = start;
	
	while ( ('a' <= str[end] && str[end] <= 'z') )
		end++;

	size_t length = end - start;

	return str.substr(start, length);
}

string ParseMovieHash(string str)
{
	string hash;
	//std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	int start = str.find("movie_hash=");
	if (start == string::npos)
		return hash;
	start += strlen("movie_hash=");
	int end = start;
	while ( isalpha(str.at(end)) ||  isdigit(str.at(end)) )
		end++;
	hash = str.substr(start, end-start);
	return hash;
}

bool ParseStopCommand(string str)
{
	int pos = str.find("STOPNOW");
	return (pos > 0);
}


bool ParseRestartCommand(string str){
	int pos = str.find("RESTART");
	return (pos > 0);
}

string ParseServerAddress(string str){
	string server_address;
	//std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	int start = str.find("server=");
	if (start == string::npos)
		return server_address;
	start += strlen("server=");
	int end = start;
	while ( end < str.length() && str.at(end) != '&' && str.at(end) != '?')
		end++;
	server_address = str.substr(start, end-start);
	return server_address;
}

int ParsePortNumber(string str){
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	int start = str.find("port=");
	if (start == string::npos)
		return 0;
	start += strlen("port=");
	int end = start;
	while ( '0' <= str.at(end) && str.at(end) <= '9' )
		end++;
	string num = str.substr(start, end-start);
	int port = atoi(num.c_str());
	return port;
}

void ThreadLog(int thread_no, const char* s)
{
	char msg[100];
	sprintf(msg, "Thread %02d: %s", thread_no, s);
	DebugLog(msg);
}

bool CheckIsPortAvailable(int port)
{

	if (port < 1 || port > 65535 || port == listen_port || port == listen_port - 1 )
		return false;
	SOCKET test_socket = socket(AF_INET,SOCK_STREAM,0);
	if ( test_socket == INVALID_SOCKET )
		return false;

	SOCKADDR_IN si;

	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.s_addr = htonl(INADDR_ANY);

	int l = 1;
    setsockopt(test_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));

	if ( bind(test_socket,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		closesocket(test_socket);
		return false;
	}

	closesocket(test_socket);

#ifdef CHORD_LOOKUP
	port++;

	test_socket = socket(AF_INET,SOCK_STREAM,0);
	if ( test_socket == INVALID_SOCKET )
		return false;

	si.sin_port = htons(port);

    setsockopt(test_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));
	if ( bind(test_socket,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		closesocket(test_socket);
		return false;
	}

	closesocket(test_socket);
#endif

	return true;
}
