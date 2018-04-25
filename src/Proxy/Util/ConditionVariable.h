/*
ConditionVariable.h

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

#ifndef _H_CONDITION_VARIABLE_IN_VMESH_
#define _H_CONDITION_VARIABLE_IN_VMESH_

#include "../Common/VMeshCommon.h"
#include "../Util/Mutex.h"
#include "../Util/Win32Support.h"

_VMESH_NAMESPACE_HEADER

/**	@class ConditionVariable
*	This class declares the data structure for conditiona variable.
*/
class ConditionVariable
{
private:
	/**	@var condVar
	* The system dependant conditiona variable structure.
	*/
	pthread_cond_t condVar;

public:
	ConditionVariable();

	~ConditionVariable();

	/** @fn int wait(Mutex& _mutex);
	*	@param _mutex: the mutex for the critical region that the thread is holding.
	* @return 0 if a signal is received
	*	Wait for signals from the condition variable.
	*/
	int wait(Mutex& _mutex);

	/** @fn int timedWait(Mutex& _mutex, int _millisecond);
	*	@param _mutex: the mutex for the critical region that the thread is holding.
	*	@param _millisecond: the waiting time for timeout, measured in millisecond.
	* @return 0 if a signal is received; negative number if a time out occurs
	*	Wait for signals from the condition variable or a certain time period.
	*/
	int timedWait(Mutex& _mutex, int _millisecond);
	
	int timedWait(Mutex& _mutex, struct timeval timeout);

	/** @fn int signal();
	* @return 0 if success
	*	Send a signals to wake up one thread waiting for the conditiona varible.
	*/
	int signal();

	/** @fn int boardcast();
	* @return 0 if success
	*	Send a signals to wake up all threads waiting for the conditiona varible.
	*/
	int boardcast();
};


_VMESH_NAMESPACE_FOOTER
#endif
