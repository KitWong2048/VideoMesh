#ifndef _VMESH_REMOTE_NODE_LIST_H
#define _VMESH_REMOTE_NODE_LIST_H

#include <vector>
#include <algorithm>
#include "../Common/VMeshCommon.h"
#include "../Node/VMeshRemoteNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

/**	@file VMeshRemoteNodeList.h
 *	This file contains the definition of class VMeshRemoteNodeList
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshRemoteNodeList
 *	This class provides the an abstract of collection of VMeshRemoteNode
 */
class VMeshRemoteNodeList
{
public:
	/**	@fn VMeshRemoteNodeList()
	 *	Default constructor for a blank list;
	 */
	VMeshRemoteNodeList();
	
	/**	@fn VMeshRemoteNodeList(VMeshLocalNodeList& oldList)
	 *	Copy constructor from a VMeshLocalList, which represents the same nodes;
	 *	@param oldList VMeshLocalNodeList to be copied
	 */
	VMeshRemoteNodeList(VMeshLocalNodeList& oldList);

	/**	@fn VMeshRemoteNodeList(const char* _serial, int _length)
	 *	Constructor using serialized char array;
	 *	@param _serial serialized char array
	 *	@param _length length of the array
	 */
	VMeshRemoteNodeList(const char* _serial, int _length);
	
	/**	@fn ~VMeshRemoteNodeList()
	 *	Destructor;
	 */
	~VMeshRemoteNodeList();

	/**	@fn serialize(int& _length)
	 *	Creates a serialized char array for this object;
	 *	@param _length length of the array
	 *	@return pointer of the serialized char array
	 */
	char* serialize(int& _length, bool isKeepBoostrap);

	/**	@fn getSize()
	 *	get the number of VMeshRemoteNode in the list;
	 *	@return the number of VMeshRemoteNode in the list
	 */
	int getSize();

	/**	@fn clearall()
	 *	remove all the nodes in the list;
	 *	only shallow deletion is performed and reset all pointers in this list;
	 */
	void clearall();
	
	/**	@fn addNode(VMeshRemoteNode* _node)
	 *	add a VMeshRemoteNode to this list;
	 *	shallow copy is done;
	 *	@param _node the pointer of VMeshRemoteNode to be added
	 */
	void addNode(VMeshRemoteNode* _node);

	/**	@fn removeNode(VMeshRemoteNode* _node)
	 *	remove the VMeshRemoteNode from the list if they points to the same location;
	 *	only shallow deletion is performed;
	 *	@param _node the pointer of VMeshRemoteNode to be removed
	 *	@return true if the node is deleted successfully
	 */
	
	bool sharrowRemoveNode(VMeshRemoteNode* _node);

	bool deepRemoveNode(VMeshRemoteNode* _node);
	
	void addNodeByPingTime(VMeshRemoteNode* _node);
	

	/**	@fn getSegmentNode(int n)
	 *	get an avaliable node that is with segment n, in which isSocketUsing() must return false;
	 *	if the number of nodes (neglecting isSocketusing() results) of segment n <= CONNECT_BOOTSTRAP_THRESHOLD, then the bootstrap may be returned;
	 *	@param n the segment ID
	 *	@return the pointer of an avaliable VMeshRemoteNode that is holding segment n if exist
	 *	@return NULL if failed to find such a node
	 */
	VMeshRemoteNode* getSegmentNode(int n);
	
	/**	@fn getNode(int n)
	 *	get the VMeshRemoteNode in the list at position n;
	 *	the order of node in the list is conserved until sortSegmentID() or sortPingTime() is called;
	 *	@param n the position
	 *	@return the pointer of VMeshRemoteNode at position n
	 *	@return NULL if there are no VMeshRemoteNode at position n
	 */
	VMeshRemoteNode* getNode(int n);

	/**	@fn exist(VMeshRemoteNode* node)
	 *	check if this node is in the list already by checking their IP address, port and segment ID;
	 *	segment ID would not be checked if node is with INITIAL_SEGMENTID;
	 *	@param node the pointer of VMeshRemoteNode to check existence
	 *	@return the true if the node exists in the list already, false otherwise
	 */
	int exist(VMeshRemoteNode* node);

	void updatePingTime(int index, VMeshRemoteNode* _node);
	/**	@fn merge(VMeshRemoteNodeList& _list)
	 *	merge another VMeshRemoteNodeList to this list;
	 *	transfer the nodes from _list to this list;
	 *	_list is empty after calling this funtion;
	 *	@param _list the VMeshRemoteNodeList to be merged
	 */
	void merge(VMeshRemoteNodeList* _list);

	/**	@fn extractNeighbourList(int segID, int maxNum)
	 *	creates a sub-list that contains VMeshRemoteNode of neighboring segment ID;
	 *	these ID include segID-1, segID, segID+1;
	 *	maxNum is the maximum number of nodes included for each segment ID;
	 *	so there are at most 3*maxNum nodes inside;
	 *	@param segID the segment ID targetted
	 *	@param maxNum the maximum number of nodes included for each segment ID
	 *	@return the extracted VMeshRemoteNodeList containing neighboring segment
	 */
	VMeshRemoteNodeList* extractNeighborList(int segID, int maxNum);

	VMeshRemoteNodeList* getNearestNeighbor(int maxNum);

	/**	@fn sortSegmentID()
	 *	sort the nodes in ascending order of the segmentID;
	 *	after sorted, the order of the nodes in the list will be changed;
	 *	the order in getNode() will be affected;
	 *	Bubblesort is used;
	 */
	void sortSegmentID();

	/**	@fn sortPingTime()
	 *	sort the nodes in ascending order of the ping time;
	 *	after sorted, the order of the nodes in the list will be changed;
	 *	the order in getNode() will be affected;
	 *	Bubblesort is used;
	 */
	void sortPingTime();


	/**	@fn showall()
	 *	log all the node information in the list in info message
	 */
	void showall();

private:
	/**	@fn removeNULL()
	 *	for internal use only
	 *	remove the null pointers in the list
	 */
	void removeNULL();

	//Mutex myLock;

	std::vector<VMeshRemoteNode*> myList;

};
_VMESH_NAMESPACE_FOOTER
#endif

