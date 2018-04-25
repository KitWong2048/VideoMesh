/*
ChordAttr.cpp

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

#include "../App/ChordAttr.h"

using namespace Util;

namespace App{
	ChordAttr::ChordAttr(){
		isMultiThreaded =  false;
		listeningIP = "127.0.0.1";
		listeningPort = 0;
		connectionTimeOut = DHT_CONNECTION_TIMEOUT;
		connectionNum = MAX_DHT_CONNECTION;
		stabilizingInterval = STABILIZING_INTERVAL;
		fixingInterval = FIXING_INTERVAL;
		neighborVisitInterval = NEIGHBOR_VISIT_INTERVAL;
		maxSearchTime = 60;
		neighborhoodSize = DEFAULT_NEIGHBOR_SIZE;
		// for logging level, LogNormal is for normal use, LogDebug prints out routing information
		// can be combined together by LogNormal | LogDebug
		logLevel = LogNormal | LogError;
		logPath = "";
	}

	string ChordAttr::checkAndCorrect(){
		stringstream ss;
		if ((connectionNum <=0) || (connectionNum > MAX_DHT_CONNECTION)){
			ss<<"DHT connection number is invalid("<<connectionNum<<"). Set to "<<MAX_DHT_CONNECTION<<"."<<endl;
			connectionNum = MAX_DHT_CONNECTION;
		}

		if (fixingInterval <= 0){
			ss<<"Fixing interval is too short. Set to "<<FIXING_INTERVAL<<"."<<endl;
			fixingInterval = FIXING_INTERVAL;
		}
		if (stabilizingInterval <= 0){
			ss<<"Stabilizing interval is too short. Set to "<<STABILIZING_INTERVAL<<"."<<endl;
			stabilizingInterval = STABILIZING_INTERVAL;
		}
		if (neighborVisitInterval <= 0){
			ss<<"Neighbor visit interval is too short. Set to "<<NEIGHBOR_VISIT_INTERVAL<<"."<<endl;
			neighborVisitInterval =  NEIGHBOR_VISIT_INTERVAL;
		}

		if (connectionTimeOut < DHT_MIN_CONNECTION_TIMEOUT){
			ss<<"Connection time out value is too short. Set to "<<DHT_MIN_CONNECTION_TIMEOUT<<"."<<endl;
			connectionTimeOut = DHT_MIN_CONNECTION_TIMEOUT;
		}

		if (neighborhoodSize <= 0){
			ss<<"The neighborhood is too small. Set to "<<DEFAULT_NEIGHBOR_SIZE<<"."<<endl;
			neighborhoodSize = DEFAULT_NEIGHBOR_SIZE;
		}

		return ss.str();
	}

}

