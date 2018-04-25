/*
main.cpp

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

#include "chord/src/Test/DistributedTest.h"
#include "chord/src/Test/PoolTest.h"
#include "chord/src/Test/Test.h"
#include "chord/src/Test/FarTimeShfitTest.h"
#include "chord/src/Util/Common.h"
#include "chord/src/Util/InetAddress.h"
#include "chord/src/Util/SeqGenerator.h"
#include "chord/src/DHT/DHTMessageFactory.h"
#include "chord/src/DHT/Node.h"
#include "chord/src/DHTSocket/SessionReference.h"

#include <map>
#include <iostream>
using namespace Util;
using namespace DHT;
using namespace std;

int main(int argc, char* argv[]){
    srand(time(0));

	Test::FarTimeShiftTest obj;
	obj.doTest(argc, argv);

//	DistributedTest obj;
//	if (argc > 1){
//		obj.repeatTests(0);
//	}else{
//		obj.repeatTests(1);
//	}

	return 0;
}

