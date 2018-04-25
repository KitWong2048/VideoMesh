#include "LogServerConfig.h"

LogServerConfig::LogServerConfig(const char* filename)
{
	InitializeCriticalSection(&m_ServerListCS);
	InitializeCriticalSection(&m_StrConfigMapCS);
	InitializeCriticalSection(&m_IntConfigMapCS);
	InitializeCriticalSection(&m_AccessLock);
	srand(time(0));
	strcpy(m_Filename, filename);
	ReadConfigFile(m_Filename);	
}	

LogServerConfig::~LogServerConfig(void)
{
	DeleteCriticalSection(&m_ServerListCS);
	DeleteCriticalSection(&m_StrConfigMapCS);
	DeleteCriticalSection(&m_IntConfigMapCS);
	DeleteCriticalSection(&m_AccessLock);
}

int LogServerConfig::GetLogServerCount()
{
	int nSize;
	EnterCriticalSection(&m_ServerListCS);
	nSize = m_ServerList.size();
	LeaveCriticalSection(&m_ServerListCS);
	return nSize;
}

NetworkAddress LogServerConfig::GetLogServerAddress(int nItem)
{
	NetworkAddress address;
	address.ip_address = "127.0.0.1";
	address.port = 0;

	EnterCriticalSection(&m_AccessLock);
	time_t currentTime = time(NULL);
	if ( currentTime - m_LastAccessTime > RENEW_INTERVAL )
		ReadConfigFile(m_Filename);

	int listCount = GetLogServerCount();

	if ( nItem < listCount && listCount > 0 )
	{
		if ( nItem < 0 )
			nItem = rand() % listCount;

		EnterCriticalSection(&m_ServerListCS);
		address = m_ServerList[nItem];
		LeaveCriticalSection(&m_ServerListCS);
	}
	LeaveCriticalSection(&m_AccessLock);

	return address;

}

string LogServerConfig::GetConfigData(const string& entry)
{
	map<string, string>::const_iterator it;
	EnterCriticalSection(&m_StrConfigMapCS);
	it = m_StrConfigMap.find(entry);
	LeaveCriticalSection(&m_StrConfigMapCS);
	if ( it == m_StrConfigMap.end() )
		return "";
	else
		return it->second;
}

int	LogServerConfig::GetNumericConfigData(const string& entry)
{
	map<string, int>::const_iterator it;
	EnterCriticalSection(&m_IntConfigMapCS);
	it = m_IntConfigMap.find(entry);
	LeaveCriticalSection(&m_IntConfigMapCS);
	if ( it == m_IntConfigMap.end() )
		return -1;
	else
		return it->second;
}

void LogServerConfig::ReadConfigFile(const char* filename)
{
	FILE* configFile;
	EnterCriticalSection(&m_ServerListCS);
	EnterCriticalSection(&m_StrConfigMapCS);
	EnterCriticalSection(&m_IntConfigMapCS);

	try
	{
		configFile = fopen(filename, "r");
		if ( configFile == NULL )
		{
			throw "Error opening input file";
		}
		char szLine[MAX_PATH];
		while ( fgets(szLine, MAX_PATH, configFile) != NULL )
		{
			TrimLeftSpace(szLine);
			TrimRightSpace(szLine);
			if ( strcmp(szLine, "[Logging Server]" ) == 0)
				ReadLogServerInfo(configFile);
			if ( strcmp(szLine, "[General]" ) == 0)
				ReadGeneralInfo(configFile);
		}

		m_ServerList = m_UpdateServerList;
		vector<NetworkAddress>().swap(m_UpdateServerList);
	}
	catch ( const char* szErrorMsg)
	{
		printf("[LogServerConfig] %s \n", szErrorMsg);
	}
	catch ( ... )
	{
		printf("[LogServerConfig] Error in reading data from file \n");
	}
	if ( configFile )
		fclose(configFile);

	m_LastAccessTime = time(NULL);
	LeaveCriticalSection(&m_IntConfigMapCS);
	LeaveCriticalSection(&m_StrConfigMapCS);
	LeaveCriticalSection(&m_ServerListCS);
}

