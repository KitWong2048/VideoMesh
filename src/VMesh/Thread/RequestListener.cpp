#include "../Thread/RequestListener.h"

using namespace std;
using App::ChordApp;

_VMESH_NAMESPACE_HEADER

RequestListener::RequestListener (string _myIP, int _myPort, int _numHandler, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest, int p_identity)
: myServer(_myIP.c_str(), _myPort)
{
	myIP = _myIP;
	myPort = _myPort;
	numHandler = _numHandler;
	myMediaInfo = _info;
	myNodeList = _list;
	myStorage = _storage;
	myBuffer = _buffer;
	mySegment_ = _segment;
	keepRunning = true;
	myStreamRequest = _myStreamRequest;
	myHandlerList = new RequestHandler* [numHandler];
	for (int i = 0; i < numHandler; i++)
		myHandlerList[i] = NULL;

	// Jack's comment:: add peer identity information into request listener
	peer_identity = p_identity;

	handlerCounter=0;
}

RequestListener::~RequestListener()
{
	for (int i = 0; i < numHandler; i++)
	{
		if (myHandlerList[i] != NULL)
		{
			if (myHandlerList[i]->getJoinableStatus())
			{
				myHandlerList[i]->stop();
			}
			delete myHandlerList[i];
			myHandlerList[i] = NULL;
			// LOG_DEBUG("Thread RequestHandler " + itos(i) + " is killed");
		}
	}
	delete [] myHandlerList;
}

VMeshRemoteNodeList* RequestListener::getConnectedNodeList()
{
	VMeshRemoteNodeList* myConnectedNodeList = new VMeshRemoteNodeList();

	for (int i = 0; i < numHandler; i++)
	{
		if ((myHandlerList[i] != NULL) && (myHandlerList[i]->getRunningStatus()))
		{
			VMeshRemoteNode* x = myHandlerList[i]->getMyNode();
			if (x != NULL)
			{
				myConnectedNodeList->addNode(x);
			}
		}
	}
	return myConnectedNodeList;
}

void RequestListener::stop()
{
	keepRunning = false;
	this->join();
}
/*
start listen to others' requests
*/
bool RequestListener::listenRequest()
{
	// Server fails to create
	if (!myServer.ready())
	{
		LOG_ERROR("RequestListener\t: Create fails");
		return false;
	}
	else
	{
		LOG_DEBUG("RequestListener\t: Started");
	}

	int i;
	bool isFull = false;

	while(keepRunning)
	{
		isFull = true;
		// remove old threads that are not running
		handlerCounter=0;
		for (i = 0; i < numHandler; i++)
		{
			if (myHandlerList[i] == NULL)
			{
				isFull = false;
			}
			else if (!myHandlerList[i]->isRunning())
			//else if (!myHandlerList[i]->getRunningStatus())
			{
				//LOG_DEBUG((string)"RequestListener\t: detected handler not running - delete handler in slot " + itos(i));
				//myHandlerList[i]->join();
				
				if (myHandlerList[i] != NULL){
					delete myHandlerList[i];
					myHandlerList[i] = NULL;
				}
				isFull = false;
			}
			else
			{
				handlerCounter++;
			}
		}
		
		if (isFull)
		{
			usleep(200000);
			continue;
		}
		//LOG_INFO("Check handling Counter:"+itos(handlerCounter));
		//LOG_INFO("RequestListener\t: Waiting connection...");
		// accept connection
		SocketClient* client = myServer.accept(500);
		if (client==NULL)
			continue;

		if ( !keepRunning )
			break;
		//LOG_INFO((string)"RequestListener\t: Accepted connection from " + string(client->getRemoteIP()) + ":" + itos(client->getRemotePort()));
		// check if there are slots available
		for (i = 0; i < numHandler; i++)
		{

			if(myHandlerList[i]==NULL)
			{
				myHandlerList[i] = new RequestHandler(myIP, myPort, client, myMediaInfo, myNodeList, myStorage, myBuffer, mySegment_, myStreamRequest, peer_identity);
				myHandlerList[i]->run();				
				break;
			}
			else if (!myHandlerList[i]->isRunning())
			//else if (!myHandlerList[i]->getRunningStatus())
			{
				myHandlerList[i]->stop();
				delete myHandlerList[i];
				myHandlerList[i]=NULL;
				//myHandlerList[i]->join();
				// add the client to a threads
				//LOG_INFO((string)"RequestListener\t: " + (client->getRemoteIP()) + ":" + itos(client->getRemotePort()) + " connected to slot " + itos(i));
				myHandlerList[i] = new RequestHandler(myIP, myPort, client, myMediaInfo, myNodeList, myStorage, myBuffer, mySegment_, myStreamRequest, peer_identity);
				myHandlerList[i]->run();				
				break;
			}
			if ( !keepRunning )
				break;
		}

		if ( !keepRunning )
			break;
		// should not occur
		/*
		if (client!=NULL)
		{
			LOG_INFO((string)"RequestListener\t: " + (client->getRemoteIP()) + ":" + itos(client->getRemotePort()) + " connects rejected: Full") ;
			delete client;
			client = NULL;
			isFull = true;
		}*/
	}
	return true;
}

/*
Override the execute() in Thread
*/
void* RequestListener::execute()
{	
	bool status = listenRequest();
	if (!status)
		LOG_ERROR("RequestListener\t: Terminated with error");
	return NULL;
}

_VMESH_NAMESPACE_FOOTER
