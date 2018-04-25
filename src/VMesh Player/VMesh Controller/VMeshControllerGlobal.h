#ifndef VMESH_CONTROLLER_GLOAL_H
#define VMESH_CONTROLLER_GLOAL_H
//--------------------------------------------------------------
// global vars
//--------------------------------------------------------------
char wwwroot[MAX_PATH];
char hostname[MAX_PATH];
CRITICAL_SECTION output_criticalsection;
CRITICAL_SECTION thread_criticalsection;
CRITICAL_SECTION vmclient_criticalsection;
int command;
int	port;
int thread_count;
string hash;
string server;
bool is_joined;
bool firstTime;
int packetSize;
int listen_port;
VMeshClient* vmClient;
HANDLE m_Thread;
HANDLE hCommand;
HANDLE hReady;
HANDLE hFailed;
HANDLE hNoPacket;
HANDLE hPaused;
HANDLE hReleaseMutex;
HANDLE hUpdateLogServerInfoThread;
unsigned threadHandle;
unsigned threadLogger;
int is_ControllerRun; //Add by Juan
#endif