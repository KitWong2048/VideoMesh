/*
AsyncHandle.h

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

/**	@file AsyncHandle.h
 *	This file contains the class to perform asynchronous actions in Chord, these actions include adding a node, looking up a key, getting a target node's fingers or neighbors.
 */

#ifndef _H_ASYNC_HANDLE_
#define _H_ASYNC_HANDLE_

#include "../Util/Common.h"
#include "../Util/Thread.h"
#include "../DHT/Node.h"

namespace App
{

	/**	Forward class definition the class ChordApp.
	 *	@See ChordApp
	 */
	class ChordApp;

	enum AsyncOpType
	{
		AsyncLookup,
		AsyncBootStrapAddNode,
		AsyncNoBootStrapAddNode,
		AsyncRemoveNode,
		AsyncGetNeighbor,
		AsyncGetFinger,
		AsyncNull
	};

	/**	@class AsyncHandle
	 *	This class allows application to perform an actions in Chord asynchronously.
	 *	These actions include adding a node, looking up a key, getting a target node's fingers or neighbors.
	 *	To perform an async action, application should first create an instance of this class dynamically.
	 *	And then either one of the argument setting function should be called.
	 *	Finally ChordApp::performAsyncAction(AsyncHandle* _async) has to be called with the instance of this class as argument.
	 *	Upon calling the function mentioned, a thread is created to perform the desired action.
	 *	When the action has done, it will will call performAction().
	 *	Applications should override performAction() for desired behavior.
	 */
	class AsyncHandle : public Util::Thread
	{
		private:
			/**	@fn void clear()
			 *	@return Nil.
			 *	This function clears the memory allocated dynamically.
			 */
			void clear();

			/**	@var arg1
			 *	The first argument of asychronous action.
			 */
			void* arg1;

			/**	@var arg2
			 *	The second argument of asychronous action.
			 */
			void* arg2;

			/**	@var arg3
			 *	The third argument of asychronous action.
			 */
			void* arg3;

		protected:
			/**	@var app
			 *	The Chord application where the action should perform.
			 */
			ChordApp* app;

			/**	@var hopCount
			 *	The number of hops taken for making a lookup.
			 *	This variable is meaningful for look up operation only
			 */
			int hopCount;

			/**	@var dest
			 *	The destination (i.e. the successor) of the lookup ID.
			 *	This variable is meaningful for look up operation only
			 */
			DHT::Node* dest;

			/**	@var searchPath
			 *	Ar array of DHT::Node represents the search path taken by the lookup. The number of entry is equal to hopCount + 1.
			 *	This variable is meaningful for look up operation only
			 */
			DHT::Node* searchPath;

			/**	@var nodes
			 *	The fingers or neighbors of a node, with last entry pointing to NULL.
			 *	This variable is meaningful for getting fingers or neighbors operation only
			 */
			DHT::Node** nodes;

			/**	@var status
			 *	The status of adding or removing a node. If it is 0, that means the operation is successful.
			 *	This variable is meaningful for node adding or removing operation only
			 */
			int status;

			/**	@fn void* execute()
			 *	@return NULL.
			 *	This function is the entry point of the worker thread.
			 */
			void* execute();

			/**	@var opType
			 *	The type of to be performed by the async handler.
			 */
			AsyncOpType opType;

			/**	@fn void performAction()
			 *	@return Nil.
			 *	This function is called upon the Chord action is performed.
			 */
			virtual void performAction();

		public:
			AsyncHandle(ChordApp* _app);

			virtual ~AsyncHandle();

			/**	@fn void setLookupArg(const DHT::DHTNetworkID& _id)
			 *	@param _id: The ID to lookup.
			 *	@return Nil.
			 *	This function sets the argument for performing a Chord lookup asynchronously.
			 */
			void setLookupArg(const DHT::DHTNetworkID& _id);

			/**	@fn void setGetFingerArg(const DHT::Node& _target)
			 *	@param _target: The node whose fingers (i.e. long range contacts) are in interest.
			 *	@return Nil.
			 *	This function sets the argument for finding out the fingers of a node, given its ID and address asynchronously.
			 */
			void setGetFingerArg(const DHT::Node& _target);

			/**	@fn void setGetNeighborArg(const DHT::Node& _target)
			 *	@param _target: The node whose neighbors (i.e. short range contacts) are in interest.
			 *	@return Nil.
			 *	This function sets the argument for finding out the neighbors of a node, given its ID and address asynchronously.
			 */
			 void setGetNeighborArg(const DHT::Node& _target);

			/**	@fn void setAddNodeArg(const DHT::DHTNetworkID& _id, const char* _appValue, const DHT::Node* _bootstrap = NULL)
			 *	@param _id: The ID of the node to be added.
			 *	@param _appValue: The application value associated with the node.
			 *	@param _bootstrap: The bootstrap which can help the new node to join.
			 *	@return Nil.
			 *	This function allows sets the argument for adding a node asynchronously.
			 */
			void setAddNodeArg(const DHT::DHTNetworkID& _id, const char* _appValue, const DHT::Node* _bootstrap = NULL);

			/**	@fn void setRemoveNodeArg(const DHT::DHTNetworkID& _id)
			 *	@param _id: The ID of the node to be removed.
			 *	@return Nil.
			 *	This function sets the argument for removing a node asynchronously.
			 */
			void setRemoveNodeArg(const DHT::DHTNetworkID& _id);

			/**	@fn void isDHTRunning()
			 *	@return true if DHT is running; false otherwise.
			 */			
			bool isDHTRunning();
	};

}

#endif
