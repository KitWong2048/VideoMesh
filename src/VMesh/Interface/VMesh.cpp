#include "../Interface/VMesh.h"

using namespace std;
using namespace App;
using namespace DHT;
using namespace DHTSocket;
using namespace Util;
//using namespace Vivaldi;

_VMESH_NAMESPACE_HEADER

// predicate function for sorting PeerInfo
bool less_PeerInfo(const PeerInfo & p1, const PeerInfo & p2) {
	if (p1.ip != p2.ip)
        	return p1.ip < p2.ip;
	else if (p1.port != p2.port)
		return p1.port < p2.port;
	else
		return p1.pingTime < p2.pingTime;
}
// Comment by jack 14/12/2010
/*VMesh::VMesh (string _myIP, int _myPort, int _localStorageCapacity)
{
	// initialize my IP and port
	myIP = _myIP;
	myPort = _myPort;
	isSuperNode = false;

	if (_localStorageCapacity <= 0)
		localStorageCapacity = LOCAL_STORAGE_CAPACITY;
	else
	{
		isSuperNode = true;
		localStorageCapacity = _localStorageCapacity;
		LOG_INFO("VMesh started in Supernode mode: storing " + itos(localStorageCapacity) + " segments");
	}

	bootstrapIP = "";
	bootstrapPort = 0;

	myHash = "";
	myStorage = NULL;
	myMediaInfo = NULL;


	// initialize my LocalNodeList
	myNodeList = new VMeshLocalNodeList();

	// Jack's comment::initialize Central Lookup Manager
	mySegment_ =
#ifdef CHORD_LOOKUP
		new ChordManager(myIP, myPort);
#elif defined(CENTRAL_LOOKUP)
		new CentralLookupManager(myIP, myPort);
#else
# error Segment Lookup Methods not defined
#endif
	myBuffer = NULL;

	// the threads will be created and executed when start()
	myListener = NULL;
	myStreamRequest = NULL;

	// Vivaldi thread
	//myVivaldiRequest = NULL;

}*/

// VMesh Constructor
VMesh::VMesh (string _myIP, int _myPort, int _localStorageCapacity, int _identity, int _exchangeNeighbor)
{
	// initialize my IP and port
	myIP = _myIP;
	myPort = _myPort;
	isSuperNode = false;
	identity = _identity;
	exchangeNeighbor = _exchangeNeighbor;

	// Check identity to define the total bandwidth
	switch(identity)
	{
		case SERVER_IDENTITY:
			maxConnection = SERVER_TOTAL_BANDWIDTH / CONSUME_BANDWIDTH;
			//cout << "Check maximum connection::\t" << maxConnection << endl;
			LOG_INFO("Check maximum connection::\t" + itos(maxConnection));
			break;

		case PROXY_IDENTITY:
			maxConnection = PROXY_TOTAL_BANDWIDTH / CONSUME_BANDWIDTH;
			//cout << "Check maximum connection::\t" << maxConnection << endl;
			LOG_INFO("Check maximum connection::\t" + itos(maxConnection));
			break;

		case CLIENT_IDENTITY:
			maxConnection = CLIENT_TOTAL_BANDWIDTH / CONSUME_BANDWIDTH;
			//cout << "Check maximum connection::\t" << maxConnection << endl;
			LOG_INFO("Check maximum connection::\t" + itos(maxConnection));
			break;
	}
	
	if (_localStorageCapacity <= 0)
		localStorageCapacity = LOCAL_STORAGE_CAPACITY;
	else
	{
		isSuperNode = true;
		localStorageCapacity = _localStorageCapacity;
		LOG_INFO("VMesh started in Supernode mode: storing " + itos(localStorageCapacity) + " segments");
	}

	bootstrapIP = "";
	bootstrapPort = 0;

	myHash = "";
	myStorage = NULL;
	myMediaInfo = NULL;


	// initialize my LocalNodeList
	myNodeList = new VMeshLocalNodeList();

	// Jack's comment::initialize Central Lookup Manager
	mySegment_ =
#ifdef CHORD_LOOKUP
		new ChordManager(myIP, myPort);
#elif defined(CENTRAL_LOOKUP)
		new CentralLookupManager(myIP, myPort, identity);
#else
# error Segment Lookup Methods not defined
#endif
	myBuffer = NULL;

	// the threads will be created and executed when start()
	myListener = NULL;
	myStreamRequest = NULL;

	// Vivaldi thread
	//myVivaldiRequest = NULL;
}

