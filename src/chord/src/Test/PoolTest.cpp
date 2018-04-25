/*
PoolTest.cpp

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

#include "chord/src/Test/PoolTest.h"

#define TEST_PORT 21222

PoolTest::PoolTest(const char* _localIP, int _localPort) : ServerHandle(){
//	PoolSocketMgr::employ(3, 3, _localIP, _localPort);
	mgr = PoolSocketMgr::invokePoolSocketMgr();
	for (int i = 0; i < 10; i++)
		threads[i] = NULL;
}

PoolTest::~PoolTest(){
//	PoolSocketMgr::dismiss();
}

void PoolTest::poolSeverTest(int argv){
	SocketClient* clt;
	SocketServer* srv;
//	int index;

	srv = new SocketServer("127.0.0.1", argv);

	if (!srv->ready()){
		cout<<"Fail to listen on port "<<argv<<endl;
		return;
	}else cout<<"Listening on port "<<argv<<endl;

	while (true){
		clt = srv->accept(10000);

		if (clt != NULL){
			cout<<"A client is accepted"<<endl;
			mgr->delegate(*clt, *this);
			delete clt;
		}else{
			if (argv == 21222)
				poolClientTest(21223);
			else if (argv == 21223)
				poolClientTest(21222);
		}
	}

}

void PoolTest::poolClientTest(int argv){
	int counter = 0;
	int status;
	char* buf;
	VirtualPoolSocket* socket;

	socket = mgr->connect("127.0.0.1", argv, 1000);
	sleep(3);
	cout<<endl<<endl<<endl;

	if (socket == NULL){
		cout<<"Fail to connect to the server"<<endl;
	}else{
		cout<<"Connected to the server"<<endl;
		while (counter <= 500){
			buf = new char[1024];
			sprintf(buf, "Message %04d", counter);
			usleep(1000);
			status = socket->send(buf, strlen(buf) + 1);
			cout<<"client send("<<buf<<") Status = "<<status<<endl;
			delete []buf;
			if (status<= 0)
				break;
			else{
				status = socket->receive(buf, 5000);
				cout<<"client recv() Status = "<<status;
				if (buf != NULL){
					cout<<" Received: "<<buf<<endl;
					delete []buf;
				}
				if (status<= 0){
					cout<<endl;
					break;
				}

			}

			counter++;
		}
		socket->close();
		delete socket;
		cout<<"leaving"<<endl;
	}

}

int PoolTest::response(VirtualPoolSocket* _socket){
	int index = -1;
	for (int i = 0; i < 10; i++){
		if (threads[i] == NULL){
			index = i;
			break;
		}else if ((!threads[i]->getRunningStatus()) && (threads[i]->getJoinableStatus())){
			threads[i]->join(NULL);
			delete threads[i];
			threads[i] = NULL;
			index = i;
			break;
		}
	}
	if (index >= 0){
		threads[index] = new ServerThread(_socket);
		threads[index]->run();
	}

	return 0;
}


PoolTest::ServerThread::ServerThread(VirtualPoolSocket* _socket) : Thread(){
	socket = _socket;
}

void* PoolTest::ServerThread::execute(){
	char* buf;
	int status;

	cout<<"start running response"<<endl;

	while (true){
		status = socket->receive(buf, 2000);
		cout<<"server recv() Status = "<<status;
		if (status<= 0){
			cout<<endl;
			break;
		}else if (buf != NULL){
			cout<<" Received: "<<buf<<endl;
			status = socket->send(buf, status);
			cout<<"server send("<<buf<<") Status = "<<status<<endl;
			if (status<= 0)
				break;
			delete []buf;
		}
		else cout<<" Received: (NULL)"<<endl;
	}
	socket->close();
	delete socket;
	socket = NULL;

	return NULL;
}
