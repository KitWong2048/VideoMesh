/*
Stabilizer.h

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

/**	@file Stabilizer.h
 *	This file contains the declaration of stabilizer which is used to maintain Chord.
 */

#ifndef _H_STABILIZER_
#define _H_STABILIZER_

#include "../Util/Common.h"
#include "../Util/Thread.h"
#include "../DHT/Chord.h"

namespace DHT{
	/**	@class Stablizer
	 *	This class declares the stabilizer which is used to maintain Chord.
	 */
	class Stabilizer : public Util::Thread{
	private:
		/** @var log
		 * The reference to all the local nodes.
		 */
		Chord* chord;

		/** @var fixingInterval
		 * The interval for the stabilizer to fix fingers.
		 */
		unsigned int fixingInterval;

		/** @var stabilizingInterval
		 * The interval for the stabilizer to notify others.
		 */
		unsigned int stabilizingInterval;

		/** @var neighborVisitInterval
		 * The interval for the stabilizer to visit neighbors.
		 */
		unsigned int neighborVisitInterval;

		/** @var log
		 * The reference used for logging.
		 */
		Logger* log;

	protected:
		/** @fn void* execute()
		 * @return NULL;
		 *	This is the starting point of running the stabilizer.
		 * Override the execute() method in Thread class.
		 */
		void* execute();

	public:
		Stabilizer(Chord& _chord, unsigned int _fixingInterval = FIXING_INTERVAL, unsigned int _stabilizingInterval = STABILIZING_INTERVAL, unsigned int _neighborVisitInterval = NEIGHBOR_VISIT_INTERVAL);

		virtual ~Stabilizer();

		/**	@fn void changeStepInInterval(unsigned int _fixingInterval, unsigned int _stabilizingInterval, unsigned int _neighborVisitInterval)
		 *	@param _fixingInterval: The interval for running finger table fixing protocol.
		 *	@param _stabilizingInterval: The interval for getting successor list from the successor.
		 *	@param _neighborVisitInterval: The interval for checking if neighbors are alive.
		 *	@return Nil.
		 *	This function allows changing how often the stabilizer runs.
		 */
		void changeStepInInterval(unsigned int _fixingInterval, unsigned int _stabilizingInterval, unsigned int _neighborVisitInterval);
		
		/**	@fn void isDHTRunning()
		 *	@return true if DHT is running; false otherwise.
		 */			
		bool isDHTRunning();		
	};

}

#endif
