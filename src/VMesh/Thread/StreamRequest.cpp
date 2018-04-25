#include "../Thread/StreamRequest.h"
//#include "../VivaldiAdaptor/VivaldiAdaptor.h"

using namespace std;
using namespace log4cxx;
using App::ChordApp;
using DHT::DHTNetworkID;
using DHT::Node;

_VMESH_NAMESPACE_HEADER

StreamRequest::StreamRequest (string _myIP, int _myPort, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, string _hash, int _maxConnection, int p_identity, int _exchangeNeighbor)
: mutexLock("StreamRequest Lock")
{
	myIP = _myIP;
	myPort = _myPort;
	myMediaInfo = _info;
	myNodeList = _list;
	myStorage = _storage;
	myBuffer = _buffer;
	mySegment_ = _segment;
	keepRunning = true;
		
	myHash = _hash;
	//myParentList = new VMeshRemoteNodeList();
	myExchangeTable=new ExchangeTable();
	
	myQueue = new SchedulerQueue();
	for (int i=0;i<MAX_STREAM_PARENTS; i++)
		requestSlot[i] = NULL;

	isPacketFetched = false;
	counter = 0;
	inActiveCounter = 0;
	isSuperNode = false;

	maxConnection = _maxConnection;
	peer_identity = p_identity;
	exchangeNeighbor = _exchangeNeighbor;

	//myVivaldiAdaptor = new VivaldiAdaptor("..\\..\\..\\Vivaldi\\peers.txt");
	//myVivaldiAdaptor = NULL;
	//myVivaldiAdaptor = new VivaldiAdaptor((string) STORAGEFILE_PATH + "peers.txt");
}

StreamRequest::~StreamRequest()
{
	// LOG_DEBUG("StreamRequest Destructor is called");
	for (int i=0;i<MAX_STREAM_PARENTS; i++)
	{
		if (requestSlot[i]!=NULL)
		{
			if (requestSlot[i]->getJoinableStatus())
			{
				requestSlot[i]->stop();
			}
			delete requestSlot[i];
			requestSlot[i] = NULL;
			// LOG_DEBUG("Thread PacketRequest " + itos(i) + " is killed");
		}
	}
	if (myQueue!=NULL)
	{
		delete myQueue;
		myQueue = NULL;
	}
	if (myExchangeTable!=NULL)
	{
		delete myExchangeTable;
		myExchangeTable = NULL;
	}
	/*if (myVivaldiAdaptor!=NULL)
	{
		delete myVivaldiAdaptor;
		myVivaldiAdaptor = NULL;
	}*/
}

bool StreamRequest::addBootstrap(string _IP, int _Port)
{
	VMeshRemoteNode* p = new VMeshRemoteNode(_IP, _Port, INITIAL_SEGMENTID, peer_identity);
	
	// connect to the boostrap peer
	int tryconnect = 0;
	while (1)
	{
		if (p->connectControlSocket())
		{
			if (p->pingControlSocket())
				break;
			else
				return false;
		}
		else
			return false;
	}

	// retrieve all the local segment node of the boostrap peer
	VMeshRemoteNodeList* returnList = retrieveLocalNode(p->getControlSocketClient());

	p->closeControlSocket();

	if (returnList == NULL)
	{
		delete p;
		return false;
	}
	
	int nodesAdded = addToParent(returnList, true);

	if ( returnList != NULL )
		delete returnList;
	delete p;
	return (nodesAdded>0);
}

void StreamRequest::addPacket(unsigned int _PacketID)
{
	// check if the packet is myStorage
	if (myStorage->existPacket(_PacketID/1000000000, _PacketID%1000000000))
		return;

	// check if the packet is in myBuffer
	// if not, then request this packet from other peers
	else if (!myBuffer->write(_PacketID, NULL, 0))
	{
		myQueue->fetchToBuffer(_PacketID);
	}
}

void StreamRequest::store(unsigned int _PacketID)
{
	myQueue->fetchToStorage(_PacketID);
}

void StreamRequest::clearQueueToBuffer()
{
	myQueue->clearAllToBuffer();
}



ExchangeTable * StreamRequest::getExchangeTable()
{
	return myExchangeTable;
}

/*
VMeshRemoteNodeList* StreamRequest::getParentList()
{
	return myParentList;
}
*/

void StreamRequest::stop()
{
	keepRunning = false;
	this->join();
}

