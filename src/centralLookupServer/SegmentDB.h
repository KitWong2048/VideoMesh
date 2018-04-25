#pragma once
#include <string.h>
#include <string>
#include <map>

#include "PeerList.h"
#include "Node.h"
//#include "../VMesh/Common/VMeshCommon.h"

class SegmentDB
{
public:
	SegmentDB(void);
	~SegmentDB(void);

	std::string Query(const char* command, const char* para, const char* data, int identity);

private:
	int addNode(Node node, int identity);
	int removeNode(Node node, int identity);
	int createMovie(Node node);
	int deleteMovie(Node node);
	std::string nodeLookup(Node node, int identity);

	// map a hashed segment name to a list of peers containinig it

	std::map<std::string, PeerList*> moviesDB_;
	
};
