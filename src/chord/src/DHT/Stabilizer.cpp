/*
Stabilizer.cpp

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

#include "../DHT/Stabilizer.h"

using namespace Util;

namespace DHT{
	Stabilizer::Stabilizer(Chord& _chord, unsigned int _fixingInterval, unsigned int _stabilizingInterval, unsigned int _neighborVisitInterval) : Thread(){
		chord = &_chord;
		log = chord->getLogger();
		changeStepInInterval(_fixingInterval, _stabilizingInterval, _neighborVisitInterval);
	}

	Stabilizer::~Stabilizer(){
	}

	void Stabilizer::changeStepInInterval(unsigned int _fixingInterval, unsigned int _stabilizingInterval, unsigned int _neighborVisitInterval ){
		if (_fixingInterval > 0)
			this->fixingInterval = _fixingInterval;
		else this->fixingInterval = FIXING_INTERVAL;

		if (_stabilizingInterval > 0)
			this->stabilizingInterval = _stabilizingInterval;
		else this->stabilizingInterval = STABILIZING_INTERVAL;

		if (_neighborVisitInterval > 0)
			this->neighborVisitInterval= _neighborVisitInterval;
		else this->neighborVisitInterval = NEIGHBOR_VISIT_INTERVAL;
	}

	void* Stabilizer::execute(){
		LocalNodeCollection* localRef = NULL;
		unsigned int fixingAcc(0), stabilizingAcc(0), neighborAcc(0);
		int sleepingTime(0);

		// wake up periodically to stablilize
		while (isDHTRunning()){
			log->writeLog("Stabilizer: Kick in.\n", LogStabilizer);			

            fixingAcc += sleepingTime;
            if ((fixingAcc >= fixingInterval) && (isDHTRunning())){
				localRef = chord->localNodes;
                localRef->fixFingers();
                fixingAcc = 0;
            }

            stabilizingAcc += sleepingTime;
            if ((stabilizingAcc >= stabilizingInterval) && (isDHTRunning())){
				localRef = chord->localNodes;
                localRef->stabilize();
                stabilizingAcc = 0;
            }

            neighborAcc += sleepingTime;
            if ((neighborAcc >= neighborVisitInterval) && (isDHTRunning())){
				localRef = chord->localNodes;
                localRef->visitNeighbor();
                neighborAcc = 0;
            }

            if (log->isLogThisLevel(LogStabilizer))
                chord->print(LogNormal);

			if (isDHTRunning()){
				// sleep for a while and then wake up for stabilizer process
				// because it has the check whether the DHT has been ordered to stop, the stabilizer thread should sleep for at most MIN_STABILIZER_INTERVAL
                sleepingTime = min(fixingInterval, stabilizingInterval);
                sleepingTime = min(sleepingTime, MIN_STABILIZER_INTERVAL);
                sleep(sleepingTime);
			}

			log->writeLog("Stabilizer: Done.\n", LogStabilizer);
		}

		return NULL;
	}
	
	bool Stabilizer::isDHTRunning(){
		if (chord == NULL)
			return false;
		else return chord->getDHTRunningStatus();
	}

}