void LogServerConfig::ReadLogServerInfo(FILE* file)
{

	NetworkAddress newServerAddress;
	char szLine[MAX_PATH];
	char temp[MAX_PATH];
	
	char numbers[] = "1234567890";

	int nStartPos, nEndPos;

	bool bPortFirst = true;
	if ( fgets(szLine, MAX_PATH, file) == NULL )
		throw "Error in reading log server information from file, code 01";

	if ( strstr(szLine, "ServerPort") != NULL )
	{
		nStartPos = strcspn(szLine, numbers);

		if ( nStartPos >= strlen(szLine) )
			throw "Error in reading log server port number, code 02";

		nEndPos = strrchr(szLine, '\"') - szLine;
		strncpy(temp, szLine + nStartPos, nEndPos - nStartPos);
		temp[nEndPos - nStartPos] = '\0';

		newServerAddress.port = atoi(temp);
		if ( newServerAddress.port ==  0 )
			throw "Error in reading log server port number, code 03";
	}
	else if (strstr(szLine, "ServerAddr") != NULL )
	{
		bPortFirst = false;
		nStartPos = strcspn(szLine, numbers);

		if ( nStartPos >= strlen(szLine) )
			throw "Error in reading log server ip address, code 04";

		nEndPos = strrchr(szLine, '\"') - szLine;

		strncpy(temp, szLine + nStartPos, nEndPos - nStartPos);
		temp[nEndPos - nStartPos] = '\0';

		newServerAddress.ip_address = temp;
	}
	else throw "Error in reading log server information from file, code 05";

	if ( fgets(szLine, MAX_PATH, file) == NULL )
		throw "Error in reading log server information from file, code 06";
		
	if ( strstr(szLine, "ServerPort") != NULL )
	{
		if ( bPortFirst == true )
			throw "Error in reading log server port number, code 07";
		nStartPos = strcspn(szLine, numbers);

		if ( nStartPos >= strlen(szLine) )
			throw "Error in reading log server port number, code 08";

		nEndPos = strrchr(szLine, '\"') - szLine;

		strncpy(temp, szLine + nStartPos, nEndPos - nStartPos);
		temp[nEndPos - nStartPos] = '\0';

		newServerAddress.port = atoi(temp);
		if ( newServerAddress.port ==  0 )
			throw  "Error in reading log server port number, code 09";
	}
	else if (strstr(szLine, "ServerAddr") != NULL )
	{
		if ( bPortFirst == false )
			throw "Error in reading log server ip address, code 10";
		nStartPos = strcspn(szLine, numbers);

		if ( nStartPos >= strlen(szLine) )
			throw "Error in reading log server ip address, code 11";

		nEndPos = strrchr(szLine, '\"') - szLine;

		strncpy(temp, szLine + nStartPos, nEndPos - nStartPos);
		temp[nEndPos - nStartPos] = '\0';

		newServerAddress.ip_address = temp;
	}
	else throw "Error in reading log server information from file, code 12";

	m_UpdateServerList.push_back(newServerAddress);
}

void LogServerConfig::ReadGeneralInfo(FILE* file)
{
	int backetCharPos[4];
	char szLine[MAX_PATH];
	char temp[2][MAX_PATH];
	char numbers[] = "1234567890";
	int i, j;
	int nStrLen;

	while ( fgets(szLine, MAX_PATH, file) != NULL )
	{
		TrimLeftSpace(szLine);
		TrimRightSpace(szLine);
		if ( strcmp(szLine, "[Logging Server]" ) == 0 )
		{
			ReadLogServerInfo(file);
			continue;
		}

		nStrLen = strlen(szLine);
		backetCharPos[0] = strchr(szLine, '\"') - szLine;

		// Read other 3 '"' characters in a line
		for ( j = 1; j < 4; j++ )
		{
			for ( i = backetCharPos[j - 1] + 1; i< nStrLen; i++ )
				if ( szLine[i] == '\"' )
					break;
			if ( i >= nStrLen )
				throw "Error in reading configuration data";

			else backetCharPos[j] = i;
		}
		
		int nLen;
		for ( i = 0; i < 2; i++ )
		{
			nLen = backetCharPos[i * 2 + 1] - backetCharPos[i * 2] - 1;
			strncpy(temp[i], szLine + backetCharPos[i * 2] + 1, nLen );
			temp[i][nLen] = '\0';
		}

		string configEntry = temp[0];

		// test if the data contain numbers only
		if ( strspn(temp[1], numbers) == strlen(temp[1]) )
		{
			int configData = atoi(temp[1]);
			m_IntConfigMap[configEntry] = configData;
		}
		else
		{
			string configData = temp[1];
			m_StrConfigMap[configEntry] = configData;
		}
	}
}

void LogServerConfig::TrimLeftSpace(char* szInput)
{
	int nStrLen = strlen(szInput);
	int i, j;
	for ( i = 0; i < nStrLen; i++ )
		if (!( szInput[i] == ' ' || szInput[i] == '\n' ))
			break;

	if ( i > 0 && i < nStrLen )
	{
		for ( j = i; j < nStrLen; j++ )
			szInput[ j - i ] = szInput[j];

		szInput[ j - i ] = '\0';
	}
	else if ( i >= nStrLen )
		szInput[0] = '\0';

}

void LogServerConfig::TrimRightSpace(char* szInput)
{
	int nStrLen = strlen(szInput);
	int i, j;
	for ( i = nStrLen - 1 ; i >= 0; i-- )
		if (!( szInput[i] == ' ' || szInput[i] == '\n' ))
			break;

	if ( i ==0 && ( szInput[0] == ' ' || szInput[0] == '\n') )
		szInput[0] = '\0'; 
	else szInput[ i + 1 ] = '\0';
	
}