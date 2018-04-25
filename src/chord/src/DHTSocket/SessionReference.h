/*
SessionReference.h

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

/**	@file SessionReference.h
 *	This file declares the class for represent a session reference number during a secting using in the socket pool to communicate.
 */

#ifndef _H_SESSION_REFERENCE_
#define _H_SESSION_REFERENCE_

#include "../Util/InetAddress.h"

namespace DHTSocket{
	/**	@class SessionReference
	 *	This class represents a session reference number during a secting using in the socket pool to communicate.
	 */
	class SessionReference{
		private:
			Util::InetAddress addr;

			unsigned long seqNo;

		public:
			SessionReference();

			SessionReference(const Util::InetAddress& _addr, unsigned long _seqNo);

			virtual ~SessionReference();

			Util::InetAddress getAddr() const;

			unsigned long getSeqNo() const;

			void setAddr(const Util::InetAddress& _addr);

			void setSeqNo(unsigned long _seqNo);

			SessionReference& operator=(const SessionReference& _ref);

			bool operator==(const SessionReference& _ref) const;

			bool operator<(const SessionReference& _ref) const;

			int toBin(char*& _bin) const;

			int fromBin(const char* _bin);

			friend ostream& operator<<(ostream& _out, const SessionReference& _ref);
	};

}

#endif