VMesh::~VMesh()
{
	this->stop();
	if (myMediaInfo != NULL)
	{
		delete myMediaInfo;
		myMediaInfo = NULL;
	}
	if (myNodeList != NULL)
	{
		delete myNodeList;
		myNodeList = NULL;
	}
	if (myStorage != NULL)
	{
		delete myStorage;
		myStorage = NULL;
	}
	if (myBuffer != NULL)
	{
		delete myBuffer;
		myBuffer = NULL;
	}
	if (mySegment_ != NULL)
	{
		delete mySegment_;
		mySegment_ = NULL;
	}
}

// For VMeshServer to publish movie and join into the DHT network
bool VMesh::publish(string _filename)
{
	// bootstrap node and publish movie

	// initialize the buffer
	LOG_INFO("Buffer\t: Creating...");
	CircularBuffer* tempBuffer = new CircularBuffer(BUFFER_SIZE);
	LOG_INFO("Buffer\t: Created successfully");
	myBuffer = tempBuffer;


	// start DHT
	LOG_INFO("LookUp Segment\t: Creating...");
	if (mySegment_->run() < 0)
	{
		LOG_ERROR("LookUp\t: Fail to create");
		delete mySegment_;
		mySegment_ = NULL;
		return false;
	}

	// wait for the chord to start
	int sleepingTime = 0;
	while ((sleepingTime < 30) && (!mySegment_->isRunning())){
		sleep(1);
		sleepingTime += 1;
	}

	if (!mySegment_->isRunning()){
		LOG_ERROR("LookUp\t: Fail to start");
		delete mySegment_;
		mySegment_ = NULL;
		return false;
	}

	LOG_INFO("LookUp Segment\t: Started successfully");

	// use publisher to initialize my MediaInfo and BootstrapStorage
	BootstrapStorage* tempStorage;
	bool isPublishOK = Publisher::publish(tempStorage, myMediaInfo, _filename, 0, SEEK_RESOLUTION, SEGMENT_LENGTH/SEEK_RESOLUTION);

	if (!isPublishOK)
	{
		LOG_ERROR("Publisher\t: Fail to publish " + _filename );
		return false;
	}
	else
	{
		myStorage = tempStorage;
		LOG_INFO("Publisher\t: Published successfully");
	}

	// add the bootstrap node to VMeshLocalNodeList
	VMeshLocalNode* bootstrap = new VMeshLocalNode(myMediaInfo, myIP, myPort, true, identity);
	myNodeList->addNode(bootstrap);

	//myChord->addNode(bootstrap->genDHTNetworkID(), bootstrap->genAppValue());
	//int result = mySegment_->addNode(*bootstrapId);

	//NetworkID* bootstrapId = new CentralLookupNetworkID(myIP, myPort, BOOTSTRAP_SEGMENTID, myMediaInfo->getMediaHash());
	NetworkID* bootstrapId = new CentralLookupNetworkID(myIP, myPort, BOOTSTRAP_SEGMENTID, myMediaInfo->getMediaHash(), identity);

	int result = mySegment_->join(*bootstrapId,  myMediaInfo->getMediaHash());

	delete bootstrapId;

	// print DHT status
	mySegment_->print();

	return true;
}

// For VMesh Client join movie
bool VMesh::join(string _hash, string _remoteIP, int _remotePort)
{
	bootstrapIP = _remoteIP;
	bootstrapPort = _remotePort;

	log4cxx::Logger::set_hash(_hash);


	// join via the bootstrap node
	if (_hash.length()!=2*MEDIAINFOHASH_LENGTH)
		return false;
	myHash = _hash;

	// initialize the buffer
	CircularBuffer* tempBuffer = new CircularBuffer(BUFFER_SIZE);
	LOG_INFO("Buffer\t: Created successfully");
	myBuffer = tempBuffer;

	// start DHT
	LOG_INFO("LookUp\t: Starting...");
	if (mySegment_->run() < 0)
	{
		LOG_ERROR("LookUp\t: Fail to start");
		delete mySegment_;
		mySegment_ = NULL;
		return false;
	}

	// wait for the chord to start
	int sleepingTime = 0;
	while ((sleepingTime < 30) && (!mySegment_->isRunning())){
		sleep(1);
		sleepingTime += 1;
	}

	if (!mySegment_->isRunning()){
		LOG_ERROR("LookUp\t: Fail to start");
		delete mySegment_;
		mySegment_ = NULL;
		return false;
	}

	// create myStorage
	myStorage = new PeerStorage(PEERSTORAGE_SIZE);


	NetworkID* bootstrapId = new 
#ifdef CHORD_LOOKUP
		//Comment by Juan, Mar. 22: to solve port problem
		ChordNetworkID(_remoteIP, _remotePort+1, INITIAL_SEGMENTID, _hash);
		//ChordNetworkID(_remoteIP, _remotePort, INITIAL_SEGMENTID, _hash);
		//End by Juan
#elif defined(CENTRAL_LOOKUP)
		//CentralLookupNetworkID(_remoteIP, DEFAULT_LOOKUP_PORT, INITIAL_SEGMENTID, _hash);
		CentralLookupNetworkID(_remoteIP, DEFAULT_LOOKUP_PORT, INITIAL_SEGMENTID, _hash, identity);
#endif

	int result = mySegment_->join(*bootstrapId, _hash);
	delete bootstrapId;

	if (result <0)
	{
		return false;
	}

	// print DHT status
	mySegment_->print();

	return true;
}

