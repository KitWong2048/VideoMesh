/*
DistributedTest.h

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

#ifndef _H_DISTRIBUTED_TEST_
#define _H_DISTRIBUTED_TEST_

#include "chord/src/DHT/Chord.h"
#include "chord/src/DHT/FingerTable.h"
#include "chord/src/DHT/LocalNodeHandle.h"
#include "chord/src/Util/SHA1Util.h"
#include "chord/src/Util/Common.h"
#include "chord/src/Util/InetAddress.h"
#include "chord/src/App/ChordApp.h"

using namespace DHT;

class DistributedTest{
public:
	void repeatTests(int bootStrap, bool isPlanetTest = false);

	int doTest(int bootStrap, int runningTime, int& successHit, int& totalHit);
};

#endif
