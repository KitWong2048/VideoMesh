/*
Chord.h

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

/**	@file Chord.h
 *	This file contains the class for running Chord service.
 *	Each instance of this will run on each thread.
 */
#ifndef _H_CHORD_
#define _H_CHORD_


#include "../Util/Common.h"
#include "../Util/Thread.h"
#include "../Util/Mutex.h"
#include "../Util/ConditionVariable.h"
#include "../DHTSocket/SocketClient.h"
#include "../DHTSocket/SocketServer.h"
#include "../DHTSocket/PoolSocketMgr.h"
#include "../App/ChordApp.h"
#include "../DHT/LocalNodeCollection.h"
#include "../DHT/DHTRequest.h"
#include "../DHT/DHTService.h"

namespace DHT
{
	/**	Forward class definition the class Stabilizer.
	 *	@See Stabilizer
	 */
	class Stabilizer;

	/**	Forward class definition the class LocalNodeCollection.
	 *	@See LocalNodeCollection
	 */
	class LocalNodeCollection;

	/**	@class Chord
	 *	This class encapsulates the details of the Chord DHT.
	 */
	class Chord : public Util::Thread, public DHTSocket::ServerHandle
	{
	private:
		/**	@var app
		 * The application which the Chord DHT network interworks.
		 */
		App::ChordApp* app;

		/**	@var attr
		 * The attributes of Chord.
		 */
		App::ChordAttr attr;

		/**	@var localNodes
		 * The collection of virtual nodes on the local host.
		 */
		LocalNodeCollection* localNodes;

		/**	@var log
		 * The handler to logging.
		 */
		Logger* log;

		 /** @var chordStabilizer
		  * The stabilizer object runs periodically to maintain data structure.
		  */
		 Stabilizer* chordStabilizer;

		 /** @var socketPoolMgr
		  * The socket manager which handle socket connection with remote peers
		  */
		 DHTSocket::PoolSocketMgr* socketPoolMgr;

		 /** @var isDHTRunning
		  * Indicate whether the thread for chord  object is running
		  */
		 bool isDHTRunning;

		 #ifndef DHT_WIN32
			/** @var runningDHTCondition
			* The conditional variable for use with the member isDHTRunning
			*/
			Util::ConditionVariable runningDHTCondition;
		#endif

		/** @var runningDHTLock
		 * The lock for accessing the member isDHTRunning
		 */
		Util::Mutex runningDHTLock;

		/** @var srvThreadLock
		* The lock for accessing the member srvThreadPool
		*/
		Util::Mutex srvThreadLock;

		/** @var srvThreadPool
		* Thread pool for handling client requests.
		*/
		DHTService** srvThreadPool;

	protected:
		/** @fn void* execute()
		 * @return NULL;
		 *	This is the starting point of running Chord.
		 * Override the execute() method in Thread class.
		 */
		void* execute();

	public:
		Chord(App::ChordApp* _app, const App::ChordAttr& _attr);

		~Chord();

		/** @fn void* doExecuteDHT()
		 * @return NULL;
		 * Actual implementation of DHT logic.
		 */
		void* doExecuteDHT();

		/**	@fn int addNode(const DHTNetworkID& _id, const char* _appValue)
		 *	@param _id: The ID of the node to be created.
		 *	@param _appValue: The application value associated with the node.
		 *	@return 0 if the node is successfully created.
		 *	This function allows Chord to add a node and corresponding key from Chord.
		 *	If there are some other local nodes on Chord, one of them will be used to help the new node join Chord.
		 *	In case there is no such node, the new node will start a new Chord network.
		 */
		int addNode(const DHTNetworkID& _id, const char* _appValue);

		/**	@fn int addNode(const DHTNetworkID& _id, const char* _appValue, const Node& _bootstrap)
		 *	@param _id: The ID of the node to be created.
		 *	@param _appValue: The application value associated with the node.
		 *	@param _bootstrap: The bootstrap node to help the newly created node join Chord.
		 *	@return 0 if the new node is successfully created.
		 *	This function allows Chord to add a new node and corresponding key from Chord with help of a bootstrap node.
		 */
		int addNode(const DHTNetworkID& _id, const char* _appValue, const Node& _bootstrap);

		/**	@fn int removeNode(const DHTNetworkID& _id)
		 *	@param _id: The ID of the node to be removed from Chord.
		 *	@return 0 if the node is successfully removed.
		 *	This function allows Chord to remove a node and corresponding key from Chord.
		 */
		int removeNode(const DHTNetworkID& _id);

		/**	@fn Node* lookup(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord.
		 */
		Node* lookup(const DHTNetworkID& _id, int* _hopCount, Util::LogLevel _logLevel);

