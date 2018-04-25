/*
DHTMessageFactory.cpp

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

#include "../DHT/DHTMessageFactory.h"

using namespace Util;

namespace DHT{
	CharMessage::CharMessage(){
		body = NULL;
		len = 0;
	}

	CharMessage::~CharMessage(){
		// we do not de allocate the memory assigned to body, application should clear memory explicitly
	}

	void CharMessage::constructStr(const string& _str){
		len = _str.size() + 1;
		body = new char[len];
		strcpy(body, _str.c_str());
		body[len - 1] = '\0';
	}

	void CharMessage::clear(){
		if (body != NULL){
			delete []body;
			body = NULL;
		}
		len = 0;
	}

	DHTMessageFactory::DHTMessageFactory(){
	}

	DHTMessageFactory::~DHTMessageFactory(){
	}

	CharMessage DHTMessageFactory::produceDHTMsg(const Node& _src, const Node& _dest, DHTMsgType _type, const CharMessage* _msgBody){
		CharMessage output;
		char* tmp;
		int tmpLen;
		short int tmpType;

		if (_msgBody != NULL){
			output.body = new char[2 * (ID_LENGTH + ADDRESS_LENGTH) + DHT_MSG_TYPE_LENGTH + _msgBody->len];
		}else{
			output.body = new char[2 * (ID_LENGTH + ADDRESS_LENGTH) + DHT_MSG_TYPE_LENGTH];
		}
		// copy source node
		tmpLen = _src.toBin(tmp);
		memmove(output.body, tmp, tmpLen);
		output.len = tmpLen;
		delete []tmp;

		// copy dest node
		tmpLen = _dest.toBin(tmp);
		memmove(output.body + output.len, tmp, tmpLen);
		output.len += tmpLen;
		delete []tmp;

		// copy message type
		tmpType = _type;
		tmpType = htons(tmpType);
		memmove(output.body + output.len, &tmpType, DHT_MSG_TYPE_LENGTH);
		output.len += DHT_MSG_TYPE_LENGTH;

		// copy message body
		if (_msgBody != NULL)
			if (_msgBody->len > 0){
				memmove(output.body + output.len, _msgBody->body, _msgBody->len);
				output.len += _msgBody->len;
			}

		return output;
	}

	int DHTMessageFactory::analyseDHTMsg(const CharMessage& _msg, Node& _src, Node& _dest, DHTMsgType& _type, CharMessage& _msgBody){
		int pos;
		short int tmpType;

		_src.fromBin(_msg.body);
		pos = ID_LENGTH + ADDRESS_LENGTH;

		_dest.fromBin(_msg.body + pos);
		pos += ID_LENGTH + ADDRESS_LENGTH;

		memmove(&tmpType, _msg.body + pos, DHT_MSG_TYPE_LENGTH);
		tmpType = ntohs(tmpType);
		if ((tmpType >= DHT_MSG_TYPE_MIN) && (tmpType <= DHT_MSG_TYPE_MAX))
			_type = (DHTMsgType) tmpType;
		else _type = DHTError;
		pos += DHT_MSG_TYPE_LENGTH;

		if (_msg.len > pos){
			_msgBody.len = _msg.len - pos;
			_msgBody.body = new char[_msgBody.len];
			memmove(_msgBody.body, _msg.body + pos, _msgBody.len);
		}else{
			_msgBody.len = 0;
			_msgBody.body = NULL;
		}

		return 0;
	}

	DHTMsgType DHTMessageFactory::translateMsgToType(const char* _msg){
		DHTMsgType x;

		if (!strcmp(_msg, MSG_SRV_PING))
			x = DHTSrvPing;
		else if (!strcmp(_msg, MSG_SRV_PRED))
			x = DHTSrvPred;
		else if (!strcmp(_msg, MSG_SRV_SUCC))
			x = DHTSrvSucc;
		else if (!strcmp(_msg, MSG_SRV_CLOSEST_FINGER))
			x = DHTSrvClosestFinger;
		else if (!strcmp(_msg, MSG_SRV_NOTIFY))
			x = DHTSrvNotify;
		else	if (!strcmp(_msg, MSG_REQ_PING))
			x = DHTReqPing;
		else if (!strcmp(_msg, MSG_REQ_PRED))
			x = DHTReqPred;
		else if (!strcmp(_msg, MSG_REQ_SUCC))
			x = DHTReqSucc;
		else if (!strcmp(_msg, MSG_REQ_CLOSEST_FINGER))
			x = DHTReqClosestFinger;
		else if (!strcmp(_msg, MSG_REQ_NOTIFY))
			x = DHTReqNotify;
		else if (!strcmp(_msg, MSG_REQ_ALL_FINGER))
			x = DHTReqAllFingers;
		else if (!strcmp(_msg, MSG_SRV_ALL_FINGER))
			x = DHTSrvAllFingers;
		else if (!strcmp(_msg, MSG_REQ_ALL_NEIGHBOR))
			x = DHTReqAllNeighbor;
		else if (!strcmp(_msg, MSG_SRV_ALL_NEIGHBOR))
			x = DHTSrvAllNeighbor;

		else x = DHTError;

		return x;
	}

	string DHTMessageFactory::translateTypeToMsg(DHTMsgType _type){
		string y;

		switch (_type){
			case DHTSrvPing:
				y = MSG_SRV_PING; break;

			case DHTSrvPred:
				y = MSG_SRV_PRED; break;

			case DHTSrvSucc:
				y = MSG_SRV_SUCC; break;

			case DHTSrvClosestFinger:
				y = MSG_SRV_CLOSEST_FINGER; break;

			case DHTSrvNotify:
				y = MSG_SRV_NOTIFY; break;

			case DHTSrvAllFingers:
				y = MSG_SRV_ALL_FINGER; break;

			case DHTSrvAllNeighbor:
				y = MSG_SRV_ALL_NEIGHBOR; break;

			case DHTSrvSelf:
				y = MSG_SRV_SELF; break;

			case DHTReqPing:
				y = MSG_REQ_PING; break;

			case DHTReqPred:
				y = MSG_REQ_PRED; break;

			case DHTReqSucc:
				y = MSG_REQ_SUCC; break;

			case DHTReqClosestFinger:
				y = MSG_REQ_CLOSEST_FINGER; break;

			case DHTReqNotify:
				y = MSG_REQ_NOTIFY; break;

			case DHTReqAllFingers:
				y = MSG_REQ_ALL_FINGER; break;

			case DHTReqAllNeighbor:
				y = MSG_REQ_ALL_NEIGHBOR; break;

			case DHTReqSelf:
				y = MSG_REQ_SELF; break;

			case DHTError:
				y = MSG_ERROR; break;

			default:
				y = MSG_ERROR;
		}

		return y;
	}

	DHTMsgType DHTMessageFactory::toggleMsgType(DHTMsgType _type){
		DHTMsgType y;

		switch (_type){
			case DHTSrvAllFingers:
				y = DHTReqAllFingers; break;

			case DHTSrvAllNeighbor:
				y = DHTReqAllNeighbor; break;

			case DHTSrvPing:
				y = DHTReqPing; break;

			case DHTSrvPred:
				y = DHTReqPred; break;

			case DHTSrvSucc:
				y = DHTReqSucc; break;

			case DHTSrvClosestFinger:
				y = DHTReqClosestFinger; break;

			case DHTSrvNotify:
				y = DHTReqNotify; break;

			case DHTReqAllFingers:
				y = DHTSrvAllFingers; break;

			case DHTReqAllNeighbor:
				y = DHTSrvAllNeighbor; break;

			case DHTReqPing:
				y = DHTSrvPing; break;

			case DHTReqPred:
				y = DHTSrvPred; break;

			case DHTReqSucc:
				y = DHTSrvSucc; break;

			case DHTReqClosestFinger:
				y = DHTSrvClosestFinger; break;

			case DHTReqNotify:
				y = DHTSrvNotify; break;

			case DHTReqSelf:
				y = DHTSrvSelf; break;

			case DHTSrvSelf:
				y = DHTReqSelf; break;

			default:
				y = _type;
		}

		return y;
	}

}