// add a DHT node to the node list
int StreamRequest::addDHTNode(NetworkID* dhtNode)
{
	if (dhtNode==NULL)
		return 0;

#ifdef CHORD_LOOKUP
	ChordNetworkID* chordNode = (ChordNetworkID*) dhtNode;
	string movieHash = chordNode->genDHTNetworkID().getIDHex();
#elif defined(CENTRAL_LOOKUP)
	CentralLookupNetworkID* lookupNode = (CentralLookupNetworkID*) dhtNode;
	string movieHash = lookupNode->genDHTNetworkID().getIDHex();
#elif defined(TABLE_EXCH_LOOKUP)
	TableExchangeLookupNetworkID* teNode = (TableExchangeLookupNetworkID*) dhtNode;
	string movieHash = teNode->genDHTNetworkID().getIDHex();
#endif
	
	if (movieHash.substr(0, 2*MEDIAINFOHASH_LENGTH)!=myHash)
	{
		LOG_ERROR((string)"The node is of another movie (" + movieHash.substr(0, MEDIAINFOHASH_LENGTH*2) + ")");
		//SetEvent(m_hFailed);
		return 0;
	}

	VMeshRemoteNode* p = new VMeshRemoteNode(dhtNode->getIP(), dhtNode->getPort(), dhtNode->getSegment(), dhtNode->getPeerIdentity());
#ifdef CHORD_LOOKUP
	bool deleteNode = ((p->getIP()==myIP) && (p->getPort()==myPort)) || (myParentList->exist(p));
#elif defined(CENTRAL_LOOKUP)
	//bool deleteNode = ((lookupNode->getIP()==myIP) && (lookupNode->getPort()==myPort)) || (myParentList->exist(p)) || !(p->connect() && p->ping());
	bool deleteNode = ((lookupNode->getIP()==myIP) && (lookupNode->getPort()==myPort)) || !(p->connectControlSocket() && p->pingControlSocket());
	p->closeControlSocket();
#elif defined(TABLE_EXCH_LOOKUP)
	bool deleteNode = false;
#endif
	
	if (deleteNode)
	{
		delete p;
		return 0;
	}
	else
	{
		LOG_DEBUG((string)"Added => " + (p->toString(false)));
		//myParentList->addNode(p);
		myExchangeTable->insert(p);
		p = NULL;
		return 1;
	}
}

int StreamRequest::addParentsbyDHT(int _segmentID)
{
	int nodesAdded = 0;

	LOG_DEBUG(">> Segment lookup ["+ itos(_segmentID) +"] through LookupManager...");
	int hopCount = 10;

	// myChord->print(LogNormal);
	// cout << "*****" << ID.getIDHex() << endl;

#ifdef CHORD_LOOKUP
	NetworkID* nodeId = new ChordNetworkID(myIP, myPort, _segmentID, myHash);
#elif defined(CENTRAL_LOOKUP)
	//NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, _segmentID, myHash);
	NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, _segmentID, myHash, peer_identity);
#elif defined(TABLE_EXCH_LOOKUP)
	NetworkID* nodeId = new TableExchangeLookupNetworkID(myIP, myPort, segmentID, myHash);
#endif

	list<NetworkID*> searchPath;



	// Jack's comment::Call central lookup manager to send "NODELOOKUP" action to central lookup server
	NetworkID* resultNode = mySegment_->lookupFullPath(*nodeId, searchPath, hopCount);

	for(list<NetworkID*>::iterator it = searchPath.begin(); it != searchPath.end(); it++)
	{
			// For printout
		//stringstream ss();
		//stringstream ss2();

		std::cout << "StreamRequest::Node Lookup from central lookup server::\t" << (*it)->getIP().c_str() << ":" << (*it)->getPort() << "::" << (*it)->getPeerIdentity() << "\n";
		nodesAdded += addDHTNode(*it);
	}

	if (resultNode != NULL)
	{
		nodesAdded += addDHTNode(resultNode);

		list<NetworkID*> targetNeighbour = mySegment_->getNeighbor(*resultNode);
		for (list<NetworkID*>::iterator it=targetNeighbour.begin() ; it != targetNeighbour.end(); it++ )
		{
			nodesAdded += addDHTNode(*it);
			delete *it;
		}

		delete resultNode;
		resultNode = NULL;
	}
	else
	{
		//cout<<"	(NULL)"<<endl;
		LOG_DEBUG("Lookup parents fails: Segment:" + nodeId->getSegment());
		hopCount = -1;
	}
	LOG_DEBUG(">> Lookup Ends");
	return nodesAdded;
}


