/**	
 *	StreamRequest
 *
 *  The de facto overlay manager for VMesh
 *	@author Kelvin Chan, Philip Yang
 */

#ifndef _STREAM_REQUEST_H
#define _STREAM_REQUEST_H

#include <list>
#include <time.h>
#include "../Common/VMeshCommon.h"
#include "../Common/MediaInfo.h"
#include "../Node/VMeshLocalNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Node/VMeshRemoteNode.h"
#include "../Node/VMeshRemoteNodeList.h"
#include "../Thread/PacketRequest.h"
#include "../Common/SchedulerQueue.h"
#include "../Util/Buffer.h"
#include "../Storage/Storage.h"
#include "../Storage/PeerStorage.h"
#include "../Util/SocketClient.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"
//#include "../VivaldiAdaptor/VivaldiAdaptor.h"
#include "../Util/ExchangeTable.h"

_VMESH_NAMESPACE_HEADER

/**	@class StreamRequest
 *	This class implements a thread that	maintains enough VMeshRemoteNode in parent list
	and other functions for streaming
 */
class StreamRequest : public Thread 
{
public:
	/**	@fn StreamRequest (string _myIP, int _myPort, MediaInfo* _info, VMeshLocalNodeList* _list, 
		Storage* _storage, Buffer* _buffer, SegmentManager* _segment, string _hash)
	 *	Constructor that requires some shared object
	 *	@param _myIP local IP address
	 *	@param _myPort local port number
	 *	@param _info MediaInfo
	 *	@param _list VMeshLocalNodeList storing local nodes of completed segment
	 *	@param _storage Storage of segments
	 *	@param _buffer Buffer
	 *	@param _segment SegmentManager instance
	 *	@param _hash media hash value
	 */
	StreamRequest (string _myIP, int _myPort, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, string _hash, int _maxConnection, int p_identity, int _exchangeNeighbor);
	
	/**	@fn ~StreamRequest()
	 *	Destructor
	 */
	virtual ~StreamRequest();

	/**	@fn addBootstrap(string _IP, int _Port)
	 *	add the inital node explicitly to the parent list so that no lookup is required
	 *	@param _IP ip address of the inital node
	 *	@param _Port port number of the inital node
	 *	@return true if the node is alive, false otherwise
	 */
	bool addBootstrap(string _IP, int _Port);

	/**	@fn addPacket(unsigned int _PacketID)
	 *	Add the packets to the waiting queue, which will be stored in Buffer
	 *	@param _PacketID Packet ID
	 */
	void addPacket(unsigned int _PacketID);

	/**	@fn store(unsigned int _PacketID)
	 *	Add the packets to the waiting queue, which will be stored in Storage
	 *	@param _PacketID Packet ID
	 */
	void store(unsigned int _PacketID);

	/**	@fn clearQueueToBuffer()
	 *	clear all the packets(which will be stored in Buffer only) in the waiting queu
	 */
	void clearQueueToBuffer();

	/**	@fn getMediaInfo()
	 *	get the MediaInfo 
	 *	@return MediaInfo
	 */
	MediaInfo* getMediaInfo();

	/**	@fn getParentList()
	 *	get the VMeshRemoteNodeList for storing parents
	 *	@return the VMeshRemoteNodeList for storing parents
	 */
	//VMeshRemoteNodeList* getParentList();
	ExchangeTable * getExchangeTable();


	/**	@fn addParentsbyDHT(int _segmentID)
	 *	Look for more parents by using DHT
	 *	and add them to the myParentList
	 *	@param _segmentID the preferred segment ID (but not all added nodes have this segment)
	 *	@return the number of nodes added
	 */
	int addParentsbyDHT(int _segmentID);
		
	/**	@fn addParentsbyNeighbour(int _segmentID)
	 *	Look for more parents by contacting the current parent List
	 *	and add them to the myParentList
	 *	@param _segmentID the preferred segment ID (but not all added nodes have this segment)
	 *	@return the number of nodes added
	 */
	int addParentsbyNeighbour(int _segmentID);

