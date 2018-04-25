/*
Thread.cpp

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

#include "../Util/Thread.h"

namespace Util{
	Thread::Thread(){
		isRunning = false;
		isCancelled = false;
		isJoinable = false;
	}

	Thread::~Thread(){
	}

	int Thread::run(void* _argv){
		int status = -1;

		if (!isRunning){
			isRunning = true;			
			#ifdef DHT_WIN32
				threadHandle = CreateThread(NULL, 0, callExecuteWin32, this, 0, NULL);
				if (threadHandle != NULL)
					status = 0;
				else status = -1;
			#else
				status = pthread_create(&threadHandle, NULL, callExecute, this);
			#endif					
			isJoinable = true;
		}else status = 0;
		return status;
	}

	void* Thread::execute(){
		return NULL;
	}

	int Thread::join(void** _return){
		if (isJoinable){
			isJoinable = false;
			#ifdef DHT_WIN32
				WaitForSingleObject(threadHandle, INFINITE);
				return 0;
			#else
				return pthread_join(threadHandle, _return);
			#endif			
		}else return 0;
	}

	int Thread::detach(){		
		#ifdef DHT_WIN32
			return 0;
		#else
			return pthread_detach(threadHandle);
		#endif			
	}

	int Thread::cancel(){
		isCancelled = true;
		#ifdef DHT_WIN32
			return TerminateThread(threadHandle, 0);
		#else
			return pthread_cancel(threadHandle);
		#endif			
	}

	bool Thread::getRunningStatus(){
		return isRunning;
	}

	bool Thread::getCancelStatus(){
		return isCancelled;
	}

	bool Thread::getJoinableStatus(){
		return isJoinable;
	}

	HANDLE Thread::getThreadHandler()
	{
		return	threadHandle;
	}

	#ifdef DHT_WIN32
		DWORD __stdcall callExecuteWin32(void* _arg){
			Thread* t;
			
			t = (Thread*) _arg;
			if (t != NULL){
				t->isRunning = true;
				t->execute();
				t->isRunning = false;
			}
			ExitThread(0);
		}
	#else
		void* callExecute(void* _arg){
			Thread* t;
			void* result = NULL;

			t = (Thread*) _arg;
			if (t != NULL){
				t->isRunning = true;
				result = t->execute();
				t->isRunning = false;
			}

			return result;
		}
	#endif
}
