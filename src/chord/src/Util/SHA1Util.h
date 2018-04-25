/*
SHA1Util.h

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

/**	@file SHA1Util.h
 *	This file contains the interface for getting a SHA1 hash value.
 */

#ifndef _H_SHA1_UTIL_
#define _H_SHA1_UTIL_

#include "../Util/Common.h"
#include "../Util/SHA1.h"
#include "../Util/Win32Support.h"

namespace Util{
	/** @fn unsigned char* sha1Hash(const char* msg)
	 * @param msg: the msg to be hashed on
	 * @return the SHA1 value in binary form;
	 * Hash a message and get the hashed value.
	 */
	unsigned char* sha1Hash(const char* msg);

	/** @fn char* sha1HashHex(const char* msg)
	 * @param msg: the msg to be hashed on
	 * @return the SHA1 value in hexadecimal value;
	 * Hash a message and get the hashed value.
	 */
	char* sha1HashHex(const char* msg);
}

#endif
