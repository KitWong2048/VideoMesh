/*
Chord.cpp

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

#include "../DHT/Chord.h"
#include "../DHT/Stabilizer.h"

using namespace DHTSocket;
using namespace Util;
using namespace App;

namespace DHT{

	Chord::Chord(ChordApp* _app, const ChordAttr& _attr) : Thread(){
		app = _app;
		attr = _attr;
		isDHTRunning = false;

		// init logging
		log = new Logger();
		if (attr.logLevel < 0)
			attr.logLevel = LogCommon;
		if (log->openLogStream(attr.logPath.c_str(), attr.logLevel) != 0){
			cerr<<"Failed to open log "<<attr.logPath<<"."<<endl;
			cerr<<"Logging is directed to standard output."<<endl;
		}

		// make correction to the attribute
		log->writeLog(attr.checkAndCorrect(), LogCommon);

		// init threads pool
		if (attr.isMultiThreaded){
			srvThreadPool = new DHTService*[attr.connectionNum];
			for (int i = 0; i < attr.connectionNum; i++){
				srvThreadPool[i] = NULL;
			}
		}else srvThreadPool = NULL;

		chordStabilizer = NULL;

		localNodes = new LocalNodeCollection(*this);

		socketPoolMgr = NULL;
	}

	Chord::~Chord(){
		stop();

		// stop the stabilizer thread
		if (chordStabilizer != NULL){
			if (chordStabilizer->getJoinableStatus()){
				log->writeLog("Chord: Collecting stabilizer.\n", LogDebug);
				chordStabilizer->join();
				log->writeLog("Chord: Done collecting stabilizer.\n", LogDebug);
			}
			delete chordStabilizer;
		}

		if (srvThreadPool != NULL){
			// stop and deallocate all server threads
			log->writeLog("Chord: Collecting all server threads.\n", LogDebug);
			for (int i = 0; i < attr.connectionNum; i++){
				if (srvThreadPool[i] != NULL){
					if (srvThreadPool[i]->getJoinableStatus())
						srvThreadPool[i]->join();
					delete srvThreadPool[i];
				}
			}
			log->writeLog("Chord: Done collecting all server threads.\n", LogDebug);

			delete []srvThreadPool;
		}

		delete localNodes;

		log->closeLogStream();
		delete log;
	}

	int Chord::addNode(const DHTNetworkID& _id, const char* _appValue){
		int status = 0;
		bool isAdded = false;
		LocalNodeHandle* localBootstrap, *newLocalNode;
		Node node(_id, attr.listeningIP.c_str(), attr.listeningPort), *succ;

		node.fromBinApp(_appValue);
		// wait until the Chord has been running in the background thread
		// in WIN32 version, we use busy sleeping to wait for the condition;
		// in Linux, we use conition variable to wait
		#ifdef DHT_WIN32
			unsigned int sleptTime = 0;
			while ((!isDHTRunning) && (sleptTime < attr.connectionTimeOut)){
				sleep(5);
				sleptTime += 5;
			}
		#else
			this->runningDHTLock.lock();
			while (!isDHTRunning){
				log->writeLog("Waiting for the Chord object to go up...", LogDebug);
				if (this->runningDHTCondition.timedWait(this->runningDHTLock, attr.connectionTimeOut) < 0){
					// time out, exit with fail status
					log->writeLog("The Chord object is not running.\n", LogDebug);
				}else log->writeLog("The Chord object has boardcast a signal.\n", LogDebug);
			}
			this->runningDHTLock.release();
		#endif
		if (!isDHTRunning)
			return -1;

		// try to find the successor of the _id
		succ = this->lookup(_id, NULL, LogDebug);
		if (succ != NULL){
			if (*succ != _id){
				// if the successor can be found, ask it to help join
				status = this->addNode(_id, _appValue, *succ);
				isAdded = true;
			}else{
				// if the successor in DHT has the same ID as the new node, conflict arises
				status = -1;
			}
			delete succ;
		}

		// in case no successor can help
		if ((!isAdded) && (status != -1)){
			// check whether the new node exists in the collection
			newLocalNode = localNodes->getLocalNode(_id);
			if (newLocalNode == NULL)
				newLocalNode = new LocalNodeHandle(node, *this);
			else newLocalNode->setNode(node);
			// find an existing local node to copy
			localBootstrap = localNodes->getSucceedingLocalNode(_id);
			if (localBootstrap != NULL){
				newLocalNode->copyReference(*localBootstrap);
				newLocalNode->addReference(localBootstrap->getNode());
				localBootstrap->subtractUseCount();
			}

			status = localNodes->addNode(*newLocalNode);
			delete newLocalNode;
			if (status == 0)
				log->writeLog("The node " + _id.getIDHex() + " has been added.\n", LogNormal);
			else
				log->writeLog("Failed to add the node " + _id.getIDHex() + ".\n", LogNormal);
		}

		return status;
	}

	int Chord::addNode(const DHTNetworkID& _id, const char* _appValue, const Node& _bootstrap){
		int status = 0;
		LocalNodeHandle *newLocalNode;
		Node node(_id, attr.listeningIP.c_str(), attr.listeningPort);

		node.fromBinApp(_appValue);
		// check whether the new node exists in the collection
		newLocalNode = localNodes->getLocalNode(_id);
		if (newLocalNode == NULL)
			newLocalNode = new LocalNodeHandle(node, *this);
		else newLocalNode->setNode(node);

		status = newLocalNode->join(_bootstrap);

		if (status >= 0){
			// add to the collection of local nodes
			localNodes->addNode(*newLocalNode);
			log->writeLog("The node " + _id.getIDHex() + " has been added.\n", LogNormal);
		}else
			log->writeLog("Failed to add the node " + _id.getIDHex() + ".\n", LogNormal);
		delete newLocalNode;

		return status;
	}

	int Chord::removeNode(const DHTNetworkID& _id){
		int status = 0;

		status = localNodes->removeNode(_id);
		if (status == 0)
			log->writeLog("The node " + _id.getIDHex() + " has been removed.\n", LogNormal);
		else
			log->writeLog("Failed to remove the node " + _id.getIDHex() + ".\n", LogNormal);

		return status;
	}

	Node* Chord::lookup(const DHTNetworkID& _id, int* _hopCount, LogLevel _logLevel)
	{
		LocalNodeHandle* l;
		Node* n = NULL;

		l = this->localNodes->getPrecedingLocalNode(_id);
		if (l != NULL)
		{
			//	 if the target ID is equal local node id, return the node
			if (_id == l->getNode())
			{
				n = new Node(l->getNode());
				_hopCount = 0;
			}
			else	
				n = l->findSuccessor(_id, _hopCount, _logLevel);
			
			l->subtractUseCount();
		}

		return n;
	}

	Node* Chord::lookupFullPath(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, LogLevel _logLevel)
	{
		LocalNodeHandle* l;
		Node* n = NULL;

		l = this->localNodes->getPrecedingLocalNode(_id);
		if (l != NULL)
		{
			n = l->findID(_id, _searchPath, _hopCount, _logLevel);
			l->subtractUseCount();
		}
		return n;
	}

	Node* Chord::recursiveLookup(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, LogLevel _logLevel)
	{
		LocalNodeHandle* l;
		Node* n = NULL;

		l = this->localNodes->getPrecedingLocalNode(_id);
		if (l != NULL)
		{
			n = l->findID(_id, _searchPath, _hopCount, _logLevel);
			l->subtractUseCount();
		}
		return n;
	}

	LocalNodeHandle* Chord::getLocalNode(const DHTNetworkID& _id){
		return this->localNodes->getLocalNode(_id);
	}

	LocalNodeHandle* Chord::getPrecedingLocalNode(const DHTNetworkID& _id){
		return this->localNodes->getPrecedingLocalNode(_id);
	}

	LocalNodeHandle* Chord::getSucceedingLocalNode(const DHTNetworkID& _id){
		return this->localNodes->getSucceedingLocalNode(_id);
	}

	void* Chord::execute(){
		return doExecuteDHT();
	}

	void* Chord::doExecuteDHT(){
		SocketServer* listeningSocket = NULL;

		//listeningSocket = new SocketServer(attr.listeningIP.c_str(), attr.listeningPort);
		listeningSocket = new SocketServer(0, attr.listeningPort);
		if (!listeningSocket->ready()){
			log->writeLog("Chord: Failed to listen.\n", LogError);
			listeningSocket->close();
			delete listeningSocket;
			return NULL;
		}else log->writeLog("Chord: Chord DHT network service has started.\n", LogNormal);

		socketPoolMgr = PoolSocketMgr::invokePoolSocketMgr();
		if (socketPoolMgr == NULL){
			log->writeLog("Chord: Failed to invoke socket pool manager.\n", LogError);
			listeningSocket->close();
			delete listeningSocket;
			return NULL;
		}else log->writeLog("Chord: Pool manager is employed.\n", LogNormal);

		// start the stabilizer which periodically kicks in to refresh
		if (chordStabilizer == NULL){
			chordStabilizer = new Stabilizer(*this, attr.fixingInterval, attr.stabilizingInterval, attr.neighborVisitInterval);
			chordStabilizer->run();
		}

		//listen to incoming remote requests
		isDHTRunning = true;
		#ifndef DHT_WIN32
			runningDHTLock.lock();
			runningDHTCondition.boardcast();
			runningDHTLock.release();
		#endif

		if (attr.isMultiThreaded){
			while (isDHTRunning)
				response(listeningSocket);
			listeningSocket->close();
			delete listeningSocket;
			log->writeLog("Chord: Stop executing...\n", LogNormal);
		}else{
			socketPoolMgr->delegate(*listeningSocket, *this);
		}

		return NULL;
	}

	void Chord::print(LogLevel _logLevel){
		stringstream ss;
		ss<<"Listening IP: "<<attr.listeningIP<<endl;
		ss<<"Listening port: "<<attr.listeningPort<<endl;
		ss<<"Size of connection pool: "<<attr.connectionNum<<endl;
		ss<<"Connection time out: "<<attr.connectionTimeOut<<endl;
		ss<<"Use worker threads: "<<(attr.isMultiThreaded ? "YES" : "NO")<<endl;
		ss<<"Stabilizing interval: "<<attr.stabilizingInterval<<endl;
		ss<<"Fixing interval: "<<attr.fixingInterval<<endl;
		ss<<"Log file: "<<(attr.logPath == "" ? "STD OUT" : attr.logPath)<<endl;
		log->writeLog(ss.str(), _logLevel);
		localNodes->print(_logLevel);
	}

	void Chord::addReference(const DHTNetworkID& _localNodeID, const Node& _finger){
		localNodes->addReference(_localNodeID, _finger);
	}

	void Chord::removeReference(const DHTNetworkID& _localNodeID, const DHTNetworkID& _fingerID){
		localNodes->removeReference(_localNodeID, _fingerID);
	}

	void Chord::stop(){
		if (isDHTRunning){
			isDHTRunning = false;
			
			// unregister itself from the socket pool
			if (socketPoolMgr != NULL){
				log->writeLog("Chord: Undelegating from socket manager.\n", LogDebug);
				socketPoolMgr->undelegate(this);
				socketPoolMgr = NULL;
			}
		}
	}

	unsigned int Chord::getTimeOutValue(){
		return attr.connectionTimeOut;
	}

	Logger* Chord::getLogger(){
		return this->log;
	}

	PoolSocketMgr* Chord::getSocketPoolMgr(){
		return socketPoolMgr;
	}

	bool Chord::isMultiThreaded(){
		return attr.isMultiThreaded;
	}

	Node** Chord::getFingersOrNeighbor(const Node& _node, bool _isFingers){
		Node **allNodes, aNode;
		LocalNodeHandle* local;
		DHTRequest* req;

		allNodes = NULL;
		local = localNodes->getLocalNode(_node);
		if (local != NULL){
			if (_isFingers)
				allNodes = local->returnAllFingers();
			else allNodes = local->returnAllNeighbor();
			local->subtractUseCount();
		}else{
			local = this->localNodes->getSucceedingLocalNode(_node);
			if (local != NULL){
				aNode = local->getNode();
				if (_isFingers)
					req = new DHTRequest(DHTReqAllFingers, *this, &aNode, &_node);
				else req = new DHTRequest(DHTReqAllNeighbor, *this, &aNode, &_node);
				allNodes = (Node**) req->makeRequest();
				delete req;
				local->subtractUseCount();
			}
		}

		return allNodes;
	}

	bool Chord::getDHTRunningStatus(){
		return isDHTRunning;
	}

	double Chord::getMaxSearchTime(){
		return attr.maxSearchTime;
	}

	unsigned int Chord::getNeighborhoodSize(){
		return attr.neighborhoodSize;
	}

	int Chord::response(VirtualPoolSocket*  _socket)
	{
		int threadNo = -1;
		DHTService* srvHandle;

		log->writeLog("Chord: Running response.\n", LogDeepDebug);

		if ((_socket != NULL) && (isDHTRunning)){
			// a new client
//			log->writeLog("Chord: Accepting and dispatching a request.\n", LogDeepDebug);

			if (attr.isMultiThreaded){
				// find resources to dispatch the request
				srvThreadLock.lock();
				for (int i = 0; i < attr.connectionNum; i++){
					if (srvThreadPool[i] == NULL){
						threadNo = i;
						break;
					}else if ((!srvThreadPool[i]->getRunningStatus()) && (srvThreadPool[i]->getJoinableStatus())){
						// the thread has finished, join it
						srvThreadPool[i]->join(NULL);
						delete srvThreadPool[i];
						srvThreadPool[i] = NULL;
						threadNo = i;
						break;
					}
				}

				// dispatch the request
				if (threadNo >= 0){
					srvThreadPool[threadNo] = new DHTService(*this, *_socket);
					srvThreadPool[threadNo]->run();
				}else{
					_socket->close();
					delete _socket;
				}
				srvThreadLock.release();
			}else{
				srvHandle = new DHTService(*this, *_socket);
				srvHandle->serveRequest();
				delete srvHandle;
			}
		}

		return 0;
	}

	int Chord::response(SocketServer* _socket){
		SocketClient* incomingSocket;
		// accept an incoming remote request
		log->writeLog("Chord: Waiting for remote request.\n", LogDeepDebug);
		incomingSocket = _socket->accept(attr.connectionTimeOut);

		if ((incomingSocket != NULL) && (isDHTRunning)){
			// a connection is made
			log->writeLog("Chord: A peer is accepted.\n", LogDeepDebug);
			socketPoolMgr->delegate(*incomingSocket, *this);
			delete incomingSocket;
			incomingSocket = NULL;
		}else{
			// time out
			if (attr.isMultiThreaded){
				log->writeLog("Chord: Cleaning up server threads.\n", LogDeepDebug);
				// find an available thread to handle the incoming request
				srvThreadLock.lock();
				for (int i = 0; i < attr.connectionNum; i++)
					if (srvThreadPool[i] != NULL)
						if ((!srvThreadPool[i]->getRunningStatus()) && (srvThreadPool[i]->getJoinableStatus())){
							// the thread has finished, join it
							srvThreadPool[i]->join(NULL);
							delete srvThreadPool[i];
							srvThreadPool[i] = NULL;
						}
				srvThreadLock.release();
			}
			log->writeLog("Chord: Cleaning up local nodes.\n", LogDeepDebug);
			localNodes->cleanupGarbage();
		}

		return 0;
	}

}
