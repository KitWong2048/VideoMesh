/*
Logger.h

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

/**	@file Logger.h
 *	This file contains the class for logging.
 */
#ifndef _H_LOGGER_
#define _H_LOGGER_

/**	@class Logger
 *	This class declares the logging mechanism used in Chord.
 */

#include <iostream>
#include <fstream>
#include "../Util/Mutex.h"
#include "../Util/Common.h"

using namespace std;

namespace DHT{
	class Logger{
	private:
		Logger();

		~Logger();

		/**	@var *out
		 * The output stream of the log.
		 */
		ostream* out;

		/**	@var logMutex
		 * The mutex protects the output stream from simutaneous accsss.
		 */
		Util::Mutex logMutex;

		/**	@var logLevel
		 * The level of logging.
		 */
		int logLevel;

		/**	@fn int openLogStream(const char* _path = NULL, int _logLevel = Util::LogCommon)
		 *	@param _path: The full path to the log file.
		 *	@param _logLevel: The log level.
		 *	@return 0 if the log is successfully opened.
		 *	This function open an output streams to the log file.
		 *	If no parameter is passed into the function, std::cout will be the default output stream.
		 */
		int openLogStream(const char* _path = NULL, int _logLevel = Util::LogCommon);

		/**	@fn int closeLogStream()
		 *	@return 0 if the log is successfully closed.
		 *	This function close the output streams to the log file.
		 */
		int closeLogStream();

	public:
		/**	@fn int writeLog(const string& _data, Util::LogLevel _msgLevel)
		 *	@return 0 if the data is successfully write to log.
		 *	This function writes data to the stream in the logger object.
		 */
		int writeLog(const string& _data, Util::LogLevel _msgLevel);

		/**	@fn int getLogLevel()
		 *	@return the logging level.
		 *	This function gets the logging level.
		 */
		int getLogLevel();

		/**	@fn bool isLogThisLevel(Util::LogLevel _msgLevel)
		 *  @param _msgLevel: the logging level to be checked
		 *	@return true if the level provided is in the currect logging level.
		 *	This function checks whether the logger logs a certain level.
		 */
		 bool isLogThisLevel(Util::LogLevel _msgLevel);

		/**
		 *	The instance of Chord is responsible for opening and closing the log.
		 */
		friend class Chord;
	};
}
#endif
