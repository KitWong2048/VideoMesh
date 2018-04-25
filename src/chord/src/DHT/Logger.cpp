/*
Logger.cpp

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

#include "../DHT/Logger.h"

using namespace Util;

namespace DHT{
	Logger::Logger() : logMutex("LogLock"){
		out = NULL;
		logLevel = LogCommon;
	}

	Logger::~Logger(){
	}

	int Logger::openLogStream(const char* _path, int _logLevel){
		fstream* fout;
		int status = 0;

		this->logLevel = _logLevel;

		if (_path == NULL){
			// the path is empty, associate the stream with standard output
			this->out = &std::cout;
		}else if (!strcmp(_path, "")){
			this->out = &std::cout;
		}else if (this->logLevel == LogNone){
			this->out = &std::cout;
		}else{
			// open an output stream to the file
			//fout = new fstream();
			fout = new fstream();
			fout->open(_path);
			if (fout->is_open()){
				status = 0;
				this->out = fout;
			}else{
				status = -1;
				delete fout;
			}
		}

		return status;
	}

	int Logger::closeLogStream(){
		int status = 0;
		fstream* fout;

		// close the log if it has been associated with a file
		if (this->out != &std::cout){
			fout = (fstream*) this->out;
			if (fout->is_open())
				fout->close();
		}

		return status;
	}

	int Logger::writeLog(const string& _data, LogLevel _msgLevel){
		int status = 0;

		if (isLogThisLevel(_msgLevel)){
			if (out != NULL){
				logMutex.lock();
				*(this->out)<<_data;
				this->out->flush();
				logMutex.release();
			}else{
				cout<<_data;
				status = -1;
			}
		}

		return status;
	}

	int Logger::getLogLevel(){
		return this->logLevel;
	}

	bool Logger::isLogThisLevel(LogLevel _msgLevel){
		if ((logLevel & _msgLevel) == _msgLevel)
			return true;
		else return false;
	}

}
