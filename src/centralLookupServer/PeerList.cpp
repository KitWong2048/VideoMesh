
#include "PeerList.h"

PeerList::PeerList(std::string hash) : mediaHash_(hash)
{
	/* initialize random seed: */
	srand ( time(NULL) );
	for(int i= 0; i< SEGMENT_SIZE; i++)
		::InitializeCriticalSection(&sectionLock[i]);
}

PeerList::~PeerList(void)
{
	for(int i= 0; i< SEGMENT_SIZE; i++)
		::DeleteCriticalSection(&sectionLock[i]);
}

int PeerList::addNode(Node node,int identity)
{
	int segment = node.getSegment();
	
	std::vector<PeerRecord>::iterator it;
	
	// Add the node to every segment for bootstrap
	if (segment == BOOTSTRAP_SEGMENTID)
	{
		/*
		for(int i= 0; i< SEGMENT_SIZE; i++)
		{
			PeerRecord record;
			record.hashKey = node.genHashKey();
			record.lastUpdate = 2147483647;   //consider modification
			record.peer_identity=node.getPeerIdentity();
			SegmentPeer[i].push_back(record);
		}*/
		return 0;
	}

	PeerRecord record;
	::EnterCriticalSection(&sectionLock[segment]);
	record.hashKey = node.genHashKey();
	record.lastUpdate = time(NULL);
	record.peer_identity=node.getPeerIdentity();
	it = findWithHash(SegmentPeer[segment].begin(), SegmentPeer[segment].end(), node.genHashKey());
	// Add the Node when it is not found in the current record
	if(it != SegmentPeer[segment].end())
	{
		it->lastUpdate = time(NULL);
		::LeaveCriticalSection(&sectionLock[segment]);
		return -1;
	}
	SegmentPeer[segment].push_back(record);
	::LeaveCriticalSection(&sectionLock[segment]);
	return 0;
}

int PeerList::removeNode(Node node)
{
	std::vector<PeerRecord>::iterator it;
	
	for(int i= 0; i< SEGMENT_SIZE; i++)
	{
		node.setInfo(node.getMediaHash(), i, node.getIP(), node.getPort(), node.getPeerIdentity());
		// iterator to vector element
		::EnterCriticalSection(&sectionLock[i]);
		it = findWithHash (SegmentPeer[i].begin(), SegmentPeer[i].end(), node.genHashKey());
		if(it != SegmentPeer[i].end())
			SegmentPeer[i].erase(it);
		::LeaveCriticalSection(&sectionLock[i]);
	}
	return 0;
}

Node* PeerList::lookup(int segment, int& count, int identity)
{
	
	if(segment > SEGMENT_SIZE)
	{
		count = 0;
		return NULL;
	}

	/*delete out data peers*/
	std::vector<PeerRecord>::iterator it; // iterator to vector element

	std::vector<PeerRecord> lookup_result;
	lookup_result.clear();//just in case

	::EnterCriticalSection(&sectionLock[segment]);
	for(it=SegmentPeer[segment].begin();it != SegmentPeer[segment].end();)
	{
			// Erase the out dated Node
		if((time(NULL) - it->lastUpdate) > NODE_TIMEOUT)
		{
			printf("%s(%ld) Out Dated\n", it->hashKey.c_str(), it->lastUpdate);
			it = SegmentPeer[segment].erase(it);
			//numOfOutDated++;
		}
		else
			it++;
	}
	lookup_result.insert(lookup_result.begin(),SegmentPeer[segment].begin(),SegmentPeer[segment].end());

	::LeaveCriticalSection(&sectionLock[segment]);

	/*filter out undesirable peers according to the identity of the caller*/
	if(identity==PROXY_IDENTITY||identity==SERVER_IDENTITY)
	{
		for(it=lookup_result.begin();it!=lookup_result.end();)
		{
			if(it->peer_identity==CLIENT_IDENTITY)
			{
				it=lookup_result.erase(it);
			}
			else
				it++;
		}
	}

	//count=count>lookup_result.size()?lookup_result.size():count;
	//printf("%d Out Dated Node is Deleted\n", numOfOutDated);
	if(count>lookup_result.size())
	{
		count=lookup_result.size();
	}
	else
	{
		while(lookup_result.size()>count)
		{
			int rnum = rand() % (lookup_result.size());
			lookup_result.erase(lookup_result.begin()+rnum);
		}
	}
	
	Node* peer = new Node[count];
	for(int i =0; i< count; i++)
	{
		peer[i].setIDHex(lookup_result[i].hashKey.c_str());
		peer[i].setPeerIdentity(lookup_result[i].peer_identity);		// Jack's comment:: for checking the identity of lookup node
	}
	return peer;

}


