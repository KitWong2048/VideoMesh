/*
ChordApp.h

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

/**	@file ChordApp.h
 *	This file contains the class to interface Chord.
 */

#ifndef _H_CHORD_APP_
#define _H_CHORD_APP_

#include <list>
#include "../Util/Common.h"
#include "../DHT/LocalNodeHandle.h"
#include "../App/ChordAttr.h"
#include "../App/AsyncHandle.h"


/**	Forward class definition the class Chord.
*	@See DHT::Chord
*/
class DHT::Chord;

namespace App
{
	/**	@class ChordApp
	 *	This class allows application to interface Chord network.
	 *	Applications which want to make use of Chord network should inherit from this class or create an instance of this class.
	 */
	class ChordApp
	{
	private:
		/**	@var chord
		 * The Chord DHT network which the application will interact.
		 */
		DHT::Chord* chord;

		/**	@var asyncHandleList
		 * The list of handles to asynchronous actions invoked.
		 */
		list<AsyncHandle*> asyncHandleList;

	public:
		ChordApp(const ChordAttr& _attr);

		~ChordApp();

		/**	@fn int run
		 *	@return 0 if Chord is fired up successfully.
		 *	This function starts executing Chord network in a new thread.
		 * Once triggerd, nodes can be added to and removed from the chord. Application can also query Chord for keys.
		 */
		int run();

		/**	@fn int stop
		 *	@return 0 if Chord is stopped successfully.
		 *	This function stops Chord network.
		 */
		int stop();

		/**	@fn int addNode(const DHT::DHTNetworkID& _id, const char* _appValue)
		 *	@param _id: The ID of the node to be created.
		 *	@param _appValue: The application value associated with the node.
		 *	@return 0 if the node is successfully created.
		 *	This function allows Chord to add a node and corresponding key from Chord.
		 *	If there are some other local nodes on Chord, one of them will be used to help the new node join Chord.
		 *	In case there is no such node, the new node will start a new Chord network.
		 */
		int addNode(const DHT::DHTNetworkID& _id, const char* _appValue);

		/**	@fn int addNode(const DHT::DHTNetworkID& _id, const char* _appValue, const DHT::Node& _bootstrap)
		 *	@param _id: The ID of the node to be created.
		 *	@param _appValue: The application value associated with the node.
		 *	@param _bootstrap: The bootstrap node to help the newly created node join Chord.
		 *	@return 0 if the new node is successfully created.
		 *	This function allows Chord to add a new node and corresponding key from Chord with help of a bootstrap node.
		 */
		int addNode(const DHT::DHTNetworkID& _id, const char* _appValue, const DHT::Node& _bootstrap);

		/**	@fn int removeNode(const DHT::DHTNetworkID& _id)
		 *	@param _id: The ID of the node and corresponding to be removed from Chord.
		 *	@return 0 if the node is successfully removed.
		 *	This function allows Chord to remove a node and corresponding key from Chord.
		 */
		int removeNode(const DHT::DHTNetworkID& _id);

		/**	@fn DHT::Node* lookup(const DHT::DHTNetworkID& _id, int& _hopCount)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _hopCount: The number of hops the search took.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord.
		 */
		DHT::Node* lookup(const DHT::DHTNetworkID& _id, int& _hopCount);

		/**	@fn DHT::Node* lookupFullPath(const DHT::DHTNetworkID& _id, DHT::Node*& _searchPath, int& _hopCount)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _searchPath: The path of the search has taken. _searchPath[0] is the local node which initiate the search and _searchPath[_hopCount] is the last node in the search path. If the function returns a successor, the last entry is the predecessor for target ID.
		 *	@param _hopCount: The number of hops the search took.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord, in an iterative fashion. In addition to the destination node, the search path is also returned.
		 */
		DHT::Node* lookupFullPath(const DHT::DHTNetworkID& _id, DHT::Node*& _searchPath, int& _hopCount);

		/**	@fn DHT::Node* recursiveLookup(const DHT::DHTNetworkID& _id, DHT::Node*& _searchPath, int& _hopCount)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _searchPath: The path of the search has taken. _searchPath[0] is the local node which initiate the search and _searchPath[_hopCount] is the last node in the search path. If the function returns a successor, the last entry is the predecessor for target ID.
		 *	@param _hopCount: The number of hops the search took.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord, in a recursive fashion. In addition to the destination node, the search path is also returned.
		 *  The recurseive API has not been implemented at this moment so it behaves the same as the iterative version.
		 */
		DHT::Node* recursiveLookup(const DHT::DHTNetworkID& _id, DHT::Node*& _searchPath, int& _hopCount);

		/**	@fn void performAsyncAction(AsyncHandle* _async)
		 *	@param _async: The structure for providing search ID, what to do after the search has returned.
		 *	@return none.
		 *	This function provides asynchornous asynchrouns action in Chord.
		 */
		void performAsyncAction(AsyncHandle* _async);

		/**	@fn void cleanAsyncList()
		 *	This function clears the list of asynchrnous action handles which have already finished.
		 */
		void cleanAsyncList();

		/**	@fn void print(Util::LogLevel _logLevel = Util::LogNormal)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of the DHT.
		 */
		void print(Util::LogLevel _logLevel = Util::LogNormal);

			/**	@fn DHT::Node** getFingers(const DHT::Node& _node)
		 *	@param _node: The node whose fingers (i.e. long range contacts) are in interest
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function allows Chord to find out the fingers of a node, given its ID and address.
		 */
		DHT::Node** getFingers(const DHT::Node& _node);

		/**	@fn DHT::Node** getNeighbor(const DHT::Node& _node)
		 *	@param _node: The node whose neighbor (i.e. short range contacts) are in interest
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function allows Chord to find out the neighbors of a node, given its ID and address.
		 */
		DHT::Node** getNeighbor(const DHT::Node& _node);

		/**	@fn bool isDHTRunning()
		 *	@return true if the DHT is running; false otherwise.
		 *	This function checks whether the thread responsible for DHT is running.
		 */
		bool isDHTRunning();

		friend class Chord;
	};
}
#endif
