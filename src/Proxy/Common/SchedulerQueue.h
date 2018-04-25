/**	
* SchedulerQueue
*
* For query scheduling purpose
*
* @author Kelvin Chan, Philip Yang
*/
#ifndef _SCHEDULER_QUEUE_H
#define _SCHEDULER_QUEUE_H

#include <list>
#include <algorithm>
#include "../Common/VMeshCommon.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

_VMESH_NAMESPACE_HEADER

/** @struct SchedulerEntry
 *  A struct to store the scheduling packet information
 */
struct SchedulerEntry
{
	unsigned int PacketID;		/*!< packet ID */
	char dest;					/*!< destination: 's' for storage; 'b' for buffer */
};

bool operator==(const SchedulerEntry &e1, const SchedulerEntry &e2);

/**	@class SchedulerQueue
 *	This class provides the queueing mechanism for a packet to be fetchws
 */
class SchedulerQueue
{
public:
	/**	@fn SchedulerQueue()
	 *	Default constructor
	 */
	SchedulerQueue();

	/**	@fn ~SchedulerQueue()
	 *	Default destructor
	 */
	~SchedulerQueue();

	/**	@fn empty()
	 *	check if the queue is empty
	 *	@return true if the queue is empty
	 */
	bool empty();

	
	/**	@fn getFetchingSize()
	 *	return the size of fetching packets
	 *	@return the size of fetching packets
	 */
	int getFetchingSize();

	/**	@fn getWaitingSize()
	 *	return the size of waiting packets
	 *	@return the size of waiting packets
	 */
	int getWaitingSize();

	/**	@fn fetchToBuffer(unsigned int _ID)
	 *	enqueue a PacketID, which will be put in buffer
	 *	@param _ID Packet ID
	 */
	void fetchToBuffer(unsigned int _ID);

	/**	@fn fetchToStorage(unsigned int _ID)
	 *	enqueue a PacketID, which will be put in Storage
	 *	@param _ID Packet ID
	 */
	void fetchToStorage(unsigned int _ID);

	/**	@fn clearAllToBuffer()
	 *	clear all the packets(which will be fetched to buffer only) in the waiting list
	 */
	void clearAllToBuffer();

	/**	@fn pop()
	 *	dequeue a waiting Packet, and put it to the fetching list
	 */
	SchedulerEntry pop();

	/**	@fn success(SchedulerEntry e)
	 *	the retrieval of previously poped Packet ID is success
	 *	remove the ID from the fetchingList
	 *	@param e the SchedulerEntry previously poped
	 */
	void success(SchedulerEntry e);

	
	/**	@fn fail(SchedulerEntry e)
	 *	the retrieval of previously poped Packet ID is fail
	 *	put the ID back into waitingList
	 *	@param e the SchedulerEntry previously poped
	 */
	void fail(SchedulerEntry e);

private:
	Mutex myLock;

	std::list<SchedulerEntry> waitingList[VMESH_PACKET_RETRY];
	std::list<SchedulerEntry> fetchingList[VMESH_PACKET_RETRY];
};

_VMESH_NAMESPACE_FOOTER
#endif