// Table Exchange 
// gossiping with neighbors to get segments
//TODO: update the local node via the method
int StreamRequest::addParentsbyNeighbour(int _segmentID)
{
	LOG_DEBUG(">>Table Exchange...");
	
	// search the parent from the closest to the farmost
	int nodesAdded = 0;
	//myParentList->sortPingTime();
	VMeshRemoteNodeList* neighborlist=myExchangeTable->getNearestNeighbor(exchangeNeighbor);



	for (int i = 0; i < neighborlist->getSize(); i++)
	{
		// get a neighbor node first
		VMeshRemoteNode *p = neighborlist->getNode(i);
		if (p==NULL)
			break;
		
		bool tryNode = true;
		/*
		if (p->getSegmentID()!=INITIAL_SEGMENTID)
			if ((p->getSegmentID()<_segmentID-1) || (p->getSegmentID()>_segmentID+1))
				tryNode = false;
		*/
		if(p->getIP()==myIP&&p->getPort()==myPort)
			tryNode=false;
		if(p->getSegmentID()==INITIAL_SEGMENTID)
			tryNode=false;
		if (p->isBootstrap())
			tryNode = false;
		if (!tryNode)
			continue;

		LOG_DEBUG(">> Asking " + p->toString());
		//ping if the socket is connected already
		//if(!p->pingControlSocket() || p->getControlSocketPingTime()==PING_FAIL)
		//{
			int retry=0;
			while (retry<VMESH_CONNECT_RETRY)
			{
				if(p->connectControlSocket())
				{
					if(p->pingControlSocket())
					{
						//supply my RemoteNodeList and LocalNodeList to this node
						if(!(peer_identity==CLIENT_IDENTITY&&p->getPeerIdentity()==PROXY_IDENTITY))
						{
							int serial_len = 0;
							char* serialized=myExchangeTable->serialize(serial_len);
							char* send_msg=new char[serial_len+1];
							send_msg[0] = SUPPLY_LOCALNODELIST;
							memcpy(send_msg+1, serialized, serial_len);
							SocketClient* cli = p->getControlSocketClient();
							bool send_result=false;
							send_result=cli->send(send_msg, serial_len+1);
							STAT_AGGREGATE(UP_CONTROL, serial_len + 1);
							delete [] send_msg;
							delete [] serialized;
						}
						// ask the node for the parent nodes and local nodes
						if(!(peer_identity==PROXY_IDENTITY&&p->getPeerIdentity()==CLIENT_IDENTITY))
						{
							VMeshRemoteNodeList* returnList=retrieveParentList(p->getControlSocketClient(), _segmentID);		
							int justAdded = addToParent(returnList, true);
							nodesAdded += justAdded;
						}

						p->clrSocketUsing();
						p->closeControlSocket();

						break;
					}
				}
				retry++;
			}
			
			if(retry>=VMESH_CONNECT_RETRY)
			{

				//p->clrSocketUsing();
				myExchangeTable->reduceAccessCount(p);
				if ((p!=NULL) && (!p->isBootstrap()))
				{
					LOG_DEBUG((string)"Removed => " + (p->toString(false)) + " in StreamRequest");
					
					myExchangeTable->removeNode(p);
					//delete p;
					p = NULL;
				}
				else if ((p!=NULL) && (p->isBootstrap()))
				{
					LOG_ERROR("SR (" + itos((int)threadHandle%1000) +")\t: Error! Server is dead!");
					delete p;
					p = NULL;
					break;
				}

				continue;
			}

		//}
		
		
	
		myExchangeTable->reduceAccessCount(p);
		p=NULL;
		if (nodesAdded >= MAX_PEERS)
			break;
	}


	delete neighborlist;
	neighborlist=NULL;
	LOG_DEBUG(">> Lookup Ends");
	return nodesAdded;
}

// retrieve the local nodes of a particular Peer through the SocketClient
VMeshRemoteNodeList* StreamRequest::retrieveLocalNode(SocketClient* cli)
{
	if (cli==NULL)
		return NULL;

	// ask the node for the local nodes
	char sendmsg = (char)REQ_LOCALNODELIST;
	char* recvmsg;
	int recvlen;
	if (!cli->send(&sendmsg, 1))
	{
		LOG_DEBUG("StreamRequest\t: Send Local Node Request error");
		return NULL;
	}
	else
	{
		STAT_AGGREGATE(UP_CONTROL, 1);
	}
	if ((recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT)) <= 0)
	//if ((recvlen = cli->receive(recvmsg, -1)) <= 0)
	{
		LOG_DEBUG("StreamRequest\t: Receive Local Node Request error");
		return NULL;
	}
	else
	{
		STAT_AGGREGATE(DOWN_CONTROL, recvlen);
	}
	if (recvmsg[0]!=(char)RPY_LOCALNODELIST_OK)
	{
		LOG_DEBUG("StreamRequest\t: Receive Unknown Local Node Reply");
		delete [] recvmsg;
		return NULL;
	}
	VMeshRemoteNodeList* returnList = new VMeshRemoteNodeList(recvmsg+1, recvlen-1);
	delete [] recvmsg;
	return returnList;
}

