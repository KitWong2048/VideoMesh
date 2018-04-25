/*
LocalNodeHandle.h

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

/**	@file LocalNodeHandle.h
 *	This file contains the class for encapsulating the node local to the application.
 */

#ifndef _H_LOCAL_NODE_HANDLE_
#define _H_LOCAL_NODE_HANDLE_

#include "../DHT/FingerTable.h"
#include "../DHT/NodeBucket.h"
#include "../DHT/DHTRequest.h"
#include "../Util/Mutex.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	@class LocalNodeHandle
	 *	This class declares the node local to the application.
	 */
	class LocalNodeHandle{
	private:
		/**	@var node
		 * The node which is encapsulated in the instance.
		 */
		Node node;

		/**	@var *chord
		 * The Chord DHT network which the node belongs to.
		 */
		Chord* chord;

		/**	@var predecessor
		 * Predecessor of this local node.
		 */
		Node predecessor;

		/**	@var fingers
		 * Finger table of this local node.
		 */
		FingerTable* fingers;

		/**	@var successors
		 * Successor list of this local node, note that the immediate successor is always the 1st entry in finger table, this list acts as a buffer to prevent failure``	.
		 */
		NodeBucket* successors;

		/** @var log
		 * The reference used for logging.
		 */
		Logger* log;

		/** @var fixFingerIndex
		 * The index of the next finger to fix.
		 */
		int fixFingerIndex;

		/** @var useCount
		 * The number of pointers pointer to the object.
		 */
		int useCount;

		/** @var localNodeMutex
		 * The mutex for the object.
		 */
		Util::Mutex localNodeMutex;


	public:
		/** @fn LocalNodeHandle(const Node& _node, Chord& _chord)
		 *	A constructor.
		 *	@param _node: Physical node information for the node of this handler.
		 *	@param _chord: A pointer to the chord object to register.
		 */
		LocalNodeHandle(const Node& _node, Chord& _chord);

		LocalNodeHandle(const LocalNodeHandle& _localNode);

		~LocalNodeHandle();

		Node getNode() const;

		int setNode(const Node& _node);

		Chord* getChord();

		void setChord(Chord* _chord);

		/** @fn Node* getSuccessor()
		 *	@return a pointer to the successor of local node
		 *	Find the successor of the local node.
		 */
		Node* getSuccessor();

		/** @fn Node* getPredecessor()
		 *	@return a pointer to the predecessor of local node
		 *	Find the predecessor of the local node.
		 */
		Node* getPredecessor();

		/** @fn Node* findSuccessor(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL)
		 *	@param _id: The ID to search for.
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@param _hint: If a node is given as a hunt, it will be asked for successor at the first step in hope of speeding up the process
		 *	@return a pointer to the successor node
		 *	Find the successor of the given ID
		 */
		Node* findSuccessor(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL);

		/** @fn Node* findPredecessor(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL)
		 *	@param _id: The ID to search for.
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@param _hint: If a node is given as a hunt, it will be asked for successor at the first step in hope of speeding up the process
		 *	@return a pointer to the predecessor node
		 *	Find the predecessor of the given ID
		 */
		Node* findPredecessor(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL);

		/** @fn bool findSuccPred(const DHTNetworkID& _id, Node*& _pred, Node*& _succ, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL)
		 *	@param _id: The ID to search for.
		 *	@param _pred: This outputs the predecessor of the search ID
		 *	@param _succ: This outputs the successor of the search ID
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@param _hint: If a node is given as a hunt, it will be asked for successor at the first step in hope of speeding up the process
		 *	@return true if the search is successful; false otherwise;
		 *	Find the predecessor and successor of the given ID at the same time
		 */
		bool findSuccPred(const DHTNetworkID& _id, Node*& _pred, Node*& _succ, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL);

		/** @fn Node* findID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL)
		 *	@param _id: The ID to search for.
		 *	@param _searchPath: This outputs the path of searching as an array
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@param _hint: If a node is given as a hunt, it will be asked for successor at the first step in hope of speeding up the process
		 *	@return successor of the search ID if it succeeds; NULL otherwise
		 *	Find the successor of the given ID and output the nodes visited at the same time
		 */
		Node* findID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL);

		/** @fn Node* recursiveFindID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL)
		 *	@param _id: The ID to search for.
		 *	@param _searchPath: This outputs the path of searching as an array
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@param _hint: If a node is given as a hunt, it will be asked for successor at the first step in hope of speeding up the process
		 *	@return successor of the search ID if it succeeds; NULL otherwise
		 *	Find the successor of the given ID and output the nodes visited at the same time
		 */
		Node* recursiveFindID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel, const Node* _hint = NULL);

		/** @fn Node* findClosestPredcedingFinger(const DHTNetworkID& _id)
		 *	@param _id: The ID to search for.
		 *	@return a pointer to the closest node
		 *	Get the closest node preceding to the given ID.
		 */
		Node* findClosestPredcedingFinger(const DHTNetworkID& _id);

		/**	@fn int addReference(const Node& _node)
		 *	@param _node: The reference node to be added.
		 *	@return 0 if the reference is successfully added.
		 *	This function allows adding a new reference into or updating the entry in the reference set.
		 */
		int addReference(const Node& _node);

		/**	@fn int removeReference(const DHTNetworkID& _id)
		 *	@param _id: The ID of the referenced node to be removed.
		 *	@return 0 if the reference is successfully removed.
		 *	This function allows removing a reference.
		 */
		int removeReference(const DHTNetworkID& _id);

		/** @fn LocalNodeHandle& operator=(const LocalNodeHandle& _localNode)
		 *	@param _localNode: The LocalNodeHandle object to be copied.
		 *	@return a reference to this instance
		 *	Overloaded assignment operator.
		 */
		LocalNodeHandle& operator=(const LocalNodeHandle& _localNode);

		/**	@fn int checkPredecessor()
		 *	@return 1 if it is dead; 0 otherwise
		 *	This function checks whether the predecessor is dead and hence remove it if it is.
		 */
		int checkPredecessor();

		/**	@fn int stabilize()
		 *	@return negative number if there is any error
		 *	This function runs the stabilization protocol.
		 */
		int stabilize();

		/**	@fn int fixFingers()
		 *	@return negative number if there is any error
		 *	This function runs the finger fixing protocol.
		 */
		int fixFingers();

		/**	@fn int visitNeighbor()
		 *	@return negative number if there is any error
		 *	This function checks whether all the neighbor  is alive.
		 */
		int visitNeighbor();

		/**	@fn int join(const Node& _bootstrap)
		 *	@param _bootstrap: the bootstrap node to contact for joining DHT
		 *	@return 0 if success
		 *	This function allows a node to join an existing DHT.
		 * We assume that this new local node has not been added to the collection yet.
		 */
		int join(const Node& _bootstrap);

		/**	@fn void print(Util::LogLevel _logLevel)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of the node.
		 */
		void print(Util::LogLevel _logLevel);

		/**	@fn Node** returnAllFingers()
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function returns the fingers of the node.
		 */
		Node** returnAllFingers();

		/**	@fn Node** returnAllNeighbor()
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function returns the neighbors of the node.
		 */
		Node** returnAllNeighbor();

		/**	@fn int copyReference(const LocalNodeHandle& _copy)
		 *	@param _copy: The local node from which references are copied.
		 *	@return 0 if success
		 *	This function copies fingers and neighbors references from a local node.
		 */
		int copyReference(const LocalNodeHandle& _copy);

		/**	@fn LocalNodeHandle* clone()
		 *	@return a deep copy of the instance which invokes this function
		 *	This function gives a deep copy of the invoker.
		 */
		LocalNodeHandle* clone();

		/**	@fn int pingNode(const Node& _node)
		 *	@return an integer >= 0 if the node to be pinged is responsive; <0 otherwise
		 *	This function pings a node given its ID and address.
		 */
		int pingNode(const Node& _node);

		/** @fn bool isIsland()
		 *	@return true if the local node does not know of any other nodes; false otherise
		 *	Check whether the local node is an island by itself.
		 */
		bool isIsland();

		/** @fn bool isReferenceExist(const Node& _node)
		 *	@param _node: the node to check
		 *	@return true if the node exist in the routing structure.
		 *	Check whether a certain node exists.
		 */
		bool isReferenceExist(const Node& _node);

		/** @fn int getUseCount()
		 *	@return the number of pointers pointing to the object
		 */
		int getUseCount();

		/** @fn void addUseCount()
		 *	increment the number of pointers pointing to the object
		 */
		void addUseCount();

		/** @fn int subtractUseCount()
		 *	decrement the number of pointers pointing to the object
		 */
		void subtractUseCount();

	};

}
#endif
