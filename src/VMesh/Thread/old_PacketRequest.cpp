#include "../Thread/PacketRequest.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

PacketRequest::	PacketRequest (MediaInfo* _mediaInfo, SchedulerQueue* _queue, ExchangeTable* _exchangeTable, VMeshLocalNodeList* _myList, Buffer* _buffer, Storage* _storage, int _maxConnection)
{
	srand(time(0));
	int i;
	for ( i = 0 ; i < 10 ; i++ )
		timeTaken[i] = 0;
	myMediaInfo = _mediaInfo;
	myQueue = _queue;
	//myParentList = _parentList;
	myNodeList = _myList;
	myBuffer = _buffer;
	myStorage = _storage;
	myState = NO_PACKET;
	desiredSegment = BOOTSTRAP_SEGMENTID;
	waitingRelease = false;
	myNode = NULL;
	myRemoteNodeList=NULL;
	keepRunning = true;
	bActive = true;
	nInactiveCount = 0;
	transferRate = DEFAULT_MIN_TRANSFER_RATE;

	maxConnection = _maxConnection;
	connectionCounter = 0;

	myExchangeTable = _exchangeTable;
}

PacketRequest::~PacketRequest()
{
	// blank destructor
	#ifdef PR_DEBUG
		LOG_DEBUG("PacketRequest Destructor is called");
	#endif
}

PacketRequestState PacketRequest::getState()
{
	return myState;
}

int PacketRequest::getDesiredSegment()
{
	return desiredSegment;
}

void PacketRequest::stop()
{
	keepRunning = false;
	this->join();
	while (this->getState()!=ENDED)
		usleep(10000);
}