VMeshRemoteNodeList* StreamRequest::retrieveParentList(SocketClient* cli, int _segmentID)
{
	if (cli==NULL)
		return NULL;
	// ask the node for the parent nodes
	char sendmsg[4];
	sendmsg[0] = (char)REQ_PARENTLIST;
	sendmsg[1] = _segmentID / 256;
	sendmsg[2] = _segmentID % 256;
	sendmsg[3] = peer_identity;
	char* recvmsg;
	int recvlen;
	if (!cli->send(sendmsg, 4))
	{
		LOG_DEBUG("StreamRequest\t: Send Parent List Request error");
		return NULL;
	}
	else
	{
		STAT_AGGREGATE(UP_CONTROL, 4);
	}
	if ((recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT)) <= 0)
	//if ((recvlen = cli->receive(recvmsg, -1)) <= 0)
	{
		LOG_DEBUG("StreamRequest\t: Receive Parent List Request error");
		return NULL;
	}
	else
	{
		STAT_AGGREGATE(DOWN_CONTROL, recvlen);
	}
	if (recvmsg[0]!=(char)RPY_PARENTLIST_OK)
	{
		LOG_DEBUG("StreamRequest\t: Receive Unknown Parent List Reply");
		delete [] recvmsg;
		return NULL;
	}
	VMeshRemoteNodeList* returnList = new VMeshRemoteNodeList(recvmsg+1, recvlen-1);
	//myExchangeTable->insert(recvmsg+1,recvlen-1);
	delete [] recvmsg;
	return returnList;
}

int StreamRequest::addToParent(VMeshRemoteNodeList* &list, bool needSync)
{
	if (list == NULL)
		return 0;
	if (list->getSize()==0)
	{
		list->clearall(); 
		delete list;
		list = NULL;
		return 0;
	}
	
	VMeshRemoteNode *q = list->getNode(0);

	int nodesAdded = 0;
	while (list->getSize()>0)
	{
		q = list->getNode(0);
		//LOG_DEBUG((string)"Received: " + q->toString());
		if ((q!=NULL) && ((q->getIP()!=myIP) || (q->getPort()!=myPort))  && q->connectControlSocket())
		{
			LOG_DEBUG((string)"Added => " + (q->toString(false)));
			//myParentList->addNode(q);
			q->pingControlSocket();
			q->closeControlSocket();
			myExchangeTable->insert(q);	
			nodesAdded++;
		}
		list->sharrowRemoveNode(q);
	}
	delete list;
	list = NULL;
	return nodesAdded;
}


// register a finished segment to the DHT
void StreamRequest::checkSegment()
{
	if (myMediaInfo == NULL)
		return;

	for (int i=0; i<myMediaInfo->getNumSegments(); i++)
	{	
		if ((myNodeList->getNodeBySegmentID(i)==NULL) && (myStorage->isSegmentComplete(i)))
		{
			/*
			// remove the key of INITIAL_SEGMENTID in chord
			if (myNodeList->getSize()==0)
			{
				unsigned char key[MEDIAINFOHASH_LENGTH + SEGMENTID_LENGTH + COORDINATE_LENGTH];	//10
				
				for (int i=0; i<MEDIAINFOHASH_LENGTH; i++)
				{
					if (myHash[2*i]<='9')
						key[i] = myHash[2*i] - '0';
					else
						key[i] = myHash[2*i] - 'a' + 10;
					key[i] *= 16;
					if (myHash[2*i+1]<='9')
						key[i] += myHash[2*i+1] - '0';
					else
						key[i] += myHash[2*i+1] - 'a' + 10;
				}

				key[MEDIAINFOHASH_LENGTH] = INITIAL_SEGMENTID / 256;
				key[MEDIAINFOHASH_LENGTH + 1] = INITIAL_SEGMENTID % 256;
				
				const char* temp = myIP.c_str();
				sscanf(temp, "%d %*c %d %*c %d %*c %d", &key[MEDIAINFOHASH_LENGTH + 2], &key[MEDIAINFOHASH_LENGTH + 3], &key[MEDIAINFOHASH_LENGTH + 4], &key[MEDIAINFOHASH_LENGTH + 5]);
				DHTNetworkID ID;
				ID.setID((unsigned char*)key);
				myChord->removeNode(ID);
			}
			*/

			VMeshLocalNode* node = new VMeshLocalNode(myMediaInfo, myIP, myPort, i, peer_identity);
#ifdef CHORD_LOOKUP
			NetworkID* nodeId = new ChordNetworkID(myIP, myPort, i, myMediaInfo->getMediaHash());
#elif defined(CENTRAL_LOOKUP)
			//NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, i, myMediaInfo->getMediaHash());
			NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, i, myMediaInfo->getMediaHash(),peer_identity);
#elif defined(TABLE_EXCH_LOOKUP)
			NetworkID* nodeId = new TableExchangeLookupNetworkID(myIP, myPort, i, myMediaInfo->getMediaHash());
#endif

			mySegment_->addNode(*nodeId);
			delete nodeId;

			node->setDHTRegistered();
			myNodeList->addNode(node);
			/*
			*
			*/
			myExchangeTable->insert(new VMeshRemoteNode(*node));
			LOG_INFO ((string)"StreamRequest\t: Finish storing segment " + itos(i));
		}
	}
}


