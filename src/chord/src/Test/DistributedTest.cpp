/*
DistributedTest.cpp

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

#include "chord/src/Test/DistributedTest.h"


bool isSmallTest = true;

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

void DistributedTest::repeatTests(int bootStrap, bool isPlanetTest){
	int successHit, totalHit;
	int accTime, sleepingTime, runningTime, totalTime;
	int testCount = 0;

	if (isSmallTest){
		totalTime = 12 * 60;
	}else{
		// sleep for a while to begin, to avoid huge nubmer of nodes simultaneous arrival
		if (bootStrap != 0)
			sleep(rand() % (5 * 60));
		totalTime = 12 * 3600;
	}

	accTime = 0;
	while (accTime < totalTime){
		if (isSmallTest)
			runningTime = 8 * 60;
		else
			runningTime = rand() % 5000 + 900;

		testCount++;
		cout<<"Test "<<testCount<<" started at "<<now()<<endl;

		doTest(bootStrap, runningTime, successHit, totalHit);

		cout<<"Test "<<testCount<<" finished at "<<now()<<endl;
		cout<<"Performance of the test = "<<successHit<<"/"<<totalHit<<endl;

		accTime += runningTime;
		// sleep for a while before continue next test
		sleepingTime = rand() % 60 + 60;
		sleep(sleepingTime);
		accTime += sleepingTime;
		cout<<"There are at least "<<totalTime - accTime<<" seconds left."<<endl;
	}
}

// if bootStrap = 0, the bootstrap node will be added to Chord; otherwise the bootstrap node will be used to help add the new node
int DistributedTest::doTest(int bootStrap, int runningTime, int& successHit, int& totalHit){
	ChordApp* app;
	ChordAttr attr;
	int hopCount;
	Node* searchPath = NULL;
	char binAppValue[APP_VALUE_LEN];

	// for adding node
	DHTNetworkID ID;
	char hashKey[65];
	char* hashValue;
	Node bootStrapNode;

	// for finding random value
	Node* resultNode = NULL;
	DHTNetworkID dummyID;

	// 	time counter for testing purpose
	int accTime = 0;
	int sleepingTime = 0;
	int printTime = 0;

	successHit = 0;
	totalHit = 0;


	// default bootstrap node, which is assumed listening at 143.89.145.91:21222
	bootStrapNode.setNodeIP("143.89.145.91");
	bootStrapNode.setNodePort(21222);

	// prepare attributes for chord, see ChordApp.h for ChordAttr description
	attr.listeningIP = getHostIP();
	attr.listeningPort = bootStrapNode.getNodePort() + (bootStrap == 0 ? 0 : (rand() % 10000) + 1);
	attr.connectionTimeOut = DHT_CONNECTION_TIMEOUT;
	attr.connectionNum = MAX_DHT_CONNECTION;
	attr.stabilizingInterval = STABILIZING_INTERVAL;
	attr.fixingInterval = FIXING_INTERVAL;
	attr.neighborVisitInterval = NEIGHBOR_VISIT_INTERVAL;
	attr.maxSearchTime = 60;
	attr.neighborhoodSize = DEFAULT_NEIGHBOR_SIZE;
	// for logging level, LogNormal is for normal use, LogDebug prints out routing information. They can be combined together by LogNormal | LogDebug
	attr.logLevel = LogNormal | LogError;
	attr.logPath = "";
	//attr.logPath = "DHTLog.txt";


	// application value for the associated node, in this example we use the address
	bzero(binAppValue, APP_VALUE_LEN);
	for (int i = 0; i < APP_VALUE_LEN; i++){
		binAppValue[i] = rand() % 256;
		// printf("binAppValue[%d] = %03hhu\n", i, binAppValue[i]);
	}

	// calculate the new node ID value
	bzero(hashKey, sizeof(hashKey));
	sprintf(hashKey, "%s:%d", attr.listeningIP.c_str(), attr.listeningPort);
	hashValue = sha1HashHex(hashKey);
	ID.setIDHex(hashValue);
	delete []hashValue;

	bootStrapNode.setNodeID(ID);

	// make a new chord instance
	PoolSocketMgr::employ(25, 25, attr.listeningIP.c_str(), attr.listeningPort);
	app = new ChordApp(attr);
	if (app->run() < 0){
		cout<<"Failed to new a DHT instance."<<endl;
		delete app;
		return -1;
	}

	// wait for the chord to start
	sleepingTime = 0;
	while ((sleepingTime < 60) && (!app->isDHTRunning())){
		sleep(5);
		sleepingTime += 5;
	}
	if (!app->isDHTRunning()){
		cout<<"Failed to start a DHT."<<endl;
		delete app;
		return -1;
	}

	cout<<"Start running DHT."<<endl;
	if (bootStrap == 0)
		app->addNode(ID, binAppValue);
	else
		app->addNode(ID, binAppValue, bootStrapNode);

	struct timeval x, y, z;
	// sleep random amount of time and then make a search for a random ID
	app->print(LogNormal);
	srand((unsigned)time(0));
	while (true){
		// sleep between 0 to 60 before making a request
		sleepingTime = rand() % 30 + 1;
		cout<<"Sleep "<<sleepingTime<<"s"<<endl;
		sleep(sleepingTime);
		bzero(hashKey, sizeof(hashKey));
		sprintf(hashKey, "%d", rand());
		hashValue = sha1HashHex(hashKey);
		dummyID.setIDHex(hashValue);
		delete []hashValue;

		bzero(&x, sizeof(x));
		gettimeofday(&x, NULL);
		//resultNode = app->lookup(dummyID, hopCount);
		resultNode = app->lookupFullPath(dummyID, searchPath, hopCount);
		bzero(&y, sizeof(y));
		gettimeofday(&y, NULL);
		if (y.tv_usec < x.tv_usec){
			z.tv_usec = 1000000 + y.tv_usec - x.tv_usec;
			z.tv_sec = y.tv_sec - x.tv_sec - 1;
		}else{
			z.tv_usec = y.tv_usec - x.tv_usec;
			z.tv_sec = y.tv_sec - x.tv_sec;
		}
		totalHit++;
		// async action, use either one set action before performAsyncAction
//		AsyncHandle* async;
//		async = new AsyncHandle(app);
//		async->setLookupArg(dummyID);
//		async->setGetFingerArg(Node(dummyID, "127.0.0.1", 12345));
//		async->setGetNeighborArg(Node(dummyID, "127.0.0.1", 12345));
//		async->setAddNodeArg(dummyID, binAppValue);
//		async->setRemoveNodeArg(dummyID);
//		app->performAsyncAction(async);

		cout<<dummyID<<"   =>   "<<endl;
		if (searchPath != NULL){
			if (hopCount >= 0)
				for (int i = 0; i <= hopCount; i++)
					cout<<"	"<<(DHTNetworkID) searchPath[i]<<"   =>   "<<endl;
			delete []searchPath;
		}
		if (resultNode != NULL){
			cout<<"	"<<*resultNode<<endl;
			delete resultNode;
			resultNode = NULL;
			successHit++;
		}else{
			 cout<<"	(NULL)"<<endl;
			 hopCount = -1;
		}

		cout<<"Hop(s) = "<<hopCount<<endl;
		cout<<"Latency = "<<z.tv_sec<<"."<<z.tv_usec<<endl;

		printTime += sleepingTime;

		if (printTime >= 300){
			app->print(LogNormal);
			printTime = 0;
		}

		accTime += sleepingTime;
		if ((accTime >= runningTime) && (bootStrap != 0)){
			cout<<"Test is ceased to execute."<<endl;
			break;
		}
	}

	app->stop();
	cout<<"Stopping..."<<endl;
	delete app;
	cout<<"Done!"<<endl;

	PoolSocketMgr::dismiss();

	return 0;
}
