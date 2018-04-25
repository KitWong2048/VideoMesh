#include "CentralLookupManager.h"

std::list<string> CentralLookupManager::myNodeList;

string CentralLookupManager::serverIP_;

time_t CentralLookupManager::lastRefresh = 0;

boost::asio::io_service io_service;

DWORD WINAPI refreshThread(LPVOID lpParam);

CentralLookupManager::CentralLookupManager(string _myIP, int _myPort, int _identity) : myIP_(_myIP), myPort_(_myPort), identity(_identity), running(false), isBootstrap(false), joined(false), refreshThHandle(NULL), sock(io_service)
{

}

CentralLookupManager::~CentralLookupManager(void)
{
}

int CentralLookupManager::run()
{
	running = true;
	return 0;
}

int CentralLookupManager::stop()
{
	if (refreshThHandle != NULL)
	{
		TerminateThread(refreshThHandle, 0);
		joined = false;
	}
	if(isBootstrap)
	{
		// create an initial DHT key for accessing DHT
		CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(myIP_, myPort_, BOOTSTRAP_SEGMENTID, myHash_, identity);

		// add the initial node to the DHT
		sendToServer(serverIP_, 10080, "DELETEMOVIE", myTempNode->genDHTNetworkID().getIDHex());
		delete myTempNode;
	}
	else
	{
		// create an initial DHT key for accessing DHT
		CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(myIP_, myPort_, BOOTSTRAP_SEGMENTID, myHash_, identity);
		removeNode(*myTempNode);
		delete myTempNode;
	}
	running = false;
	return 0;
}

int CentralLookupManager::addNode(const NetworkID& _id)
{
	// create an initial DHT key for accessing DHT
	CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(_id.getIP(), _id.getPort(), _id.getSegment(), _id.getMediaHash(), identity);

	// If add the Node of itself add to the list for later use
	if(_id.getIP() == myIP_)
	{
		myNodeList.push_back(myTempNode->genDHTNetworkID().getIDHex());
	}

	// add the initial node to the DHT

	/*Socket sock(serverIP_);
	sock.SocketConnect();
	sock.SocketSend("ADDNODE", myTempNode->genDHTNetworkID().getIDHex());
	sock.SocketClose();*/
	sendToServer(serverIP_, 10080, "ADDNODE", myTempNode->genDHTNetworkID().getIDHex());

	delete myTempNode;
	return 0;
}

int CentralLookupManager::removeNode(const NetworkID& _id)
{
	// create an initial DHT key for accessing DHT
	CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(_id.getIP(), _id.getPort(), _id.getSegment(), _id.getMediaHash(), identity);

	// update the Node list
	if(_id.getIP() == myIP_)
	{
		list<std::string>::iterator it = find (myNodeList.begin(), myNodeList.end(), myTempNode->genDHTNetworkID().getIDHex());
		if(it != myNodeList.end())
			myNodeList.erase(it);
	}

	// Delete node to the DHT

	sendToServer(serverIP_, 10080, "REMOVENODE", myTempNode->genDHTNetworkID().getIDHex());

	delete myTempNode;
	return 0;
}

NetworkID* CentralLookupManager::lookup(const NetworkID& _id, int& _hopCount)
{
	/*TODO*/
//	return new CentralLookupNetworkID();
	//return new CentralLookupNetworkID(*myChord_->lookup(DHT::DHTNetworkID(_id.toBin()), _hopCount));
	return NULL;
}

