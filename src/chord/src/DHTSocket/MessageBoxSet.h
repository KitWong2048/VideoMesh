/*
MessageBoxSet.h

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

/**	@file MessageBoxSet.h
 *	This file declares the class for handling messages received from the socket pool.
 */

#ifndef _H_MESSAGE_BOX_
#define _H_MESSAGE_BOX_

#include <map>
#include "../Util/Common.h"
#include "../Util/Mutex.h"
#include "../Util/ConditionVariable.h"
#include "../DHTSocket/SessionReference.h"

namespace DHTSocket{
	/**	@class MessageBoxSet
	 *	This class encapsulates the data structure for a set of message boxes which handle messages arriving at the pool sockets.
	 */
	class MessageBoxSet{
	private:
		/**	@class Box
		 *	This class encapsulates the data structure of one message box which is responsible for messages for a virtual pool socket.
		 */
		class Box{
			public:
				Box();

				~Box();

				/** @var boxMutex
				 * The mutex guarding access to the box.
				 */
				Util::Mutex boxMutex;

				/** @var boxCondVar
				 * The conditional varaible for taking message from the box.
				 */
				Util::ConditionVariable boxCondVar;

				/** @var msg
				 * The message inside the box
				 */
				char* msg;

				/** @var len
				 * Length of the message
				 */
				int len;
		};

		/** @var boxes
		 * The set of boxes indexed by the session reference number.
		 */
		map<SessionReference, Box*> boxes;

		/** @var boxBuildingMutex
		 * The lock for accessing themember boxes.
		 */
		Util::Mutex boxBuildingMutex;

	public:
		MessageBoxSet();

		~MessageBoxSet();

		/** @fn bool isBoxExist(const SessionReference& _id);
		 * @param _id: the ID of the box to be checked
		 * @return true if the box with the input ID exists
		 *	Check whether a box with certain ID exists.
		 */
		bool isBoxExist(const SessionReference& _id);

		/** @fn bool createBox(const SessionReference& _id);
		 * @param _id: the ID of the box to be created
		 * @return 0 if success
		 *	Create a box with certain ID.
		 */
		int createBox(const SessionReference& _id);

		/** @fn bool destroyBox(const SessionReference& _id);
		 * @param _id: the ID of the box to be destroyed
		 * @return 0 if success
		 *	Destroy a box with certain ID.
		 */
		int destroyBox(const SessionReference& _id);

		/** @fn int deliver(const SessionReference& _id, char* _buf, int _len);
		 * @param _id: the ID of the box where the message is delivered
		 * @param _buf: the message to be delivered
		 * @param _len: the length of the message
		 * @return 0 if success
		 *	Deliver a message to a box.
		 */
		int deliver(const SessionReference& _id, char* _buf, int _len);

		/** @fn int take(const SessionReference& _id, char*& _buf, int _timeout = -1);
		 * @param _id: the ID of the box where the message is taken from
		 * @param _buf: the place to the store message
		 * @param _timeout: amount of time waiting for the message, measured in millisecond
		 * @return the length of the message
		 *	Take a message from a box.
		 */
		int take(const SessionReference& _id, char*& _buf, int _timeout = -1);
	};
}
#endif
