/**
* Table Exchange Manager
* 
* The peer will periodically solicit its neighbors for 
* segment table updating information
*/
#pragma once
#include "LookUpCommon.h"
#include "SegmentManager.h"
#include "TableExchangeLookupNetworkID.h" // this will be changed to something else
#include "AsioSyncSocket.h"
#include <time.h>
#include <string>
#include <list>
#include <algorithm>

#define DEFAULT_LOOKUP_PORT	10080

class TableExchangeLookupManager :
	public SegmentManager
{
public:
	TableExchangeLookupManager(string _myIP, int _myPort);
	virtual ~TableExchangeLookupManager(void);

	virtual int run();

	virtual int stop();

	virtual int addNode(const NetworkID& _id);

	virtual int removeNode(const NetworkID& _id);

	// get the node containing the required segment
	virtual NetworkID* lookup(const NetworkID& _id, int& _hopCount);

	virtual NetworkID* lookupFullPath(const NetworkID& _id, list<NetworkID*>& _searchPath, int& _hopCount);

	virtual std::list<NetworkID*> getNeighbor(const NetworkID& _id);

	virtual void print();

	virtual bool isRunning();

	virtual int join(const NetworkID& _id, const string _hash);

	//static std::list<string> myNodeList;
	//static string serverIP_;
	//static time_t lastRefresh;

	//string sendToServer(const string& server_ip, int port, const string& command, const string& hash, const string& content = "");

	void RefreshThreadFunc();

private:
	string myIP_;
	int myPort_;
	string myHash_;	
	int serverPort_;
	bool running;
	bool joined;
	bool isBootstrap;
	HANDLE refreshThHandle;
	AsioSyncSocket sock;
};
