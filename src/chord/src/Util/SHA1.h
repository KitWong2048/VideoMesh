/*
SHA1.h

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

/**	@file SHA1.h
 *	This file contains the implementation of SHA1 has function.
 */

#ifndef _H_SHA1_
#define _H_SHA1_

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

namespace Util{
	typedef struct
	{
		uint32 total[2];
		uint32 state[5];
		uint8 buffer[64];
	}
	sha1_context;

	void sha1_starts( sha1_context *ctx );
	void sha1_update( sha1_context *ctx, uint8 *input, uint32 length );
	void sha1_finish( sha1_context *ctx, uint8 digest[20] );

/*
 * those are the standard FIPS-180-1 test vectors
 */
//static char *msg[] =
//{
//    "abc",
//    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
//    NULL
//};
//
//static char *val[] =
//{
//    "a9993e364706816aba3e25717850c26c9cd0d89d",
//    "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
//    "34aa973cd4c4daa4f61eeb2bdbad27316534016f"
//};

}

#endif

