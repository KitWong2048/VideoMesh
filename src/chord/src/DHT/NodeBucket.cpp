/*
NodeBucket.cpp

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

#include "../DHT/NodeBucket.h"
#include "../DHT/Chord.h"

using namespace Util;

namespace DHT{
	NodeBucket::NodeBucket(int _bucketSize, const DHTNetworkID& _refID, Chord& _chord){
		refID = _refID;
		chord = &_chord;
		log = chord->getLogger();
		nodes = new list<Node*>;
		bucketSize = _bucketSize;
	}

	NodeBucket::NodeBucket(const NodeBucket& _nodeList){
		*this = _nodeList;
	}

	NodeBucket::~NodeBucket(){
		list<Node*>::iterator l;
		for (l = nodes->begin(); l != nodes->end(); l++){
			if (*l != NULL)
				delete *l;
		}
		nodes->clear();

		delete nodes;
	}

	int NodeBucket::getSize(){
		return nodes->size();
	}

	Node* NodeBucket::addNode(const Node& _node){
		list<Node*>::iterator l;
		Node* returnNode;
		int currentSize;
		stringstream ss;

		returnNode = NULL;
		if (refID == _node)
			return NULL;

		this->nodesMutex.lock();
		ss<<"Adding finger: "<<_node<<endl;
		log->writeLog(ss.str(), LogDeepDebug);

		// find the proper position to insert the new node
		for (l = nodes->begin(); l != nodes->end(); l++){
			if ((**l) == _node){
				(**l) = _node;
				break;
			}else if (order(refID, _node, **l)){
				nodes->insert(l, new Node(_node));
				break;
			}
		}

		currentSize = nodes->size();

		// the new node is not inserted, so insert at back
		if ((l == nodes->end()) && (currentSize < bucketSize))
			nodes->push_back(new Node(_node));

		// check whether the list is longer than the maximum size, if yes, remove the last item
		if (currentSize > bucketSize){
			returnNode = nodes->back();
			nodes->pop_back();
		}

		this->nodesMutex.release();

		this->print(LogDeepDebug);

		return returnNode;
	}

	Node* NodeBucket::removeNode(const DHTNetworkID& _id){
		list<Node*>::iterator l;
		Node* returnNode;
		stringstream ss;

		returnNode = NULL;
		this->nodesMutex.lock();

		for (l = nodes->begin(); l != nodes->end(); l++){
			if (**l == _id){
				returnNode = *l;
				nodes->erase(l);
				break;
			}
		}

		this->nodesMutex.release();

		if (returnNode != NULL){
			ss<<"Removing finger: "<<*returnNode<<endl;
			log->writeLog(ss.str(), LogDeepDebug);
			this->print(LogDeepDebug);
		}

		return returnNode;
	}

	void NodeBucket::print(LogLevel _logLevel){
		list<Node*>::iterator l;
		stringstream ss;
		int i = 0;

		if (log->isLogThisLevel(_logLevel)){
			ss<<"Bucket: "<<endl;
			this->nodesMutex.lock();
			for (l = nodes->begin(); l != nodes->end(); l++){
				ss<<"["<<i<<"] "<<**l<<endl;
				i++;
			}

			if (nodes->begin() == nodes->end()){
				ss.str("");
				ss<<"Bucket: (empty)"<<endl;
			}

			log->writeLog(ss.str(), _logLevel);
			this->nodesMutex.release();
		}
	}

	NodeBucket* NodeBucket::clone(){
		NodeBucket* aClone;
		list<Node*>::iterator l;

		this->nodesMutex.lock();
		aClone = new NodeBucket(this->bucketSize, this->refID, *(this->chord));
		for (l = this->nodes->begin(); l != this->nodes->end(); l++){
			aClone->nodes->push_back(new Node(**l));
		}
		this->nodesMutex.release();

		return aClone;
	}

	Node* NodeBucket::operator[](int _pos){
		list<Node*>::iterator l;
		Node* returnNode = NULL;

		l = nodes->begin();

		if (l != nodes->end()){
			this->nodesMutex.lock();
			for (int j = 0; j < bucketSize && l != nodes->end(); j++, l++)
				if (_pos == j){
					returnNode = new Node(**l);
					break;
				}
			this->nodesMutex.release();
		}

		return returnNode;

	}

	NodeBucket& NodeBucket::operator=(const NodeBucket& _nodeList){
		list<Node*>::iterator l;

		this->nodesMutex.lock();

		this->bucketSize = _nodeList.bucketSize;
		this->refID = _nodeList.refID;
		this->chord = _nodeList.chord;
		this->log = _nodeList.log;
		for (l = this->nodes->begin(); l != this->nodes->end(); l++){
			if (*l != NULL)
				delete *l;
		}
		this->nodes->clear();

		for (l = _nodeList.nodes->begin(); l != _nodeList.nodes->end(); l++)
			this->nodes->push_back(new Node(**l));
		this->nodesMutex.release();

		return *this;
	}

	Node** NodeBucket::returnAllNodes(){
		list<Node*>::iterator l;
		Node** allNodes;
		int i = 0;

		this->nodesMutex.lock();
		allNodes = new Node*[this->bucketSize + 1];
		for (l = nodes->begin(); l != nodes->end(); l++)
			if (*l != NULL){
				allNodes[i] = new Node(**l);
				i++;
			}

		while (i <= this->bucketSize){
			allNodes[i] = NULL;
			i++;
		}
		this->nodesMutex.release();

		return allNodes;
	}

	Node* NodeBucket::getClosestSucc(const DHTNetworkID& _id){
		list<Node*>::iterator k, l;
		Node* returnNode;

		returnNode = NULL;
		this->nodesMutex.lock();

		k = nodes->begin();
		for (l = nodes->begin(); l != nodes->end(); l++){
			if (order(**k, _id, **l, false, true)){
				k = l;
			}
		}

		if (k != nodes->end())
			returnNode = new Node(**k);

		this->nodesMutex.release();
		return returnNode;
	}

	Node* NodeBucket::getClosestPred(const DHTNetworkID& _id){
		list<Node*>::iterator k, l;
		Node* returnNode;

		returnNode = NULL;

		if (nodes->begin() == nodes->end()){
			return NULL;
		}

		this->nodesMutex.lock();

		k = nodes->begin();
		for (l = nodes->begin(); l != nodes->end(); l++){
			if (order(**k, **l, _id, false, false)){
				k = l;
			}
		}

		returnNode = new Node(**k);

		this->nodesMutex.release();
		return returnNode;
	}

	bool NodeBucket::order(const DHTNetworkID& _lower, const DHTNetworkID& _middle, const DHTNetworkID& _upper, bool _lowerClosed, bool _upperClosed){
		return _middle.isInInterval(_lower, _upper, _lowerClosed, _upperClosed);
	}

	bool NodeBucket::isNodeExist(const Node& _node){
		bool status = false;
		list<Node*>::iterator l;

		this->nodesMutex.lock();
		for (l = nodes->begin(); l != nodes->end(); l++)
			if ((*l)->isIdentical(_node)){
				status = true;
				break;
			}
		this->nodesMutex.release();

		return status;
	}
}
