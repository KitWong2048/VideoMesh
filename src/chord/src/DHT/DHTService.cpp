/*
DHTService.cpp

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

#include "../DHT/DHTService.h"
#include "../DHT/Chord.h"
#include "../DHT/AppValue.h"

using namespace DHTSocket;
using namespace Util;

namespace DHT{
	DHTService::DHTService(Chord& _chord, VirtualPoolSocket& _socket) : Thread(){
		this->chord = &_chord;
		this->log = chord->getLogger();
		this->targetID = NULL;
		this->notifyValue = NULL;
		this->answeringNode = NULL;
		this ->queryingNode = NULL;
		this->local = NULL;
		this->result = NULL;
		this->socket = &_socket;
	}

	DHTService::~DHTService(){
		if (targetID != NULL)
			delete targetID;
		if (notifyValue != NULL)
			delete []notifyValue;
		if (queryingNode != NULL)
			delete queryingNode;
		if (answeringNode != NULL)
			delete answeringNode;
		if (socket != NULL){
			socket->close();
			delete socket;
		}
	}

	void* DHTService::execute(){
		return serveRequest();
	}

	void* DHTService::serveRequest(){
		CharMessage inputBuf, buf;
		stringstream ss;

		// parse the request msg to obtain target ID, querying node and answering node
		inputBuf.len = socket->receive(inputBuf.body, chord->getTimeOutValue());
		if ((inputBuf.len <= 0) || (inputBuf.body == NULL)){
			log->writeLog("Chord: Service received error.\n", LogService);
			return NULL;
		}

		ss<<"Chord: Received request."<<endl;
		this ->queryingNode = new Node();
		this ->answeringNode = new Node();
		if (msgFactory.analyseDHTMsg(inputBuf, *this->queryingNode, *this->answeringNode, this->type, buf) < 0){
			this->type = DHTError;
			ss<<"Chord: Failed to parse the request message."<<endl;
		}else{
			// the message is successfully parsed
			// check for queryingNode
			if (queryingNode == NULL){
				type = DHTError;
				ss<<"Chord: Failed to parse the querying node from the message."<<endl;
			}
			// check for answering
			if (this->answeringNode == NULL){
				this->type = DHTError;
				ss<<"Chord: Failed to parse the answering node from the message."<<endl;
			}else{
				this->local = chord->getLocalNode(*answeringNode);
				// if there is no local node with ID as specied in the answering ID of the request msg AND the answering ID equals querying ID,
				// that means the node making request does not care whoever replies, so get someone to answer.
				if ((this->local == NULL) && (queryingNode != NULL))
						if (*queryingNode == *answeringNode){
							this->local = chord->getPrecedingLocalNode(*answeringNode);
							if (this->local != NULL){
								*answeringNode = this->local->getNode();
							}
						}

				if (this->local == NULL){
					this->type = DHTError;
					ss<<(DHTNetworkID) *answeringNode<<": Failed to get the local node object."<<endl;
				}

			}
			log->writeLog(ss.str(), LogDeepDebug);

			if (this->type == DHTReqClosestFinger){
				this->targetID = new DHTNetworkID();
				this->targetID->setID((unsigned char*)buf.body);
			}else if (this->type == DHTReqNotify){
				this->notifyValue = new char[buf.len];
				memmove(this->notifyValue, buf.body, buf.len);
			}
			this->type = msgFactory.toggleMsgType(this->type);
		}
		buf.clear();
		inputBuf.clear();

		switch (type){
			case DHTSrvSelf:
			case DHTSrvPred:
			case DHTSrvSucc:
			case DHTSrvClosestFinger:
				result = srvReturnSingle(); break;

			case DHTSrvPing:
			case DHTSrvNotify:
				result = srvReturnNone(); break;

			case DHTSrvAllFingers:
			case DHTSrvAllNeighbor:
				result = srvReturnMany(); break;

			default:
				result = NULL;
		}

		if (socket != NULL){
			socket->close();
			delete socket;
			socket = NULL;
		}

		if (this->local != NULL)
			this->local->subtractUseCount();

		return result;
	}

	bool DHTService::doSend(){
		CharMessage buf;
		bool status;

		buf = msgFactory.produceDHTMsg(*queryingNode, *answeringNode, type, &srvMsg);
		if (socket->send(buf.body, buf.len))
			status = true;
		else{
			log->writeLog(answeringNode->getIDHex() + " failed to send the service message.\n", LogDeepDebug);
			status = false;
		}

		srvMsg.clear();
		buf.clear();

		return status;
	}

	void* DHTService::srvReturnSingle(){
		stringstream ss;
		string s;
		Node* node;

		switch (type){
			case DHTSrvSelf:
				node = new Node(local->getNode()); break;

			case DHTSrvPred:
				node = local->getPredecessor(); break;

			case DHTSrvSucc:
				node = local->getSuccessor(); break;

			case DHTSrvClosestFinger:
				node = local->findClosestPredcedingFinger(*targetID); break;

			default:
				node = NULL;
		}


		if (node == NULL)
			node = new Node(*answeringNode);

		srvMsg.len = node->toBinFull(srvMsg.body);

		if (doSend()){
			ss.str("");
			switch (type){
				case DHTSrvSelf:
					ss<<(DHTNetworkID) *answeringNode<<" answered Self request with "<<*node<<endl;
					break;

				case DHTSrvPred:
					ss<<(DHTNetworkID) *answeringNode<<" answered Pred request with "<<*node<<endl;
					break;

				case DHTSrvSucc:
					ss<<(DHTNetworkID) *answeringNode<<" answered Succ request with "<<*node<<endl;
					break;

				case DHTSrvClosestFinger:
					ss<<(DHTNetworkID) *answeringNode<<" answered closest ID request with "<<*node<<endl;
					break;

				default:
					break;;
			}
			log->writeLog(ss.str(), LogService);
		}

		delete node;

		result = NULL;
		return result;
	}

	void* DHTService::srvReturnNone(){
		stringstream ss;

		if ((type == DHTSrvNotify) || (type == DHTSrvPing))
			if (doSend()){
				ss.str("");
				if (type == DHTSrvNotify){
					if (this->notifyValue != NULL){
						ss<<(DHTNetworkID) *answeringNode<<" was notified by "<<*queryingNode<<endl;
						queryingNode->fromBinApp(this->notifyValue);
						local->addReference(*queryingNode);
					}
				}else
					ss<<(DHTNetworkID) *answeringNode<<" was pinged by "<<*queryingNode<<endl;

				log->writeLog(ss.str(), LogService);
			}

		result = NULL;
		return result;
	}

	void* DHTService::srvReturnMany(){
		stringstream ss;
		Node **allNodes, *aNode;
		int i(0), maxNum(0);
		char* tmp;
		int tmpLen;

		switch (type){
			case DHTSrvAllFingers:
				allNodes = this->local->returnAllFingers();
				maxNum = MAX_FINGER_RETURN;
				break;

			case DHTSrvAllNeighbor:
				allNodes = this->local->returnAllNeighbor();
				maxNum = MAX_NEIGHBOR_RETURN;
				break;

			default:
				allNodes = NULL;
		}

		if (allNodes != NULL){
			aNode = allNodes[0];
			srvMsg.len = 0;
			srvMsg.body = new char[maxNum * NODE_LEN];
			while ((aNode != NULL) && (i < maxNum)){
				tmpLen = aNode->toBinFull(tmp);
				memmove(srvMsg.body + srvMsg.len, tmp, tmpLen);
				srvMsg.len += tmpLen;
				delete []tmp;
				delete aNode;
				i++;
				aNode = allNodes[i];
			}
			delete []allNodes;

			if (doSend()){
				ss.str("");
				if (type == DHTSrvAllFingers)
					ss<<(DHTNetworkID) *answeringNode<<" answered All Fingers request"<<endl;
				else ss<<(DHTNetworkID) *answeringNode<<" answered All Neighbor request"<<endl;
				log->writeLog(ss.str(), LogService);
			}
		}

		result = NULL;
		return result;
	}

}
