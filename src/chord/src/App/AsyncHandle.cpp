/*
AsyncHandle.cpp

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

#include "../App/AsyncHandle.h"
#include "../App/ChordApp.h"

using namespace Util;
using namespace DHT;

namespace App{
	AsyncHandle::AsyncHandle(ChordApp* _app) : Thread(){
		app = _app;
		hopCount = -1;
		dest = NULL;
		searchPath = NULL;
		nodes = NULL;
		opType = AsyncNull;
		arg1 = NULL;
		arg2 = NULL;
		arg3 = NULL;
	}

	AsyncHandle::~AsyncHandle(){
		clear();
	}

	void AsyncHandle::clear(){
		if (dest != NULL){
			delete dest;
			dest = NULL;
		}
		if (searchPath != NULL){
			delete []searchPath;
			searchPath = NULL;
		}
		if (nodes != NULL){
			int i = 0;
			while (nodes[i] != NULL){
				delete nodes[i];
				i++;
			}
			delete []nodes;
			nodes = NULL;
		}
	}

	void* AsyncHandle::execute(){
		DHTNetworkID *id(NULL);
		Node *node(NULL);
		char* appValue(NULL);

		if (!isDHTRunning())
			return NULL;
		
		switch (opType){
			case AsyncLookup:
				id = (DHTNetworkID*) arg1;
				dest = app->lookupFullPath(*id, searchPath, hopCount); break;

			case AsyncGetFinger:
				node = (Node*) arg1;
				nodes = app->getFingers(*node); break;

			case AsyncGetNeighbor:
			node = (Node*) arg1;
				nodes = app->getNeighbor(*node); break;

			case AsyncBootStrapAddNode:
				id = (DHTNetworkID*) arg1;
				appValue = (char*) arg2;
				node = (Node*) arg3;
				status = app->addNode(*id, appValue, *node); break;

			case AsyncNoBootStrapAddNode:
				id = (DHTNetworkID*) arg1;
				appValue = (char*) arg2;
				status = app->addNode(*id, appValue); break;

			case AsyncRemoveNode:
				id = (DHTNetworkID*) arg1;
				status = app->removeNode(*id); break;

			default:
				break;
		}		

		if (isDHTRunning())
			performAction();
		
		// free memory allocated
		switch (opType){
			case AsyncLookup:
			case AsyncGetFinger:
			case AsyncGetNeighbor:
			case AsyncRemoveNode:
				if (id != NULL)
					delete id;
				break;

			case AsyncBootStrapAddNode:
				if (id != NULL)
					delete id;
				if (appValue != NULL)
					delete []appValue;
				if (node != NULL)
					delete node;
				break;

			case AsyncNoBootStrapAddNode:
				if (id != NULL)
					delete id;
				if (appValue != NULL)
					delete []appValue;
				break;

			default:
				break;
			
			clear();
		}

		arg1 = NULL;
		arg2 = NULL;
		arg3 = NULL;

		return NULL;
	}

	void AsyncHandle::setLookupArg(const DHTNetworkID& _id){
		arg1 = new DHTNetworkID(_id);
		opType = AsyncLookup;
	}

	void AsyncHandle::setGetFingerArg(const Node& _target){
		arg1 = new Node(_target);
		opType = AsyncGetFinger;
	}

	void AsyncHandle::setGetNeighborArg(const Node& _target){
		arg1 = new Node(_target);
		opType = AsyncGetNeighbor;
	}

	void AsyncHandle::setAddNodeArg(const DHTNetworkID& _id, const char* _appValue, const Node* _bootstrap){
		arg1 = new DHTNetworkID(_id);
		arg2 = new char[APP_VALUE_LEN];
		memmove(arg2, _appValue, APP_VALUE_LEN);
		if (_bootstrap != NULL){
			arg3 = new Node(*_bootstrap);
			opType = AsyncBootStrapAddNode;
		}else{
			opType = AsyncNoBootStrapAddNode;
		}
	}

	void AsyncHandle::setRemoveNodeArg(const DHTNetworkID& _id){
		arg1 = new DHTNetworkID(_id);
		opType = AsyncRemoveNode;
	}

	void AsyncHandle::performAction(){
		if (dest != NULL)
			cout<<"Async lookup results: "<<*dest<<"."<<endl;
	}
	
	bool AsyncHandle::isDHTRunning(){
		if (app == NULL)
			return false;
		else return app->isDHTRunning();
	}
}