bool VMesh::start()
{
	// allocate memory and execute the thread
	myStreamRequest = new StreamRequest(myIP, myPort, myMediaInfo, myNodeList, myStorage, myBuffer, mySegment_, myHash, maxConnection, identity, exchangeNeighbor);

	if ( isSuperNode )
		myStreamRequest->setSuperNode();

	if (myHash!="")
	{
		int retry = 0;
		while (1)
		{
			if (myStreamRequest->addBootstrap(bootstrapIP, bootstrapPort))
				break;
			//if ((++retry)>=VMESH_CONNECT_RETRY)
			else
			{
				LOG_ERROR((string)"VMesh cannot join through this node. " + bootstrapIP + ":" + itos(bootstrapPort) + " is dead.");
				return false;
			}
		}

		/*start get media info
		*/
		
		/*int sleepingTime = 0;
		while ((sleepingTime < 600) && (myStreamRequest->getMediaInfo()==NULL))
		{
			sleep(1);
			sleepingTime ++;
		}
		if (myStreamRequest->getMediaInfo()!=NULL)
		{
			myMediaInfo = myStreamRequest->getMediaInfo();
			//LOG_INFO("MediaInfo is received successfully");
		}
		else
		{
			//LOG_ERROR("MediaInfo is failed to receive");
			return false;
		}*/
		myMediaInfo=myStreamRequest->getMediaInfo();
		if(myMediaInfo==NULL)
		{
			return false;		
		}
		/*ends
		*/

			// When the node local storage capacity larger than media file's number of segments
		// only use the media file's number of segments to create local storage capacity
		int nNumSegments = myMediaInfo->getNumSegments();
		if ( localStorageCapacity > nNumSegments || localStorageCapacity <= 0 )
			localStorageCapacity = nNumSegments;

		// choose which segment to store
		srand((unsigned int)time(0));

		int* chosenSegmentID = new int[localStorageCapacity];
		int i;
		
		if ( localStorageCapacity == nNumSegments )
		{	
			for ( i = 0; i < nNumSegments; i++ )
				chosenSegmentID[i] = i;

			//myStreamRequest->addParentsbyDHT(chosenSegmentID[i]);
		}
		else
		{
			for (i = 0; i<localStorageCapacity; i++)
				chosenSegmentID[i] = -1;

			i = 0;

			// Base on the number of segments to choose segments randomly and store into local storage
			// Stop choosing segments when chose segments duplicated
			while (true)
			{
				bool repeated = false;
				chosenSegmentID[i] = rand() % ( nNumSegments );

				//chosenSegmentID[i] = rand() % 5;	// for use in testing only!!!
				for (int j = 0; j<i; j++)
					if (chosenSegmentID[i]==chosenSegmentID[j])
						repeated = true;
				if (!repeated)
					i++;
				if (i >= localStorageCapacity || i >= nNumSegments )
					break;
			}

			//myStreamRequest->addParentsbyDHT(chosenSegmentID[i]);
		}

		/*
		* just for testing 
		*/
		chosenSegmentID[0]=0;
		chosenSegmentID[1]=1;

		//get some parent from central lookup
		myStreamRequest->addParentsbyDHT(chosenSegmentID[0]);
		myStreamRequest->addParentsbyDHT(chosenSegmentID[1]);

		myStreamRequest->run();			//start stream request thread
		
		// queue the packets of chosen segments
		for (i=0; i<localStorageCapacity; i++)
		{
			if (chosenSegmentID[i]==-1)
				break;
			LOG_DEBUG((string)"Local Storage is going to store Segment " + itos(chosenSegmentID[i]));

			// add the Packets of this segment to the waiting queue to be got
			for (int j=0; j<NUM_STREAM; j++)
			{
				unsigned int startPID = myMediaInfo->getStartingPIDbySID(chosenSegmentID[i], j);
				int num = myMediaInfo->getNumPacketsinSegment(chosenSegmentID[i], j);
				LOG_DEBUG("Type : " + itos(j) + " start : " + itos(startPID) + "\tlen : " +itos(num));
				//sleep(5);

				// fetch the packets into storage
				for (int k=0; k<num; k++)
					myStreamRequest->store((unsigned int)((unsigned int)k + startPID + j *1000000000));
			}
		}

		if ( chosenSegmentID )
			delete [] chosenSegmentID;

		myListener = new RequestListener(myIP, myPort, MAX_SERVE_CONNECTION, myMediaInfo, myNodeList, myStorage, myBuffer, mySegment_, myStreamRequest, identity);
		myListener->run();
	}
	else
	{
		myStreamRequest=NULL;
		myListener = new RequestListener(myIP, myPort, MAX_BOOTSTRAP_CONNECTION, myMediaInfo, myNodeList, myStorage, myBuffer, mySegment_, myStreamRequest, identity);
		myListener->run();
	}

	//Comment by jack [1-9-2010]
	//Don't run Vivaldi
	//myVivaldiRequest = new VivaldiRequest(myIP, myPort+500, 5, 10, (string) STORAGEFILE_PATH + "peers.txt");
	//myVivaldiRequest->run();

	return true;
}

