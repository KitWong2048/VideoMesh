/*
InetAddress.h

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

/**	@file InetAddress.h
 *	This file contains the class for encapsulating IP address and port.
 */

#ifndef _H_INET_ADDRESS_
#define _H_INET_ADDRESS_

#include "../Util/Common.h"
#include "../Util/Win32Support.h"

using namespace std;

namespace Util{
	/**	@class InetAddress
	 *	This class declares the data structure for encapsulating IP address and port.
	 */
	class InetAddress{
	private:
		/**	@var IP
		 * IP address.
		 */
		in_addr IP;

		/**	@var port
		 * Port number.
		 */
		unsigned short port;

	public:
		InetAddress();

		InetAddress(const char* _IP, int _port);

		InetAddress(const InetAddress& _inetaddress);

		InetAddress& operator=(const InetAddress& _inetaddress);

		bool operator==(const InetAddress& _inetaddress) const;

		bool operator<(const InetAddress& _inetaddress) const;

		char* getAddress() const;

		void setAddress(const char* _address);

		string getIP() const;

		void setIP(const char* _IP);

		int getPort() const;

		void setPort(int _port);
	};

}

#endif
