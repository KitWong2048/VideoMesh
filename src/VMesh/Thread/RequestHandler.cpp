#include "../Thread/RequestHandler.h"

using namespace std;
using App::ChordApp;

_VMESH_NAMESPACE_HEADER

//static string vivaldiMsg = "";
static int numVM = 0;

RequestHandler::RequestHandler (string _myIP, int _myPort, SocketClient* _client, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest, int p_identity)
{
	timeTaken[0] = timeTaken[1] = timeTaken[2] = timeTaken[3] = timeTaken[4] = 0;
	myIP = _myIP;
	myPort = _myPort;
	myClient = _client;
	myMediaInfo = _info;
	myNodeList = _list;
	myStorage = _storage;
	myBuffer = _buffer;
	mySegment_ = _segment;
	myStreamRequest = _myStreamRequest;
	keepRunning = true;
	myNode = new VMeshRemoteNode(myClient->getRemoteIP(), myClient->getRemotePort(), INITIAL_SEGMENTID);

	// Jack's comment:: add peer identity information into request listener
	peer_identity = p_identity;

	//myVivaldiAdaptor = new VivaldiAdaptor("..\\..\\..\\Vivaldi\\peers.txt");
	//myVivaldiAdaptor = new VivaldiAdaptor((string) STORAGEFILE_PATH + "peers.txt");
}


RequestHandler::~RequestHandler()
{
	LOG_DEBUG((string)"RequestHandler destructor is called");
	
	if (myClient != NULL){
		delete myClient;
		myClient = NULL;
	}
	
	if (myNode != NULL){
		delete myNode;
		myNode = NULL;
	}
	/*if (myVivaldiAdaptor != NULL){
		delete myVivaldiAdaptor;
		myVivaldiAdaptor = NULL;
	}*/
}

VMeshRemoteNode* RequestHandler::getMyNode()
{
	return myNode;
}

void RequestHandler::echo(char* msg, int length)
{
	char content[4096];
	memcpy(content, msg, length);
	content[length] = 0;
	LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " : (" + itos(content[0]) + ") "+ (content+1));
	myClient->send(msg, length);
	STAT_AGGREGATE(UP_CONTROL, length);
}


void RequestHandler::ping(char* msg, int length)
{
	/*
	req msg:	1 byte(REQ_PING)
	reply msg:	1 byte(RPY_PING)
	*/

	//LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <PING>");
	char reply = (char)(RPY_PING);
	myClient->send(&reply, 1);
	STAT_AGGREGATE(UP_CONTROL, 1);
}

void RequestHandler::supply_LocalNodeList(char* msg, int length)
{
	LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <SUPPLY LOCALNODELIST>");
	if (myStreamRequest==NULL)
		return;
	VMeshRemoteNodeList* returnList = new VMeshRemoteNodeList(msg+1, length-1);
	if (returnList->getSize()==0)
	{
		returnList->clearall();
		delete returnList;
		return;
	}
	ExchangeTable* table=myStreamRequest->getExchangeTable();
	VMeshRemoteNode *q = returnList->getNode(0);
	while (returnList->getSize()>0)
	{
		q = returnList->getNode(0);

		//LOG_DEBUG((string)"Received: " + q->toString());
		/*if ((q!=NULL)&& ((q->getIP()!=myIP) || (q->getPort()!=myPort))&& q->connect())
		{
			LOG_DEBUG((string)"Added => " + (q->toString(false)));
			q->ping();
			table->insert(q);	
		}*/
		
		// Jack's comment:: close ping connection
		//LOG_DEBUG((string)"Received: " + q->toString());
		if ((q!=NULL)&& ((q->getIP()!=myIP) || (q->getPort()!=myPort))&& q->connectControlSocket())
		{
			LOG_DEBUG((string)"Added => " + (q->toString(false)));
			q->pingControlSocket();
			q->closeControlSocket();
			table->insert(q);	
		}
		//returnList->deepRemoveNode(q);
		returnList->sharrowRemoveNode(q);
	}
	delete returnList;
	returnList=NULL;
	return;
}

void RequestHandler::req_MediaInfo(char* msg, int length)
{
	/*
	req msg:	1 byte(REQ_MEDIAINFO) + MEDIAINFOHASH_LENGTH bytes(Movie Hash in ASCII)
	reply msg:	1 byte(RPY_MEDIAINFO_OK) + n bytes (serialized MediaInfo)
				1 byte(RPY_MEDIAINFO_FAIL)
	*/
	string reqHash (msg+1, length-1);
	LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <REQ MEDIAINFO \"" + reqHash + "\">");

	// Compare myMediaInfo movie hash with request message media info movie hash
	if ((myMediaInfo!=NULL) && (length-1==2*MEDIAINFOHASH_LENGTH) && (memcmp(myMediaInfo->getMediaHash(), msg+1, length-1)==0))
	{
		char* data = NULL;
		int len;
		myMediaInfo->serialize(data, len);

		char* data2 = new char[len+1];
		memcpy(data2 + 1, data, len);
		delete[] data;
		data2[0] = RPY_MEDIAINFO_OK;

		myClient->send(data2, len+1);
		STAT_AGGREGATE(UP_CONTROL, len+1);
		delete[] data2;
		
	}
	else
	{
		char reply = (char)(RPY_MEDIAINFO_FAIL);
		myClient->send(&reply, 1);
		STAT_AGGREGATE(UP_CONTROL, 1);
	}
	
}

