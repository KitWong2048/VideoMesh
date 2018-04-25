/*
Node.cpp

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

#include "../DHT/Node.h"
#include "../DHT/AppValue.h"

using namespace Util;

namespace DHT{

	Node::Node() : DHTNetworkID(), address(){
		appValue = NULL;
	}

	Node::Node(const DHTNetworkID& _nodeID) : DHTNetworkID(_nodeID), address(){
		appValue = NULL;
	}

	Node::Node(const DHTNetworkID& _nodeID, const char* _IP, int _port) : DHTNetworkID(_nodeID), address(_IP, _port){
		appValue = NULL;
	}

	Node::Node(const Node& _node) : DHTNetworkID(_node){
		this->appValue = NULL;
		*this = _node;
	}

	Node::~Node(){
		if (appValue != NULL){
			delete appValue;
		}
	}

	DHTNetworkID Node::getNodeID() const{
		return (DHTNetworkID) *this;
	}

	int Node::setNodeID(const DHTNetworkID& _nodeID){
		DHTNetworkID::operator=(_nodeID);

		return 0;
	}

	string Node::getNodeIP() const{
		return address.getIP();
	}

	int Node::setNodeIP(const char* _IP){
		address.setIP(_IP);
		return 0;
	}

	int Node::getNodePort() const{
		return address.getPort();
	}

	int Node::setNodePort(int _port){
		address.setPort(_port);
		return 0;
	}

	int Node::toBin(char*& _output) const{
		char* binAddr;
		_output = new char[ID_LENGTH + ADDRESS_LENGTH];

		binAddr = address.getAddress();
		memmove(_output, IDValue, ID_LENGTH);
		memmove(_output + ID_LENGTH, binAddr, ADDRESS_LENGTH);
		delete []binAddr;

		return ID_LENGTH + ADDRESS_LENGTH;
	}

	int Node::fromBin(const char* _input){
		setID((unsigned char*)_input);
		address.setAddress(_input + ID_LENGTH);
		return 0;
	}

	int Node::toBinApp(char*& _output) const{
		if (appValue != NULL)
			return appValue->toBin(_output);
		else{
			_output = NULL;
			return 0;
		}
	}

	int Node::fromBinApp(const char* _input){
		if (appValue == NULL){
			appValue = new AppValue();
		}
		return appValue->fromBin(_input);
	}

	int Node::toBinFull(char*& _output) const{
		char* tmp;

		_output = new char[NODE_LEN];
		memset(_output, 0, NODE_LEN);
		toBin(tmp);
		memmove(_output, tmp, ID_LENGTH + ADDRESS_LENGTH);
		delete []tmp;
		if (appValue != NULL){
			appValue->toBin(tmp);
			memmove(_output + ID_LENGTH + ADDRESS_LENGTH, tmp, APP_VALUE_LEN);
			delete []tmp;
		}
		return NODE_LEN;
	}

	int Node::fromBinFull(const char* _input){
		fromBin(_input);
		if (appValue == NULL){
			appValue = new AppValue();
		}
		return appValue->fromBin(_input + ID_LENGTH + ADDRESS_LENGTH);
	}

	bool Node::isIdentical(const Node& _node){
		if (this->address == _node.address)
			return DHTNetworkID::operator==(_node);
		else return false;
	}

	bool Node::operator==(const Node& _node) const{
		return DHTNetworkID::operator==(_node);
	}

	bool Node::operator<(const Node& _node) const{
		if (getNodeID() < _node.getNodeID())
			return true;
		else
			return false;
	}

	Node& Node::operator=(const Node& _node){
		DHTNetworkID::operator=(_node);
		this->address = _node.address;
		if (_node.appValue == NULL){
			if (this->appValue != NULL){
				delete this->appValue;
				this->appValue = NULL;
			}
		}else{
			if (this->appValue == NULL)
				this->appValue = new AppValue();
			*(this->appValue) = *(_node.appValue);
		}

		return *this;
	}

	AbstractNodeValue* Node::getAppValue() const{
		AppValue* value;

		if (appValue == NULL)
			value = NULL;
		else{
			value = new AppValue();
			*value = *appValue;
		}

		return value;
	}

	int Node::setNodeFrmStr(const string& _msg){
		string::size_type index = 0;
		string::size_type nextIndex;

		// format of the string is [ID]@[IP]:[Port]
		// find starting position of IP
		nextIndex = _msg.find('@', index);
		if (nextIndex == string::npos)
			return -1;
		DHTNetworkID::setIDHex(_msg.substr(0, nextIndex).c_str());

		// fidn starting position of port
		index = nextIndex;
		nextIndex = _msg.find(':', index);
		if (nextIndex == string::npos)
			return -1;
		address.setIP((_msg.substr(index + 1, nextIndex - index - 1)).c_str());
		address.setPort(atoi(_msg.substr(nextIndex + 1).c_str()));

		return 0;
	}

	ostream& operator<<(ostream& _out, const Node& _node){
		_out<<(DHTNetworkID) _node<<"@"<<_node.address.getIP()<<":"<<_node.address.getPort();
		if (_node.appValue != NULL)
			_out<<"("<<_node.appValue->toStr()<<")";
		return _out;
	}
}
