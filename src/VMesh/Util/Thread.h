/*
Thread.h

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

/**	@file Thread.h
 *	This file contains the class for encapsulating thread support which is system dependent.
 */

#ifndef _H_THREAD_IN_VMESH_
#define _H_THREAD_IN_VMESH_

#include "../Common/VMeshCommon.h"

_VMESH_NAMESPACE_HEADER
/**	@class Thread
 *	This class declares the data structure for threading.
 */
class Thread
{
protected:
	/**	@var threadHandle
	 * Thread ID.
	 */
	#ifdef _VMESH_WIN32_
		HANDLE threadHandle;
	#else
		pthread_t threadHandle;
	#endif	

	/**	@var isRunning
	 * Indicate whether the thread is running.
	 */
	bool isRunning;

	/**	@var isCancelled
	 * Indicate whether the thread is cancelled.
	 */
	bool isCancelled;

	/**	@var isJoinable
	 * Indicate whether the thread is isJoinable.
	 */
	bool isJoinable;

protected:
	/** @fn virtual void* execute()
	 *	@return: return NULL for Thread class;
	 *	This is the entry point of the new thread.
	 * Subclass should overload this function to implement the responsibility of the new thread.
	 */
	virtual void* execute();

public:
	Thread();

	virtual ~Thread();

	/** @fn virtual int run(void* _argv = NULL);
	*	@param _argv: the argument for running the thread
	*	@return 0 is success; otherwise return failure code of the underlying thread library
	*	Trigger a new thread to start execution.
	*/
	virtual int run(void* _argv = NULL);

	/** @fn int join();
	 *	@param _return: the address where the execution result should be stored.
	 * @return: return 0 is success; otherwise return failure code of the underlying thread library
	 *	Wait until the thread has finished and reclaims any resources allocated.
	 */
	int join(void** _return = NULL);

	/** @fn int detach();
	 * @return: return 0 is success; otherwise return failure code of the underlying thread library
	 *	Detach the thread, which means resources allocated to the thread will be reclaimed without joining.
	 */
	int detach();

	/** @fn int cancel();
	 *	Cancel the executing thread.
	 * @return: return 0 is success; otherwise return failure code of the underlying thread library
	 */
	int cancel();

	/** @fn getRunningStatus();
	 *	get the RunningStatus
	 * @return: true if running, false otherwise
	 */
	bool getRunningStatus();

	/** @fn getCancelStatus();
	 *	get the CancelStatus
	 * @return: true if cancelled, false otherwise
	 */
	bool getCancelStatus();

	/** @fn getJoinableStatus();
	 *	get the JoinableStatus
	 * @return: true if joinable, false otherwise
	 */
	bool getJoinableStatus();

	#ifdef _VMESH_WIN32_
		/** @fn friend DWORD __stdcall callExecuteWin32(void* _arg)
		*	A dummy function for executing Thread::execute().
		*	This is needed because the LibC requires a function of such interface as the entry point.
		*	This function is for Win32 platform.
		* @return whatever Thread::execute() returns.
		*/
		friend DWORD __stdcall callExecuteWin32(void* _arg);
	#else
		/** @fn friend void* callExecute(void* _arg)
		*	A dummy function for executing Thread::execute().
		* This is needed because the LibC requires a function of such interface as the entry point.
		* @return whatever Thread::execute() returns.
		*/
		friend void* callExecute(void* _arg);
	#endif
};

#ifdef _VMESH_WIN32_
	/** @fn DWORD __stdcall callExecuteWin32(void* _arg)
	*	Actual function declartion
	*/
	DWORD __stdcall callExecuteWin32(void* _arg);
#else
	/** @fn friend void* callExecute(void* _arg)
	*	Actual function declartion
	*/
	void* callExecute(void* _arg);
#endif

_VMESH_NAMESPACE_FOOTER
#endif /*THREAD_H_*/
