/*
FarTimeShiftTest.cpp

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

#include "chord/src/Test/FarTimeShfitTest.h"

using namespace Util;
using namespace App;

#ifndef DHT_WIN32
string now(){
	time_t timer;
	struct tm* currentTime;
	char buf[32];

	timer = time(0);
	bzero(buf, 32);
	//currentTime = localtime(&timer);
	currentTime = gmtime(&timer);
	if (currentTime != NULL)
		strftime(buf, 32, "%Y-%m-%d %H:%M:%S", currentTime);

	return string(buf);
}

string getHostIP(){
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

}
#endif

namespace Test{
	int FarTimeShiftTest::doTest(int argc, char* argv[]){
		FarTimeShift* fts;
		InetAddress DHTAddr, bootStrapAddr;
		InetAddress serverAddr;

		set<FarTimeShiftID> addedID;	

		srand(time(0));

		/*********************************************************************************/
		// init FTS
		/*********************************************************************************/
		fts = new FarTimeShift();
		#ifndef DHT_WIN32
			DHTAddr.setIP(getHostIP().c_str());
		#else
			DHTAddr.setIP(TARGET_HOST);
		#endif
		DHTAddr.setPort(BOOTSTRAP_PORT + 1 + rand() % 100);
		// for bootstrap
		if (argc > 1)
			if (!strcmp(argv[1], "0"))
				DHTAddr.setPort(BOOTSTRAP_PORT);

		cout<<"Using: "<<DHTAddr.getIP()<<":"<<DHTAddr.getPort()<<endl;
		fts->init(DHTAddr);
		bootStrapAddr.setIP(BOOTSTRAP_IP);
		bootStrapAddr.setPort(BOOTSTRAP_PORT);

		/*********************************************************************************/
		// add node
		/*********************************************************************************/
		char ID[FTS_ID_LENGTH + 1];
		serverAddr.setIP("127.0.0.1");
		if (argc > 1){
			for (int i = 0; i < 4; i++){
				ID[i] = 171; // "AB" in hex
			}
			if (!strcmp(argv[1], "0")){
				 // add the bootstrap
				 serverAddr.setPort(37890);
				fts->add(ID, serverAddr, NULL);
			}else{
				 // add a node, ID =  "ABABABABxxxxxxxxxxxx" with bootstrap
				 serverAddr.setPort(37891 + rand() % 100);
				fts->add(ID, serverAddr, &bootStrapAddr);
			}
		}else{
			// add a node, ID !=  "ABABABABxxxxxxxxxxxx" with bootstrap
			for (int i = 0; i < 4; i++){
				ID[i] = rand() % 255 + 1;
			}
			serverAddr.setPort(37891 + rand() % 100);
			fts->add(ID, serverAddr, &bootStrapAddr);
		}
		addedID.insert(ID);

		/*********************************************************************************/
		// query
		/*********************************************************************************/
		char queryID[FTS_ID_LENGTH];
		int round = 0;
		if (!fts->isDHTRunning()){
			cout<<"sleep for a while to wait DHT"<<endl;
			sleep(10);
		}
			
	//	int decision;
		while ((round < 50) && (fts->isDHTRunning())){
			sleep(15);
			fts->print();
			// calculate a random query ID
			round++;
			if (round % 5 == 0)
				fts->clean();

			//	set<FarTimeShiftID>::iterator l;
			//decision = rand() % 12;
			//if (decision > 9){
			//	printf("adding\n");
			//	for (int i = 0; i < 4; i++){
			//		ID[i] = rand() % 255 + 1;
			//	}
			//	fts->add(ID, serverAddr);
			//	addedID.insert(ID);
			//}else if (decision > 7){
			//	if (addedID.size() > 1){
			//		printf("removing\n");
			//		l = addedID.begin();
			//		fts->remove(l->ID);
			//		addedID.erase(l);
			//	}
			//}else{
				printf("searching ");

				if (rand() % 2 == 0){
					for (int i = 0; i < FTS_ID_LENGTH; i++){
						queryID[i] = rand() % 255 + 1;
						printf("%02x", (unsigned char) queryID[i]);
					}
				}else{
					for (int i = 0; i < 4; i++){
						queryID[i] = 171; // "AB" in hex
						printf("%02x", (unsigned char) queryID[i]);
					}
				}
				printf("\n");
				fts->asyncShift(queryID);
			}

	//	}

		// exit
		cout<<"clean up"<<endl;
		cout<<now()<<endl;
		
		fts->destroy();
		delete fts;

		cout<<"main thread returns"<<endl;
		cout<<now()<<endl;

		return 0;
	}
}
