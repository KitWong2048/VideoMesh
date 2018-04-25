#ifndef _EXCHANGE_TABLE_H_
#define _EXCHANGE_TABLE_H_

#include "../Node/VMeshRemoteNodeList.h"
#include "../Node/VMeshRemoteNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Node/VMeshLocalNode.h"
#include "../Common/VMeshCommon.h"
#include <hash_map>
#include "../Util/Logger.h"
#include "../Util/Mutex.h"
#include "../Common/VMeshCommon.h"


using namespace std;
_VMESH_NAMESPACE_HEADER

class ExchangeTable
{
private:

	VMeshRemoteNodeList *neighborNodeList;
	map<int, VMeshRemoteNodeList>neighborMap;
	Mutex myLock;

public:
	/*
	* constructor
	*/
	ExchangeTable();

	/*
	*destructor
	*/
	~ExchangeTable();

	/*
	*
	*/
	VMeshRemoteNode* pop(int _segmentID);

	/*
	*
	*/
	void push(VMeshRemoteNode *_node);

	/*
	*insert the parent table received by table exchange to my routing table
	*/
	void insert(const char* _serial, int _length);

	/*
	*insert a node to my routing table
	*/
	void insert(VMeshRemoteNode *_node);
	
	/*
	*insert a parent list to my routing table
	*/
	void insert(VMeshRemoteNodeList *_list);

	/*
	*get potential parents by segment ID
	*/
	VMeshRemoteNodeList * getParentBySegID(int _segmentID);
	
	//bool exist(VMeshRemoteNode* node);
	/*
	*get one parent by segment ID
	*/
	VMeshRemoteNode* getOneParentBySegID(int _segmentID,VMeshRemoteNodeList* failNodes);

	/*
	*
	*/
	void addAccessCount(VMeshRemoteNode* _node);
	void reduceAccessCount(VMeshRemoteNode* _node);

	/*
	*if this node can not be connected, delete it
	*/
	void removeNode(VMeshRemoteNode* _node);
	
	/*
	*get the size of this routing table
	*/
	int getSize();

	/*
	*serialization before sending it
	*/
	char* serialize(int& _length);

	/*
	*
	*/
	VMeshRemoteNodeList* extractNeighborList(int segID, int maxNum);

	/*
	*get nearest neighbors which appear in my routing table
	*/
	VMeshRemoteNodeList* getNearestNeighbor(int maxNum);


	/*
	*show all peers in my routing table
	*/
	void showAllByPingTime();

	void showAllBySegmentID();

	/*
	*get node by index
	*/
	VMeshRemoteNode* getNodeByIndex(int index);

};

_VMESH_NAMESPACE_FOOTER
#endif