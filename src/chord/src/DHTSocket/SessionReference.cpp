/*
SessionReference.h

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

#include "../DHTSocket/SessionReference.h"

using namespace Util;

namespace DHTSocket{
	SessionReference::SessionReference(){
	}

	SessionReference::SessionReference(const InetAddress& _addr, unsigned long _seqNo){
		addr = _addr;
		seqNo = _seqNo;
	}

	SessionReference::~SessionReference(){
	}

	InetAddress SessionReference::getAddr() const{
		return addr;
	}

	unsigned long SessionReference::getSeqNo() const{
		return seqNo;
	}

	void SessionReference::setAddr(const InetAddress& _addr){
		addr = _addr;
	}

	void SessionReference::setSeqNo(unsigned long _seqNo){
		seqNo = _seqNo;
	}

	SessionReference& SessionReference::operator=(const SessionReference& _ref){
		addr = _ref.addr;
		seqNo = _ref.seqNo;

		return *this;
	}

	bool SessionReference::operator==(const SessionReference& _ref) const{
		if ((this->addr == _ref.addr) && (this->seqNo == _ref.seqNo))
			return true;
		else return false;
	}

	bool SessionReference::operator<(const SessionReference& _ref) const{
		if (this->addr < _ref.addr)
			return true;
		else if (this->addr == _ref.addr)
			if (this->seqNo < _ref.seqNo)
				return true;
			else return false;
		else return false;
	}

	int SessionReference::toBin(char*& _bin) const{
		char* tmp;
		int i;
		tmp = addr.getAddress();
		i = htonl(seqNo);
		_bin = new char[ADDRESS_LENGTH + SEQ_NO_LENGTH];
		memmove(_bin, tmp, ADDRESS_LENGTH);
		memmove(_bin + ADDRESS_LENGTH, &i, SEQ_NO_LENGTH);
		delete []tmp;
		return ADDRESS_LENGTH + SEQ_NO_LENGTH;
	}

	int SessionReference::fromBin(const char* _bin){
		char* tmp;
		tmp = new char[ADDRESS_LENGTH];
		memmove(tmp, _bin, ADDRESS_LENGTH);
		addr.setAddress(tmp);
		memmove(&seqNo, _bin + ADDRESS_LENGTH, SEQ_NO_LENGTH);
		seqNo = ntohl(seqNo);
		delete []tmp;
		return 0;
	}

	ostream& operator<<(ostream& _out, const SessionReference& _ref){
		_out<<_ref.addr.getIP()<<":"<<_ref.addr.getPort()<<"#"<<_ref.seqNo;
		return _out;
	}
}


