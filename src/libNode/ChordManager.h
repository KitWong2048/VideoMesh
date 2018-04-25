#pragma once
#include "LookUpCommon.h"
#include "SegmentManager.h"
#include "ChordNetworkID.h"

#include "../chord/src/DHT/DHTNetworkID.h"
#include "../chord/src/DHT/Node.h"

#include <string>
#include <list>

namespace App 
{
	class ChordApp;
}

class ChordManager :
	public SegmentManager
{
public:
	ChordManager(std::string _myIP, int _myPort);
	virtual ~ChordManager(void);

	virtual int run();

	virtual int stop();

	virtual int addNode(const NetworkID& _id);

	virtual int removeNode(const NetworkID& _id);

	virtual NetworkID* lookup(const NetworkID& _id, int& _hopCount);

	virtual NetworkID* lookupFullPath(const NetworkID& _id, list<NetworkID*>& _searchPath, int& _hopCount);

	virtual std::list<NetworkID*> getNeighbor(const NetworkID& _id);

	virtual void print();

	virtual bool isRunning();

	virtual int join(const NetworkID& _id, const string _hash);

private:
	std::string genAppValue();

	App::ChordApp* myChord_;
	std::string myIP_;
	int myPort_;
};
