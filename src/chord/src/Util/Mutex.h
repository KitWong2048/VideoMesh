/*
Mutex.h

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

/**	@file Mutex.h
 *	This file contains the class for mutually exclusive lock mechanism.
 */

#ifndef _H_MUTEX_
#define _H_MUTEX_

//#define MutexCheck

#include "../Util/Common.h"
using namespace std;

namespace Util{

	/**	@class Mutex
	 *	This class declares the mutually exclusive lock mechanism.
	 */
	class Mutex{
	private:
		/**	@var mutexLock
		 * The mutually exclusive lock object.
		 */
		#ifdef DHT_WIN32				
				CRITICAL_SECTION mutexLock;
		#else
				pthread_mutex_t mutexLock;
		#endif

		#ifdef MutexCheck

	#endif

	public:
		Mutex(string _name = "");

		~Mutex();

		/**	@fn int lock()
		 *	@return 0 if the mutex is locked.
		 *	This function locks the mutex.
		 */
		int lock();

		/**	@fn int release()
		 *	@return 0 if the mutex is released.
		 *	This function releases the mutex.
		 */
		int release();

		/**	@var name
		 * Name of the lock for debugging purpose.
		 */
		string name;

		friend class ConditionVariable;
	};

}
#endif
