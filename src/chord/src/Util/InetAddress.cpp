/*
InetAddress.cpp

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

#include "../Util/InetAddress.h"

namespace Util{
	InetAddress::InetAddress(){
		bzero(&IP.s_addr, sizeof(IP.s_addr));
		port = 0;
	}

	InetAddress::InetAddress(const char* _IP, int _port){
		setIP(_IP);
		setPort(_port);
	}

	InetAddress::InetAddress(const InetAddress& _inetaddress){
		memmove(&this->IP, &_inetaddress.IP, sizeof(in_addr));
		this->port = _inetaddress.port;
	}

	InetAddress& InetAddress::operator=(const InetAddress& _inetaddress){
		memmove(&this->IP, &_inetaddress.IP, sizeof(in_addr));
		this->port = _inetaddress.port;
		return *this;
	}

	bool InetAddress::operator==(const InetAddress& _inetaddress) const{
		if ( (!memcmp(&IP.s_addr, &_inetaddress.IP.s_addr, sizeof(IP.s_addr))) && (port == _inetaddress.port) )
			return true;
		else return false;
	}

	bool InetAddress::operator<(const InetAddress& _inetaddress) const{
		int flagIP = memcmp(&IP.s_addr, &_inetaddress.IP.s_addr, sizeof(IP.s_addr));

		if (flagIP < 0)
			return true;
		else if (flagIP == 0)
			if (port < _inetaddress.port)
				return true;
			else return false;
		else return false;
	}

	char* InetAddress::getAddress() const{
		char* value = new char[ADDRESS_LENGTH];
		unsigned short tmp;
		tmp = htons(port);
		memmove(value, &IP, IP_LENGTH);
		memmove(value + IP_LENGTH, &tmp, PORT_LENGHTH);
		return  value;
	}

	void InetAddress::setAddress(const char* _address){
		unsigned short tmp;
		memmove(&IP, _address, IP_LENGTH);
		memmove(&tmp, _address + IP_LENGTH, PORT_LENGHTH);
		port = ntohs(tmp);
	}

	void InetAddress::setIP(const char* _IP){
		IP.s_addr = ::inet_addr(_IP);
	}

	void InetAddress::setPort(int _port){
		port = _port;
	}

	string InetAddress::getIP() const{
		return string(::inet_ntoa(IP));
	}

	int InetAddress::getPort() const{
		return port;
	}
}