void VMesh::stop()
{
	if (myStreamRequest != NULL)
	{
		if (myStreamRequest->getJoinableStatus())
		{
			myStreamRequest->stop();
			myStreamRequest->join();
		}
		delete myStreamRequest;
		myStreamRequest = NULL;
		LOG_DEBUG("Thread StreamRequest is killed");
	}

	// stop the threads and deallocate memory
	if (myListener != NULL)
	{
		//if (!myListener->getCancelStatus())
		//	myListener->cancel();
		if (myListener->getJoinableStatus())
		{
			myListener->stop();
			myListener->join();
		}
		delete myListener;
		myListener = NULL;
		LOG_DEBUG("Thread RequestListener is killed");
	}
	if (mySegment_ != NULL)
	{
		mySegment_->stop();
		delete mySegment_;
		mySegment_ = NULL;
		LOG_DEBUG("Lookup manager is killed");
	}

	// stop the threads and deallocate memory
	/*if (myVivaldiRequest != NULL)
	{
		if (myVivaldiRequest->getJoinableStatus())
		{
			myVivaldiRequest->stop();
			myVivaldiRequest->join();
		}
		delete myVivaldiRequest;
		myVivaldiRequest = NULL;
		LOG_DEBUG("Thread VivaldiRequest is killed");
	}*/
}

MediaInfo* VMesh::getMediaInfo()
{
	return myMediaInfo;
}

bool VMesh::fetch(int _whichInterval, unsigned int& startAPID, int& Alength, unsigned int& startVPID, int& Vlength)
{
	//LOG_DEBUG("VMesh: \t" + itos(_whichInterval) + " is fetched");
	if (myMediaInfo == NULL)
		return false;
	// not bootstrap and StreamRequest not ready
	if ((myStreamRequest==NULL) && (myHash!=""))
		return false;

	if ( _whichInterval > myMediaInfo->getMediaLength(0)/1000 && _whichInterval > myMediaInfo->getMediaLength(1)/1000)
	{
		startAPID = 0;
		Alength = 0;
		startVPID = 0;
		Vlength = 0;
		return false;
	}
	startAPID = myMediaInfo->getPIDbyInterval(_whichInterval, 0);
	Alength = myMediaInfo->getNumPacketsinInterval(_whichInterval, 0);

	if (NUM_STREAM>1)
	{
		startVPID = myMediaInfo->getPIDbyInterval(_whichInterval, 1);
		Vlength = myMediaInfo->getNumPacketsinInterval(_whichInterval, 1);
	}
	else
	{
		startVPID = 0;
		Vlength = 0;
	}

	// bootstrap
	if ((myStreamRequest==NULL) && (myHash==""))
		return true;
	for (int i=0; i<Alength || i<Vlength; i++)
	{
		if (i<Vlength)
			myStreamRequest->addPacket(1000000000 + startVPID + i);
		if (i<Alength)
			myStreamRequest->addPacket(startAPID + i);
	}
	return true;
}

