/*
VirtualPoolSocket.cpp

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

#include "../DHTSocket/VirtualPoolSocket.h"
#include "../DHTSocket/PoolSocketMgr.h"

using namespace Util;

namespace DHTSocket{

	VirtualPoolSocket::VirtualPoolSocket(unsigned long _poolRef, const SessionReference& _sessionRef, VirtualPoolSocketType _socketType, PoolSocketMgr* _mgr){
		socketType = _socketType;
		poolRef = _poolRef;
		sessionRef = _sessionRef;
		mgr = _mgr;
		isSocketDead = false;
	}

	VirtualPoolSocket::~VirtualPoolSocket(){
		if (!isSocketDead)
			this->close();
	}

	void VirtualPoolSocket::close(){
		mgr->close(*this);
		isSocketDead = true;
	}

	int VirtualPoolSocket::send(const char* _msg, int _len) const{
		if (!isSocketDead)
			return mgr->send(*this, _msg, _len);
		else return -1;
	}

	int VirtualPoolSocket::receive(char* & _msg, int _timeout) const{
		if (!isSocketDead)
			return mgr->receive(*this, _msg, _timeout);
		else return -1;
	}

	unsigned long VirtualPoolSocket::getPoolRef() const{
		return poolRef;
	}

}
