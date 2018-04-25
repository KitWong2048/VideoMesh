/*
FarTimeShift.h

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

/**	@file FarTimeShift.h
 *	This file declares the class for handling messages received from the socket pool.
 */

#ifndef _H_FAR_TIME_SHIFT_
#define _H_FAR_TIME_SHIFT_

#include <set>
#include "../App/ChordApp.h"
#include "../Util/InetAddress.h"
#include "../Util/Common.h"
#include "../DHT/Logger.h"

#define FTS_ID_LENGTH 4 // bytes FTS segments length
#define FTS_DHT_LENGTH 10 // 4 bytes FTS segments length + 6 bytes DHT address

namespace App{
	/** @class FarTimeShiftID
	 * a representation for the FTS ID, which makes it easy to use with <set> and <map>
	 */
	class FarTimeShiftID{
		public:
			char ID[FTS_ID_LENGTH];

			FarTimeShiftID();

			FarTimeShiftID(const char* charID);

			FarTimeShiftID(const FarTimeShiftID& _ID);

			FarTimeShiftID& operator=(const FarTimeShiftID& _ID);

			bool operator==(const FarTimeShiftID& _ID) const;

			bool operator<(const FarTimeShiftID& _ID) const;
	};

	/** @class FarTimeShift
	 * The class that encapsulates Far Time Shift algorithm
	 */
	class FarTimeShift{
		private:
			/** @class FTSAsyncHandle
			 * a derived class from AsyncHandle, specific for lookup in async. manner using DHT
			 */
			class FTSAsyncHandle : public AsyncHandle{
				private:
					/** @var ftsID
					 * the target FTS ID being searched for
					 */
					char ftsID[FTS_ID_LENGTH];

					/** @var isLookupDone
					 *  flag for indicating whether the async. action has finished
					 */
					bool isLookupDone;

					/** @var resultSet
					 *  the set of nodes satisfying the target FTS ID
					 */
					set<DHT::Node> resultSet;

					/** @fn bool addResult(const DHT::Node& inputNode)
					 * add a node to the result set
					 * @param inputNode - the node to be added
					 * @return true if the node is added to the result set
					 */
					bool addResult(const DHT::Node& inputNode);

				protected:
					/** @fn virtual void performAction()
					 * the action that is performed once a DHT search has returned
					 */
					virtual void performAction();

				public:
					FTSAsyncHandle(ChordApp* app);

					/** @fn void setPrefixConstraint(const char* prefix)
					 * tell the handler the target FTS ID to be searched
					 * @param prefix - the target FTS ID to be searched
					 */
					void setPrefixConstraint(const char* prefix);

					/** @fn bool isResultReady()
					 * @return true if the result is available
					 */
					bool isResultReady();

					/** @fn int getResult(Util::InetAddress* addrList, int len)
					 * get the result of last query
					 * @param addrList - list of peers where the result is placed
					 * @param len - maximum no. of peers returned
					 * @return no. of peers returned
					 */
					int getResult(Util::InetAddress* addrList, int len);
			};

			/** @var chord
			 *  a reference to the DHT network
			 */
			ChordApp* chord;

			/** @var asyncLookupHandler
			 * a handle for async lookup in the DHT
			 */
			 FTSAsyncHandle* asyncLookupHandler;

			/** @var localAddr
			 * the address for communication of the DHT
			 */
			char localAddr[ADDRESS_LENGTH];

			/** @var resultList
			 * the result of searching
			 */
			Util::InetAddress* resultList;

			/** @var ftsIDs
			 * the set of time reference added to the DHT
			 */
			set<FarTimeShiftID> ftsIDs;

		public:
			FarTimeShift();

			~FarTimeShift();

			/** @fn void init(Util::InetAddress& startUpAddr)
			 * initiate the object
			 * @param startUpAddr - start up address
			 */
			void init(Util::InetAddress& startUpAddr);

			/**
			 * @fn void add(const char* ftsID, Util::InetAddress& serverAddress, const Util::InetAddress* bootstrap = NULL)
			 * add a DHT peer
			 * @param ftsID - the reference ID to be added, which should be a char[FTS_ID_LENGTH]
			 * @param serverAddress - the address of the application which will be notified when someone tries to locate the peer
			 * @param bootstrap - the bootstrap node help to add a node
			 */
			void add(const char* ftsID, Util::InetAddress& serverAddress, const Util::InetAddress* bootstrap = NULL);

			/**
			 * @fn void remove(const char* ftsID)
			 * remove a DHT peer
			 * @param ftsID - the target ID to be removed
			 */
			void remove(const char* ftsID);

			/**
			 * @fn int shift(const char* shiftID, Util::InetAddress* addrList, int len)
			 * shift to a target position, write the list of peers to a list. This method may block.
			 * @param shiftID - the target ID of the segment, which should be a char[FTS_ID_LENGTH]
			 * @param addrList - list of peers
			 * @param len - maximum no. of peers returned
			 * @return no. of peers returned
			 */
			int shift(const char* shiftID, Util::InetAddress* addrList, int len);

			/**
			 * @fn void asyncShift(const char* shiftID)
			 * shift to a target position, write the list of peers to a list
			 * get the results using FarTimeShift::getAsyncResult(Util::InetAddress* addrList, int len).
			 * This method does not block.
			 * @param shiftID - the target ID of the segment, which should be a char[FTS_ID_LENGTH]
			 */
			void asyncShift(const char* shiftID);

			/**
			 * @fn int getAsyncResult(Util::InetAddress* addrList, int len)
			 * get the result of last asynchoronous query
			 * @param addrList - list of peers
			 * @param len - maximum no. of peers returned
			 * @return no. of peers returned
			 */
			int getAsyncResult(Util::InetAddress* addrList, int len);

			/**
			 * @fn bool isResultReady()
			 * @return whether the result of last asynchoronous query is ready
			 */
			bool isResultReady();

			/**
			 * @fn bool isSearchReady()
			 * @return whether the DHT is ready for making a search
			 */
			bool isSearchReady();

			/**
			 * @fn void destroy()
			 * clean up and destroy everything for this class and the instance cannot be used any more
			 */
			void destroy();

			/**
			 * @fn void clean()
			 * this function should be called periodically to clean up memory allocated dynamically
			 */
			void clean();

			/**
			 * @fn void print()
			 * print information for the object
			 */
			void print();	
			
			bool isDHTRunning();
	};
}

#endif