void StreamRequest::retrieveMediaInfo()
{
	if (myMediaInfo!=NULL)
		return;

	VMeshRemoteNode* node;

	// try to fetch MediaInfo for 5 times
	for (int times = 0; times<5; times++)
	{
		switch (times)
		{
		case 0:
			// use the first node
			//node = myParentList->getNode(0);
			node = myExchangeTable->getNodeByIndex(0);
			break;
		case 1:
			// use a node of segment 0
			addParentsbyDHT(0);
			//node = myParentList->getSegmentNode(0);
			node = myExchangeTable->getNodeByIndex(0);
			break;
		case 2:
		case 3:
			// default bootstrap node
			//node = myParentList->getSegmentNode(BOOTSTRAP_SEGMENTID);
			node=myExchangeTable->getOneParentBySegID(BOOTSTRAP_SEGMENTID,NULL);
			break;
		case 4:
			// find bootstrap nodes again
			addParentsbyDHT(BOOTSTRAP_SEGMENTID);
			//node = myParentList->getSegmentNode(BOOTSTRAP_SEGMENTID);
			node=myExchangeTable->getOneParentBySegID(BOOTSTRAP_SEGMENTID,NULL);
			break;
		default:
			sleep(times);
			if (myExchangeTable->getSize()==0)
				addParentsbyDHT(BOOTSTRAP_SEGMENTID);
			node = myExchangeTable->getOneParentBySegID(BOOTSTRAP_SEGMENTID,NULL);
			break;
		}
		LOG_DEBUG ((string)"StreamRequest\t: Fetch MediaInfo : Trial " + itos(times+1));
		if (node==NULL)
		{
			//LOG_DEBUG("Node is NULL");
			continue;
		}

		fetchMediaInfo(node);

		if (myMediaInfo == NULL)
		{
			// fetch fails
			if ( !node->isBootstrap() )
			{
				myExchangeTable->removeNode(node);
				node=NULL;
			}
		}
		else
		{
			// fetch success
			//LOG_DEBUG("fetch success");
			return;
		}
	}

	return;
}


void StreamRequest::fetchMediaInfo(VMeshRemoteNode* node)
{
	// try to connect to the remote node
	int retry=0;
	while (true)
	{
		if (node->connectControlSocket())
			if (node->pingControlSocket())
				break;
		if((++retry)>=VMESH_CONNECT_RETRY)
			return;
	}
	
	
	SocketClient* cli = node->getControlSocketClient();

	if (cli==NULL)
		return;

	char* msg1 = new char[myHash.size() + 1];
	msg1[0] = (char)REQ_MEDIAINFO;
	memcpy(msg1+1, myHash.c_str(), myHash.size());
	cli->send(msg1, (int)myHash.size() + 1);
	STAT_AGGREGATE(UP_CONTROL, (int)myHash.size() + 1);

	char* msg2;
	int recvlen = cli->receive(msg2, VMESH_REPLY_TIMEOUT);

	STAT_AGGREGATE(DOWN_CONTROL, recvlen);
	delete [] msg1;
	//node->clrSocketUsing();
	node->closeControlSocket();
	if (recvlen<=0)
	{
		// receive error
		return;
	}
	else if (msg2[0]==RPY_MEDIAINFO_OK)
	{
		// construct the MediaInfo
		myMediaInfo = new MediaInfo(msg2 + 1, recvlen - 1);
		delete [] msg2;
		return;
	}
	else
	{
		// received but fails
		return;
	}
}


