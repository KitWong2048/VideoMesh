#include "../Util/Utility.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

int packIntIntoChar(const int val, char* data){
	int i;
	
	for (i=0; i<sizeof(int); i++){
		data[i] = char((val >> 8*i) & 0xff);
	}
	
	return i;
}

int unpackIntFromChar(int& val, const char* data){
	int i;
	val &= 0;
		
	for (i=0; i<sizeof(int); i++){
		val |= ((int(data[i]) & 0xff) << 8*i);	
	}
	
	return i;
}

int packUIntIntoChar(const unsigned int val, char* data){
	int i;
	
	for (i=0; i<sizeof(unsigned int); i++){
		data[i] = char((val >> 8*i) & 0xff);
	}
	
	return i;
}

int unpackUIntFromChar(unsigned int& val, const char* data){
	int i;
	val &= 0;
		
	for (i=0; i<sizeof(unsigned int); i++){
		val |= ((int(data[i]) & 0xff) << 8*i);	
	}
	
	return i;
}

string itos(int val)
{
	char a[20];
	sprintf(a, "%d", val);
	return string(a);
}


string getHostIP(){
	#ifdef _VMESH_WIN32_
		WORD wVersionRequested;
		WSADATA wsaData;
		char name[255];
		PHOSTENT hostinfo;
		wVersionRequested = MAKEWORD( 1, 1 );
		char *ip;
		string strIP;

		if ( WSAStartup( wVersionRequested, &wsaData ) == 0 )
			if( gethostname ( name, sizeof(name)) == 0)
			{
				//printf("Host name: %s\n", name);

				if((hostinfo = gethostbyname(name)) != NULL)
				{
					int nCount = 0;
					while(hostinfo->h_addr_list[nCount])
					{
						ip = inet_ntoa(*(
						struct in_addr *)hostinfo->h_addr_list[nCount]);

						++nCount;
						strIP = ip;
						if (strIP.substr(0,7)!="192.168")
							return strIP;
					}
				}
			}
		return strIP;
	#else
		struct ifaddrs *ifa = NULL;
		string localIP;

		if (getifaddrs (&ifa) < 0){
		perror ("getifaddrs");
		return "";
		}

		for (; ifa; ifa = ifa->ifa_next){
			char ip[ 200 ];
			socklen_t salen;

			if (ifa->ifa_addr->sa_family == AF_INET)
				salen = sizeof (struct sockaddr_in);
			else	continue;

			if (getnameinfo (ifa->ifa_addr, salen, ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0)
				continue;

			if (strstr(ifa->ifa_name, "eth") != NULL){
				localIP = ip;
				break;
			}
		}
		for (; ifa; ifa = ifa->ifa_next);
		freeifaddrs(ifa);
		return localIP;
	#endif
}

_VMESH_NAMESPACE_FOOTER