void RequestHandler::req_Packet(char* msg, int length)
{
	/*
	req msg:	1 byte(REQ_PACKET) + 4 bytes(Packet ID)
	reply msg:	1 byte(RPY_PACKET_OK) + n bytes (serialized Packet)
				1 byte(RPY_PACKET_FAIL)
	*/
	// calculate the PcketID requesting
	unsigned int PacketID = (unsigned char) msg[1];
	PacketID <<= 8;
	PacketID += (unsigned char) msg[2];
	PacketID <<= 8;
	PacketID += (unsigned char) msg[3];
	PacketID <<= 8;
	PacketID += (unsigned char) msg[4];
	//time_t currentTime;
	if (PacketID%1000 == 0)
		LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <REQ PACKET " + itos(PacketID/1000000000) + "|" + itos(PacketID%1000000000) + ">");
	
	/*if ( PacketID % 1000 == 0 )
	{
		currentTime = time(NULL);
		int hh = currentTime / 3600 % 24 + 8;
		int mm =  (currentTime / 60) % 60;
		int ss = currentTime % 60;
		printf("Current Time: %02d:%02d:%02d \n", hh, mm, ss);
	}*/

	int type = PacketID/1000000000;
	PacketID = PacketID % 1000000000;

	bool bTimeLog = false; // ( PacketID % 5000 == 0 );
	int packetID = PacketID;
	start = GetTickCount();
	// check if we have this packet
	if (!myStorage->existPacket(type, PacketID))
	{
		// reply tells other we do not have this packet
		//LOG_DEBUG("RequestHandler\t: I do not have this Packet");
		char reply = (char)(RPY_PACKET_FAIL);
		myClient->send(&reply, 1);
		STAT_AGGREGATE(UP_CONTROL, 1);
		//LOG_DEBUG("RequestHandler\t: I replied fail");
		return;
	}

	timeTaken[0] += ( GetTickCount() - start );
	if ( bTimeLog )
	{
		printf("Packet %d: Checking packet existence takes %d.%03ds seconds\n", packetID, timeTaken[0] / 1000, timeTaken[0] % 1000);
		timeTaken[0] = 0;
	}

	start = GetTickCount();

	// Handle the packet request and get packet from the storage
	Packet* targetPacket = myStorage->getPacket(type, PacketID);

	timeTaken[1] += ( GetTickCount() - start );
	if ( bTimeLog )
	{
		printf("Packet %d: Getting packet takes %d.%03ds seconds\n", packetID, timeTaken[1] / 1000, timeTaken[1] % 1000);
		timeTaken[1] = 0;
	}

	start = GetTickCount();
	//LOG_DEBUG("RequestHandler\t: I got this Packet from storage");
	int len = 0;
	char* replyPacket;

	// Packet call the serialize function to serialize the data into a char*
	// for SocketClient send those data to the requester
	targetPacket->serialize(replyPacket, len);
	char* reply = new char[len+1];

	// Set the message state to RPY_PACKET_OK which mean success to get packets from other peers
	reply[0] = (char)RPY_PACKET_OK;
	memcpy (reply + 1, replyPacket, len);
	
	timeTaken[2] += ( GetTickCount() - start );
	if ( bTimeLog )
	{
		printf("Packet %d: Serializing packet takes %d.%03ds seconds\n", packetID, timeTaken[2] / 1000, timeTaken[2] % 1000);
		timeTaken[2] = 0;
	}

	start = GetTickCount();

	// SocketClient send the packet back to requester
	if (myClient->send(reply, len+1))
	{
		STAT_AGGREGATE(UP_DATA, len+1);
		//LOG_DEBUG("RequestHandler\t: Send Packet successfully");
	}
	else
	{
		//LOG_DEBUG("RequestHandler\t: Send Packet fails");
	}
	timeTaken[3] += ( GetTickCount() - start );
	if ( bTimeLog )
	{
		printf("Packet %d: Sending packet takes %d.%03ds seconds\n", packetID, timeTaken[3] / 1000, timeTaken[3] % 1000);
		timeTaken[3] = 0;
	}

	delete [] replyPacket;
	delete [] reply;
	delete targetPacket;

}

void RequestHandler::req_LocalNodeList(char* msg, int length)
{
	/*
	req msg:	1 byte(REQ_LOCALNODELIST)
	reply msg:	1 byte(RPY_LOCALNODELIST_OK) + serialized VMeshRemoteNodeList
	*/
	LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <REQ LOCALNODELIST>");
	
	VMeshRemoteNodeList list(*myNodeList);
	int serialLength;
	char* serialData = list.serialize(serialLength,true);
	
	char* rpymsg = new char[serialLength + 1];
	memcpy(rpymsg + 1, serialData, serialLength);
	rpymsg[0] = (char)(RPY_LOCALNODELIST_OK);
	myClient->send(rpymsg, serialLength + 1);
	STAT_AGGREGATE(UP_CONTROL, serialLength + 1);
	
	delete [] serialData;
	delete [] rpymsg;	
}

