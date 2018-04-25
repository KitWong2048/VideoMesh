#include "../Interface/VMeshServer.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

VMeshServer::VMeshServer (string _myIP, int _myPort, int _localStorageCapacity)
: myVMesh(_myIP, _myPort, _localStorageCapacity, SERVER_IDENTITY,0)
{
	temp_remoteIP = "";
	temp_remotePort = 0;
}

VMeshServer::VMeshServer(string _myIP, int _myPort, string _remoteIP, int _remotePort)
: myVMesh(_myIP, _myPort)
{
	temp_remoteIP = _remoteIP;
	temp_remotePort = _remotePort;
}

VMeshServer::~VMeshServer()
{
	return;
}
	
bool VMeshServer::start()
{
	return true;
}

void VMeshServer::stop()
{
	myVMesh.stop();
}

bool VMeshServer::moviePublish(const char* _key, string _filename)
{
	if (!myVMesh.publish(_filename))
		return false;
	if (myVMesh.start())
		return true;
	else
		return false;
}

bool VMeshServer::movieUnPublish(const char* _key)
{
	return true;
}

MediaInfo* VMeshServer::getMediaInfo(const char* _key)
{
	return myVMesh.getMediaInfo();
}

_VMESH_NAMESPACE_FOOTER