	/**	@fn stop()
	 *	stop running the thread
	 */
	void stop();

	void notifyPacketRequest();

	void setSuperNode();

private:



	/**	@fn addDHTNode(DHT::Node* dhtNode)
	 *	Add a DHT node to the parentList
	 *	@param dhtNode the DHT node to be added
	 *	@return 1 if the node is added
	 *	@return 0 if the node is not added
	 */
	int addDHTNode(NetworkID* dhtNode);

	/**	@fn retrieveLocalNode(SocketClient* cli)
	 *	request for the VMeshLocalNode of a remote node
	 *	@param cli SocketClient to send request
	 *	@return VMeshRemoteNodeList replied
	 *	@return NULL if errors ocuured (like socket error, or not responding)
	 */
	VMeshRemoteNodeList* retrieveLocalNode(SocketClient* cli);

	/**	@fn retrieveParentList(SocketClient* cli, int _segmentID)
	 *	request for the VMeshRemoteNode parents of a remote node
	 *	@param cli SocketClient to send request
	 *	@param _segmentID the segment ID preferred, together with its adjacent segments
	 *	@return VMeshRemoteNodeList replied
	 *	@return NULL if errors ocuured (like socket error, or not responding)
	 */
	VMeshRemoteNodeList* retrieveParentList(SocketClient* cli, int _segmentID);

	/**	@fn addToParent(VMeshRemoteNodeList* &list, bool needSync)
	 *	Add the Nodes in a VMeshRemoteNodeList to the parent list
	 *	@param list VMeshRemoteNodeList contains VMeshRemoteNode to be added, destructed and set to NULL after call
	 *	@param needSync true if list is constructed by VMeshLocalNodeList of another peer
	 *	@param needSync false if list is constructed by VMeshRemoteNodeList parents of another peer
	 *	@return number of nodes added
	 */
	int addToParent(VMeshRemoteNodeList* &list, bool needSync);

	/**	@fn checkSegment()
	 *	check if Storage of any VMeshLocalNode is completed and register them into DHT
	 */
	void checkSegment();

	/**	@fn retrieveMediaInfo()
	 *	Retrieve MediaInfo from the current parent list;
	 *	If success, myMediaInfo would points to a valid MediaInfo after call
	 */
	void retrieveMediaInfo();

	/**	@fn fetchMediaInfo(VMeshRemoteNode* node)
	 *	send request to a node for MediaInfo;
	 *	If success, myMediaInfo would points to a valid MediaInfo after call
	 */
	void fetchMediaInfo(VMeshRemoteNode* node);

	void checkActivity();


protected:
	/**	@fn execute()
	 *	Override the execute() in Thread
	 */
	void* execute();

private:
	string myIP;
	int myPort;

	// list storing overlay parents
	//VMeshRemoteNodeList* myParentList;
	
	//neighbor table
	ExchangeTable* myExchangeTable;


	// Jack's comment:: connecting node with peer
	VMeshRemoteNodeList* connectedNodeList;

	// the scheduler queue storing all the requested calls
	SchedulerQueue* myQueue;

	MediaInfo* myMediaInfo;

	// list storing local replicas
	VMeshLocalNodeList* myNodeList;

	// on-disk storage 
	Storage* myStorage;

	// in-memory storage
	Buffer* myBuffer;

	// segment query manager
	SegmentManager* mySegment_;

	std::string myHash;
	
	PacketRequest* requestSlot[MAX_STREAM_PARENTS];

	bool keepRunning;

	bool isPacketFetched;
	bool isSuperNode;
	unsigned int counter;
	Mutex mutexLock;
	unsigned int inActiveCounter;

	int maxConnection;
	int peer_identity;
	int exchangeNeighbor;

	//VivaldiAdaptor* myVivaldiAdaptor;
};
_VMESH_NAMESPACE_FOOTER
#endif

