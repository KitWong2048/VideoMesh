/*
ChordApp.cpp

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

#include "../App/ChordApp.h"
#include "../DHT/Chord.h"

using namespace DHT;
using namespace Util;

namespace App{

	ChordApp::ChordApp(const ChordAttr& _attr){
		chord = new DHT::Chord(this, _attr);
	}

	ChordApp::~ChordApp(){
		cleanAsyncList();
		if (chord != NULL){
			chord->stop();
			if (chord->getJoinableStatus())
				chord->join();
			delete chord;
		}
	}

	int ChordApp::run(){		
		if (chord->isMultiThreaded())
			return chord->run();		
		else
			chord->doExecuteDHT();
			return 0;		
	}

	int ChordApp::stop(){
		// signal Chord to allow time to stop
		if (chord != NULL)
			chord->stop();

		return 0;
	}

	int ChordApp::addNode(const DHTNetworkID& _id, const char* _appValue){
		return chord->addNode(_id, _appValue);
	}

	int ChordApp::addNode(const DHTNetworkID& _id, const char* _appValue, const Node& _bootstrap){
		return chord->addNode(_id, _appValue, _bootstrap);
	}

	int ChordApp::removeNode(const DHTNetworkID& _id){
		return chord->removeNode(_id);
	}

	Node* ChordApp::lookup(const DHTNetworkID& _id, int& _hopCount){
		return chord->lookup(_id, &_hopCount, LogDebug);
	}

	Node* ChordApp::lookupFullPath(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount){
		return chord->lookupFullPath(_id, _searchPath, _hopCount, LogDebug);
	}

	Node* ChordApp::recursiveLookup(const DHTNetworkID& _id, Node*& _searchPath, int& _hopCount){
		return chord->recursiveLookup(_id, _searchPath, _hopCount, LogDebug);
	}

	Node** ChordApp::getFingers(const Node& _node){
		return chord->getFingersOrNeighbor(_node, true);
	}

	Node** ChordApp::getNeighbor(const Node& _node){
		return chord->getFingersOrNeighbor(_node, false);
	}

	void ChordApp::performAsyncAction(AsyncHandle* _async){
		if (_async != NULL){
			this->asyncHandleList.push_back(_async);
			_async->run();
		}
	}

	void ChordApp::cleanAsyncList(){
		list<AsyncHandle*>::iterator l;

		for (l = asyncHandleList.begin(); l != asyncHandleList.end(); l++)
			if (*l != NULL)
				if ( ((*l)->getCancelStatus()) || ( !((*l)->getRunningStatus()) ) ){
					if ((*l)->getJoinableStatus())
						(*l)->join();
					delete *l;
					*l = NULL;
				}

		asyncHandleList.remove(NULL);		
	}

	void ChordApp::print(LogLevel _logLevel){
		chord->print(_logLevel);
	}

	bool ChordApp::isDHTRunning(){
		return chord->getDHTRunningStatus();
	}

}
