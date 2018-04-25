#ifndef _STATIC_LOGGER_H_
#define _STATIC_LOGGER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <ostream>
#include "../Util/Mutex.h"
#include "../Common/VMeshCommon.h"

// For disable Logging
//#define LOGGING_OFF

#include "../Common/Logging.h"

/**	@enum StatType
*	This enum represents different types of stat figures
*/
enum StatType{
	UP_DATA,
	DOWN_DATA,
	UP_CONTROL,
	DOWN_CONTROL
};




/**	@file Logger.h
*	This file contains the definition of class Logger
*	log4cxx framework is adapted and simplified
*	website of log4cxx: http://logging.apache.org/log4cxx/manual/Introduction.html
*	@author Kelvin Chan
*/
namespace log4cxx {

	/**	@struct stat_data
	*	This struct stores information for statistics purpose.
	*	All values are in byte
	*	Note that the data can be cleared after retrieving
	*/
	struct stat_data{
		unsigned long uplink_data;
		unsigned long downlink_data;
		unsigned long uplink_control;
		unsigned long downlink_control;
		std::string hash_key;
	};


	class myEvent;


	/**	@class Logger
	*	This class declares the logging system which allows multithreading.
	*/
	class Logger {
	public:

		/**	@fn static bool open(const char* file = 0)
		*	This function opens a file or stdout for showing log
		*	Use OPEN_LOG(file) for alias
		*	@param file filename of the output file
		*	@param file NULL if stdout
		*	@return true if open successfully
		*/
		static bool open(const char* file = 0);

		/**	@fn static void close()
		*	This function close the output for showing log
		*	Use CLOSE_LOG() for alias
		*/
		static void close();

		/**	@fn static void debug(const std::string& message)
		*	This function logs a debug message
		*	Use LOG_DEBUG(msg) for alias
		*	@param message message to be logged
		*/
		static void debug(const std::string& message);

		/**	@fn static void info(const std::string& message)
		*	This function logs an info message
		*	Use LOG_INFO(msg) for alias
		*	@param message message to be logged
		*/
		static void info(const std::string& message);

		/**	@fn static void error(const std::string& message)
		*	This function logs an error message
		*	Use LOG_ERROR(msg) for alias
		*	@param message message to be logged
		*/
		static void error(const std::string& message);

		/**	@fn static void log(const char* level, const std::string& message)
		*	This function logs a message of generic level
		*	@param level string of the level
		*	@param message message to be logged
		*/
		static void log(const char* level, const std::string& message);

		/**	@fn static stat_data retrieve_stat(bool destructive = true)
		*	This function retrieve the current stat data
		*	@param destructive clear the stat data after retrieval
		*	@return the current stat
		*/
		static stat_data retrieve_stat(bool destructive = true);

		static void set_hash(std::string hash);

		/**	@fn static void aggregate_stat(StatType type, long val)
		*	This function update the current stat data
		*	@param type which type of stat data to aggregate
		*	@param val value to be modified(added)
		*/
		static void aggregate_stat(StatType type, long val);

	private:
		/**	@var showlock
		*	The mutually exclusive lock object for log
		*/
		static VMesh::Mutex showlock;

		/**	@var statlock
		*	The mutually exclusive lock object for stat
		*/
		static VMesh::Mutex statlock;

		/**	@var myStat
		*	The variable to store statistic data
		*/
		static stat_data myStat;

		/**	@var out
		*	The stream to be logged
		*/
		static std::ostream* out;

		/**	@var myEvent
		*	The variable to store the logging event of the ACE logging System
		*/
		static myEvent* ev;
	};

#ifndef LOGGING_OFF
	class myEvent : public Logging::Event
	{
	public:
		myEvent()
		{
			Logging::setLoggingInterval(60);
			Logging::getInstance("VMesh Core")->addScheduleLog(this);
			Logging::getInstance("VMesh Core")->startScheduleLog();
		}

		int inteval_Log()
		{
			char msg[1024] = {0};
			stat_data stat = Logger::retrieve_stat(true);
			int upVideo = stat.uplink_data;
			int downVideo = stat.downlink_data;
			int upCtrl = stat.uplink_control;
			int downCtrl = stat.downlink_control;
			//sprintf(msg, "CT(sec) -1, UD(mb) %d, DD(mb) %d, UT(mb) %d, DT(mb) %d, AUD(mb) %ld, ADD(mb) %ld, AUT(mb) %ld, ADT(mb) %ld, P -1, CL -1\n", uplink*8/1000/1000, downlink*8/1000/1000, uplinkAll*8/1000/1000, downlinkAll*8/1000/1000, AggUpload*8/1000/1000, AggDownload*8/1000/1000, AggTotalUpload*8/1000/1000, AggTotalDownload*8/1000/1000);
			sprintf(msg, "UD(mb) %d, DD(mb) %d, UC(kb) %d, DC(kb) %d, P -1, CL -1, HASH %s\n", upVideo*8/1000/1000, downVideo*8/1000/1000, upCtrl*8/1000, downCtrl*8/1000, stat.hash_key.c_str());
			STAT_MSG(msg);
			return 0;
		}
	};
#else
	class myEvent
	{
	public:
		int inteval_Log(){return 0;}
	};
#endif

};

/*! @brief macro for calling Logger::open(file) */
#define OPEN_LOG(file) log4cxx::Logger::open(file)
/*! @brief macro for calling Logger::close() */
#define CLOSE_LOG() log4cxx::Logger::close()
/*! @brief macro for calling Logger::debug(msg) */
#define LOG_DEBUG(msg) log4cxx::Logger::debug(msg)
/*! @brief macro for calling Logger::info(msg) */
#define LOG_INFO(msg) log4cxx::Logger::info(msg)
/*! @brief macro for calling Logger::error(msg) */
#define LOG_ERROR(msg) log4cxx::Logger::error(msg)
/*! @brief macro for calling Logger::aggregate_stat(type, val) */
#define STAT_AGGREGATE(type, val) log4cxx::Logger::aggregate_stat(type, val)
/*! @brief macro for calling Logger::retrieve_stat(destruct) */
#define STAT_RETRIEVE(destruct) log4cxx::Logger::retrieve_stat(destruct)

#endif

