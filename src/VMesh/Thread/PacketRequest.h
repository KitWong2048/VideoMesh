#ifndef _PACKET_REQUEST_H
#define _PACKET_REQUEST_H

#include "../Common/VMeshCommon.h"
#include "../Common/MediaInfo.h"
#include "../Node/VMeshRemoteNode.h"
#include "../Node/VMeshRemoteNodeList.h"
#include "../Common/SchedulerQueue.h"
#include "../Storage/PeerStorage.h"
#include "../Storage/Storage.h"
#include "../Util/Buffer.h"
#include "../Common/Packet.h"
#include "../Util/SocketClient.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

#include "../Util/ExchangeTable.h"

/**	@file PacketRequest.h
 *	This file contains the definition of class PacketRequest
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/*! @brief show debug message for PacketRequest */
//#define PR_DEBUG

/**	@enum PacketRequestState
 *	An enum to represent different states of a PacketRequest thread
 */
enum PacketRequestState 
{
	NO_PACKET,		/**< no packets queued to be fetched */
	NO_PARENT,		/**< not enough parents */
	SERVER,			/**< getting packets from server */
	NORMAL,			/**< normal */
	ENDED			/**< the thread is ended */
};

/**	@class PacketRequest
 *	This class implements a thread that responsible for requesting a Packet from a remote node
 */
class PacketRequest : public Thread 
{
public:
	/**	@fn PacketRequest (MediaInfo* _mediaInfo, SchedulerQueue* _queue, VMeshRemoteNodeList* _parentList,
			VMeshLocalNodeList* _myList, Buffer* _buffer, Storage* _storage)
	 *	Constructor that requires some shared object
	 *	@param _mediaInfo MediaInfo
	 *	@param _queue SchedulerQueue
	 *	@param _parentList VMeshRemoteNodeList storing parents
	 *	@param _myList VMeshLocalNodeList storing local nodes of completed segment
	 *	@param _buffer Buffer
	 *	@param _storage Storage of segments
	 */
	//PacketRequest (MediaInfo* _mediaInfo, SchedulerQueue* _queue, VMeshRemoteNodeList* _parentList, VMeshLocalNodeList* _myList, Buffer* _buffer, Storage* _storage, int _maxConnection);
	PacketRequest (MediaInfo* _mediaInfo, SchedulerQueue* _queue, ExchangeTable* _exchangeTable, VMeshLocalNodeList* _myList, Buffer* _buffer, Storage* _storage, int _maxConnection);

	/**	@fn ~PacketRequest()
	 *	Destructor
	 */
	virtual ~PacketRequest();

	/**	@fn getState()
	 *	check the current state of the PacketRequest thread
	 *	@return PacketRequestState representing current state
	 */
	PacketRequestState getState();

	/**	@fn getDesiredSegment()
	 *	check the ID of the segment to which the recent retrieving/missing Packet belongs
	 *	@return segment ID that the thread prefers
	 */
	int getDesiredSegment();
	
	/**	@fn releaseNode()
	 *	the thread will setSocketUsing() of a parent node when retrieving Packet;
	 *	this call signal the PacketRequest to release node, but not the mutex of Socket (can assume setSocketUsing())
	 *	@return true the node is successfully released, while the socket grant is maintained
	 *	@return false the node cannot be released (because deleted)
	 */
	bool releaseNode();

	/**	@fn getNode()
	 *	get the currently occupied VMeshRemoteNode
	 *	@return the currently occupied VMeshRemoteNode
	 *	@return NULL pointer if no nodes are occupied currently
	 */
	VMeshRemoteNode* getNode();

	/**	@fn stop()
	 *	stop running the thread
	 */
	void stop();

	int increaseTransferRate(unsigned int step);

	int decreaseTransferRate(unsigned int step);

	bool isActive();

protected:
	/**	@fn execute()
	 *	Override the execute() in Thread
	 */
	void* execute();

private:
	bool keepRunning;
	bool waitingRelease;
	int desiredSegment;
	MediaInfo* myMediaInfo;
	SchedulerQueue* myQueue;
	//VMeshRemoteNodeList* myParentList;
	VMeshLocalNodeList* myNodeList;
	Buffer* myBuffer;
	Storage* myStorage;
	PacketRequestState myState;
	VMeshRemoteNode* myNode;
	VMeshRemoteNodeList* myRemoteNodeList;
	//HANDLE m_hReset;
	int transferRate;
	int nInactiveCount;
	bool bActive;
	
	int maxConnection;
	int connectionCounter;

	DWORD start, timeTaken[10];
	int timerCounter;

	ExchangeTable* myExchangeTable;

	void savePacket(char* recvmsg,int recvlen,char PacketDest,int PacketID,bool bTimeLog);
};
_VMESH_NAMESPACE_FOOTER
#endif

