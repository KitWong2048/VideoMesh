#include "../Util/Logger.h"

using namespace std;
using namespace log4cxx;
_USING_VMESH_NAMESPACE

std::ostream* Logger::out = &std::cout;

Mutex Logger::showlock("LoggerShowLock");
Mutex Logger::statlock("LoggerStatLock");
stat_data Logger::myStat = {0, 0, 0, 0};

myEvent* ev = new myEvent();


bool Logger::open(const char* file)
{
	bool success = true;

	if (file == NULL)
	{
		out = &std::cout;
	}
	else if (!strcmp(file, ""))
	{
		out = &std::cout;
	}
	else
	{
		// open an output stream to the file
		fstream* fout = new fstream(file, fstream::out | fstream::app);

		if (fout->is_open())
		{
			out = fout;
		}
		else
		{
			success = false;
			delete fout;
		}
	}

	return success;
}

void Logger::close()
{
	if (out!=NULL)
	{
		if (out != &std::cout){
			fstream* fout = (fstream*) out;
			if (fout->is_open())
			{
				fout->close();
				delete out;
				out = NULL;
			}
		}
	}
}

void Logger::debug(const std::string& message)
{
	//log("DEBUG", message);
	log("D", message);
	DEBUG_MSG(message.c_str());
}

void Logger::info(const std::string& message)
{
	//log("INFO", message);
	log("I", message);
	DEBUG_MSG(message.c_str());
}

void Logger::error(const std::string& message)
{
	//log("ERROR", message);
	log("E", message);
	DEBUG_MSG(message.c_str());
}
		
void Logger::log(const char* level, const std::string& message)
{
	showlock.lock();
	(*out) << "["<< level << "] " << message << "\n";
	out->flush();
	showlock.release();
}

void Logger::set_hash(std::string hash)
{
	myStat.hash_key = hash;
}

stat_data Logger::retrieve_stat(bool destructive)
{
	statlock.lock();
	stat_data curStat = myStat;
	if (destructive)
	{
		myStat.uplink_data = 0;
		myStat.downlink_data = 0;
		myStat.uplink_control = 0;
		myStat.downlink_control = 0;
	}
	statlock.release();
	return curStat;
}

void Logger::aggregate_stat(StatType type, long val)
{
	if (val<=0)
		return;
	statlock.lock();
	switch (type)
	{
	case UP_DATA:
		myStat.uplink_data += val;
		break;
	case DOWN_DATA:
		myStat.downlink_data += val;
		break;
	case UP_CONTROL:
		myStat.uplink_control += val;
		break;
	case DOWN_CONTROL:
		myStat.downlink_control += val;
		break;
	};
	statlock.release();
}
