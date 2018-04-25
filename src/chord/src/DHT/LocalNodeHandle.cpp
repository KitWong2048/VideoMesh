/*
LocalNodeHandle.cpp

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

#include "../DHT/LocalNodeHandle.h"
#include "../DHT/Chord.h"

using namespace Util;

namespace DHT{
	LocalNodeHandle::LocalNodeHandle(const Node& _node, Chord& _chord){
		node = _node;
		chord = &_chord;
		fingers = new FingerTable(_node, _chord);
		successors = new NodeBucket(chord->getNeighborhoodSize(), _node, _chord);
		predecessor = this->node;
		log = chord->getLogger();
		fixFingerIndex = (rand() % ID_BIT_LENGTH) + 1;
		useCount = 0;
	}

	LocalNodeHandle::LocalNodeHandle(const LocalNodeHandle& _localNode){
		fingers = new FingerTable(_localNode.node, *_localNode.chord);
		successors = new NodeBucket(_localNode.chord->getNeighborhoodSize(), _localNode.node, *_localNode.chord);
		*this = _localNode;
	}

	LocalNodeHandle::~LocalNodeHandle(){
		chord = NULL;
		delete fingers;
		delete successors;
	}

	Node LocalNodeHandle::getNode() const{
		return Node(node);
	}

	int LocalNodeHandle::setNode(const Node& _node){
		node = _node;
		return 0;
	}

	Chord* LocalNodeHandle::getChord(){
		return this->chord;
	}

	void LocalNodeHandle::setChord(Chord* _chord){
		chord = _chord;
	}

	Node* LocalNodeHandle::getSuccessor(){
		return this->fingers->getNthSuccessor(1);
	}

	Node* LocalNodeHandle::getPredecessor(){
		return new Node(this->predecessor);
	}

	Node* LocalNodeHandle::findSuccessor(const DHTNetworkID& _id, int* _hopCount, LogLevel _logLevel, const Node* _hint){
		Node *pred = NULL, *succ = NULL;
		int hopCount;

		if (!findSuccPred(_id, pred, succ, hopCount, _logLevel, _hint)){
			if (succ != NULL){
				delete succ;
				succ = NULL;
			}
		}

		if (pred != NULL)
			delete pred;
		if (_hopCount != NULL)
			*_hopCount = hopCount;
		return succ;
	}

	Node* LocalNodeHandle::findPredecessor(const DHTNetworkID& _id, int* _hopCount, LogLevel _logLevel, const Node* _hint){
		Node *pred = NULL, *succ = NULL;
		int hopCount;

		if (!findSuccPred(_id, pred, succ, hopCount, _logLevel, _hint)){
			if (pred != NULL){
				delete pred;
				pred = NULL;
			}
		}

		if (succ != NULL)
			delete succ;
		if (_hopCount != NULL)
			*_hopCount = hopCount;

		return pred;
	}

	bool LocalNodeHandle::findSuccPred(const DHTNetworkID& _id, Node*& _pred, Node*& _succ, int& _hopCount, LogLevel _logLevel, const Node* _hint){
		Node* searchPath;
		Node* succ;
		bool isSuccess;

		succ = this->findID(_id, searchPath, _hopCount, _logLevel, _hint);
		if (succ == NULL){
			isSuccess = false;
			_pred = NULL;
			_succ = NULL;
		}else{
			isSuccess = true;
			_pred = new Node(searchPath[_hopCount]);
			_succ = succ;
		}

		delete []searchPath;

		return isSuccess;
	}

	Node* LocalNodeHandle::findID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, LogLevel _logLevel, const Node* _hint){
		DHTRequest* req;
		bool isSuccess =  true;
		stringstream ss;
		string failedMsg = "";
		Node* previousSucc;
		Node *pred, *succ;
		list<Node> searchPath;

		_hopCount = 0;
		_searchPath = NULL;
		// get the successor of hint, if it returns, we continue the process with this hint otherwise we start from local node
		pred = NULL;
		succ = NULL;
		if (_hint != NULL){
			req = new DHTRequest(DHTReqSucc, *(this->chord), &(this->node), _hint);
			succ = (Node*) req->makeRequest();
			delete req;
			req = NULL;
			if (succ != NULL)
				pred = new Node(*_hint);
		}

		if (succ == NULL)
			// check whether the node is in island
			if (this->isIsland())
				isSuccess = false;

		// check whether the predecessor can help
		if ((succ == NULL) && (isSuccess)){
			pred = getPredecessor();
			if (pred != NULL)
				if (_id.isInInterval(*pred, this->node, false, false)){
					req = new DHTRequest(DHTReqSucc, *(this->chord), &(this->node), pred);
					succ = (Node*) req->makeRequest();
					delete req;
					req = NULL;
					if (succ == NULL){
						delete pred;
						pred = NULL;
					}
				}else{
					delete pred;
					pred = NULL;
				}
		}

		// if the above steps cannot help find a good successor, we begin from the local node
		if ((succ == NULL) && (isSuccess)){
			succ = this->getSuccessor();
			if (succ == NULL){
				isSuccess = false;
				failedMsg = "Failed to get the successor at the first step.";
			}else pred = new Node(this->node);
		}

		if (isSuccess){
			previousSucc = NULL;
			searchPath.push_back(*pred);
			ss<<"Finding "<<_id<<endl;
			ss<<_hopCount<<".) Checking ("<<(DHTNetworkID) *pred<<", "<<(DHTNetworkID) *succ<<"]......"<<endl;
			log->writeLog(ss.str(), _logLevel);
		}

		// loop until we find a node and its successor which IDs span an interval containing the search value
		struct timeval startTime, curTime;
		double elapsedTime;
		bzero(&startTime, sizeof(startTime));
		gettimeofday(&startTime, NULL);
		while ( (isSuccess) && (!_id.isInInterval(*pred, *succ, false, true)) ){
			// request the predecessor for its closest preceding finger
			if (*pred == this->node){
				// request current local node
				delete pred;
				pred = findClosestPredcedingFinger(_id);
			}else{
				// request remote node for its closest finger to the key
				req = new DHTRequest(DHTReqClosestFinger, *(this->chord), &(this->node), pred, &_id);
				delete pred;
				pred = (Node*) req->makeRequest();
				delete req;
				req = NULL;
			}

			// request the closest finger for its successor
			if (pred != NULL){
				// if the closest finger is the same as the local node, a circular reference occurs, stop it
				if (*pred == this->node){
					delete succ;
					succ = this->getSuccessor();
					break;
				}
				previousSucc = succ;
				req = new DHTRequest(DHTReqSucc, *(this->chord), &(this->node), pred);
				succ = (Node*) req->makeRequest();
				delete req;
				// if the closest finger fails, we request the successor of the successor
				if ((succ == NULL) && (previousSucc != NULL)){
					delete pred;
					pred = previousSucc;
					req = new DHTRequest(DHTReqSucc, *(this->chord), &(this->node), pred);
					succ = (Node*) req->makeRequest();
					delete req;
				}else{
					delete previousSucc;
				}

			}

			// if either pointer is NULL, the search fails
			if (pred == NULL){
				failedMsg = failedMsg + "Failed to get the predecessor in the loop.";
				isSuccess = false;
			}
			if (succ == NULL){
				failedMsg = failedMsg + "Failed to get the successor in the loop.";
				isSuccess = false;
			}

			 if (isSuccess){
				_hopCount++;
				ss.str("");
				ss<<_hopCount<<".) Checking ("<<(DHTNetworkID) *pred<<", "<<(DHTNetworkID) *succ<<"]......"<<endl;
				log->writeLog(ss.str(), _logLevel);
				searchPath.push_back(*pred);
				this->addReference(*pred);
				this->addReference(*succ);
				bzero(&curTime, sizeof(curTime));
				gettimeofday(&curTime, NULL);
				elapsedTime = curTime.tv_sec - startTime.tv_sec + (curTime.tv_usec - startTime.tv_usec) / 1000000;
				if ((elapsedTime > chord->getMaxSearchTime()) && (chord->getMaxSearchTime() != -1)){
					isSuccess = false;
					ss<<"Timeout ("<<elapsedTime<<").";
				}
			 }
		}

		ss.str("");
		 if (isSuccess){
			ss<<"Search has returned from ("<<(DHTNetworkID) *pred<<", "<<(DHTNetworkID) *succ<<"]."<<endl;
		 }else{
			 ss<<"Search has failed. "<<failedMsg<<endl;
			 if (succ != NULL){
				delete succ;
				succ = NULL;
			 }
		 }
		 if (pred != NULL)
			delete pred;
		 ss<<endl;
		 log->writeLog(ss.str(), _logLevel);

		if (searchPath.begin() != searchPath.end()){
			 _searchPath = new Node[searchPath.size()];
			 int i = 0;
			list<Node>::iterator l;
			for (l = searchPath.begin(); l != searchPath.end(); l++){
				_searchPath[i] = *l;
				i++;
			}
		}

		return succ;
	}

	Node* LocalNodeHandle::recursiveFindID(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount, LogLevel _logLevel, const Node* _hint){
		// the recurseive API has not been implemented at this moment
		return findID(_id, _searchPath, _hopCount, _logLevel, _hint);
	}

	Node* LocalNodeHandle::findClosestPredcedingFinger(const DHTNetworkID& _id){
		Node* predFinger, *pred;

		predFinger = fingers->getClosestPrecedingFinger(_id);
		pred = successors->getClosestPred(_id);

		if (predFinger != NULL){
			if (pred != NULL)
				if (pred->isInInterval(*predFinger, _id, false, true)){
					delete predFinger;
				}else{
					delete pred;

					pred = predFinger;
				}
			else pred = predFinger;
		}else if (pred == NULL)
			pred = this->getSuccessor();

		return pred;
	}

	int LocalNodeHandle::addReference(const Node& _node){
		Node *returnFinger, *returnSucc;
		Node** allNodes;
		int i;

		// add to finger table
		returnFinger = fingers->addFinger(_node);
		if (returnFinger != NULL){
			returnSucc = successors->addNode(*returnFinger);
			if (returnSucc != NULL)
				delete returnSucc;
			delete returnFinger;
		}

		// add to successor list
		returnSucc = successors->addNode(_node);
		if (returnSucc != NULL)
			delete returnSucc;

		// add to predecessor
		if (_node.isInInterval(this->predecessor, this->node, true, false))
			this->predecessor = _node;

		// exchange the data structure
		if (fingers->getSize() < successors->getSize()){
			allNodes = successors->returnAllNodes();
			if (allNodes != NULL){
				i = 0;
				while (allNodes[i] != NULL){
					returnFinger = fingers->addFinger(*allNodes[i]);
					if (returnFinger != NULL)
						delete returnFinger;
					delete allNodes[i];
					i++;
				}
				delete []allNodes;
			}
		}

		return 0;
	}

	int LocalNodeHandle::removeReference(const DHTNetworkID& _id){
		int status = 0;
		Node *lastFinger, *returnNode;

		returnNode = fingers->removeFinger(_id);
		if (returnNode != NULL)
			delete returnNode;
		returnNode = successors->removeNode(_id);
		if (returnNode != NULL)
			delete returnNode;

		if (predecessor == _id){
			lastFinger = fingers->getNthSuccessor(ID_BIT_LENGTH);
			if (lastFinger != NULL){
				predecessor = *lastFinger;
				delete lastFinger;
			}else predecessor = this->node;
		}

		return status;
	}

	LocalNodeHandle& LocalNodeHandle::operator=(const LocalNodeHandle& _localNode){
		chord = _localNode.chord;
		node = _localNode.node;
		predecessor = _localNode.predecessor;
		*fingers = *(_localNode.fingers);
		*successors = *(_localNode.successors);
		fixFingerIndex = _localNode.fixFingerIndex;
		useCount = _localNode.useCount;

		log = chord->getLogger();

		return *this;
	}

	void LocalNodeHandle::print(LogLevel _logLevel){
		stringstream ss;
		Node* pred;
		Node* succ;

		if (log->isLogThisLevel(LogNormal)){
			ss.str("");
			ss<<endl<<"This: "<<node<<endl;

			pred = getPredecessor();
			if (pred != NULL){
				ss<<"Pred: "<<*pred<<endl;
				delete pred;
			}else
				ss<<"Pred: (NULL)"<<endl;

			succ = getSuccessor();
			if (succ != NULL){
				ss<<"Succ: "<<*succ<<endl;
				delete succ;
			}else
				ss<<"Succ: (NULL)"<<endl;

			log->writeLog(ss.str(), _logLevel);
		}
		fingers->print(_logLevel);
		successors->print(_logLevel);
		log->writeLog("\n", _logLevel);
	}

	int LocalNodeHandle::checkPredecessor(){
		int isDead;

		if (this->pingNode(predecessor) < 0){
			isDead = 1;
			chord->removeReference(node, predecessor);
		}else{
			isDead = 0;
			chord->addReference(node, predecessor);
		}

		return isDead;
	}

	int LocalNodeHandle::fixFingers(){
		DHTNetworkID start;
		Node* entry;
		stringstream ss;

		entry = fingers->getNthSuccessor(fixFingerIndex);
		if (entry != NULL){
			if (!(*entry == this->node))
				if (this->pingNode(*entry) < 0){
					chord->removeReference(this->node, *entry);
					ss<<(DHTNetworkID) this->node<<": fingers["<<fixFingerIndex<<"] "<<*entry<<" is dead."<<endl;
				}

			delete entry;
		}

		start = fingers->getEntryStart(fixFingerIndex);
		ss<<(DHTNetworkID) this->node<<": Checking fingers["<<fixFingerIndex<<"] with start: "<<start<<endl;
		entry = this->findSuccessor(start, NULL, LogStabilizer);
		if (entry != NULL){
			if (this->pingNode(*entry) >= 0){
				chord->addReference(this->node, *entry);
				ss<<(DHTNetworkID) this->node<<": Candidate new finger: "<<*entry<<endl;
			}
			delete entry;
		}else ss<<(DHTNetworkID) this->node<<": No available candidate finger."<<endl;

		fixFingerIndex = (fixFingerIndex % ID_BIT_LENGTH) + 1;

		log->writeLog(ss.str(), LogStabilizer);

		return 0;
	}

	int LocalNodeHandle::stabilize(){
		int status = 0;
		int* isAlive;
		int i;
		Node *succ, *predOfSucc, *nodeToNotify;
		DHTRequest* req;
		Node** remoteNeighbor;

		// get the immediate successor, if it is dead, get the next one
		succ = this->getSuccessor();
		predOfSucc = NULL;
		nodeToNotify = NULL;

		// return if no successor or successor is the local node itself (i.e. only local node in Chord)
		if (succ == NULL)
			return -1;
		else  if (*succ == this->node){
			delete succ;
			return -1;
		}

		// get the predecessor of the immediate successor
		req = new DHTRequest(DHTReqPred, *(this->chord), &(this->node), succ);
		predOfSucc = (Node*) req->makeRequest();
		delete req;

		// if the return is NULL, the successor is dead, remove it and get the next successor
		if (predOfSucc == NULL){
			log->writeLog(this->node.getIDHex() + ": The successor " + succ->getIDHex() + " is dead.\n", LogStabilizer);
			chord->removeReference(this->node, *succ);
			delete succ;
			// get the closest node from successor list and put it in finger table
			succ = (*this->successors)[0];
			if (succ != NULL){
				chord->addReference(this->node, *succ);
			}else
				succ = this->getSuccessor();
			if (succ == NULL)
				return -1;
			else  if (*succ == this->node){
				delete succ;
				return -1;
			}
			nodeToNotify = succ;
		}else{
			if (predOfSucc->isInInterval(this->node, *succ))
				nodeToNotify = predOfSucc;
			else nodeToNotify = succ;
		}

		if (nodeToNotify == NULL){
			// no node is available to notify, this includes all successor
			log->writeLog(this->node.getIDHex() + ": There is no successor to notify.\n", LogStabilizer);
			return -1;
		}

		// notify the candidate node
		req = new DHTRequest(DHTReqNotify, *(this->chord), &(this->node), nodeToNotify);
		isAlive = (int*) req->makeRequest();
		delete req;
		if (isAlive != NULL){
			 if ((*isAlive >= 0) && (nodeToNotify == predOfSucc)){
				chord->addReference(this->node, *predOfSucc);
			}

			delete isAlive;

			// request neighbor lists from the successor
			req = new DHTRequest(DHTReqAllNeighbor, *(this->chord), &(this->node), nodeToNotify);
			remoteNeighbor = (Node**) req->makeRequest();
			delete req;
			if (remoteNeighbor != NULL){
				i = 0;
				while (remoteNeighbor[i] != NULL){
					if (!this->isReferenceExist(*remoteNeighbor[i]))
						if (this->pingNode(*remoteNeighbor[i]) >= 0){
							chord->addReference(this->node, *remoteNeighbor[i]);
						}
					delete remoteNeighbor[i];
					i++;
				}
				delete []remoteNeighbor;
			}

		}

		if (predOfSucc != NULL)
			delete predOfSucc;
		delete succ;

		return status;
	}

	int LocalNodeHandle::visitNeighbor(){
		int status = 0;
		Node* neighbor;
		int neighborSize;

		neighborSize = successors->getSize();
		for (int i = 0; i < neighborSize; i++){
			neighbor = (*successors)[i];
			if (neighbor != NULL){
				if (this->pingNode(*neighbor) >= 0){
					chord->addReference(this->node, *neighbor);
				}else chord->removeReference(this->node, *neighbor);
				delete neighbor;
			}
		}

		return status;
	}

	int LocalNodeHandle::join(const Node& _bootstrap){
		DHTRequest* req = NULL;
		Node *bootstrap, *succ, *pred, **remoteFingers;
		int* tmp = NULL;
		stringstream ss;

		// add the bootstrap node if the its ID is not the same as the local node
		if (_bootstrap != this->node){
			ss<<(DHTNetworkID) this->node<<": Bootstrap "<<_bootstrap<<" is ";
		}else ss<<(DHTNetworkID) this->node<<": Anonymous bootstrap on "<<_bootstrap.getNodeIP()<<":"<<_bootstrap.getNodePort()<<" is ";
		req = new DHTRequest(DHTReqSelf, *(this->chord), &(this->node), &_bootstrap);
		bootstrap = (Node*) req->makeRequest();
		delete req;
		req = NULL;
		if (bootstrap != NULL){
			this->addReference(*bootstrap);
			ss<<"alive.\n";
			log->writeLog(ss.str(), LogNormal);
		}else{
			ss<<"dead.\n"<<(DHTNetworkID) this->node<<": Become an island.\n";
			log->writeLog(ss.str(), LogNormal);
			return -1;
		}

		// find successor
		succ = this->findSuccessor(this->node, NULL, LogDeepDebug, bootstrap);
		if (succ != NULL){
			log->writeLog(this->node.getIDHex() + ": Successor from bootstrap is " + succ->getIDHex() + ".\n", LogNormal);
			if (*succ == this->node){
				if ((succ->getNodeIP() != this->node.getNodeIP()) || (succ->getNodePort() != this->node.getNodePort())){
					// if Chord returns a node with same ID as the new node but different IP and port, that means the ID is already used
					delete succ;
					return -1;
				}else{
					// if Chord returns local node as the successor, use the bootstarp node instead
					delete succ;
					succ = new Node(*bootstrap);
				}
			}else{
				this->addReference(*succ);
			}
		}else{
			log->writeLog(this->node.getIDHex() + ": Successor from bootstrap is NULL.\n", LogNormal);
			succ = this->getSuccessor();
		}

		// find predecessor, by asking the successor just found
		req = new DHTRequest(DHTReqPred, *(this->chord), &(this->node), succ);
		pred = (Node*) req->makeRequest();
		delete req;
		if (pred != NULL){
			if (*pred != this->node){
				this->addReference(*pred);
			}
			delete pred;
		}

		// ask the successor to update its predecessor reference to this node
		req = new DHTRequest(DHTReqNotify, *(this->chord), &(this->node), succ);
		tmp = (int*) req->makeRequest();
		delete req;
		if (tmp != NULL)
			delete tmp;

		// ask the successor for a copy of its finger table
		req = new DHTRequest(DHTReqAllFingers, *(this->chord), &(this->node), succ);
		remoteFingers = (Node**) req->makeRequest();
		delete req;

		if (remoteFingers != NULL){
			for (int i = 0; i < ID_BIT_LENGTH; i++)
				if (remoteFingers[i] != NULL){
					this->addReference(*remoteFingers[i]);
					delete remoteFingers[i];
				}
			delete []remoteFingers;
		}

		delete succ;
		delete bootstrap;

		return 0;
	}

	Node** LocalNodeHandle::returnAllFingers(){
		return this->fingers->returnAllFingers();
	}

	Node** LocalNodeHandle::returnAllNeighbor(){
		return this->successors->returnAllNodes();
	}

	int LocalNodeHandle::copyReference(const LocalNodeHandle& _copy){	
		Node** ref;
		int counter;

		ref = _copy.fingers->returnAllFingers();
		if (ref != NULL){
			counter = 0;
			while (ref[counter] != NULL){
				this->addReference(*ref[counter]);
				delete ref[counter];
				counter++;
			}
			delete []ref;
		}

		ref = _copy.successors->returnAllNodes();
		if (ref != NULL){
			counter = 0;
			while (ref[counter] != NULL){
				this->addReference(*ref[counter]);
				delete ref[counter];
				counter++;
			}
			delete []ref;
		}

		return 0;
	}

	LocalNodeHandle* LocalNodeHandle::clone(){
		LocalNodeHandle* aClone;

		aClone = new LocalNodeHandle(this->node, *(this->chord));
		aClone->predecessor = this->predecessor;
		delete aClone->fingers;
		aClone->fingers = this->fingers->clone();
		delete aClone->successors;
		aClone->successors = this->successors->clone();
		aClone->fixFingerIndex = this->fixFingerIndex;
		aClone->chord = this->chord;
		aClone->log = this->log;
		aClone->useCount = this->useCount;

		return aClone;
	}

	int LocalNodeHandle::pingNode(const Node& _node){
		DHTRequest *req;
		int returnResult, *pingResult;

		returnResult = -1;
		req = new DHTRequest(DHTReqPing, *(this->chord), &(this->node), &_node);
		pingResult = (int*) req->makeRequest();
		delete req;
		if (pingResult != NULL){
			returnResult = *pingResult;
			delete pingResult;
		}

		return returnResult;
	}

	bool LocalNodeHandle::isIsland(){
		bool status = false;
		Node* succ = this->getSuccessor();
		Node* pred = this->getPredecessor();

		if (*pred == *succ)
			if (*succ == this->node)
				status = true;

		delete succ;
		delete pred;
		return status;
	}

	bool LocalNodeHandle::isReferenceExist(const Node& _node){
		if (fingers->isFingerExist(_node))
			return true;
		else return successors->isNodeExist(_node);
	}

	int LocalNodeHandle::getUseCount(){
		int ret;
		localNodeMutex.lock();
		ret = useCount;
		localNodeMutex.release();
		return ret;
	}

	void LocalNodeHandle::addUseCount(){
		localNodeMutex.lock();
		++useCount;
		localNodeMutex.release();
	}

	void LocalNodeHandle::subtractUseCount(){
		localNodeMutex.lock();
		--useCount;
		localNodeMutex.release();
	}

}
