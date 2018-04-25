/*
MessageBoxSet.cpp

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

#include "../DHTSocket/MessageBoxSet.h"

using namespace Util;

namespace DHTSocket{

	MessageBoxSet::Box::Box(){
		msg = NULL;
		len = 0;
	}

	MessageBoxSet::Box::~Box(){
		boxMutex.lock();
		if (msg != NULL){
			delete []msg;
			msg = NULL;
		}
		boxMutex.release();
	}

	MessageBoxSet::MessageBoxSet(){
	}

	MessageBoxSet::~MessageBoxSet(){
		map<SessionReference, Box*>::iterator k, l;

		boxBuildingMutex.lock();
		k = boxes.begin();
		if (k != boxes.end()){
			l = k;
			do{
				l++;
				delete k->second;
				boxes.erase(k);
				k = l;
			}while (l != boxes.end());
		}
		boxBuildingMutex.release();

	}

	bool MessageBoxSet::isBoxExist(const SessionReference& _id){
		boxBuildingMutex.lock();

		if (boxes.find(_id) != boxes.end()){
			boxBuildingMutex.release();
			return true;
		}else{
			boxBuildingMutex.release();
			return false;
		}
	}

	int MessageBoxSet::createBox(const SessionReference& _id){
		Box* b;

		boxBuildingMutex.lock();
		if (boxes.find(_id) == boxes.end()){
			b = new Box();
			boxes.insert(make_pair(_id, b));
		}
		boxBuildingMutex.release();
		return 0;
	}

	int MessageBoxSet::destroyBox(const SessionReference& _id){
		boxBuildingMutex.lock();
		if (boxes.find(_id) != boxes.end()){
			delete boxes[_id];
			boxes.erase(_id);
		}
		boxBuildingMutex.release();
		return 0;
	}

	int MessageBoxSet::deliver(const SessionReference& _id, char* _buf, int _len){
		Box* aBox;
		char* tmp;

		if (isBoxExist(_id))
			createBox(_id);
		aBox = boxes[_id];
		aBox->boxMutex.lock();
		if (aBox->len + _len <= 0){
			aBox->msg = NULL;
			aBox->len = 0;
		}else{
			tmp = new char[aBox->len + _len];
			if (aBox->msg != NULL){
				memmove(tmp, aBox->msg, aBox->len);
				delete []aBox->msg;
			}
			memmove(tmp + aBox->len, _buf, _len);
			aBox->msg = tmp;
			aBox->len += _len;
		}
		#ifndef DHT_WIN32
			aBox->boxCondVar.boardcast();
		#endif
		aBox->boxMutex.release();

		return 0;
	}

	int MessageBoxSet::take(const SessionReference& _id, char*& _buf, int _timeout){
		Box* aBox;
		int len;

		if (isBoxExist(_id)){
			aBox = boxes[_id];
			aBox->boxMutex.lock();
			#ifdef DHT_WIN32
				int sleptTime = 0;
				aBox->boxMutex.release();
				while ((aBox->msg == NULL) && (sleptTime < 60)){					
					sleep(2);
					sleptTime += 2;
				}
				aBox->boxMutex.lock();
			#else
				while (aBox->msg == NULL){
					if (_timeout <= 0)
						aBox->boxCondVar.wait(aBox->boxMutex);
					else if (aBox->boxCondVar.timedWait(aBox->boxMutex, _timeout) < 0)
						break;
				}
			#endif
			_buf = aBox->msg;
			aBox->msg = NULL;
			len = aBox->len;
			aBox->len = 0;
			aBox->boxMutex.release();
		}else{
			_buf = NULL;
			len = 0;
		}

		return len;
	}

}