MediaInfo* StreamRequest::getMediaInfo()
{
	LOG_DEBUG("StreamRequest\t: Started");

	// if MediaInfo is not ready, then fetch the MediaInfo first
	if (myMediaInfo==NULL)
	{
		retrieveMediaInfo();
		
		// if MediaInfo is fail to fetch
		if (myMediaInfo == NULL)
		{
			LOG_ERROR("MediaInfo is failed to receive");
			return NULL;
		}
	
		LOG_INFO("MediaInfo is received successfully");
		LOG_DEBUG((string)"Movie Name    : " + myMediaInfo->getMediaName());
		LOG_DEBUG((string)"Size          : " + itos(myMediaInfo->getFileSize()));
		LOG_DEBUG((string)"Hash          : " + string(myMediaInfo->getMediaHash(), MEDIAINFOHASH_LENGTH*2));
		LOG_DEBUG((string)"# of segments : " + itos(myMediaInfo->getNumSegments()));
		PeerStorage* ps = (PeerStorage*)myStorage;
		ps->setMediaInfo(myMediaInfo);
	
	}
	
	return myMediaInfo;
}



void* StreamRequest::execute()
{
	int i;

	// duplicate the bootstrap node for more connections to increase speed
	//VMeshRemoteNode* bsNode = myParentList->getSegmentNode(BOOTSTRAP_SEGMENTID);
	VMeshRemoteNode* bsNode = myExchangeTable->getOneParentBySegID(BOOTSTRAP_SEGMENTID,NULL);
	int streamRate = myMediaInfo->getFileSize() * 8.0 /myMediaInfo->getMediaLength(0); //in kbps
	//int numBSDuplicate = streamRate / ((250.0 * 150.0) / (2*bsNode->getPingTime())); // 250ms RTT => 150kbps throughput (assumed inversely proportional)
	int numBSDuplicate = MULTIPLE_BOOTSTRAP_CONNECTION;

	numBSDuplicate = min(numBSDuplicate, MAX_STREAM_PARENTS);
	numBSDuplicate = max(numBSDuplicate, 1);
	LOG_DEBUG("BS pingtime = " + itos(2*bsNode->getPingTime()) + "ms; streaming rate = " + itos(streamRate) + "kbps; # connections = " + itos(numBSDuplicate));	
	for ( int i = 0; i < numBSDuplicate - 1; ++i) 
	{
		VMeshRemoteNode* bsNodeCopy = new VMeshRemoteNode(bsNode->getIP(), bsNode->getPort(), true, SERVER_IDENTITY);
		bsNodeCopy->setSegmentID(INITIAL_SEGMENTID);

		// Jack's comment::Parent List add Node
		//this->myParentList->addNode(bsNodeCopy);
		this->myExchangeTable->insert(bsNodeCopy);
		bsNodeCopy->setSegmentID(BOOTSTRAP_SEGMENTID);
		//LOG_DEBUG("Added one more BS Node. total node = " + itos(myParentList->getSize()));
	}


	if (!keepRunning)
		return NULL;

	// the MediaInfo is ready, then fetch packets
	// create the PacketRequest threads and launch them
	// The maximum request slot = 5, each request slot holds a Packet Request
	for (i=0;i<MAX_STREAM_PARENTS; i++)
	{
		requestSlot[i] = new PacketRequest(myMediaInfo, myQueue, myExchangeTable, myNodeList, myBuffer, myStorage, maxConnection);
		requestSlot[i]->run();
		usleep(100000);		// wait 0.1s
	}

	//belows are for regular maintenance
	int whichLocalNode = -1;
	VMeshLocalNode* myLocalNode = NULL;
	int seconds = -1;	// counter (per 10 minutes)
	// keep track of the last 2 segmentID so that the same segmentID would not be searched frequently
	int lastDesiredSegment[2] = {BOOTSTRAP_SEGMENTID, BOOTSTRAP_SEGMENTID};
	int justAdded;
	while (keepRunning)
	{
		sleep(1);
		seconds++;
		seconds %= 360000;

		// reset the segmentID, so that the same segmentID can be searched again
		if (seconds%CLEAR_LOOKUP_HISTORY_INTERVAL==0)
		{
			lastDesiredSegment[0] = -1;
			lastDesiredSegment[1] = -1;
		}

		// display the parent list
		if (seconds%CONSOLE_SHOW_PARENT_LIST_INTERVAL == 0)
		{
			// output Parent List for Vivaldi
			/*if (myVivaldiAdaptor != NULL)
			{
				string *ipList = new string[myParentList->getSize()];
				int *portList = new int[myParentList->getSize()];
				int numAddress = 0;

				for (int i = 0; i < myParentList->getSize(); i++)
				{
					VMeshRemoteNode* parentNode = myParentList->getNode(i);

					if (parentNode != NULL)
					{
						ipList[numAddress] = parentNode->getIP();
						portList[numAddress] = parentNode->getPort()+500;

						++numAddress;					
					}
				}

				//myVivaldiAdaptor->writeFile(ipList, portList, numAddress);

				delete [] ipList;
				delete [] portList;

				ipList = NULL;
				portList = NULL;
			}*/

			// display parent list
			LOG_INFO((string)"/========= ParentList: " + itos(myExchangeTable->getSize())+" =========\\");
			myExchangeTable->showAllBySegmentID();
			//myParentList->showall();

			// display local cached segments
			string local = "{ ";
			for (int i=0; i<myNodeList->getSize(); i++)
				local = local + itos(myNodeList->getNode(i)->getSegmentID()) + " ";
			local = local + "}";
			char buffer_size[20];
			sprintf(buffer_size, "%.1f", 0.05 + 100.0*(myBuffer->getSize()/(double)myBuffer->getCapacity()));
			LOG_INFO("Segment  : " + local);
			LOG_INFO("Buffer   : " + string(buffer_size) + "% of " + itos(myBuffer->getCapacity()/1048576) + " MB, " + itos(myBuffer->getElementCount()) + " packets");
			LOG_INFO("Fetching Packets: " + itos(myQueue->getFetchingSize()));
			LOG_INFO("Waiting Packets : " + itos(myQueue->getWaitingSize()));
			stat_data curStat = STAT_RETRIEVE(false);
			LOG_INFO("Uplink (Control/Data): " + itos(curStat.uplink_control) + "/" + itos(curStat.uplink_data));
			LOG_INFO("Downlink (Control/Data): " + itos(curStat.downlink_control) + "/" + itos(curStat.downlink_data));
			
			// PacketRequest threads status		
			for (int i=0; i<MAX_STREAM_PARENTS; i++)
			{				
				if (requestSlot[i]->getState()==NORMAL)
				{
					VMeshRemoteNode* parentNode = requestSlot[i]->getNode();
					if (parentNode!=NULL)
						LOG_INFO("Retrieving from: " + parentNode->toString(false));
				}
				/*
				else if (requestSlot[i]->getState()==NO_PARENT)
				{
					LOG_INFO("Not enough peers for segment " + itos(requestSlot[i]->getDesiredSegment()));
				}
				*/
			}
			LOG_INFO((string)"\\=================================/");
		}		

		// show active parent in every second (for monitoring)
		if (seconds%1==0)
		{
			string peerList = "[P] ";
			bool hasParent = false;
			// PacketRequest threads status
			for (int i=0; i<MAX_STREAM_PARENTS; i++)
			{
				if (requestSlot[i]->getState()==NORMAL)
				{
					VMeshRemoteNode* parentNode = requestSlot[i]->getNode();
					if (parentNode!=NULL)
					{
						hasParent = true;
						//LOG_INFO("Retrieving from: " + parentNode->toString(false));
						peerList = peerList + parentNode->getIP();
						peerList = peerList + ";";
					}
				}
				/*
				else if (requestSlot[i]->getState()==NO_PARENT)
				{
					LOG_INFO("Not enough peers for segment " + itos(requestSlot[i]->getDesiredSegment()));
				}
				*/
			}
			if (hasParent)
				LOG_DEBUG(peerList);
		}

		checkActivity();

		// register keys into DHT if a segment is finished
		checkSegment();

		// periodically refresh the DHT key
		// to prevent timeout in central lookup server, refer to NODE_TIMEOUT in Common.h of centralLookupServer
		if (seconds % REG_SEGMENT_INTERVAL == 0)
			for (int i = 0; i < myNodeList->getSize(); ++i)
			{
				int segID_ = myNodeList->getNode(i)->getSegmentID();
#ifdef CHORD_LOOKUP
				NetworkID* nodeId = new ChordNetworkID(myIP, myPort, segID_, myMediaInfo->getMediaHash());
#elif defined(CENTRAL_LOOKUP)
				//NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, segID_, myMediaInfo->getMediaHash());
				NetworkID* nodeId = new CentralLookupNetworkID(myIP, myPort, segID_, myMediaInfo->getMediaHash(),peer_identity);
#elif defined(TABLE_EXCH_LOOKUP)
				NetworkID* nodeId = new TableExchangeLookupNetworkID(myIP, myPort, segID_, myMediaInfo->getMediaHash());
#endif
				mySegment_->addNode(*nodeId);
				delete nodeId;
				LOG_INFO ((string)"StreamRequest\t: Registered segment " + itos(segID_) + " (period = " + itos(REG_SEGMENT_INTERVAL) + ")");
			}

		// regular update my neighbouring segment list (i.e. prev, cur, nex seg list)
		if (seconds%REFRESH_ADJACENT_LIST_INTERVAL == 0)
		{
			
			if(peer_identity==PROXY_IDENTITY)
			{
				justAdded = addParentsbyNeighbour(PROXY_SEGMENTID);
				LOG_DEBUG(itos(justAdded)+"  neighbor nodes have be found by exchanging adjacent lists...");
			}
			else if(peer_identity==CLIENT_IDENTITY)
			{
				justAdded = addParentsbyNeighbour(PROXY_SEGMENTID);
				LOG_DEBUG(itos(justAdded)+"  neighbor nodes have be found by exchanging adjacent lists...");
			}
			
				/*
				for (i=0;i<myNodeList->getSize(); i++)
				{
					justAdded = addParentsbyNeighbour(myNodeList->getNode(i)->getSegmentID());
					if (justAdded < MAX_STREAM_PARENTS)
						// Jack's comment::Through central lookup manager to update 3 segment lists
						addParentsbyDHT(myNodeList->getNode(i)->getSegmentID());
				}
				*/
		}

		// check if there are enough parents
		for (i=0;i<MAX_STREAM_PARENTS; i++)
		{
			//int aaaaa=requestSlot[i]->getState();
			if ((requestSlot[i]->getState()==NO_PARENT)||(requestSlot[i]->getState()==SERVER))
			{
				if((requestSlot[i]->getDesiredSegment()!=lastDesiredSegment[0]) &&
						(requestSlot[i]->getDesiredSegment()!=lastDesiredSegment[1]))
				{
					lastDesiredSegment[1] = lastDesiredSegment[0];
					lastDesiredSegment[0] = requestSlot[i]->getDesiredSegment();
					//justAdded = addParentsbyNeighbour(lastDesiredSegment[0]);
					//if (justAdded < MAX_STREAM_PARENTS)
						addParentsbyDHT(lastDesiredSegment[0]);
					//break;
				}
			}
		}
	}

	return NULL;
}