void* PacketRequest::execute()
{
	/*indicates if measure time comsuption in each step*/
	bool bTimeLog = false; //( PacketID % 1000 == 0 );

	char sendmsg[6];
	char* recvmsg;
	int recvlen;
	char msg[100];
	int PacketSeg;
	SchedulerEntry entry;
	unsigned int PacketID;
	char PacketDest = 'x';
	myNode = NULL;
	myState = NO_PACKET;


	VMeshRemoteNodeList*failNodes=new VMeshRemoteNodeList();
	int requestRetry=0;
	int getNodeRetry=0;
	int connectRetry=0;

	/*begins the main loop*/
	while (keepRunning)
	{
		if (!keepRunning)
			break;//just in case
			
		/*begins pops a packet in the waiting queue*/
		while (myQueue->empty())
		{
			myState = NO_PACKET;
			if (myNode!=NULL)
			{
				//myNode->clrSocketUsing();
				myExchangeTable->reduceAccessCount(myNode);
				myExchangeTable->insert(myNode);
				myNode = NULL;
			}
			if (!keepRunning)
				break;
			usleep(MAX_STREAM_PARENTS * 100000);// wait MAX_STREAM_PARENTS*0.1 s
		}
		entry = myQueue->pop();
		if (entry.dest=='x')
		{
			continue;
		}
		else
		{
			PacketID = entry.PacketID;
			PacketDest = entry.dest;
		}
		if (!keepRunning)
			break;
		int packetID = PacketID % 1000000000;
		// see which segment is this packet belongs
		PacketSeg = myMediaInfo->getSIDbyPID(PacketID%1000000000, PacketID/1000000000);
		if (PacketSeg==-1)
		{
			// wrong segment number for this packet ID
			continue;
		}
		desiredSegment = PacketSeg;
		/*ends pops a packet in the waiting queue*/
		

		#ifdef PR_DEBUG
			LOG_DEBUG((string)"PR (" + itos((int)threadHandle%1000) +")\t: Trying to get Packet " + itos(PacketID));
		#endif
		myState = NORMAL;
		/*begins check if there is already a node can handle this packet*/
		if (myNode!=NULL)
		{
			bool throwNode = true;
			if (myNode->getSegmentID()==PacketSeg)
				throwNode = false;
			if (myNode->getSegmentID()==BOOTSTRAP_SEGMENTID)
			{
				if(myExchangeTable->getParentBySegID(PacketSeg)->getSize()==0)
				{
					throwNode=false;
					myState=SERVER;
				}
				else
					throwNode=true;
			
			}
			if (throwNode)
			{
				//myNode->clrSocketUsing();
				myExchangeTable->reduceAccessCount(myNode);
				myExchangeTable->insert(myNode);
				myNode=NULL;
				myState = NO_PARENT;
			}
			
			//if (myNode->isBootstrap())
			//	myState = SERVER;
		}
		/*ends check if there is already a node can handle this packet*/

		
		
		if (!keepRunning)//check
			break;
		

		/*begins try to request a packet*/
		//failNodes->clearall();
		for(requestRetry=0;requestRetry<3;requestRetry++)
		{

			/*begins get one node and try to request a packet*/
			if(myNode==NULL)
			{
				myNode=myExchangeTable->popOneParentBySegID(PacketSeg);
				if(myNode == NULL)//fails to get a node
				{
					myState = NO_PARENT;
				//myQueue->fail(entry);
					#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: NO Parents for packet " + itos(PacketID) + "!");
					#endif
					usleep(1000000);		// wait 1 s
					continue;
				}
				myExchangeTable->addAccessCount(myNode);
				if(myNode->isBootstrap())
					myState=SERVER;
			}
			/*ends get one node and try to request a packet*/


			/*start timer*/
			timerCounter = 0;start = GetTickCount();
			
			/*begin try connect*/
			if(!myNode->ping() || myNode->getPingTime()==PING_FAIL)
			{
				for(connectRetry=0;connectRetry<VMESH_CONNECT_RETRY;connectRetry++)
				{
					if (myNode->connect())
					{
						if (myNode->ping())
						{
							break;
						}
					}
				
				}
				if(connectRetry>=VMESH_CONNECT_RETRY)//indicates that connection fails
				{
					#ifdef PR_DEBUG
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: fail to connect");
					#endif
					myExchangeTable->reduceAccessCount(myNode);
					if (!myNode->isBootstrap())
					{
						myExchangeTable->removeNode(myNode);
						LOG_DEBUG((string)"Removed => " + (myNode->toString(false)) + " in PR (" + itos((int)threadHandle%1000) + ")");
					}
					else
					{

						LOG_ERROR("PR (" + itos((int)threadHandle%1000) +")\t: Error! Server is dead!");
					}
					//delete myNode;
					myNode = NULL;
					continue;
				}
			}
			/*end try connect*/
			
			/*begin to request a packet*/
			SocketClient* cli = myNode->getSocketClient();//a socket client
			sendmsg[0] = (char)REQ_PACKET;
			sprintf(sendmsg+1, "%c%c%c%c", (char)(PacketID/16777216), (char)((PacketID/65536)%256), (char)((PacketID/256)%256), (char)(PacketID%256));//construct the request message		
			
			if ((cli!=NULL) && !cli->send(sendmsg, 5))
			{
				LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Send Request error");
				//myQueue->fail(entry);
				if (myNode!=NULL)
				{
					failNodes->addNode(myNode);
					//myNode->clrSocketUsing();
					myExchangeTable->reduceAccessCount(myNode);
					//delete myNode;
					myNode = NULL;
				}
				continue;
			}
			else
			{
				STAT_AGGREGATE(UP_CONTROL, 5);
			}
			
			if (cli!=NULL)
			{
				if ( bActive ) 
					recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT);
				else
					recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT, DEFAULT_MIN_TRANSFER_RATE);
				if (recvlen <= 0)
				{
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Receive Request error");
					//myQueue->fail(entry);
					if (myNode!=NULL)
					{
						failNodes->addNode(myNode);
						//myNode->clrSocketUsing();
						myExchangeTable->reduceAccessCount(myNode);
						//delete myNode;
						myNode = NULL;
					}
					continue;
				}
				
				if ((recvmsg!=NULL) && (recvmsg[0] == RPY_PACKET_OK))
				{
					#ifdef PR_DEBUG
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Received reply says OK");
					#endif
					STAT_AGGREGATE(DOWN_CONTROL, 1);
					STAT_AGGREGATE(DOWN_DATA, recvlen-1);
					start = GetTickCount();
					if ( Packet::validateSerializedData(recvmsg + 1, recvlen - 1) == false )	// check the data integrity of received bytes
					{
						sprintf(msg, " !!! ~~~~~~~~~~~~~ Packet %d checksum verification fails ~~~~~~~~~~~~~ !!!", PacketID);
						LOG_ERROR(msg);
						if (myNode!=NULL)
						{
							failNodes->addNode(myNode);
							//myNode->clrSocketUsing();
							myExchangeTable->reduceAccessCount(myNode);
							//delete myNode;
							myNode = NULL;
						}
						continue;
					}

					myQueue->success(entry);
					//myExchangeTable->finishUsing(myNode);
					
					/*end timer*/
					timeTaken[timerCounter] += (GetTickCount() - start);
					if ( bTimeLog )
					{
						printf("Packet %d: Packet request takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter]  / 1000, timeTaken[timerCounter] % 1000);
						timerCounter++;
					}
					/*begin buffer or store*/
					savePacket(recvmsg,recvlen,PacketDest,PacketID,bTimeLog);
					/*end buffer or store*/

					connectionCounter++;
					break;
				}
				else if((recvmsg==NULL) || (recvmsg[0] == RPY_PACKET_FAIL))
				{
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Received reply but failed");
					STAT_AGGREGATE(DOWN_CONTROL, recvlen);
					if (recvmsg!=NULL)
					{
						delete [] recvmsg;
						recvmsg = NULL;
					}
					if(myNode!=NULL)
					{
						failNodes->addNode(myNode);
						//myNode->clrSocketUsing();
						myExchangeTable->reduceAccessCount(myNode);
						//delete myNode;
						myNode = NULL;
					}
					continue;
				
				}

			}
			/*end requesting a packet*/
			

			
		}
		
		myExchangeTable->insert(failNodes);
		failNodes->sharrowclearall();

		if(requestRetry>=3)
		{
			myQueue->fail(entry);
		}
		/*ends try to request a packet*/


	}
	/*ends the main loop*/

	return NULL;
}

