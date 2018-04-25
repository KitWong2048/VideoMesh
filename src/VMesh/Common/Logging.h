#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <cstdlib>
#include <string>

#ifndef LOGGING_OFF
// log the debug message to the server
#define DEBUG_MSG(X) \
	Logging::getInstance()->write_msg(4, X);
// log the statistic message to the server
#define STAT_MSG(X) \
	Logging::getInstance()->write_msg(8, X);
// get the timer handler and start the timer
#define GET_TIMER(X) \
	X = Logging::getInstance()->getTimer();
// stop the timer and log the duration to the server
#define STOP_TIMER(X, Y, Z) \
	Z = Logging::getInstance()->stopTimer(X, Y, Z);
#else
#define STAT_MSG(X) \
	do{}while(0);
#define DEBUG_MSG(X) \
	do{}while(0);
#define GET_TIMER(X) \
	do{}while(0);
#define STOP_TIMER(X, Y, Z) \
	do{}while(0);
#endif

#define MAXLOGMSGLEN 4097

#ifdef _WIN32
	#ifdef DLL_EXPORTS
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif
#else
	#define DLL_API
#endif

class Timer;
class Timer_Dispatcher;
class LogEvent;
class LogRecord;
class QMgr;
class DMgr;
class FMgr;
class Disp_Network;


/**
 * Logging Class.
 * Every client program used the logging service 
 * should include this header file.
 *
 * @author    Tommy Lau <aming@ust.hk>
 * @date      2008-01-24
 *
 */
class DLL_API Logging
{
public:

	/**
	 * Abstract class of Logging Event.
	 * inteval_log of this class will be called periodically
	 * after added into Logging Class
	 *
	 * @author    Tommy Lau <aming@ust.hk>
	 * @date      2008-01-24
	 *
	 */
	class Event
	{
	public:
		/**
		 * Vritual method call when the Logging scheduler timeout.
		 *
		 * @return	int			return 0 if success
		 *
		 */
		virtual int inteval_Log() = 0;
	};

	/**
	 * Get Timer.
	 *
	 * @return	int				return the Timer ID, -1 if cannot get Timer
	 *
	 */
	int getTimer();

	/**
	 * Stop Timer and Send out the message.
	 *
	 * @param		Id				Timer Id got by getTimer
	 * @param		msg				Message to be logged, log nothing if msg is NULL
	 * @param		offset		Time between getTimer and stopTimer will be deducted by offset
	 * @return	int				return 0 if success
	 *
	 */
	int stopTimer(int Id, char* msg = NULL, int offset = 0);
	/**
	 * Write the message to the Logging Service.
	 *
	 * @param		priority	Level of the message
	 * @param		msg				Message to be written
	 * @return	int				return 0 if success
	 *
	 */
	int write_msg(int priority, const char* msg);
	/**
	 * Start Scheduled Log.
	 *
	 * @return	int				return 0 if success
	 *
	 */
	int startScheduleLog();
	/**
	 * Stop Scheduled Log.
	 *
	 * @return	int				return 0 if success
	 *
	 */
	int stopScheduleLog();
	/**
	 * Add Schedule Log.
	 *
	 * @param		cb				Pointer to event to be log
	 * @return	int				return 0 if success
	 *
	 */
	int addScheduleLog(Event* cb);

	/**
	 * Set the Logging Server Info, Log data will send to new server.
	 *
	 * @param	ipAddr			IP Address of the Log Server
	 * @param	port			Port Number of the Log Server
	 * @return	int				return 0 if success
	 *
	 */
	int setLoggingServer(std::string ipAddr, int port);

	/**
	 * Set the interval of dispatch log messsage
	 *
	 * @param	interval			the Interval of dispatch log messsage
	 * @return	int					return 0 if success
	 *
	 */
	static int setLoggingInterval(int interval);

	/**
	 * Get an Instance of Logging.
	 *
	 * @param		procName	Process name which will be recorded in the Log message
	 * @return	int				return 0 if success
	 *
	 */
	static Logging* getInstance(char procName[] = "");
	/**
	 * Release.
	 *
	 * @return	int				return 0 if success
	 *
	 */
	static int release();
	
private:
	/**
	 * Constructor.
	 *
	 * @param     filename  File name of the configuration file
	 *
	 */
	Logging(char filename[] = "svc.conf");
	/**
	 * Constructor.
	 *
	 * @param     logger_port  Port Number of the Logging Server
	 * @param     logger_host  IP Address of the Logging Server
	 *
	 */
	//Logging(unsigned short logger_port, char logger_host[]);
	/**
	 * Destructor.
	 *
	 */
	~Logging();

	Timer*			p_timer;
	static Logging*	_pInstance;
	char			s_procName[128];
	LogEvent*		p_event;
	QMgr*			p_qmgr;
	DMgr*			p_dmgr;
	FMgr*			p_fmgr;
	Disp_Network*	p_networkDisp;

	static int logInterval;
};


extern Logging* _pInstance;

#endif
