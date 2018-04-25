/*
DHTRequest.cpp

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

#include "../DHT/DHTRequest.h"
#include "../DHT/LocalNodeHandle.h"
#include "../DHT/Chord.h"

using namespace DHTSocket;
using namespace Util;

namespace DHT{
	DHTRequest::DHTRequest(DHTMsgType _type, Chord& _chord,
		const Node* _queryingNode, const Node* _answeringNode, const DHTNetworkID* _targetID) : Thread(){

		this->type = _type;
		this->result = NULL;
		this->socket = NULL;
		this->chord = &_chord;
		this->log = chord->getLogger();
		isAnsweringNodeFail = false;

		if (_targetID != NULL)
			this->targetID = new DHTNetworkID(*_targetID);
		else this->targetID = NULL;

		if (_queryingNode != NULL)
			this->queryingNode = new Node(*_queryingNode);
		else this->queryingNode = NULL;

		if (_answeringNode != NULL){
			this->answeringNode = new Node(*_answeringNode);
			this->localAnsweringNode = chord->getLocalNode(*(this->answeringNode));
		}else{
			this->answeringNode = NULL;
			this->localAnsweringNode  = NULL;
		}

	}

	DHTRequest::~DHTRequest(){
		if (this->targetID != NULL)
			delete this->targetID;

		if (this->queryingNode != NULL)
			delete this->queryingNode;

		if (this->answeringNode != NULL)
			delete this->answeringNode;

		if (this->socket != NULL){
			this->socket->close();
			delete this->socket;
		}
	}

	void* DHTRequest::execute(){
		return makeRequest();
	}

	void* DHTRequest::makeRequest(){
		stringstream ss;
		bool isConnected = false;
		PoolSocketMgr* mgr;


		if ((this->queryingNode == NULL) || (this->answeringNode == NULL)){
			return NULL;
		}
		// connect to the remote answering node
		if (this->localAnsweringNode == NULL){
		    mgr = chord->getSocketPoolMgr();
		    if (mgr != NULL)
                socket = mgr->connect(answeringNode->getNodeIP().c_str(), answeringNode->getNodePort(), chord->getTimeOutValue(), *chord);
            else socket = NULL;

			if (socket != NULL){
				isConnected = true;
				ss<<(DHTNetworkID) *queryingNode<<": Connected to "<<*answeringNode<<" for "<<msgFactory.translateTypeToMsg(this->type)<<endl;
			}else{
				isConnected = false;
				ss<<(DHTNetworkID) *queryingNode<<": Failed to connect to "<<*answeringNode<<" for "<<msgFactory.translateTypeToMsg(this->type)<<endl;
				result = NULL;
				isAnsweringNodeFail = true;
			}
			log->writeLog(ss.str(), LogDebug);
		}
		// make request if the remote node is connected or the answering node is local
		if ((isConnected) || (this->localAnsweringNode != NULL)){
			switch (type){
				case DHTReqSelf:
				case DHTReqPred:
				case DHTReqSucc:
				case DHTReqClosestFinger:
					result = reqReturnSingle(); break;

				case DHTReqPing:
				case DHTReqNotify:
					result = reqReturnNone(); break;

				case DHTReqAllFingers:
				case DHTReqAllNeighbor:
					result = reqReturnMany(); break;

				default:
					result = NULL;
			}
		}

		if (isConnected){
			this->socket->close();
			delete this->socket;
			this->socket = NULL;
		}

		if (isAnsweringNodeFail){
			chord->removeReference(*queryingNode, *answeringNode);
		}

		if (this->localAnsweringNode != NULL)
			this->localAnsweringNode->subtractUseCount();

		return result;
	}

	CharMessage DHTRequest::doSendRecv(const CharMessage* _reqMsg){
		stringstream ss;
		CharMessage buf, analyzedBuf;
		Node rQueryingNode, rAnsweringNode;
		DHTMsgType rType;
		int byteSent;

		if (socket == NULL)
			return analyzedBuf;

		buf = msgFactory.produceDHTMsg(*queryingNode, *answeringNode, type, _reqMsg);

		byteSent = socket->send(buf.body, buf.len);
		if (byteSent != buf.len){
			buf.clear();
			ss<<(DHTNetworkID) *queryingNode<<": Failed to send the request message to "<<(DHTNetworkID) *answeringNode<<endl;
		}else{
			buf.clear();
			buf.len = socket->receive(buf.body, chord->getTimeOutValue());
			if (( buf.len <= 0 ) || (buf.body == NULL)){
				ss<<(DHTNetworkID) *queryingNode<<": Failed to receive the service message from "<<(DHTNetworkID) *answeringNode<<endl;
				isAnsweringNodeFail = true;
			}else{
				if (msgFactory.analyseDHTMsg(buf, rQueryingNode, rAnsweringNode, rType, analyzedBuf) != 0){
					ss<<(DHTNetworkID) *queryingNode<<": Failed to parse the service message"<<endl;
					analyzedBuf.clear();
				}else if (analyzedBuf.body == NULL){
					// the buffer is correctly analyzed but there is no additional message in the body so we put an empty string into it
					analyzedBuf.constructStr("");
				}
			}
			buf.clear();
		}

		log->writeLog(ss.str(), LogRequest);
		return analyzedBuf;
	}

	Node* DHTRequest::reqReturnSingle(){
		Node* resultNode = NULL;
		CharMessage buf, reqMsg;
		stringstream ss;

		// send the message to remote node; or check if there is a local node which can help
		switch (type){
			case DHTReqSelf:
				if (localAnsweringNode != NULL)
					resultNode = new Node(localAnsweringNode->getNode());
				else buf = doSendRecv();
				break;

			case DHTReqPred:
				if (localAnsweringNode != NULL)
					resultNode = localAnsweringNode->getPredecessor();
				else buf = doSendRecv();
				break;

			case DHTReqSucc:
				if (localAnsweringNode != NULL)
					resultNode = localAnsweringNode->getSuccessor();
				else buf = doSendRecv();
				break;

			case DHTReqClosestFinger:
				if (localAnsweringNode != NULL){
					resultNode = localAnsweringNode->findClosestPredcedingFinger(*targetID);
				}else{
					reqMsg.body = (char*) targetID->toBin();
					reqMsg.len = ID_LENGTH;
					buf = doSendRecv(&reqMsg);
					reqMsg.clear();
				}
				break;

			default:
				break;
		}

		// receive the message and return
		if ((buf.body != NULL) && (resultNode == NULL)){
			if (buf.len == NODE_LEN){
				resultNode = new Node();
				resultNode->fromBinFull(buf.body);
			}else{
				ss<<(DHTNetworkID) *queryingNode<<": Incorrect service message length ("<<buf.len<<")"<<endl;
				log->writeLog(ss.str(), LogError);
			}
		}
		buf.clear();
		result = resultNode;

		return (Node*) result;
	}

	int* DHTRequest::reqReturnNone(){
		int* isAlive;
		CharMessage buf, reqMsg;

		if ((*queryingNode == *answeringNode) || (localAnsweringNode == NULL)){
			if (type == DHTReqNotify){
				reqMsg.len = queryingNode->toBinApp(reqMsg.body);
				buf = doSendRecv(&reqMsg);
				reqMsg.clear();
			}else buf = doSendRecv();
			if (buf.body != NULL){
				isAlive = new int;
				*isAlive = 0;
				buf.clear();
			}else isAlive = NULL;
		}else if (localAnsweringNode != NULL){
			isAlive = new int;
			*isAlive = 0;
		}

		result = isAlive;

		return (int*) result;
	}

	Node** DHTRequest::reqReturnMany(){
		CharMessage buf;
		Node **allFingers, aFinger;
		int i, nodeSize;
		int charPos;

		// send the message to remote node; or check if there is a local node which can help
		switch (type){
			case DHTReqAllFingers:
				if (localAnsweringNode != NULL)
					allFingers = localAnsweringNode->returnAllFingers();
				else{
					allFingers = NULL;
					buf = doSendRecv();
					nodeSize = ID_BIT_LENGTH;
				}
				break;

			case DHTReqAllNeighbor:
				if (localAnsweringNode != NULL){
					allFingers = localAnsweringNode->returnAllNeighbor();
				}else{
					allFingers = NULL;
					buf = doSendRecv();
					nodeSize = chord->getNeighborhoodSize();
				}
				break;

			default:
				allFingers = NULL;
				nodeSize = 0;
		}

		// receive the message and return
		if ((allFingers == NULL) && (buf.body != NULL)){
			// parse the msg main body to get the list of fingers
			allFingers = new Node*[ID_BIT_LENGTH];
			i = 0;
			charPos = 0;
			while ((charPos + NODE_LEN <= buf.len - 1) && (i < ID_BIT_LENGTH)) {
				if (aFinger.fromBinFull(buf.body + charPos) == 0){
					allFingers[i] = new Node(aFinger);
					i++;
				}
				charPos += NODE_LEN;
			}
			for (; i < ID_BIT_LENGTH; i++)
				allFingers[i] = NULL;
		}

		buf.clear();
		result = allFingers;

		return (Node**) result;
	}

}