//Node* PeerList::lookup(int segment, int& count)
//{
//	// iterator to vector element:
//	std::vector<PeerRecord>::iterator it;
//
//	if(segment > SEGMENT_SIZE)
//	{
//		count = 0;
//		return NULL;
//	}
//
//	::EnterCriticalSection(&sectionLock[segment]);
//	if(SegmentPeer[segment].size() < count)
//	{
//		int numOfOutDated = 0;
//		count = SegmentPeer[segment].size();
//		std::vector<PeerRecord>::iterator it = SegmentPeer[segment].begin();
//		while(it != SegmentPeer[segment].end())
//		{
//			// Erase the out dated Node
//			if((time(NULL) - it->lastUpdate) > NODE_TIMEOUT)
//			{
//				printf("%s(%ld) Out Dated\n", it->hashKey.c_str(), it->lastUpdate);
//				it = SegmentPeer[segment].erase(it);
//				numOfOutDated++;
//				count--;
//			}
//			else
//			{
//				it++;
//			}
//		}
//		printf("%d Out Dated Node is Deleted\n", numOfOutDated);
//	}
//	else
//	{
//		// Random count number of SegmentPeer and push at the front of the vector
//		int numOfOutDated = 0;
//		for(int i =0; i< count; i++)
//		{
//			int rnum = rand() % (SegmentPeer[segment].size() - i - numOfOutDated);
//			if((time(NULL) - (SegmentPeer[segment].begin()+i)->lastUpdate) >NODE_TIMEOUT)
//			{
//				numOfOutDated++;
//				iter_swap(SegmentPeer[segment].begin()+i, SegmentPeer[segment].end()-numOfOutDated);
//				printf("%s(%ld) Out Dated\n", (SegmentPeer[segment].end()-numOfOutDated)->hashKey.c_str(), (SegmentPeer[segment].end()-numOfOutDated)->lastUpdate);
//				if((i + 1 + numOfOutDated) >= SegmentPeer[segment].size() )
//				{
//					count = i;
//					break;
//				}
//				rnum = rand() % (SegmentPeer[segment].size() - i - numOfOutDated);
//			}
//			iter_swap(SegmentPeer[segment].begin()+i, SegmentPeer[segment].begin()+i+rnum);
//		}
//		SegmentPeer[segment].erase (SegmentPeer[segment].end()-numOfOutDated,SegmentPeer[segment].end());
//		printf("%d Out Dated Node is Deleted\n", numOfOutDated);
//	}
//	
//	Node* peer = new Node[count];
//	for(int i =0; i< count; i++)
//	{
//		peer[i].setIDHex(SegmentPeer[segment][i].hashKey.c_str());
//	}
//	::LeaveCriticalSection(&sectionLock[segment]);
//
//	return peer;
//}

std::vector<PeerRecord>::iterator PeerList::findWithHash(std::vector<PeerRecord>::iterator begin, std::vector<PeerRecord>::iterator end, std::string hashKey)
{
	std::vector<PeerRecord>::iterator it;
	for(it = begin; it != end; it++)
	{
		if(it->hashKey == hashKey)
			return it;
	}
	return it;
}
