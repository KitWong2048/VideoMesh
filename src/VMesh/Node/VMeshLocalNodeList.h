#ifndef _VMESH_LOCAL_NODE_LIST_H
#define _VMESH_LOCAL_NODE_LIST_H

#include <vector>
#include "../Common/VMeshCommon.h"
#include "../Node/VMeshLocalNode.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

/**	@file VMeshLocalNodeList.h
 *	This file contains the definition of class VMeshLocalNodeList
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshLocalNodeList
 *	This class provides the an abstract of collection of VMeshLocalNode
 */
class VMeshLocalNodeList {
public:
	/**	@fn VMeshLocalNodeList()
	 *	Default constructor for a blank list;
	 */
	VMeshLocalNodeList();
	
	/**	@fn ~VMeshLocalNodeList()
	 *	Default destructor and destruct all the VMeshLocalNode in the list;
	 */
	~VMeshLocalNodeList();

	/**	@fn getSize()
	 *	get the number of VMeshLocalNode in the list;
	 *	@return the number of VMeshLocalNode in the list
	 */
	int getSize();

	/**	@fn addNode(VMeshLocalNode* _node)
	 *	add a VMeshLocalNode to this list;
	 *	shallow copy is done;
	 *	@param _node the pointer of VMeshLocalNode to be added
	 */
	void addNode(VMeshLocalNode* _node);
	
	/**	@fn getNodeBySegmentID(int _segmentID)
	 *	get the VMeshLocalNode in the list with _segmentID;
	 *	if there is a bootstrap node, this node would be always returned, because it contains all segments;
	 *	@param _segmentID the segment ID
	 *	@return the pointer of VMeshLocalNode containing the segment
	 *	@return NULL if the segment does not exists in the list
	 */
	VMeshLocalNode* getNodeBySegmentID(int _segmentID);

	/**	@fn getNode(int i)
	 *	get the VMeshLocalNode in the list at position i;
	 *	the order of node in the list is conserved after addNode();
	 *	@param i the position
	 *	@return the pointer of VMeshLocalNode at position i
	 *	@return NULL if there are no VMeshLocalNode at position i
	 */
	VMeshLocalNode* getNode(int i);

private:
	std::vector<VMeshLocalNode*> myList;
};
_VMESH_NAMESPACE_FOOTER
#endif
