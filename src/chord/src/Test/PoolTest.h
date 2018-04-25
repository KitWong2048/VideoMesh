#ifndef _H_POOLTEST_
#define _H_POOLTEST_

#include "chord/src/DHTSocket/PoolSocketMgr.h"
#include "chord/src/DHTSocket/VirtualPoolSocket.h"
#include "chord/src/DHTSocket/SocketServer.h"
#include "chord/src/DHTSocket/ServerHandle.h"
#include "chord/src/Util/Thread.h"
#include <iostream>
using namespace std;
using namespace DHTSocket;
using namespace Util;

class PoolTest : public ServerHandle{
private:
	PoolSocketMgr* mgr;

	class ServerThread : public Thread{
		private:
			VirtualPoolSocket* socket;

		protected:
			void* execute();

		public:
			ServerThread(VirtualPoolSocket* _socket);
	};

	ServerThread* threads[10];

public:
	PoolTest(const char* _localIP, int _localPort);

	virtual ~PoolTest();

	void* execute();

	void poolSeverTest(int argv);

	void poolClientTest(int argv);

	int response(VirtualPoolSocket* _socket);
};

#endif
