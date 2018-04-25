/*
LocalNodeCollection.cpp

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

#include "../DHT/LocalNodeCollection.h"
#include "../DHT/Chord.h"

using namespace Util;

namespace DHT{
	LocalNodeCollection::LocalNodeCollection(Chord& _chord) : nodeLock("LocalsLock"){
		chord = &_chord;
		this->log = chord->getLogger();
	}

	LocalNodeCollection::~LocalNodeCollection(){
		map<DHTNetworkID, LocalNodeHandle*>::iterator k, l;
		k = localNodes.begin();
		l = k;
		if (l != localNodes.end())
			do{
				l++;
				if (k->second != NULL)
					delete k->second;
				localNodes.erase(k);
				k = l;
			}while (l != localNodes.end());
	}

	int LocalNodeCollection::getSize(){
		return localNodes.size();
	}

	LocalNodeHandle* LocalNodeCollection::getLocalNode(const DHTNetworkID& _id){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		LocalNodeHandle* lookup;

		nodeLock.lock();

		l = localNodes.find(_id);
		if (l != localNodes.end()){
			lookup = l->second;
			lookup->addUseCount();
		}else
			lookup = NULL;

		nodeLock.release();

		return lookup;
	}

	LocalNodeHandle* LocalNodeCollection::getPrecedingLocalNode(const DHTNetworkID& _id){
		map<DHTNetworkID, LocalNodeHandle*>::iterator k, l;
		LocalNodeHandle* precedingNode;

		nodeLock.lock();
		if (localNodes.empty()){
			// empty collection of local nodes
			precedingNode = NULL;
		}else{

			// search for the first ID which is greater than or equal to the input value
			l = localNodes.begin();
			k = l;
			// l should stop at the first element larger than or equal to _id while k stop at the last element smaller than _id
			while ((l != localNodes.end()) && (l->first < _id)){
				k = l;
				l++;
			}

			// if l stops at the beginning, we take the last element for k
			if (l == localNodes.begin()){
				while (l != localNodes.end()){
					k = l;
					l++;
				}
			}

			precedingNode = k->second;
			precedingNode->addUseCount();
		}

		nodeLock.release();

		return precedingNode;
	}

	LocalNodeHandle* LocalNodeCollection::getSucceedingLocalNode(const DHTNetworkID& _id){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		LocalNodeHandle* succeedingNode;

		nodeLock.lock();

		if (localNodes.empty()){
			// empty collection of local nodes
			succeedingNode = NULL;
		}else{
			// search for the first ID which is greater than or equal to the input value
			l = localNodes.begin();
			while ((l != localNodes.end()) && (l->first < _id))
				l++;

			if (l == localNodes.end())
				l = localNodes.begin();

			succeedingNode = l->second;
			succeedingNode->addUseCount();
		}

		nodeLock.release();



		return succeedingNode;
	}

	int LocalNodeCollection::addNode(const LocalNodeHandle& _localNode){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		DHTNetworkID targetNodeID(_localNode.getNode());

		nodeLock.lock();
		l = localNodes.find(targetNodeID);
		if (l == localNodes.end())
			localNodes.insert(make_pair(targetNodeID, new LocalNodeHandle(_localNode)));
		else{
			delete l->second;
			l->second = new LocalNodeHandle(_localNode);
		}

		nodeLock.release();

		return 0;
	}

	int LocalNodeCollection::removeNode(const DHTNetworkID& _id){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;

		nodeLock.lock();
		l = localNodes.find(_id);
		if (l != localNodes.end()){
			garbageNodes.push_back(l->second);
			localNodes.erase(l);
		}
		nodeLock.release();

		return 0;
	}

	void LocalNodeCollection::print(LogLevel _logLevel){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;

		for (l = localNodes.begin(); l != localNodes.end(); l++)
			l->second->print(_logLevel);
	}

	void LocalNodeCollection::addReference(const DHTNetworkID& _localNodeID, const Node& _finger){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		LocalNodeHandle* local;

		nodeLock.lock();
		l = localNodes.find(_localNodeID);
		if (l != localNodes.end())
			local = l->second;
		else local = NULL;

		if (local != NULL){
			local->addReference(_finger);
		}
		nodeLock.release();
	}

	void LocalNodeCollection::removeReference(const DHTNetworkID& _localNodeID, const DHTNetworkID& _fingerID){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		LocalNodeHandle* local;

		nodeLock.lock();
		l = localNodes.find(_localNodeID);
		if (l != localNodes.end())
			local = l->second;
		else local = NULL;

		if (local != NULL){
			local->removeReference(_fingerID);
		}
		nodeLock.release();
	}

	void LocalNodeCollection::doHouseKeeping(int _opCode){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;
		set<LocalNodeHandle*> nodeSet;
		set<LocalNodeHandle*>::iterator s;

		// first push all pointers to a set in order to avoid prolong locking for the map
		nodeLock.lock();
		for (l = localNodes.begin(); l != localNodes.end(); l++)
			if (l->second != NULL){
				l->second->addUseCount();
				nodeSet.insert(l->second);
			}
		nodeLock.release();

		// then process local nodes one by one
		for (s = nodeSet.begin(); s != nodeSet.end(); s++){
			switch (_opCode){
				case 1:
					(*s)->stabilize();
					(*s)->checkPredecessor();
					break;

				case 2:
					(*s)->fixFingers();
					break;

				case 3:
					(*s)->visitNeighbor();
					break;

				default:
					break;
			}
			(*s)->subtractUseCount();
		}

	}

	void LocalNodeCollection::stabilize(){
		doHouseKeeping(1);
	}

	void LocalNodeCollection::fixFingers(){
		doHouseKeeping(2);
	}

	void LocalNodeCollection::visitNeighbor(){
		doHouseKeeping(3);
	}

	LocalNodeCollection* LocalNodeCollection::clone(){
		LocalNodeCollection* aClone;
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;

		aClone = new LocalNodeCollection(*this->chord);
		nodeLock.lock();
		for (l = localNodes.begin(); l != localNodes.end(); l++){
			if (l->second != NULL)
				aClone->localNodes.insert(make_pair(l->first, l->second->clone()));
		}
		nodeLock.release();

		return aClone;
	}

	void LocalNodeCollection::cleanupGarbage(){
		list<LocalNodeHandle*>::iterator k, l;

		k = garbageNodes.begin();
		if (k != garbageNodes.end()){
			l = k;
			do{
				l++;
				if ((*k)->getUseCount() == 0){
					delete *k;
					garbageNodes.erase(k);
				}
				k = l;
			}while (l != garbageNodes.end());
		}

	}

	void LocalNodeCollection::resetNodesUseCount(){
		map<DHTNetworkID, LocalNodeHandle*>::iterator l;

		nodeLock.lock();
		for (l = localNodes.begin(); l != localNodes.end(); l++){
			while (l->second->getUseCount() > 0)
				l->second->subtractUseCount();
		}
		nodeLock.release();
	}

}