		/**	@fn Node* lookupFullPath(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _searchPath: This outputs the path of searching as an array
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord and returns the search path at the same time.
		 */
		Node* lookupFullPath(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel);

		/**	@fn Node* recursiveLookup(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel)
		 *	@param _id: The ID of the key to lookup.
		 *	@param _searchPath: The path of the search has taken. _searchPath[0] is the local node which initiate the search and _searchPath[_hopCount] is the last node in the search path. If the function returns a successor, the last entry is the predecessor for target ID.
		 *	@param _hopCount: The number of hops the search took.
		 *	@param _logLevel: The level of logging for running this function.
		 *	@return the succssor of the key.
		 *	This function provides look up for a key in Chord, in a recursive fashion. In addition to the destination node, the search path is also returned.
		 */
		Node* recursiveLookup(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, Util::LogLevel _logLevel);

		/**	@fn LocalNodeHandle* getLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID of the node to be returned.
		 *	@return a pointer to the node if found; NULL otherwise.
		 *	This function returns a node from the collection if that node has the ID requested.
		 */
		LocalNodeHandle* getLocalNode(const DHTNetworkID& _id);

		/**	@fn LocalNodeHandle* getPrecedingLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID which the return node should preceed to.
		 *	@return a local node which is proceed to the given ID.
		 *	This function finds a local node from the collection which is proceed to the given ID.
		 */
		LocalNodeHandle* getPrecedingLocalNode(const DHTNetworkID& _id);

		/**	@fn LocalNodeHandle* getSucceedingLocalNode(const DHTNetworkID& _id)
		 *	@param _id: the ID which the return node should succeed to.
		 *	@return a local node which is succeed to the given ID.
		 *	This function finds a local node from the collection which is succeed to the given ID.
		 */
		LocalNodeHandle* getSucceedingLocalNode(const DHTNetworkID& _id);

		/**	@fn void print(Util::LogLevel _logLevel)
		 *	@param _logLevel: Under what logging level should it prints out the message
		 *	This function prints the details of the DHT.
		 */
		void print(Util::LogLevel _logLevel);

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

		/**	@fn void stop()
		 *	This function stops the DHT and cancels all threads serving peers.
		 */
		void stop();

		/**	@fn Node** getFingersOrNeighbor(const Node& _node, bool _isFingers = true)
		 *	@param _node: The node whose fingers or neighbor are in interest
		 *	@param _isFingers: true to get entry from finger table, otherwise from neigbhor
		 *	@return an array of pointers to nodes, with the last entry in the array as NULL.
		 *	This function allows Chord to find out the fingers or neighbors of a node, given its ID and address.
		 */
		Node** getFingersOrNeighbor(const Node& _node, bool _isFingers = true);

		/**	@fn int response(DHTSocket::VirtualPoolSocket* _socket)
		 *	@param _socket: The socket for doing I/O with the conneted peer
		 *	@return 0 if success.
		 *	This function overrides the implementat in ServerHandle. This is supposed to be called by the socket pool manager when there is a new request.
		 */
		int response(DHTSocket::VirtualPoolSocket* _socket);

		/**	@fn int response(DHTSocket::SocketServer* _socket)
		 *	@param _socket: The socket for accepting a peer
		 *	@return 0 if success.
		 *	This function overrides the implementat in ServerHandle. This is supposed to be called by the socket pool manager when there is a new peer connected.
		 */
		int response(DHTSocket::SocketServer* _socket);

		/**	@fn bool getDHTRunningStatus()
		 *	@return true if the DHT is running; false otherwise.
		 *	This function checks whether the thread responsible for DHT is running.
		 */
		bool getDHTRunningStatus();

		/**	@fn double getMaxSearchTime()
		 *	@return the maximum search time allowed
		 *	Get the maximum search time allowed
		 */
		double getMaxSearchTime();

		/**	@fn unsigned int getNeighborhoodSize()
		 *	@return the maximum size of the neighborhood list
		 *	Get the maximum size of the neighborhood list
		 */
		unsigned int getNeighborhoodSize();

		/**	@fn unsigned int getTimeOutValue()
		 *	@return the configuration value for connection time out
		 *	Get the configuration value for connection time out
		 */
		unsigned int getTimeOutValue();

		/**	@fn Logger* getLogger()
		 *	@return a reference to the log object
		 *	Get a reference to the log handler
		 */
		Logger* getLogger();

		/**	@fn DHTSocket::PoolSocketMgr* getSocketPoolMgr()
		 *	@return a reference to the socket manager object
		 *	Get a reference to the socket manager
		 */
		DHTSocket::PoolSocketMgr* getSocketPoolMgr();

		/**	@fn bool isMultiThreaded()
		 *	@return true if the attribute of the DHT is set to multithreaded
		 */
		bool isMultiThreaded();

		friend class Stabilizer;
	};
}
#endif
