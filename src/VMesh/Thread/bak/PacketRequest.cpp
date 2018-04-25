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
	while (keepRunning)
	{
		myState = NORMAL;

		if (!keepRunning)
			break;

		// if there are no packets waiting
		while (myQueue->empty())
		{
			myState = NO_PACKET;
			if (myNode!=NULL)
			{
				#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
				#endif
				myNode->clrSocketUsing();
				myExchangeTable->reduceAccessCount(myNode);
				//myNode->reduceAccessCount();
				myNode = NULL;
			}
			if (!keepRunning)
				break;
			usleep(MAX_STREAM_PARENTS * 100000);		// wait MAX_STREAM_PARENTS*0.1 s
		}

		// pop from the queue which Packet to fetch
		entry = myQueue->pop();
		// error entry is pop()ed, try again
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

		#ifdef PR_DEBUG
			LOG_DEBUG((string)"PR (" + itos((int)threadHandle%1000) +")\t: Trying to get Packet " + itos(PacketID));
		#endif
		bool bTimeLog = false; //( PacketID % 1000 == 0 );
		int packetID = PacketID % 1000000000;

		try
		{
			// see which segment is this packet belongs
			PacketSeg = myMediaInfo->getSIDbyPID(PacketID%1000000000, PacketID/1000000000);
			desiredSegment = PacketSeg;

			if (PacketSeg==-1)
			{
				// wrong segment number for this packet ID
				continue;
			}

			// throw the currently using node
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
				//if ((myNode->getSegmentID()==BOOTSTRAP_SEGMENTID) && (myExchangeTable->getParentBySegID(PacketSeg)->getSize()==0))
				//	throwNode = false;
				if (waitingRelease)
					throwNode = true;

				if (throwNode)
				{
					#ifdef PR_DEBUG
						LOG_DEBUG((string)"PR (" + itos((int)threadHandle%1000) +")\t: Released Socket from " + myNode->getIP() + ":" + itos(myNode->getPort()));
					#endif

					if (waitingRelease)
					{
						waitingRelease = false;
						myExchangeTable->reduceAccessCount(myNode);
						//myNode->reduceAccessCount();
						myNode = NULL;
						usleep(1000000);		// wait 1s
					}
					else
					{
						myNode->clrSocketUsing();
						//myNode->reduceAccessCount();
						myExchangeTable->reduceAccessCount(myNode);
						myNode = NULL;
					}
				}
			}

			// Debug
			//printf("PacketRequest::Check maximum connection::\t%d",itos(maxConnection));

			// get the node responsible for this Packet
			while (myNode == NULL)
			{
				// get the node from myParentList
				// try to find a parent in the parent list to provide segment for myNode
				if (myNode==NULL)
				{
					//myNode = myParentList->getSegmentNode(PacketSeg);
					myNode = myExchangeTable->getOneParentBySegID(PacketSeg,NULL);
					if (myNode!=NULL)
					{
						/*if (myNode->getAccessCount() < maxConnection)
						{
							printf("PacketRequest::This node already reach the maximum connection!!!");
							break;
						}*/
						myExchangeTable->addAccessCount(myNode);
						//myNode->addAccessCount();
					}
				}
				if (myNode==NULL)
					break;
				if (!myNode->setSocketUsing())
				{
					myExchangeTable->reduceAccessCount(myNode);
					//myNode->reduceAccessCount();
					myNode = NULL;
					break;
				}
				#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Lock Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
				#endif

				// ping if the socket is connected already
				if (myNode->ping() && myNode->getPingTime()!=PING_FAIL)
					break;

				// connect it several times and discard this node if fails
				int retry=0;
				while (true)
				{
					if (myNode!=NULL)
					{
						#ifdef PR_DEBUG
							LOG_DEBUG((string)"PR (" + itos((int)threadHandle%1000) +")\t: Trying Parent " + myNode->getIP() + ":" + itos(myNode->getPort()));
						#endif
						
						/*if (connectionCounter > maxConnection)
						{
							LOG_INFO("PacketRequest::This node already reach the maximum connection!!!");
							break;
						}*/

						if (myNode->connect())
						{
							//LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Connect Ping Time1\t:" + itos(myNode->getPingTime()));
							if (myNode->ping())
							{
								// supply myself to this node for updating the LocalNodeList
								/*if (myNodeList->getSize()>0)
								{
									VMeshRemoteNodeList* myself = new VMeshRemoteNodeList(*myNodeList);
									int serial_len;
									char* serialized = myself->serialize(serial_len);
									char* send_msg = new char[serial_len+1];
									send_msg[0] = SUPPLY_LOCALNODELIST;
									memcpy(send_msg+1, serialized, serial_len);
									SocketClient* cli = myNode->getSocketClient();
									cli->send(send_msg, serial_len+1);
									STAT_AGGREGATE(UP_CONTROL, serial_len+1);
									delete [] send_msg;
									delete [] serialized;
									delete myself;
								}*/

								break;
							}
						}
					}

					// Fail to find the parent provides segment to myNode
					// 1. There are NO parent for packet
					// 2. The central lookup server is dead
					if((++retry)>=VMESH_CONNECT_RETRY)
					{
						if (myNode!=NULL)
						{
							myNode->clrSocketUsing();
							myExchangeTable->reduceAccessCount(myNode);
							#ifdef PR_DEBUG
								LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: fail to connect");
							#endif
							if (!myNode->isBootstrap())
							{
								LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: NO Parents for packet " + itos(PacketID) + "!");
								LOG_DEBUG((string)"Removed => " + (myNode->toString(false)) + " in PR (" + itos((int)threadHandle%1000) + ")");
								//myParentList->removeNode(myNode);
								myExchangeTable->reduceAccessCount(myNode);
								myExchangeTable->removeNode(myNode);
								
								//myNode->reduceAccessCount();
								//if (myNode->isRemovable())
								//	delete myNode;
								//else
								//	LOG_DEBUG((string)"Not removable => " + (myNode->toString(false)) + " in PR (" + itos((int)threadHandle%1000) + ")");
							}
							else
							{
								LOG_ERROR("PR (" + itos((int)threadHandle%1000) +")\t: Error! Server is dead!");
								//myNode->reduceAccessCount();
								//SetEvent(m_hReset);
								//LOG_ERROR("PR (" + itos((int)threadHandle%1000) +")\t: Close the program");
								//exit(0);
							}
							myNode = NULL;
						}
						break;
					}
				}
				break;
			}

			
			// oops! no nodes available
			if (myNode==NULL)
			{
				desiredSegment = PacketSeg;
				myState = NO_PARENT;
				myQueue->fail(entry);
				#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: NO Parents for packet " + itos(PacketID) + "!");
				#endif
				usleep(1000000);		// wait 1 s
				continue;
			}
			
			// at this point, the node should be ready

			if (myNode->isBootstrap())
				myState = SERVER;
			// randomly update the ping time
			/*
			if (myNode->isBootstrap())
			{
				if (rand() % 20000==0)
					myNode->ping();
			}
			else
			{
				if (rand() % 5000==0)
					myNode->ping();
			}
			*/

			#ifdef PR_DEBUG
				LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Trying to get packet " + itos(PacketID)+ " from " + myNode->getIP() +":"+ itos(myNode->getPort()));
			#endif

		} catch(...)
		{
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Exception caught");
			myState = NO_PARENT;
			myQueue->fail(entry);
			#ifdef PR_DEBUG
				LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
			#endif
			myNode->clrSocketUsing();
			//myNode->reduceAccessCount();
			myExchangeTable->reduceAccessCount(myNode);
			myNode = NULL;
			continue;
		}

		if (!keepRunning)
			break;

		// Debug
		//LOG_DEBUG("Current connection number::\t" + itos(connectionCounter));

		try
		{
			int retry=0;
			bool cont = false, connected;
			while (true)
			{
				timerCounter = 0;
				connected = true;
				//if (!myNode->ping() || myNode->getPingTime()==PING_FAIL)
				start = GetTickCount();
				
				if (!myNode->ping())
				{
					connected = myNode->connect();
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Connect Ping Time2\t:" + itos(myNode->getPingTime()));
				}
				
				timeTaken[timerCounter] += ( GetTickCount() - start );
				if ( bTimeLog )
				{
					printf("Packet %d: Pinging remote node takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter]  / 1000, timeTaken[timerCounter] % 1000);
					timeTaken[timerCounter] = 0;
				}
				timerCounter++;

				if (connected)
				{
					start = GetTickCount();
					SocketClient* cli = myNode->getSocketClient();
					timeTaken[timerCounter] += ( GetTickCount() - start );
					if ( bTimeLog )
					{
						printf("Packet %d: Getting client socket takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
						timeTaken[timerCounter] = 0;
					}
					timerCounter++;

					start = GetTickCount();
					sendmsg[0] = (char)REQ_PACKET;
					sprintf(sendmsg+1, "%c%c%c%c", (char)(PacketID/16777216), (char)((PacketID/65536)%256), (char)((PacketID/256)%256), (char)(PacketID%256));			
					if ((cli!=NULL) && !cli->send(sendmsg, 5))
					{
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Send Request error");
						myQueue->fail(entry);
						if (myNode!=NULL)
						{
#ifdef PR_DEBUG
							LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
#endif
							myNode->clrSocketUsing();
							//myNode->reduceAccessCount();
							myExchangeTable->reduceAccessCount(myNode);
							myNode = NULL;
						}
						cont = true;
						break;
					}
					else
					{
						STAT_AGGREGATE(UP_CONTROL, 5);
					}

					timeTaken[timerCounter] += ( GetTickCount() - start );
					if ( bTimeLog )
					{
						printf("Packet %d: Sending packet request takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
						timeTaken[timerCounter] = 0;
					}
					timerCounter++;

#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Sent request");
#endif

					// Receive packet request and process the packet request
					start = GetTickCount();
					if (cli!=NULL)
					{

						if ( bActive ) 
							recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT);
						else
							recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT, DEFAULT_MIN_TRANSFER_RATE);
						/*if ( entry.dest == 'b' )
						{
						}
						else
							recvlen = cli->receive(recvmsg, VMESH_REPLY_TIMEOUT, transferRate); */
						if (recvlen <= 0)
						//if ((recvlen = cli->receive(recvmsg, -1)) <= 0)
						{
							LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Receive Request error");
							myQueue->fail(entry);
							if (myNode!=NULL)
							{
#ifdef PR_DEBUG
								LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
#endif
								myNode->clrSocketUsing();
								//myNode->reduceAccessCount();
								myExchangeTable->reduceAccessCount(myNode);
								myNode = NULL;
							}
							cont = true;
							break;
						}
						if ( Packet::validateSerializedData(recvmsg + 1, recvlen - 1) == false )	// check the data integrity of received bytes
						{
							sprintf(msg, " !!! ~~~~~~~~~~~~~ Packet %d checksum verification fails ~~~~~~~~~~~~~ !!!", PacketID);
							LOG_ERROR(msg);
						}
						else
						{
							timeTaken[timerCounter] += ( GetTickCount() - start );
							if ( bTimeLog )
							{
								printf("Packet %d: Receiving packet takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
								timeTaken[timerCounter] = 0;
							}
							timerCounter++;
#ifdef PR_DEBUG
							LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Received reply");
#endif
							break;
						}
					}

				}
				if((retry++)>=VMESH_CONNECT_RETRY)
				{
					if (myNode!=NULL)
					{
#ifdef PR_DEBUG
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: fail to connect");
#endif
						if (!myNode->isBootstrap())
						{
							LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: NO Parents for packet " + itos(PacketID) + "!");
							myNode->clrSocketUsing();		////////////////////////////////////////////////
							//myNode->reduceAccessCount();
							myExchangeTable->reduceAccessCount(myNode);
							myNode = NULL;
							desiredSegment = PacketSeg;
							myState = NO_PARENT;
							myQueue->fail(entry);
							usleep(1000000);		// wait 1 s
						}
						else
						{
							LOG_ERROR("PR (" + itos((int)threadHandle%1000) +")\t: Error! Server is dead!");
							
						}
					}
					cont = true;
					break;
				}
			}

			if (cont)
			{
				continue;
			}
			
			if ((recvmsg!=NULL) && (recvmsg[0] == RPY_PACKET_OK))
			{
				// reply says ok
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
					myQueue->fail(entry);
					continue;
				}

				
                myQueue->success(entry);
				connectionCounter++;
				timeTaken[timerCounter] += ( GetTickCount() - start );
				if ( bTimeLog )
				{
					printf("Packet %d: Marking success to queue takes %d.%03ds seconds\n", packetID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
					timeTaken[timerCounter] = 0;
				}
				timerCounter++;

				#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Received packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " from " + myNode->getIP() +":"+ itos(myNode->getPort()));
				#endif
				
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
					continue;
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
						printf("Packet %d: Adding packet to storage takes %d.%03ds seconds\n\n", packetID, timeTaken[timerCounter] / 1000, timeTaken[timerCounter] % 1000);
						timeTaken[timerCounter] = 0;
					}
					timerCounter++;

					if ( p->getPacketID() % 100 == 0 )
					{
						
						sprintf(msg, "Packet %u: CheckSum = %d", p->getPacketID(), p->getCheckSum());
						//LOG_DEBUG(msg);
					}
					delete p;
					delete [] recvmsg;
					recvmsg = NULL;
					#ifdef PR_DEBUG
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ " written to Storage successfully");
					#endif
					continue;
				}
				else
				{
					if (recvmsg!=NULL)
						delete [] recvmsg;
					recvmsg = NULL;
					#ifdef PR_DEBUG
						LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Packet " + itos(PacketID/1000000000)+ "|"+ itos(PacketID%1000000000)+ ": Wrong destination!");
					#endif
					throw;
				}
			}
			else if ((recvmsg==NULL) || (recvmsg[0] == RPY_PACKET_FAIL))		// Problem:: Client wont find another node to recover the segment!!!!!!
			{
				// reply says not ok
				LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Received reply but failed");
				STAT_AGGREGATE(DOWN_CONTROL, recvlen);
				if (recvmsg!=NULL)
				{
					delete [] recvmsg;
					recvmsg = NULL;
				}
				myQueue->fail(entry);
				myNode->setSegmentID(INITIAL_SEGMENTID);
				#ifdef PR_DEBUG
					LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
				#endif
				myNode->clrSocketUsing();
				//myNode->reduceAccessCount();
				myExchangeTable->reduceAccessCount(myNode);
				myNode = NULL;
				continue;
			}
			else
			{
				// reply unknown
				LOG_DEBUG((string)"PR (" + itos((int)threadHandle%1000) +")\t: Received unknown reply, type = " + itos(recvmsg[0]));
				STAT_AGGREGATE(DOWN_CONTROL, recvlen);
				throw;
			}
		} catch(...)
		{
			LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Exception caught");
			myState = NO_PARENT;
			myQueue->fail(entry);
			#ifdef PR_DEBUG
				LOG_DEBUG("PR (" + itos((int)threadHandle%1000) +")\t: Release Socket from " + myNode->getIP() +":"+ itos(myNode->getPort()));
			#endif
			myNode->clrSocketUsing();
			//myNode->reduceAccessCount();
			myExchangeTable->reduceAccessCount(myNode);
			myNode = NULL;
			continue;
		}

	}

	if ( myNode != NULL )				////////////////////////////////////////////////
		myNode->clrSocketUsing();		////////////////////////////////////////////////
	myState = ENDED;
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

_VMESH_NAMESPACE_FOOTER
