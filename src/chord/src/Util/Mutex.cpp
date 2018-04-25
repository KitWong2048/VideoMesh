/*
Mutex.cpp

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

#include "../Util/Common.h"
#include "../Util/Mutex.h"

namespace Util{

	Mutex::Mutex(string _name){
		name = _name;
		#ifdef DHT_WIN32
			InitializeCriticalSection(&mutexLock);
		#else
			pthread_mutex_init(&mutexLock, NULL);
		#endif					
	}

	Mutex::~Mutex(){
		#ifdef DHT_WIN32
			DeleteCriticalSection(&mutexLock);
		#else
			pthread_mutex_destroy(&mutexLock);
		#endif							
	}

	int Mutex::lock(){
		#ifdef DHT_WIN32
			EnterCriticalSection(&mutexLock);
			return 0;
		#else
			return pthread_mutex_lock(&mutexLock);
		#endif							
	}

	int Mutex::release(){
		#ifdef DHT_WIN32
			LeaveCriticalSection(&mutexLock);
			return 0;
		#else
			return pthread_mutex_unlock(&mutexLock);
		#endif							
	}

}
