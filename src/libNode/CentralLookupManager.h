#pragma once
#include "LookUpCommon.h"
#include "SegmentManager.h"
#include "CentralLookupNetworkID.h"
#include "AsioSyncSocket.h"

#include <time.h>

#include <string>
#include <list>
#include <algorithm>

#define DEFAULT_LOOKUP_PORT	10080

using std::string;


class CentralLookupManager :
	public SegmentManager
{
public:
	CentralLookupManager(string _myIP, int _myPort, int _indentity);
	virtual ~CentralLookupManager(void);

	virtual int run();

	virtual int stop();

	virtual int addNode(const NetworkID& _id);

	virtual int removeNode(const NetworkID& _id);

	virtual NetworkID* lookup(const NetworkID& _id, int& _hopCount);

	virtual NetworkID* lookupFullPath(const NetworkID& _id, list<NetworkID*>& _searchPath, int& _hopCount);

	virtual std::list<NetworkID*> getNeighbor(const NetworkID& _id);

	virtual void print();

	// heart beat checking
	// check if the (node/server) is still running
	virtual bool isRunning();

	virtual int join(const NetworkID& _id, const string _hash);

	// the list of nodes of the overlay mesh
	static std::list<string> myNodeList;

	// the ip of the sever
	static string serverIP_;

	// periodically refresh the list to reflect the most recent status
	static time_t lastRefresh;

	string sendToServer(const string& server_ip, int port, const string& command, const string& hash, const string& nodeListString="");

	void RefreshThreadFunc();

private:
	string myIP_;
	int myPort_;
	int identity;
	string myHash_;	
	int serverPort_;
	bool running;
	bool joined;
	bool isBootstrap;
	HANDLE refreshThHandle;
	AsioSyncSocket sock;
};
