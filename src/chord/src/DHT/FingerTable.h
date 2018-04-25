/*
FingerTable.h

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

/**	@file FingerTable.h
 *	This file contains the declaration of finger table used in Chord.
 */

#ifndef _H_FINGER_TABLE_
#define _H_FINGER_TABLE_

#include <vector>
#include "../Util/Common.h"
#include "../Util/Mutex.h"
#include "../DHT/Node.h"
#include "../DHT/Logger.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	@class FingerTable
	 *	This class declares the finger table used for routing in Chord.
	 */
	class FingerTable{
	private:
		/**	@class FingerTableEntry
		*	This class declares an entry in the finger table.
		*/
		class FingerTableEntry{
		public:
			/**	@var start
			* The actual tight upperbound of the interval which this finger is responsible.
			*/
			DHTNetworkID actualStart;

			/**	@var start
			* The upperbound of the interval which this finger is responsible.
			*/
			DHTNetworkID start;

			/**	@var end
			* The lowerbound of the interval which this finger is responsible..
			*/
			DHTNetworkID end;

			/**	@var successor
			* The node of this finger entry.
			*/
			Node successor;

			/**	@var failCount
			* The counter of fail connecting to this entry. If the counter exceeds a certain threshold, the entry will be removed.
			*/
			int failCount;

			FingerTableEntry(const DHTNetworkID& _actualStart, const DHTNetworkID& _start, const DHTNetworkID& _end, const Node& _successor);

			~FingerTableEntry();
		};

		/**	@var fingers
		 * The collection of fingers as a finger table.
		 */
		vector<FingerTable::FingerTableEntry*> fingers;

		/**	@var local
		 * The local node to which the finger table belong.
		 */
		Node local;

		/**	@var fingerLock
		 * The lock for accessing entry.
		 */
		Util::Mutex fingerLock;

		/** @var log
		 * The reference used for logging.
		 */
		Logger* log;

		/** @var chord
		 * A reference to the DHT network
		 */
		Chord* chord;

	public:
		FingerTable(const Node& _localNode, Chord& _chord);

		FingerTable(const FingerTable& _fingerTable);

		~FingerTable();

		/**	@fn int addFinger(const Node& _finger)
		 *	@param _finger: The finger node to be added.
		 *	@return the node that is replaced by the new finger.
		 *	This function allows adding a new node into or updating the entry in the finger table.
		 */
		Node* addFinger(const Node& _candidate);

		/**	@fn int removeFinger(const DHTNetworkID& _id)
		 *	@param _id: The ID of the finger node to be removed.
		 *	@return the node that is removed.
		 *	This function allows removing a finger from the finger table.
		 */
		Node* removeFinger(const DHTNetworkID& _id);

		/**	@fn Node getClosestPrecedingFinger(const DHTNetworkID& _id)
		 *	@param _id: The ID which the finger node ID is closest.
		 *	@return the finger node with closest ID .
		 *	This function finds the finger node which has ID closest to a target value.
		 */
		Node* getClosestPrecedingFinger(const DHTNetworkID& _id);

		/**	@fn Node Node* getNthSuccessor(int _n);
		 *	@param _n: The rank of the desired successor,  1 <= _n <= No of Bits in ID.
		 *	@return the N th successor to the local node.
		 *	This function finds the successor at then Nth entry of the finger table, notice that nth entry may be = (n - 1)th entry.
		 */
		Node* getNthSuccessor(int _n);

		/** @fn FingerTable& operator=(const FingerTable& _fingerTable)
		 *	@param _fingerTable: The FingerTable object to be copied.
		 *	@return a reference to this instance
		 *	Overloaded assignment operator.
		 */
		FingerTable& operator=(const FingerTable& _fingerTable);

		/** @fn bool isEntryPresent(int _pos);
		 *	@param _pos: The position of the entry, 1 <= _pos <= No of Bits in ID.
		 *	@return true if the entry position is not empty.
		 *	Check whether a certain position is empty.
		 */
		bool isEntryPresent(int _pos);

		/** @fn bool isFingerExist(const Node& _node)
		 *	@param _node: the node to check
		 *	@return true if the node exist in the finger table.
		 *	Check whether a certain node exists.
		 */
		bool isFingerExist(const Node& _node);

		/** @fn DHTNetworkID getEntryStart(int _pos);
		 *	@param _pos: The position of the entry, 1 <= _pos <= No of Bits in ID.
		 *	@return the start ID.
		 *	Get the starting ID of the desired entry position.
		 */
		DHTNetworkID getEntryStart(int _pos);

		/**	@fn void print(LogLevel _logLevel)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of the fingertable.
		 */
		void print(Util::LogLevel _logLevel);

		/**	@fn Node** returnAllFingers()
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function returns the fingers of the node.
		 */
		Node** returnAllFingers();

		/**	@fn FingerTable* clone()
		 *	@return a deep copy of the instance which invokes this function
		 *	This function gives a deep copy of the invoker.
		 */
		FingerTable* clone();

		/**	@fn int getSize()
		 *	@return the number of entry in the finger table
		 *	This function gives the number of entry in the finger table.
		 */
		int getSize();
	};
}

#endif