bool PacketRequest::releaseNode()
{
	waitingRelease = true;
	while(myNode!=NULL)
		usleep(100);
	if (waitingRelease)
	{
		waitingRelease = false;
		return false;
	}
	else
	{
		return true;
	}
}

VMeshRemoteNode* PacketRequest::getNode()
{
	return myNode;
}

int PacketRequest::increaseTransferRate(unsigned int step)
{
	transferRate += TRANSFER_RATE_STEP_SIZE * step;

	if ( transferRate > DEFAULT_MAX_TRANSFER_RATE )
		transferRate = DEFAULT_MAX_TRANSFER_RATE;

	nInactiveCount = 0;
	bActive = true;

	return transferRate;
}

int PacketRequest::decreaseTransferRate(unsigned int step)
{
	transferRate -= TRANSFER_RATE_STEP_SIZE * step;

	if ( transferRate < DEFAULT_MIN_TRANSFER_RATE )
		transferRate = DEFAULT_MIN_TRANSFER_RATE;
	
	/*if ( nInactiveCount < INACTIVE_COUNT )
		nInactiveCount++;

	if ( nInactiveCount >= INACTIVE_COUNT ) */
		bActive = false;

	return transferRate;
}

bool PacketRequest::isActive()
{
	return bActive;
}


void PacketRequest::savePacket(char* recvmsg,int recvlen,char PacketDest,int PacketID,bool bTimeLog)
{
	if (PacketDest == 'b')				//packet request for play back the video, packet store into the Buffer
	{
		#ifdef PR_DEBUG
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " start writing to Buffer");
		#endif
		myBuffer->write(PacketID, recvmsg + 1, recvlen - 1);
		delete [] recvmsg;
		recvmsg = NULL;
		#ifdef PR_DEBUG
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " written to Buffer successfully");
		#endif
	}
	else if (PacketDest == 's')			//packet request for caching, packet store into the Storage
	{
		#ifdef PR_DEBUG	
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " start writing to Storage");
		#endif
		start = GetTickCount();
		PeerStorage* ps = (PeerStorage*) myStorage;
		Packet* p = new Packet(recvmsg + 1, recvlen - 1, false);
		ps->PeerStorage::addPacket(PacketID/1000000000, p);
					
		timeTaken[timerCounter] += ( GetTickCount() - start );
		if ( bTimeLog )
		{
			printf("Packet %d: Adding packet to storage takes %d.%03ds seconds\n\n", PacketID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
			timeTaken[timerCounter] = 0;
		}
		timerCounter++;
		delete p;
		delete [] recvmsg;
		recvmsg = NULL;
		#ifdef PR_DEBUG
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " written to Storage successfully");
		#endif
	}
	else
	{
		if (recvmsg!=NULL)
			delete [] recvmsg;
		recvmsg = NULL;
		#ifdef PR_DEBUG
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ ": Wrong destination!");
		#endif
	}


}


_VMESH_NAMESPACE_FOOTER
