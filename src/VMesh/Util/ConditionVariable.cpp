/*
ConditionVaraible.cpp

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

#include "../Util/ConditionVariable.h"

_VMESH_NAMESPACE_HEADER

ConditionVariable::ConditionVariable(){
	pthread_cond_init(&condVar, NULL);
}

ConditionVariable::~ConditionVariable(){
	pthread_cond_destroy(&condVar);
}

int ConditionVariable::wait(Mutex& _mutex){
	return pthread_cond_wait (&condVar, &(_mutex.mutexLock));
}

int ConditionVariable::timedWait(Mutex& _mutex, int _millisecond){
	#ifndef _VMESH_WIN32_
		struct timeval now;
		struct timespec timeout;

		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + _millisecond / 1000;
		timeout.tv_nsec = (now.tv_usec + (_millisecond % 1000) * 1000) * 1000;
	#else
		int timeout = _millisecond;
	#endif

     if (pthread_cond_timedwait(&condVar, &(_mutex.mutexLock), &timeout) == ETIMEDOUT)
     	return -1;
     else return 0;
}

int ConditionVariable::timedWait(Mutex& _mutex, struct timeval timeout){
	#ifndef _VMESH_WIN32_
		struct timeval now;
		gettimeofday(&now, NULL);
	     
		timeout.tv_sec += now.tv_sec;
		timeout.tv_usec += now.tv_usec;
	     
		int i = timeout.tv_usec / (1000 * 1000);
		timeout.tv_usec %= (1000 * 1000);
		timeout.tv_sec += i;
	     
		struct timespec t;
		t.tv_sec = timeout.tv_sec;
		t.tv_nsec = timeout.tv_usec*1000;
	#else
		int t = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
	#endif
    
     if (pthread_cond_timedwait(&condVar, &(_mutex.mutexLock), &t) == ETIMEDOUT)
     	return -1;
     else return 0;
}

int ConditionVariable::signal(){
	return pthread_cond_signal (&condVar);
}

int ConditionVariable::boardcast(){
	return pthread_cond_broadcast (&condVar);
}

_VMESH_NAMESPACE_FOOTER
