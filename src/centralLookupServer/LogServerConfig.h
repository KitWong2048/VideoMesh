#pragma once
#include <vector>
#include <map>
#include <ctime>
#include <string>
#include "Common.h"
using std::vector;
using std::map;
using std::string;

#define MAX_PATH			256
#define RENEW_INTERVAL		300		// 300 seconds

struct NetworkAddress
{
	string ip_address;
	int port;
};

class LogServerConfig
{
public:
	LogServerConfig(const char* filename);
	~LogServerConfig(void);

	int GetLogServerCount();
	NetworkAddress GetLogServerAddress(int nItem = -1);
	string GetConfigData(const string& entry);
	int	GetNumericConfigData(const string& entry);

private:
	char m_Filename[MAX_PATH];
	vector<NetworkAddress> m_ServerList;
	vector<NetworkAddress> m_UpdateServerList;

	map<string, int> m_IntConfigMap;
	map<string, string> m_StrConfigMap;

	time_t m_LastAccessTime;

	CRITICAL_SECTION	m_ServerListCS;
	CRITICAL_SECTION	m_IntConfigMapCS;
	CRITICAL_SECTION	m_StrConfigMapCS;
	CRITICAL_SECTION	m_AccessLock;

	void ReadConfigFile(const char* filename);
	void TrimLeftSpace(char* szInput);
	void TrimRightSpace(char* szInput);
	void ReadLogServerInfo(FILE* file);
	void ReadGeneralInfo(FILE* file);
};