void StreamRequest::checkActivity()
{
	int i;
	int newTransferRate=0;

	if ( isSuperNode == false )
	{
		mutexLock.lock();
		if ( isPacketFetched )
		{
			for ( i = 0; i < MAX_STREAM_PARENTS; i++ )
				newTransferRate = requestSlot[i]->increaseTransferRate( INCREASE_STEP );

			inActiveCounter = 0;
			isPacketFetched = false;
		}
		else
			inActiveCounter++;

		mutexLock.release();

		if ( inActiveCounter >= INACTIVE_COUNT_PERIOD )
		{
			inActiveCounter = 0;

			for ( i = 0; i < MAX_STREAM_PARENTS; i++ )
				newTransferRate = requestSlot[i]->decreaseTransferRate( INCREASE_STEP );
		}
	}
	
	counter++;
	if ( counter >= CONSOLE_SHOW_PARENT_LIST_INTERVAL )
	{
		counter = 0;
		if ( (MAX_STREAM_PARENTS > 0) && (requestSlot[0]->isActive()) )
		{
			printf("Transfer rate of segment packets: Unlimited \n");
			printf("Transfer rate of playback packets: Unlimited \n");
		}
		else
		{
			//printf("Transfer rate of segment packets: %d KB/s\n", DEFAULT_MIN_TRANSFER_RATE / 1024);
			//printf("Transfer rate of playback packets: %d KB/s\n", DEFAULT_MIN_TRANSFER_RATE / 1024);

			printf("Transfer rate of playback packets: %lf KB/s\n", newTransferRate / 1024);
			printf("Transfer rate of playback packets: %lf KB/s\n", newTransferRate / 1024);
		}	
	}
}

void StreamRequest::notifyPacketRequest()
{
	mutexLock.lock();
	isPacketFetched = true;
	mutexLock.release();
}

void StreamRequest::setSuperNode()
{
	isSuperNode = true;
}

_VMESH_NAMESPACE_FOOTER
