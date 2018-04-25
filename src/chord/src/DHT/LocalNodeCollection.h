/*
LocalNodeCollection.h

Copyright (c) 2007 Hong Kong University of Science and Technology ("HKUST")
This source code is an intellectual property owned by HKUST and funded by
Innovation and Technology Fund (Ref No. GHP/045/05)

Permission is hereby granted, to any person and party obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the
Software with the rights to use, copy, modify and merge, subject to the
following conditions:

1. Commercial use of this source code or any derivatives works from this source
 code in any form is not allowed

2. Redistribution of this source code or any derivatives works from this source
 code in any form is not allowed

3. Any of these conditions can be waived if you get permission from the
copyright holder

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**	@file LocalNodeCollection.h
 *	This file contains the class to abstract a collection of vitrual nodes in DHT.
 */

#ifndef _H_LOCAL_NODE_COLLECTION_
#define _H_LOCAL_NODE_COLLECTION_

#include <map>
#include <set>
#include "../Util/Common.h"
#include "../Util/Mutex.h"
#include "../DHT/LocalNodeHandle.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	@class LocalNodeCollection
	 *	This class declares a structure to handle a collection of virtual nodes in DHT.
	 */
	class LocalNodeCollection{
	private:
		/**	@var localNodes
		 * The storage of all local nodes.
		 * ID of the node is used as the mapping key.
		 */
		map<DHTNetworkID, LocalNodeHandle*> localNodes;

		/**	@var garbageNodes
		 * The storage of all local nodes that are going to be removed.
		 * The garbage will actually be deleted when there is no more pointer referenceing the object.
		 */
		list<LocalNodeHandle*> garbageNodes;

		/**	@var nodeLock
		 * Lock for exclusive access to localNodes.
		 */
		Util::Mutex nodeLock;

		/** @var chord
		 * A reference to the DHT network
		 */
		 Chord* chord;

		/** @var log
		 * The reference used for logging.
		 */
		Logger* log;

		void doHouseKeeping(int _opCode);

	public:
		LocalNodeCollection(Chord& _chord);

		~LocalNodeCollection();

		/**	@fn int getSize()
		 *	@return number of nodes in this collection.
		 *	This function returns the number of nodes in this collection.
		 */
		int getSize();

		/**	@fn LocalNodeHandle* getLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID of the node to be returned.
		 *	@return a pointer to the node if found; NULL otherwise.
		 *	This function returns a node from the collection if that node has the ID requested.
		 */
		LocalNodeHandle* getLocalNode(const DHTNetworkID& _id);

		/**	@fn LocalNodeHandle* getPrecedingLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID which the return node should preceed to.
		 *	@return a local node which is proceed to the given ID.
		 *	This function finds a local node from the collection which is proceeding to the given ID.
		 */
		LocalNodeHandle* getPrecedingLocalNode(const DHTNetworkID& _id);

		/**	@fn LocalNodeHandle* getSucceedingLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID which the return node should succeed to.
		 *	@return a local node which is succeed to the given ID.
		 *	This function finds a local node from the collection which is succeeding to the given ID.
		 */
		LocalNodeHandle* getSucceedingLocalNode(const DHTNetworkID& _id);

		/**	@fn int addNode(const LocalNodeHandle& _localNode)
		 *	@param _localNode: the node to be added.
		 *	@return 0 if the node is added.
		 *	This function adds a node to the collection if that node does not exist.
		 *	This function updates the node if it is found in the collection.
		 */
		int addNode(const LocalNodeHandle& _localNode);

		/**	@fn int removeNode(const DHTNetworkID& _id)
		 *	@param _id: the ID of the node to be removed.
		 *	@return 0 if the node is removed.
		 *	This function removes a node from the collection.
		 */
		int removeNode(const DHTNetworkID& _id);

		/**	@fn void addReference(const DHTNetworkID& _localNodeID, const Node& _finger)
		*	@param _localNodeID: The ID of the local node to which the reference is added.
		 *	@param _finger: The reference node to be added.
		 *	This function allows adding a new reference into or updating the entry in the reference set of a local node.
		 */
		void addReference(const DHTNetworkID& _localNodeID, const Node& _finger);

		/**	@fn void removeReference(const DHTNetworkID& _localNodeID, const DHTNetworkID& _fingerID)
		*	@param _localNodeID: The ID of the local node from which the reference is removed.
		 *	@param _fingerID: The ID of the finger to be removed.
		 *	This function allows removing a reference from a local node.
		 */
		void removeReference(const DHTNetworkID& _localNodeID, const DHTNetworkID& _fingerID);

		/**	@fn void stabilize()
		 *	This function runs the stabilization protocol for each node in the collection.
		 */
		void stabilize();

		/**	@fn void fixFingers()
		 *	This function runs the finger fixing protocol for each node in the collection.
		 */
		void fixFingers();

		/**	@fn void visitNeighbor()
		 *	This function checks whether all the neighbor of each node in the collection is alive.
		 */
		void visitNeighbor();

		/**	@fn void print(Util::LogLevel _logLevel)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of each node.
		 */
		void print(Util::LogLevel _logLevel);

		/**	@fn LocalNodeCollection* clone()
		 *	@return a deep copy of the instance which invokes this function
		 *	This function gives a deep copy of the invoker.
		 */
		LocalNodeCollection* clone();

		/**	@fn cleanupGarbage()
		 *	This function removes the garbaged local nodes from the memory.
		 */
		 void cleanupGarbage();

		 void resetNodesUseCount();
	};
}
#endif