void VMesh::clear_fetch()
{
	if (myStreamRequest!=NULL)
		myStreamRequest->clearQueueToBuffer();
	//LOG_DEBUG("VMesh: Packets waiting in Queue are cleared");
}

Packet* VMesh::getPacket(int _type, unsigned int _PacketID)
{
	if (myMediaInfo == NULL)
	{
		return NULL;
	}

	if (myBuffer->exist(1000000000 * _type + _PacketID))
	{
		unsigned int length;
		const char* buf = myBuffer->read(1000000000 * _type + _PacketID, length);

		if (buf!=NULL)
		{
			Packet* target = new Packet(buf, length);
			return target;
		}
	}
	if (myStorage->existPacket(_type, _PacketID))
	{
		
		return myStorage->getPacket(_type, _PacketID);
	}
	else
	{
		return NULL;
	}
}

bool VMesh::existPacket(int _type, unsigned int _PacketID)
{
	if ( myStreamRequest )
		myStreamRequest->notifyPacketRequest();
	if (myMediaInfo == NULL)
		return false;
	else if (myBuffer->exist(1000000000 * _type + _PacketID))
		return true;
	else if (myStorage->existPacket(_type, _PacketID))
		return true;
	else
		return false;
}

PeerInfoList VMesh::getPeerInfo()
{
	PeerInfoList myPeerInfoList;
	
	if (myHash!="")	// bootstrap server don't have parent list
	{
		//VMeshRemoteNodeList* myParentList = myStreamRequest->getParentList();
		ExchangeTable *myExchangeTable=myStreamRequest->getExchangeTable();

		for (int i=0; i<myExchangeTable->getSize(); i++)
		{
			VMeshRemoteNode* p =  myExchangeTable->getNodeByIndex(i);
			if (p!=NULL)
			{
				PeerInfo info;
				info.ip = p->getIP();
				info.port = p->getPort();
				info.isBootstrap = p->isBootstrap();
				info.segmentID = p->getSegmentID();
				info.pingTime = p->getPingTime();
				if ((info.pingTime==PING_NOT_READY) || (info.pingTime==PING_FAIL) || (info.pingTime==PING_TIMEOUT))
					info.pingTime = -1;
				if ( p->isSocketUsing())
				{
					info.isConnected = DL;
				}
				else
				{
					info.isConnected = DISCONNECTED;
				}
				myPeerInfoList.push_back(info);
			}
		}
	}
	
	if (myListener!=NULL)
	{
		VMeshRemoteNodeList* myConnectedList = myListener->getConnectedNodeList();
		for (int i=0; i<myConnectedList->getSize(); i++)
		{
			VMeshRemoteNode* p = myConnectedList->getNode(i);
			if (p!=NULL)
			{
				PeerInfo info;
				info.ip = p->getIP();
				info.port = p->getPort();
				info.isBootstrap = p->isBootstrap();
				info.segmentID = p->getSegmentID();
				info.pingTime = p->getPingTime();
				if ((info.pingTime==PING_NOT_READY) || (info.pingTime==PING_FAIL) || (info.pingTime==PING_TIMEOUT))
					info.pingTime = -1;
				info.isConnected = UL;
				myPeerInfoList.push_back(info);
			}
		}
		myConnectedList->clearall();
		delete myConnectedList;
	}
	
	myPeerInfoList.sort(less_PeerInfo);
	
	for (int i=0; i<myNodeList->getSize(); i++)
	{
		VMeshLocalNode* p = myNodeList->getNode(i);
		if ((p!=NULL)&&(myStorage->isSegmentComplete(p->getSegmentID())))
		{
			PeerInfo info;
			info.ip = p->getIP();
			info.port = p->getPort();
			info.isBootstrap = p->isBootstrap();
			info.segmentID = p->getSegmentID();
			info.pingTime = 0;
			info.isConnected = MYSELF;
			myPeerInfoList.push_front(info);
		}
	}
	
	return myPeerInfoList;
	
}
int VMesh::getSeekResolution()
{
	return SEEK_RESOLUTION;
}

_VMESH_NAMESPACE_FOOTER
