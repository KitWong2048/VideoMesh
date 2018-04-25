/*
SHA1Util.cpp

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

#include "../Util/SHA1Util.h"

namespace Util{

	char* sha1HashHex(const char* msg){
		unsigned char* sha1sum;
		char* hexOutput;

		hexOutput = new char[SHA1_LENGTH * 2 + 1];
		sha1sum = sha1Hash(msg);
		for(int j = 0; j < 20; j++ )
			sprintf( hexOutput + j * 2, "%02x", sha1sum[j] );
		delete []sha1sum;
		hexOutput[SHA1_LENGTH * 2] = '\0';

		return hexOutput;
	};

	unsigned char* sha1Hash(const char* msg){
		sha1_context ctx;
		unsigned char* sha1sum;

		sha1sum = new unsigned char[SHA1_LENGTH + 1];

		if (msg != NULL){
			sha1_starts( &ctx );
			sha1_update( &ctx, (uint8 *) msg, strlen( msg ) );
			sha1_finish( &ctx, sha1sum );
			sha1sum[SHA1_LENGTH] = '\0';
		}else bzero(sha1sum, SHA1_LENGTH + 1);

		return sha1sum;
	};

}