NetworkID* CentralLookupManager::lookupFullPath(const NetworkID& _id, list<NetworkID*>& _searchPath, int& _hopCount)
{
	list<NetworkID*> neighborList;

	// create an initial DHT key for accessing DHT
	CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(_id.getIP(), _id.getPort(), _id.getSegment(), _id.getMediaHash(), identity);

	string nodeListStr = "";

	for(list<std::string>::iterator i = myNodeList.begin(); i != myNodeList.end(); i++)
	{
		nodeListStr = nodeListStr + *i;
		nodeListStr = nodeListStr + "\n";
	}

	lastRefresh = time(NULL);
	// add the initial node to the DHT

	// Request a lookup service of the network
	// TODO: change the "well-known" port to be a parameter of the system
	std::string listString = sendToServer(serverIP_, 10080, "NODELOOKUP", myTempNode->genDHTNetworkID().getIDHex(), nodeListStr);

	delete myTempNode;

	//NetworkID* resultId = new CentralLookupNetworkID(_id.getIP(), _id.getPort(), _id.getSegment(), _id.getMediaHash());
	NetworkID* resultId = new CentralLookupNetworkID(_id.getIP(), _id.getPort(), _id.getSegment(), _id.getMediaHash(), _id.getPeerIdentity());
	if (listString != "")
	{
		char* token;
		char* str =new char[listString.length()+1];
		strcpy(str, listString.c_str());
		str[listString.length()] = 0;

		token = strtok (str,"\n");

		// for debug
		printf("[DEBUG MSG]CentralLookupManager::Get Node from Central Lookup Server::\t%s\n", token);

		while (token != NULL)
		{
			NetworkID* temp = new CentralLookupNetworkID((const char*)token);
			_searchPath.push_back(temp);
			token = strtok (NULL, "\n");
		}
		delete str;
	}
	printf("[DEBUG MSG]CentralLookupManager::Get %d Node in Search Path\n", _searchPath.size());
	return resultId;
}

list<NetworkID*> CentralLookupManager::getNeighbor(const NetworkID& _id)
{
	list<NetworkID*> neighborList;
	return neighborList;
}

void CentralLookupManager::print()
{
}

bool CentralLookupManager::isRunning()
{
	return running;
}

int CentralLookupManager::join(const NetworkID& _id, const string _hash)
{
	myHash_ = _hash;
	// create an initial DHT key for accessing DHT
	CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(myIP_, myPort_, BOOTSTRAP_SEGMENTID, myHash_, identity);

	// bootstrap node initialization
	serverIP_ = _id.getIP();
	serverPort_ = _id.getPort();

	// add the initial node to the DHT
	if(_id.isBootstrap())
	{		
		std::string recv =  sendToServer(serverIP_, 10080, "CREATEMOVIE", myTempNode->genDHTNetworkID().getIDHex());
		isBootstrap = true;
	}
	else
		if (joined == false)
		{
			DWORD dwThreadId;

			// Allocate memory for thread data.
			void* pData = new CentralLookupNetworkID(myIP_, myPort_, QUERY_SEGMENTID, myHash_, identity);
	
			refreshThHandle = CreateThread( 
				NULL,				// default security attributes
				0,					// use default stack size  
				refreshThread,		// thread function 
				this, //pData,				// argument to thread function 
				0,					// use default creation flags 
				&dwThreadId);		// returns the thread identifier 
		
			joined = true;
		}
	delete myTempNode;
	running = true;
	return 0;
}

DWORD WINAPI refreshThread(LPVOID lpParam)
{
	CentralLookupManager* thisCLMgr = (CentralLookupManager*) lpParam;
	thisCLMgr->RefreshThreadFunc();
	//CentralLookupNetworkID* myTempNode = (CentralLookupNetworkID*) lpParam;
	return 0;
}
	
void CentralLookupManager::RefreshThreadFunc()
{
	CentralLookupNetworkID* myTempNode = new CentralLookupNetworkID(myIP_, myPort_, QUERY_SEGMENTID, myHash_, identity);
	while(true)
	{
		if((time(NULL) - lastRefresh) > REFRESH_SERVER_INTERVAL)
		{
			lastRefresh = time(NULL);
			string nodeListStr = "";

			for(list<std::string>::iterator i = myNodeList.begin(); i != myNodeList.end(); i++)
			{
				nodeListStr = nodeListStr + *i;
				nodeListStr = nodeListStr + "\n";
			}

			if(nodeListStr != "")
			{
				// add the initial node to the DHT
				string listString = sendToServer(serverIP_, 10080, "NODELOOKUP", myTempNode->genDHTNetworkID().getIDHex(), nodeListStr);
			}

			Sleep(REFRESH_SERVER_INTERVAL*1000);
		}
		else
		{
			Sleep((REFRESH_SERVER_INTERVAL - (time(NULL) - lastRefresh))*1000);
		}
	}
	delete myTempNode;

}
string CentralLookupManager::sendToServer(const string& server_ip, int port, const string& command, const string& hash, const string& nodeListString)
{
	std::string result	= "";
	std::string content = "";
	char node_identity[5];
	itoa(identity, node_identity, 5);

	if ( sock.Connect(server_ip, port) )
		result = sock.SendCommand(command, hash, content, node_identity);
	return result ;	
}
