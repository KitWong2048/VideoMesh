#pragma once
#include "LookUpCommon.h"
#include "NetworkID.h"
#include <list>

/**
* The abstract class for managing segment information
*
*/
class SegmentManager
{
public:
	SegmentManager(void) {};
	virtual ~SegmentManager(void);

	virtual int run() = 0;

	virtual int stop() = 0;

	// add a segment
	virtual int addNode(const NetworkID& _id) = 0;

	// remove a segment
	virtual int removeNode(const NetworkID& _id) = 0;

	virtual NetworkID* lookup(const NetworkID& _id, int& _hopCount) = 0;

	virtual NetworkID* lookupFullPath(const NetworkID& _id, std::list<NetworkID*>& _searchPath, int& _hopCount) = 0;

	virtual std::list<NetworkID*> getNeighbor(const NetworkID& _id) = 0;

	virtual void print() = 0;

	virtual bool isRunning() = 0;

	virtual int join(const NetworkID& _id, const std::string _hash) = 0;
};
