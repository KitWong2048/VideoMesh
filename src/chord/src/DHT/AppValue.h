/*
AppValue.h

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

/**	@file AppValue.h
 *	This file contains the class which defines the node value as the address of the application node.
 */

#ifndef _H_APP_VALUE_
#define _H_APP_VALUE_

#include "../Util/Common.h"
#include "../Util/InetAddress.h"
#include "../DHT/AbstractNodeValue.h"

namespace DHT{
	/**	@class AppValue
	 *	This class defines the node value as the address of the application node.
	 */
	class AppValue : public AbstractNodeValue{
	private:
		char value[APP_VALUE_LEN];

	public:
		AppValue();

		virtual ~AppValue();

		/** @fn virtual int toBin(char*& _output) const
		 * Output the value to a binary representation.
		 * @param _output: output parameter, an array of char which allocated dynamically
		 * @return the length of the array
		 */
		virtual int toBin(char*& _output) const;

		/** @fn virtual int fromBin(const char* _input)
		 * Get the value from binary message.
		 * @param _input: the message to be parsed
		 * @return 0 if the parising is successful
		 */
		virtual int fromBin(const char* _input);

		/** @fn virtual int len() const
		 * @return the lenght of the value in number of bytes
		 */
		virtual int len() const;

		/** @fn virtual AbstractNodeValue& operator=(const AbstractNodeValue& _value)
		 *	Assignment operator overloading.
		 * Make a deep copy of the input object.
		 * @param _value: the value to be copied.
		 * @return a reference to the caller
		 */
		virtual AbstractNodeValue& operator=(const AbstractNodeValue& _value);

		/** @fn virtual string toStr()
		 * @return a string representation of the object
		 */
		virtual string toStr();

	};
}

#endif
