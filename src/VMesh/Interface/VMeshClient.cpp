#include "../Interface/VMeshClient.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

VMeshClient::VMeshClient(string _myIP, int _myPort, string _remoteIP, int _remotePort, int _localStorageCapacity, int _clientExchangeNeighbor)
: myVMesh(_myIP, _myPort, _localStorageCapacity, CLIENT_IDENTITY, _clientExchangeNeighbor)
{
	temp_remoteIP = _remoteIP;
	temp_remotePort = _remotePort;
	currentTime = 0;
	fetchedAID = 0;
	fetchedVID = 0;
	nextAID = 0;
	nextVID = 0;
	prefetch = 1000;
}

VMeshClient::~VMeshClient()
{
	return;
}
	
bool VMeshClient::start()
{
	currentTime = 0;
	return true;
}

void VMeshClient::stop()
{
	myVMesh.stop();
	return;
}

bool VMeshClient::movieJoin(const char* _key)
{
	string temp_key (_key);
	if (!myVMesh.join(temp_key, temp_remoteIP, temp_remotePort))
	{
		LOG_ERROR("VMesh starts fails: Error in join");
		return false;
	}
	if (!myVMesh.start())
	{
		LOG_ERROR("VMesh starts fails: Error in start");
		return false;
	}

	MediaInfo* info = myVMesh.getMediaInfo();

	// estimate the average size of a packet
	int avgSize = info->getFileSize()/(unsigned int)(info->getNumSegments() * info->getNumPacketsinSegment(0, 0));
	LOG_DEBUG("Estimated packet size: " + itos(avgSize));

	// number of packets to prefetch
	prefetch = (int)PREFETCH_SIZE/avgSize;
	LOG_DEBUG("Prefetch Buffer: " + itos(prefetch) + " packets");

	return true;
}

bool VMeshClient::movieLeave(const char* _key)
{
	return true;
}

MediaInfo* VMeshClient::getMediaInfo(const char* _key)
{
	return myVMesh.getMediaInfo();
}

void VMeshClient::setTime(int _sec)
{
	myVMesh.clear_fetch();
	fetchedAID = 0;
	fetchedVID = 0;
	while (fetchedAID<=0 && fetchedVID<=0)
		if (!myVMesh.fetch(_sec++, nextAID, fetchedAID, nextVID, fetchedVID))
			break;
	currentTime = _sec - 1;
}


Packet* VMeshClient::getPacket(int _type)
{
	Packet* pac = NULL;
	if (_type==0)
	{
		pac = myVMesh.getPacket(_type, nextAID);
		if (pac!=NULL)
		{
			nextAID++;
			fetchedAID--;
		}
		else
		{
			//LOG_DEBUG("" + itos(currentTime) + " is fetched, nextAID = " + itos(nextAID) + ", fetchedAID = " + itos(fetchedAID));
		}
	}
	else
	{
		pac = myVMesh.getPacket(_type, nextVID);
		if (pac!=NULL)
		{
			nextVID++;
			fetchedVID--;
		}
	}

	while ((fetchedAID + fetchedVID)<prefetch)
	{
		unsigned int dummy1, dummy3;
		int dummy2, dummy4;

		myVMesh.fetch(++currentTime, dummy1, dummy2, dummy3, dummy4);
		fetchedAID += dummy2;
		fetchedVID += dummy4;
		//LOG_DEBUG("" + itos(currentTime) + " is fetched, nextAID = " + itos(nextAID) + ", fetchedAID = " + itos(fetchedAID) + ", added = " + itos(dummy2));
		if (dummy2>0 || dummy4>0)
			break;
		if (currentTime > myVMesh.getMediaInfo()->getMediaLength(0)/1000)
			break;
	}
	return pac;
}

bool VMeshClient::existPacket(int _type)
{
	if (_type==0)
		return myVMesh.existPacket(_type, nextAID);
	else
		return myVMesh.existPacket(_type, nextVID);
}

_VMESH_NAMESPACE_FOOTER
