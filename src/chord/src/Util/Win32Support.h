/*
Win32Support.h

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

/**	@file Win32Support.h
 *	This file defines misc. functions and structures.
 */

#ifndef _H_FUNCTION_
#define _H_FUNCTION_

#include "../Util/Common.h"

namespace Util{
	#ifdef DHT_WIN32
		#define ETIMEDOUT -1

		void bzero (void *block, size_t size);

		void sleep (unsigned int seconds);

		struct timezone {
			int  tz_minuteswest; /* minutes W of Greenwich */
			int  tz_dsttime;     /* type of dst correction */
		};

		#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
			#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
		#else
			#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
		#endif

		int gettimeofday(struct timeval *tv, struct timezone *tz);

		// supporting network
		typedef int socklen_t;

		int inet_aton (const char *name, struct in_addr *addr);

		// supporting condition variable
		typedef void pthread_condattr_t;

		typedef CRITICAL_SECTION pthread_mutex_t;

		typedef struct{
			int waiters_count_;
			// Count of the number of waiters.

			CRITICAL_SECTION waiters_count_lock_;
			// Serialize access to <waiters_count_>.

			int release_count_;
			// Number of threads to release via a <pthread_cond_broadcast> or a
			// <pthread_cond_signal>.

			int wait_generation_count_;
			// Keeps track of the current "generation" so that we don't allow
			// one thread to steal all the "releases" from the broadcast.

			HANDLE event_;
			// A manual-reset event that's used to block and release waiting
			// threads.
		}pthread_cond_t;

		int pthread_cond_init (pthread_cond_t *cv, const pthread_condattr_t *);

		int pthread_cond_wait (pthread_cond_t *cv, pthread_mutex_t *external_mutex);

		int pthread_cond_timedwait (pthread_cond_t *cv, pthread_mutex_t *external_mutex, int* timeout);

		int pthread_cond_signal (pthread_cond_t *cv);

		int pthread_cond_broadcast (pthread_cond_t *cv);

		int pthread_cond_destroy(pthread_cond_t *cv);

	#endif
}

#endif
