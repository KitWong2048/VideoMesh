/*
DHTRequest.h

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

/**	@file DHTRequest.h
 *	This file contains the class encapsulating the request issued from other nodes in the DHT network.
 */

#ifndef _H_DHT_REQUEST_
#define _H_DHT_REQUEST_

#include "../Util/Common.h"
#include "../Util/Thread.h"
#include "../DHTSocket/VirtualPoolSocket.h"
#include "../DHTSocket/PoolSocketMgr.h"
#include "../DHT/Node.h"
#include "../DHT/DHTMessageFactory.h"
#include "../DHT/Logger.h"

namespace DHT{
	/**	Forward class definition the class Chord.
	 *	@See Chord
	 */
	class Chord;

	/**	Forward class definition the class LocalNodeHandle.
	 *	@See LocalNodeHandle
	 */
	class LocalNodeHandle;

	/**	@class DHTRequest
	 *	This class encapsulates the request issued from other nodes in the DHT network.
	 */
	class DHTRequest : public Util::Thread
	{
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
		 * The local node which is making the request.
		 */
		 Node* queryingNode;

		/**	@var answeringNode
		* The remote node to which is answering the request.
		*/
		Node* answeringNode;

		/**	@var localAnsweringNode
		* In case the answering node is another virtual node in the same process, the request can be answered by this object.
		*/
		LocalNodeHandle* localAnsweringNode;

		/**	@var targetID
		* The ID to be searched for.
		*/
		DHTNetworkID* targetID;

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

		/** @fn void* reqReturnNone()
		 * @return result obtained from the request, NULL representing failure, a pointer to an integer with non-negative values meaning the remote peer responses correctly
		 * Make a request that does not need to report any node.
		 * This can be PING or NOTIFY request
		 */
		int* reqReturnNone();

		/** @fn Node* reqReturnSingle()
		 * @return result obtained from the request, NULL representing failure, a pointer to the node answered by the remote peer
		 * Make a request that needs to report one node.
		 * This can be SELF, SUCC, PRED or CLOSEST_FINGER request
		 */
		Node* reqReturnSingle();

		/** @fn Node** reqReturnMany()
		 * @return result obtained from the request, NULL representing failure, a pointer to an array of nodes answered by the remote peer, with number of entries equal to at most the number of bits of ID
		 * Make a request that needs to report one node.
		 * This can be FINGER or NEIGHBOR request
		 */
		Node** reqReturnMany();

		/** @fn CharMessage doSendRecv(const CharMessage* _reqMsg = NULL)
		 * @param _reqMsg message in addition to the header to be sent
		 * @return the message received
		 * Send the message and receive a response from the remote side
		 */
		CharMessage doSendRecv(const CharMessage* _reqMsg = NULL);

		bool isAnsweringNodeFail;

	protected:
		/** @fn virtual void* execute()
		 * @return result of the request;
		 *	This function starts a new thread to for making request.
		 * Overriden execute() in Thread class.
		 */
		void* execute();

	public:
		DHTRequest(Util::DHTMsgType _type, Chord& _chord, const Node* _queryingNode, const Node* _answeringNode, const DHTNetworkID* _targetID = NULL);

		virtual ~DHTRequest();

		/** @fn void* makeRequest()
		 * @return result obtain from the request, usually NULL
		 * Make a request to the connected remote peer.
		 */
		void* makeRequest();


	};
}

#endif
