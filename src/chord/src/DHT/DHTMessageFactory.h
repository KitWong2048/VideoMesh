/*
DHTMessageFactory.h

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

/**	@file DHTMessageFactory.h
 *	This file contains the classes for handling messages exchanged by peers.
 */

#ifndef _H_DHT_MESSAGE_FACTORY_
#define _H_DHT_MESSAGE_FACTORY_

#include "../Util/Common.h"
#include "../DHT/Node.h"

using namespace std;

namespace DHT
{
	/**	@class CharMessage
	 *	This class is an aggregate structure for handling message exchanging over sockets.
	 *  It consists of a char representation of the message and the length of the message.
	 */
	class CharMessage
	{
	public:
		CharMessage();

		~CharMessage();

		/** @fn void constructStr(const string& _str)
		 * @return Nil
		 * Construct the message from a string object
		 */
		void constructStr(const string& _str);

		/** @fn void clear()
		 * @return Nil
		 * Clear the buffer
		 */
		void clear();

		/**	@var body
		 * The message. This is a public member for easy manipulation.
		 */
		char* body;

		/**	@var len
		 * The length of the message. This is a public member for easy manipulation.
		 */
		int len;
	};


	class DHTMessageFactory
	{
	public:
		DHTMessageFactory();

		~DHTMessageFactory();

		/** @fn Util::DHTMsgType toggleMsgType(Util::DHTMsgType _type)
		 * @param _type: the message type to be changed
		 * @return the toggled message type
		 * This function changes a request type to corresponding service type and vice versa
		 */
		Util::DHTMsgType toggleMsgType(Util::DHTMsgType _type);

		/** @fn CharMessage produceDHTMsg(const Node& _src, const Node& _dest, Util::DHTMsgType _type, const CharMessage* _msgBody = NULL)
		 * @param _src: the node from which the message originates
		 * @param _dest: the node to which the message destines
		 * @param _type: the type of the message
		 * @param _msgBody: the message in addition to the header to send
		 * @return a message that is ready to be sent to the the remote peer
		 * This function produces a message in the format that the remote peer can understand
		 */
		CharMessage produceDHTMsg(const Node& _src, const Node& _dest, Util::DHTMsgType _type, const CharMessage* _msgBody = NULL);

		/** @fn int analyseDHTMsg(const CharMessage& _msg, Node& _src, Node& _dest, Util::DHTMsgType& _type, CharMessage& _msgBody)
		 * @param _msg: the message to be analyzed
		 * @param _src: output parameter, the node from which the message originates
		 * @param _dest: output parameter, the node to which the message destines
		 * @param _type: output parameter, the type of the message
		 * @param _msgBody: output parameter, the message in addition to the header to send
		 * @return 0 if the message is correctly analyzed
		 * This function analyzes a message sent from the remote peer.
		 */
		int analyseDHTMsg(const CharMessage& _msg, Node& _src, Node& _dest, Util::DHTMsgType& _type, CharMessage& _msgBody);

		/** @fn Util::DHTMsgType translateMsgToType(const char* _msg)
		 * @param _msg: the message to be translated
		 * @return the translated type
		 * This function translates a message to a type
		 */
		Util::DHTMsgType translateMsgToType(const char* _msg);

		/** @fn string translateTypeToMsg(Util::DHTMsgType _type)
		 * @param _type: the type to be translated
		 * @return the translated message
		 * This function translates a type to a message
		 */
		string translateTypeToMsg(Util::DHTMsgType _type);
	};
}
#endif