void RequestHandler::req_ParentList(char* msg, int length)
{
	/*
	req msg:	1 byte(REQ_PARENTLIST) + 2 bytes(SegmentID)
	reply msg:	1 byte(RPY_PARENTLIST_OK) + serialized VMeshRemoteNodeList
	*/
	if (length!=4)
		return;
	int segID = ((unsigned char)msg[1] * 256) + (unsigned char)msg[2];
	int identity=(unsigned char)msg[3];
	LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <REQ PARENTLIST (SEG " + itos(segID) + ")>");

	//VMeshRemoteNodeList* neighbourList = NULL;

	int serialLength;
	ExchangeTable* table=NULL;
	// Jack's comment::Parent List will keep update through Stream Request Thread
	// Then Request Handler extract the neighbor list from the recent parent list
	if ((myStreamRequest!=NULL) && (myStreamRequest->getExchangeTable()!=NULL))
	{
				//char* serialData = myStreamRequest->serialize(serialLength);
		table=myStreamRequest->getExchangeTable();
		char* serialData = table->serialize(serialLength);
		char* rpymsg = new char[serialLength + 1];
		memcpy(rpymsg + 1, serialData, serialLength);
		rpymsg[0] = (char)(RPY_PARENTLIST_OK);
		myClient->send(rpymsg, serialLength + 1);
		STAT_AGGREGATE(UP_CONTROL, serialLength + 1);
		delete [] serialData;
		delete [] rpymsg;
	}
	//neighbourList = myStreamRequest->getParentList()->extractNeighbourList(segID, RPY_PARENT_LIST_SIZE);
	if (table==NULL)
	{
		char* rpymsg = new char[1];
		rpymsg[0] = (char)(RPY_PARENTLIST_OK);
		myClient->send(rpymsg,  1);
		STAT_AGGREGATE(UP_CONTROL,1);
		delete [] rpymsg;
	}
}

void RequestHandler::stop()
{
	keepRunning = false;
	this->join();
}

void* RequestHandler::execute()
{
	try
	{
		int seconds = -1;
		
		LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: " + (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " connected");
		while (keepRunning)
		{
			seconds++;
			seconds %= 3600;

			//LOG_DEBUG("RequestHandler\t : myClient == " + itos(myClient!=NULL));
			//LOG_DEBUG("RequestHandler\t : Waiting for Request " + (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()));
			char* msg = NULL;
			
			//string debug_ip = myClient->getRemoteIP();		// FOR DEBUG
			
			if ((this==NULL) || (myClient==NULL))
				return NULL;
			int length = myClient->receive(msg, VMESH_IDLE_TIMEOUT);
			STAT_AGGREGATE(DOWN_CONTROL, length);

			//LOG_DEBUG("RequestHandler\t : " + (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " received len = "+ itos(length));
			if ((length <=0) || (msg==NULL))
			{
				//LOG_DEBUG((string)"RH (" + itos((int)threadHandle%1000) +")\t: received len = "+ itos(length) + " msg == NULL? " + itos(msg==NULL));
				LOG_INFO((string)"RH (" + itos((int)threadHandle%1000) +")\t: Closed connection "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()));
				break;
			}

			string *ipList = new string[1];
			int *portList = new int[1];

			switch((MessageType)(msg[0]))
			{
			case SUPPLY_LOCALNODELIST:
				supply_LocalNodeList(msg, length);
				break;
			case PING:
				ping(msg, length);
				break;
			case REQ_MEDIAINFO:
				req_MediaInfo(msg, length);
				break;
			case REQ_PACKET:
				req_Packet(msg, length);

				// for vivaldi
				/*if ( rand()%500 == 0 )
				{
					ipList[0] = myClient->getRemoteIP();
					portList[0] = myClient->getRemotePort()+500;

					//myVivaldiAdaptor->writeFile(ipList, portList, 1);

					delete [] ipList;
					delete [] portList;

					ipList = NULL;
					portList = NULL;
				}*/

				break;
			case REQ_LOCALNODELIST:
				req_LocalNodeList(msg, length);
				break;
			case REQ_PARENTLIST:
				req_ParentList(msg, length);
				break;
			default:
				//echo(msg, length);
				LOG_ERROR((string)"RH (" + itos((int)threadHandle%1000) +")\t: R "+ (string)(myClient->getRemoteIP()) + ":" + itos(myClient->getRemotePort()) + " <UNKNOWN CONTENT> **");
			}
			
			delete[] msg;
			msg = NULL;

			
			//LOG_DEBUG("RequestHandler\t: Request Handle finished!");
		}
	} catch(...)
	{
		LOG_DEBUG("myClient\t: Exception catched");
	}
	keepRunning=false;
	myClient->close();

	//delete myClient;
	return NULL;
}

bool RequestHandler::isRunning()
{
	return keepRunning;
}

_VMESH_NAMESPACE_FOOTER
