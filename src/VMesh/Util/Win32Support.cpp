/*
Win32Support.cpp

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

#include "../Util/Win32Support.h"

_VMESH_NAMESPACE_HEADER
#ifdef _VMESH_WIN32_
void bzero (void *block, size_t size){
	memset(block, 0, size);
}

void sleep (unsigned int seconds){
	Sleep(seconds * 1000);
}

void usleep (unsigned int seconds){
	Sleep(seconds / 1000);
}

int gettimeofday(struct timeval *tv, struct timezone *tz){
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv){
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tmpres /= 10;  /*convert into microseconds*/
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz){
		if (!tzflag){
		_tzset();
		tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}


int inet_aton (const char *name, struct in_addr *addr){
	addr->s_addr = ::inet_addr(name);
	return 1;
}

int pthread_cond_init (pthread_cond_t *cv, const pthread_condattr_t *){
	return 0;
}

int pthread_cond_wait (pthread_cond_t *cv, pthread_mutex_t *external_mutex){
	return 0;
}

int pthread_cond_timedwait (pthread_cond_t *cv, pthread_mutex_t *external_mutex, int* timeout){
	return 0;
}

int pthread_cond_signal (pthread_cond_t *cv){
	return 0;
}

int pthread_cond_broadcast (pthread_cond_t *cv){
	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cv){
	return 0;
}

_VMESH_NAMESPACE_FOOTER

#endif

