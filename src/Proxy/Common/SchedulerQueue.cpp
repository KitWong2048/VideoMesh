#include "../Common/SchedulerQueue.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

bool operator==(const SchedulerEntry &e1, const SchedulerEntry &e2)
{
	return (e1.PacketID == e2.PacketID);
}

SchedulerQueue::SchedulerQueue() : myLock("SchedulerQueueLock")
{
	// blank constructor
	
}

SchedulerQueue::~SchedulerQueue()
{
	// blank destructor
	
}

bool SchedulerQueue::empty()
{
	myLock.lock();
	for (int i=0; i<VMESH_PACKET_RETRY; ++i)
	{
		if (!waitingList[i].empty())
		{
			myLock.release();
			return false;
		}
	}
	myLock.release();
	return true;
}

int SchedulerQueue::getFetchingSize()
{
	int size = 0;
	for (int i=0; i<VMESH_PACKET_RETRY; ++i)
		size += fetchingList[i].size();
	return size;
}

int SchedulerQueue::getWaitingSize()
{
	int size = 0;
	for (int i=0; i<VMESH_PACKET_RETRY; ++i)
		size += waitingList[i].size();
	return size;
}

void SchedulerQueue::fetchToBuffer(unsigned int _ID)
{
	myLock.lock();
	SchedulerEntry e;
	e.PacketID = _ID;
	e.dest = 'b';
	// fetching to buffer is at a higher priority at first
	waitingList[1].push_back(e);
	myLock.release();
}

void SchedulerQueue::fetchToStorage(unsigned int _ID)
{
	myLock.lock();
	SchedulerEntry e;
	e.PacketID = _ID;
	e.dest = 's';
	// fetching to storage is at a lower priority at first
	waitingList[0].push_back(e);
	myLock.release();
}

void SchedulerQueue::clearAllToBuffer()
{
	SchedulerEntry x;

	myLock.lock();
	for (int i=VMESH_PACKET_RETRY-1; i>=0; --i)
	{
		int numPacket = (int)waitingList[i].size();
		for (int j=0; j<numPacket && !waitingList[i].empty(); j++)
		{
			x = waitingList[i].front();
			waitingList[i].pop_front();
			if (x.dest!='b')
				waitingList[i].push_back(x);
		}
	}
	myLock.release();
}

SchedulerEntry SchedulerQueue::pop()
{
	SchedulerEntry x;
	x.PacketID = 0;
	x.dest = 'x';

	myLock.lock();
	for (int i=VMESH_PACKET_RETRY-1; i>=0; --i)
	{
		if (!waitingList[i].empty())
		{
			// move the packet ID from waiting to fetching list
			x = waitingList[i].front();
			fetchingList[i].push_back(x);
			waitingList[i].pop_front();
			myLock.release();
			return x;
		}
	}
	myLock.release();
	return x;	
}

void SchedulerQueue::success(SchedulerEntry e)
{
	myLock.lock();
	for (int i=0; i<VMESH_PACKET_RETRY; ++i)
	{
		fetchingList[i].remove(e);
	}
	myLock.release();
}

void SchedulerQueue::fail(SchedulerEntry e)
{
	if ( e.dest == 'x' )
		return;
	myLock.lock();
	for (int i=0; i<VMESH_PACKET_RETRY; ++i)
	{
		list<SchedulerEntry>::iterator result = find(fetchingList[i].begin(), fetchingList[i].end(), e);
		if (result!=fetchingList[i].end())
		{
			// move it from fetchingList[i] to waitingList[i+1] if stored in buffer
			// move it from fetchingList[i] to the first of waitingList[i] if stored in storage
			// *always preserve priority of buffer over storage
			// discard the packet if it is the last time
			fetchingList[i].remove(e);
			if ( i<VMESH_PACKET_RETRY-1  )
			{
				if ( e.dest == 'b' )
					waitingList[i+1].push_back(e);
				else
					waitingList[i].push_front(e);
			}
			else if (i==(VMESH_PACKET_RETRY-1) && e.dest=='s')
			{
				// stay in the waitingList if it's destination is storage
				waitingList[i].push_back(e);
			}else
			{
				LOG_ERROR("Packet " + itos(e.PacketID) + " is dropped from the SchedulerQueue");
				waitingList[i].push_back(e);
				usleep(100000);		// 0.1s
			}
			break;
		}
	}
	myLock.release();
}

_VMESH_NAMESPACE_FOOTER
