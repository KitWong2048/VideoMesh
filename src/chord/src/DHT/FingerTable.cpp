/*
FingerTable.cpp

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

#include "../DHT/FingerTable.h"
#include "../DHT/Chord.h"

using namespace Util;

namespace DHT{
	FingerTable::FingerTableEntry::FingerTableEntry(const DHTNetworkID& _actualStart, const DHTNetworkID& _start, const DHTNetworkID& _end, const Node& _successor){
		actualStart = _actualStart;
		start = _start;
		end = _end;
		successor = _successor;
		failCount = 0;
	}

	FingerTable::FingerTableEntry::~FingerTableEntry(){
	}

	FingerTable::FingerTable(const Node& _localNode, Chord& _chord) : fingerLock("FingerLock"){
		local = _localNode;
		chord = &_chord;
		this->log = chord->getLogger();

		fingers.push_back(new FingerTableEntry(local.addPower(ID_BIT_LENGTH - 1), local.addPower(0), (DHTNetworkID) local,  local));
		for (int i = 1; i < ID_BIT_LENGTH; i++)
			fingers.push_back(NULL);
	}

	FingerTable::FingerTable(const FingerTable& _fingerTable) : fingerLock("FingerLock"){
		for (int i = 0; i < ID_BIT_LENGTH; i++){
				this->fingers[i] = NULL;
		}
		*this = _fingerTable;
	}

	FingerTable::~FingerTable(){
		this->fingerLock.lock();

		for (int i = 0; i < ID_BIT_LENGTH; i++)
			if (fingers[i] != NULL){
				delete fingers[i];
				fingers[i] = NULL;
			}
		fingers.clear();
		this->fingerLock.release();
	}

	Node* FingerTable::addFinger(const Node& _candidate){
		FingerTableEntry *newEntry;
		vector<FingerTable::FingerTableEntry*>::iterator l;
		bool isAdded;
		stringstream ss;
		Node* returnNode;

		newEntry = new FingerTableEntry(local.addPower(0), local.addPower(0), local.addPower(1), _candidate);
		isAdded = false;
		returnNode = NULL;

		// find the upper bound for the candidate entry
		int i = 2;

		while (i < ID_BIT_LENGTH) {
			if (_candidate.isInInterval(newEntry->start, newEntry->end, true, false))
				break;

			newEntry->start = newEntry->end;
			newEntry->end = local.addPower(i);
			i++;
		}
		if (!_candidate.isInInterval(newEntry->start, newEntry->end, true, false)){
			newEntry->start = newEntry->end;
			newEntry->end = local;
		}
		newEntry->actualStart = newEntry->start;

		// find the first interval which contains the start of the new entry
		fingerLock.lock();
		l = fingers.begin();

		while (l != fingers.end()){
			if ((*l) != NULL && newEntry->start.isInInterval((*l)->start, (*l)->end, true, false))
				break;
			l++;
		}
		// so now (*l)->start must be <= newEntry->start
		ss<<"Adding Finger:	["<<newEntry->start<<", "<<newEntry->end<<")	"<<newEntry->successor<<endl;
		ss<<"Existing Finger:	["<<(*l)->start<<"|"<<", "<<(*l)->end<<")	"<<(*l)->successor<<endl;
		log->writeLog(ss.str(), LogDeepDebug);

		if ((*l)->start == newEntry->start){
			// same start
			if ((*l)->end == newEntry->end){
				// same interval , check which one has smaller node ID
				if ( _candidate.isInInterval((*l)->start, (*l)->successor) ){
					returnNode = new Node((*l)->successor);
					(*l)->actualStart = newEntry->actualStart;
					(*l)->successor = newEntry->successor;
					(*l)->failCount = 0;
				}
			}else { // same start but candidate has a smaller end
				(*l)->start = newEntry->end;
				fingers.insert(l, newEntry);
				isAdded = true;
			}

		}else{
			// candidate has a smaller start
			if ( _candidate.isInInterval((*l)->start, (*l)->successor) ){
				if (newEntry->end.isInInterval((*l)->start, (*l)->end)){
					newEntry->start = (*l)->start;
					(*l)->start = newEntry->end;
					if ((*l)->start == (*l)->end){
						// avoid the case when the entry has an interval which start and end are the local ID
						returnNode = new Node((*l)->successor);
						(*l)->actualStart = newEntry->actualStart;
						(*l)->start = newEntry->start;
						(*l)->end = newEntry->end;
						(*l)->successor = newEntry->successor;
						(*l)->failCount = 0;
					}else{
						fingers.insert(l, newEntry);
						isAdded = true;
					}
				}else if (newEntry->end == (*l)->end){
					returnNode = new Node((*l)->successor);
					(*l)->actualStart = newEntry->actualStart;
					(*l)->successor = newEntry->successor;
					(*l)->failCount = 0;
				}
			}

		}
		fingerLock.release();

		if (!isAdded)
			delete newEntry;

		this->print(LogDeepDebug);

		return returnNode;
	}

	Node* FingerTable::removeFinger(const DHTNetworkID& _id){
		vector<FingerTable::FingerTableEntry*>::iterator l, m;
		Node* returnNode = NULL;
		stringstream ss;

		this->fingerLock.lock();
		for (l = fingers.begin(); l != fingers.end(); l++)		
			if ((*l) != NULL){
				if ((*l)->successor == _id){
					m = l;
					m++;
					((*l)->failCount)++;
					if ((*l)->failCount >= NODE_FAIL_THRESHOLD)
						if ((m == fingers.end()) || (*m == NULL)){
							// we are removing the entry at the end of the list
							returnNode = new Node((*l)->successor);
							(*l)->successor = local;
							(*l)->failCount = 0;
						}else{
							(*m)->start = (*l)->start;
							returnNode = new Node((*l)->successor);
							delete(*l);
							*l = NULL;				////////////////////////////////////////////////
						}
				}

			}

		this->fingerLock.release();

		if (returnNode != NULL){
			ss<<"Removing finger: "<<*returnNode<<endl;
			log->writeLog(ss.str(), LogDeepDebug);
			this->print(LogDeepDebug);
		}

		return returnNode;
	}

	Node* FingerTable::getClosestPrecedingFinger(const DHTNetworkID& _id){
		Node* precedingNode = NULL;

		this->fingerLock.lock();
		for (int i = ID_BIT_LENGTH - 1; i >= 0; i--)
			if (fingers[i] != NULL){
				if (fingers[i]->successor.isInInterval(local, _id)){
					precedingNode = new Node(fingers[i]->successor);
					break;
				}
			}
		this->fingerLock.release();

		return precedingNode;
	};

	Node* FingerTable::getNthSuccessor(int _n){
		int size;
		Node* nthSuccessor = NULL;
		DHTNetworkID nthStart;

		nthStart = getEntryStart(_n);

		this->fingerLock.lock();
		size = this->fingers.size();
		if ((_n >= 1) && (_n <= size))
			for (int i = 0; i < size; i++)
				if (fingers[i] != NULL)
					if ((fingers[i]->actualStart == nthStart) || (nthStart.isInInterval(fingers[i]->start, fingers[i]->end, true, false)) ){
						nthSuccessor = new Node(this->fingers[i]->successor);
						break;
					}
		this->fingerLock.release();

		return nthSuccessor;
	}

	FingerTable& FingerTable::operator=(const FingerTable& _fingerTable){
		FingerTableEntry* entry;

		local = _fingerTable.local;
		log = _fingerTable.log;
		chord = _fingerTable.chord;

		this->fingerLock.lock();
		for (int i = 0; i < ID_BIT_LENGTH; i++){
			entry = _fingerTable.fingers[i];
			if (this->fingers[i] != NULL){
				delete this->fingers[i];
				this->fingers[i] = NULL;
			}

			if (entry != NULL)
				this->fingers[i] = new FingerTableEntry(entry->actualStart, entry->start, entry->end, entry->successor);
			else
				this->fingers[i] = NULL;
		}
		this->fingerLock.release();


		return *this;
	}

	void FingerTable::print(LogLevel _logLevel){
		FingerTableEntry* entry;
		stringstream ss;
		bool isAllEmpty;

		if (log->isLogThisLevel(_logLevel)){
			isAllEmpty = true;
			ss<<"Fingers: "<<endl;

			fingerLock.lock();
			for (int i = 0; i < ID_BIT_LENGTH; i++){
				entry = fingers[i];
				if (entry != NULL){
					isAllEmpty = false;
					ss<<"["<<i<<"] ["<<entry->start;
					//ss<<"|"<<entry->actualStart;
					ss<<", "<<entry->end<<")	"<<entry->successor<<endl;
				}
			}
			fingerLock.release();

			if (isAllEmpty){
				ss.str("");
				ss<<"Fingers: (empty)"<<endl;;
			}
			log->writeLog(ss.str(), _logLevel);
		}
	}

	bool FingerTable::isEntryPresent(int _pos){
		bool isPresent = false;
		DHTNetworkID entryStart;

		if ((_pos >= 1) && (_pos <= ID_BIT_LENGTH)){
			entryStart = this->getEntryStart(_pos);
			for (int i = 0; i < ID_BIT_LENGTH; i ++)
				if (fingers[i] != NULL)
					if (fingers[i]->actualStart == entryStart){
						isPresent = true;
						break;
					}
		}

		return isPresent;
	}

	bool FingerTable::isFingerExist(const Node& _node){
		bool status = false;
		int size;

		this->fingerLock.lock();
		size = this->fingers.size();
		for (int i = 0; i < size; i++)
			if (fingers[i] != NULL)
				if (fingers[i]->successor.isIdentical(_node)){
					status = true;
					break;
				}else if (!_node.isInInterval(fingers[i]->end, this->local))
					break;
		this->fingerLock.release();

		return status;
	}

	DHTNetworkID FingerTable::getEntryStart(int _pos){
		DHTNetworkID entryStart;

		if ((_pos >= 1) && (_pos <= ID_BIT_LENGTH))
			entryStart = local.addPower(_pos - 1);

		return entryStart;
	}

	Node** FingerTable::returnAllFingers(){
		FingerTableEntry* entry;
		Node** allNodes;

		allNodes = new Node*[ID_BIT_LENGTH + 1];

		this->fingerLock.lock();
		for (int i = 0; i < ID_BIT_LENGTH; i++){
			entry = fingers[i];
			if (entry != NULL)
				allNodes[i] = new Node(entry->successor);
			else allNodes[i] = NULL;
		}
		this->fingerLock.release();

		allNodes[ID_BIT_LENGTH] = NULL;

		return allNodes;
	}

	FingerTable* FingerTable::clone(){
		FingerTableEntry* entry;
		FingerTable* fingerTableClone;

		fingerTableClone = new FingerTable(this->local, *(this->chord));

		this->fingerLock.lock();
		for (int i = 0; i < ID_BIT_LENGTH; i++){
			if (fingerTableClone->fingers[i] != NULL){
				delete fingerTableClone->fingers[i];
				fingerTableClone->fingers[i] = NULL;
			}
			entry = this->fingers[i];
			if (entry != NULL)
				fingerTableClone->fingers[i] = new FingerTableEntry(entry->actualStart, entry->start, entry->end, entry->successor);
			else
				fingerTableClone->fingers[i] = NULL;
		}
		this->fingerLock.release();

		return fingerTableClone;
	}

	int FingerTable::getSize(){
		return fingers.size();
	}

}
