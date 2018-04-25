/*
ChordAttr.h

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

/**	@file ChordAttr.h
 *	This file contains the class to configure Chord.
 */

 #ifndef _H_CHORD_ATTR_
#define _H_CHORD_ATTR_

#include "../Util/Common.h"
#include <iostream>
using namespace std;

namespace App
{
	/**	@class ChordAttr
	 *	This class is used to configure chord.
	 */
	class ChordAttr
	{
	public:
		ChordAttr();

		/**	@fn string checkAndCorrect()
		* Check whether the fields are correct and correct the mistakes to default values if necessary.
		* @return a string that contains the mistake; empty string if no mistake
		*/
		string checkAndCorrect();

		/**	@var connectionNum
		* number of simultaneous clients
		*/
		int connectionNum;

		/**	@var connectionTimeOut
		* time out interval for connecting and receiving, measured in millisecond
		*/
		unsigned int connectionTimeOut;

		/**	@var listeningPort
		* port number to listen to
		*/
		unsigned int listeningPort;

		/**	@var listeningIP
		* IP address to listen to
		*/
		string listeningIP;

		/**	@var stabilizingInterval
		* the time interval to notify successor about local nodes, measured in second
		*/
		unsigned int stabilizingInterval;

		/**	@var fixingInterval
		* the time interval to fix finger table, measured in second
		*/
		unsigned int fixingInterval;

		/**	@var neighborVisitInterval
		* the time interval to check whether neighbor still here, measured in second
		*/
		unsigned int neighborVisitInterval;

		/**	@var logLevel
		* amount of desired log messages, can be constructed from bitwise OR
		*/
		int logLevel;

		/**	@var logPath
		* the path to log file, if this is an empty string, standard output will be used for logging
		*/
		string logPath;

		/**	@var maxSearchTime
		* the maximum latency of searching, it will be infinite long if set to -1, measured in second
		*/
		double maxSearchTime;

		/**	@var neighborhoodSize
		* the size of the neighborhood
		*/
		unsigned int neighborhoodSize;

		/**	@var isMultiThreaded
		* The DHT will use 2 threads if this option is set to NO; use more otherwise;
		*/
		bool isMultiThreaded;
	};
}

#endif
