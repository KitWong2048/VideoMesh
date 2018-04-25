/*
SeqGenerator.cpp

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

#include "../Util/SeqGenerator.h"

namespace Util{
	SeqGenerator::SeqGenerator(long _root){
		current = _root;
	}

	SeqGenerator::~SeqGenerator(){

	}

	unsigned long SeqGenerator::getNext(){
		unsigned long returnValue;

		seqMutex.lock();
		// 2 ^ 31 = 2147483648
		returnValue = current;
		current = current % (unsigned long) 2147483648UL;
		current++;
		seqMutex.release();
		return returnValue;
	}

	string SeqGenerator::getNextInStr(){
		stringstream ss;

		ss<<getNext();
		return ss.str();
	}

}
