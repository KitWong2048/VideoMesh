/*
DHTService.h

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

/**	@file DHTService.h
 *	This file contains the class for encapsulating the request issued from the application.
 */

#ifndef _H_DHT_SERVICE_
#define _H_DHT_SERVICE_

#include "../Util/Common.h"
#include "../Util/Thread.h"
#include "../DHT/Logger.h"
#include "../DHT/Node.h"
#include "../DHT/LocalNodeHandle.h"
#include "../DHT/DHTMessageFactory.h"
#include "../DHT/AbstractNodeValue.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	Forward class definition the class LocalNodeHandle.
	 *	@See LocalNodeHandle
	 */
	class LocalNodeHandle;

	/**	@class DHTService
	 *	This class encapsulates the request issued from the application.
	 */
	class DHTService : public Util::Thread{
	private:
		/**	@var type
		* The type of request.
		*/
		Util::DHTMsgType type;

		/**	@var chord
		 * The Chord DHT network.
		 */
		Chord* chord;

		/**	@var queryingNode
		 * The remote node which is making the request.
		 */
		 Node* queryingNode;

		/**	@var answeringNode
		* The local node to which is answering the request.
		*/
		Node* answeringNode;

		/**	@var local
		 * The local node which is answering the request.
		 */
		LocalNodeHandle* local;

		/**	@var targetID
		* The ID to be searched for.
		*/
		DHTNetworkID* targetID;

		/**	@var notifyValue
		* The value of the node which makes notification.
		*/
		char* notifyValue;

		/**	@var updateEntry
		* The entry of the finger table which the update node suits.
		*/
		int updateEntry;

		/**	@var result
		* Result of the request.
		*/
		void* result;

		/**	@var msgFactory
		 * For parsing request msg.
		 */
		 DHTMessageFactory msgFactory;

		/**	@var Logger* log
		 * For logging.
		 */
		 Logger* log;

		/**	@var DHTSocket::VirtualPoolSocket* socket
		 * The socket for communicating with the remote peer.
		 */
		 DHTSocket::VirtualPoolSocket* socket;

		/** @fn void* srvReturnNone()
		 * @return result obtained from the serving, usually NULL
		 * Serve a request that does not need to report any node to the requester.
		 * This can be PING or NOTIFY request
		 */
		void* srvReturnNone();

		/** @fn void* srvReturnSingle()
		 * @return result obtained from the serving, usually NULL
		 * Serve a request that needs to report one node to the requester.
		 * This can be SELF, SUCC, PRED or CLOSEST_FINGER request
		 */
		void* srvReturnSingle();

		/** @fn void* srvReturnMany()
		 * @return result obtained from the serving, usually NULL
		 * Serve a request that needs to report a list of nodes to the requester.
		 * This can be FINGER or NEIGHBOR request
		 */
		 void* srvReturnMany();

		/** @fn bool doSend()
		 * @return true if the message is successfully sent
		 * Send the message as in the member srvMsg
		 */
		bool doSend();

		/**	@var CharMessage srvMsg
		 * The body of the message (i.e. excluding header) to be sent to the remote side.
		 */
		 CharMessage srvMsg;

	protected:
		/** @fn virtual void* execute()
		 * @return result of the service;
		 *	This function starts a new thread to for answering.
		 * Overriden execute() in Thread class.
		 */
		void* execute();

	public:
		DHTService(Chord& _chord, DHTSocket::VirtualPoolSocket& _socket);

		virtual ~DHTService();

		/** @fn void* serveRequest()
		 * @return result obtain from the serving, usually NULL
		 * Server a request from the connected remote peer.
		 */
		void* serveRequest();
	};
}

#endif
