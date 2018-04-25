/*
NodeBucket.h

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

/**	@file NodeBucket.h
 *	This file contains the class for making a list of nodes.
 */

#ifndef _H_NODE_BUCKET_
#define _H_NODE_BUCKET_

#include <list>
#include "../Util/Common.h"
#include "../Util/Mutex.h"
#include "../DHT/Node.h"
#include "../DHT/Logger.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	@class NodeBucket
	 *	This class is a list of nodes. The nodes are arranged in clockwise direction with respect to a reference ID.
	 */
	class NodeBucket{
    protected:
		/**	@var refID
		 * The reference ID for arranging the node.
		 */
        DHTNetworkID refID;

		/**	@var chord
		 * The chord network to which the nodes belong to.
		 */
        Chord* chord;

		/**	@var log
		 * The handle for logging.
		 */
        Logger* log;

		/**	@var nodes
		 * The list of nodes.
		 */
        list<Node*>* nodes;

		/**	@var nodesMutex
		 * Lock for controlling access to nodes.
		 */
		Util::Mutex nodesMutex;

		/**	@var bucketSize
		 * The maximum size of the node bucket.
		 */
        int bucketSize;

		/** @fn virtual bool order(const DHTNetworkID& _lower, const DHTNetworkID& _middle, const DHTNetworkID& _upper, bool _lowerClosed = false, bool _upperClosed = false);
		 * @param _lower: the node on the lower bound.
		 * @param _middle: the node in the middle.
		 * @param _upper: the node on the upper bound.
		 * @param _lowerClosed: if the lower bound is closed, set this to true
		 * @param _upperClosed: if the upper bound is closed, set this to true
		 * @return true if _middle is in (_lower, _upper); false otherwise
		 *	The order for arranging three nodes.
		 */
        virtual bool order(const DHTNetworkID& _lower, const DHTNetworkID& _middle, const DHTNetworkID& _upper, bool _lowerClosed = false, bool _upperClosed = false);

    public:
        NodeBucket(int _listSize, const DHTNetworkID& _refID, Chord& _chord);

        NodeBucket(const NodeBucket& _nodeList);

        virtual ~NodeBucket();

		/** @fn int getSize();
		 * @return the current size of bucket
		 *	Get the number of node inside the bucket.
		 */
        int getSize();

		/** @fn Node* addNode(const Node& _node);
		 * @param _node: add a node to the bucket
		 * @return the node that is replaced by the new node
		 *	Add a node to the bucket.
		 */
        Node* addNode(const Node& _node);

		/** @fn Node* removeNode(const DHTNetworkID& _id);
		 * @param _id: the ID of the node to be removed
		 * @return the removed node if it exists
		 *	Remove a node with certain ID.
		 */
        Node* removeNode(const DHTNetworkID& _id);

		/** @fn Node* operator[](int _pos);
		 * @param _pos: the position of the desired
		 * @return a node at the position specified
		 *	Get a node at a given position which is starting from zero.
		 */
        Node* operator[](int _pos);


		/** @fn NodeBucket& operator=(const NodeBucket& _nodeList)
		 *	Assignment operator overloading.
		 * Make a deep copy of the input object.
		 * @param _nodeList: the Node to be copied.
		 * @return a reference to the caller
		 */
        NodeBucket& operator=(const NodeBucket& _nodeList);

		/** @fn void print(LogLevel _logLevel)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of the bucket.
		 */
		void print(Util::LogLevel _logLevel);

		/** @fn Node** returnAllNodes();
		 * @return an array of Node* with last entry equals to NULL
		 *	Get all the nodes in the bucket.
		 */
        Node** returnAllNodes();

		/** @fn NodeBucket* clone()
		 *	@return a deep copy of the instance which invokes this function
		 *	This function gives a deep copy of the invoker.
		 */
        NodeBucket* clone();

		/** @fn Node* getClosestSucc(const DHTNetworkID& _id);
		 * @param _id: the ID that the returned node succeeding to
		 * @return the node succeeding to a given ID
		 *	Get the node succeeding to a given ID.
		 */
        Node* getClosestSucc(const DHTNetworkID& _id);

		/** @fn Node* getClosestPred(const DHTNetworkID& _id);
		 * @param _id: the ID that the returned node preceding to
		 * @return the node preceding to a given ID
		 *	Get the node preceding to a given ID.
		 */
        Node* getClosestPred(const DHTNetworkID& _id);

		/** @fn bool isNodeExist(const Node& _node)
		 *	@param _node: the node to check
		 *	@return true if the node exist in the bucket.
		 *	Check whether a certain node exists.
		 */
		bool isNodeExist(const Node& _node);
	};

}
#endif
