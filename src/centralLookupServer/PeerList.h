#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

#include "Common.h"
#include "Node.h"

struct PeerRecord
{
	std::string	hashKey;
	time_t		lastUpdate;
	int peer_identity;
};

// PeerList
// Stores a number of peers containing the specific Segment
class PeerList
{
public:
	PeerList(std::string hash);
	~PeerList(void);
	int addNode(Node node,int identity);
	int removeNode(Node node); 
	Node* lookup(int segment, int& count,int identity);

private:
	std::vector<PeerRecord>::iterator findWithHash(std::vector<PeerRecord>::iterator begin, std::vector<PeerRecord>::iterator end, std::string hashKey);

	std::vector<PeerRecord> SegmentPeer[SEGMENT_SIZE];
	CRITICAL_SECTION sectionLock[SEGMENT_SIZE];
	std::string mediaHash_;
};
