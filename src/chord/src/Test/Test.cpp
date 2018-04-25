/*
test.cpp

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

#include "chord/src/Test/Test.h"
#define NUM_NODE 15

void socketTest(int argc){
	SocketClient* clt;

	if (argc > 1){
		clt = new SocketClient(NULL, -1);
		if (clt->connect("143.89.145.83", 34567, -1)){
			cout<<"Connected to server."<<endl;
			clt->close();
		}else cout<<"Failed to connect to server."<<endl;
	}else{
		SocketServer* srv;
		srv = new SocketServer("143.89.145.83", 34567);
		if (srv->ready()){
			cout<<"Listening..."<<endl;
			while (true){
			clt = srv->accept();
				if (clt != NULL){
					cout<<"A client is accepted"<<endl;
					cout<<"Remote IP = "<<clt->getRemoteIP()<<endl;
					cout<<"Remote Port = "<<clt->getRemotePort()<<endl;
					clt->close();
				}else cout<<"Failed to accept a client"<<endl;
			}
		}else
			cout<<"Failed to listen"<<endl;

		srv->close();
	}
}

void idTestID(){
		DHTNetworkID id1, id2;
		id1.setIDHex("01234567");
		id2.setIDHex("03030303");
		cout<<"id1 = "<<id1<<endl;
		cout<<"id2 = "<<id2<<endl;
		cout<<"id1 - id2 = "<<id1 - id2<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(0)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(1)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(2)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(3)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(4)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(5)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(6)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(7)<<endl;
		cout<<"id2.addPower() = "<<id2.addPower(8)<<endl;
}

void fingersTest(){
	ChordAttr attr;
	Chord chord(NULL, attr);
	Node n(DHTNetworkID("bb"), "127.0.0.1", 1234);
	LocalNodeHandle local(n, chord);
	FingerTable* fingers;
	fingers = new FingerTable(n, chord);

	fingers->print(LogNormal);
	cout<<"adding cd"<<endl;
	fingers->addFinger(Node(DHTNetworkID("cd"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"removing cd"<<endl;
	fingers->removeFinger(DHTNetworkID("cd"));
	fingers->print(LogNormal);

	cout<<"adding 2f"<<endl;
	fingers->addFinger(Node(DHTNetworkID("2f"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"adding 89"<<endl;
	fingers->addFinger(Node(DHTNetworkID("89"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"adding 9b"<<endl;
	fingers->addFinger(Node(DHTNetworkID("9b"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"adding bb"<<endl;
	fingers->addFinger(Node(DHTNetworkID("bb"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"adding 89"<<endl;
	fingers->addFinger(Node(DHTNetworkID("89"), "127.0.0.1", 1234));
	fingers->print(LogNormal);

	cout<<"removing 89"<<endl;
	fingers->removeFinger(DHTNetworkID("89"));
	fingers->print(LogNormal);
	cout<<"adding 9b"<<endl;
	fingers->addFinger(Node(DHTNetworkID("9b"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"adding 89"<<endl;
	fingers->addFinger(Node(DHTNetworkID("89"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"removing 2f"<<endl;
	fingers->removeFinger(DHTNetworkID("2f"));
	fingers->print(LogNormal);
	cout<<"adding 2f"<<endl;
	fingers->addFinger(Node(DHTNetworkID("2f"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"removing cd"<<endl;
	fingers->removeFinger(DHTNetworkID("cd"));
	fingers->print(LogNormal);
	cout<<"adding cd"<<endl;
	fingers->addFinger(Node(DHTNetworkID("cd"), "127.0.0.1", 1234));
	fingers->print(LogNormal);
	cout<<"removing 2f"<<endl;
	fingers->removeFinger(DHTNetworkID("2f"));
	fingers->print(LogNormal);
	cout<<"removing cd"<<endl;
	fingers->removeFinger(DHTNetworkID("cd"));
	fingers->print(LogNormal);
	cout<<"removing 89"<<endl;
	fingers->removeFinger(DHTNetworkID("89"));
	fingers->print(LogNormal);
	delete fingers;
}

void miscTest(){
	char* sha1Value;

	sha1Value = sha1HashHex("abc");
	cout<<sha1Value<<endl;
	delete []sha1Value;
	sha1Value = sha1HashHex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
	cout<<sha1Value<<endl;
	delete []sha1Value;
	sha1Value = sha1HashHex(NULL);
	cout<<sha1Value<<endl;
	delete []sha1Value;
//    DHTMessageFactory factory;
//
//    Node x(DHTNetworkID("1234567890123456789012345678ABCDEFABCDEF"), "127.211.234.1", 1997);
//    Node y(DHTNetworkID("1234567890ABCDEF1234567890ABCDEF12345678"), "123.52.28.11", 35782);
//    Node a(DHTNetworkID("0000000000000000000000000000000000000000"), "0.0.0.0", 0);
//    Node b(DHTNetworkID("0000000000000000000000000000000000000000"), "0.0.0.0", 0);
//    CharMessage msgOutput, msgInput;
//	CharMessage result;
//	DHTMsgType type = DHTReqPing;
//	char* k;
//
//	cout<<"x = "<<x<<endl;
//	cout<<"y = "<<y<<endl;
//
//	x.toBin(k);
//	a.fromBin(k);
//	cout<<a<<endl;
//	delete []k;
//
//
//	msgInput.len = 10;
//	msgInput.body = new char[msgInput.len];
//	memset(msgInput.body, 'a', msgInput.len);
//	msgOutput = factory.produceDHTMsg(x, y, DHTReqAllFingers, &msgInput);
//	factory.analyseDHTMsg(msgOutput, a, b, type, result);
//	k = new char[result.len + 1];
//	memmove(k, result.body, result.len);
//	cout<<"a = "<<a<<endl;
//	cout<<"b = "<<b<<endl;
//	cout<<"type = "<<factory.translateTypeToMsg(type)<<endl;
//	k[result.len] = '\0';
//	cout<<k<<endl;
//	for (int i = 0; i < result.len; i++)
//		cout<<"k["<<i<<"] = "<<k[i]<<endl;
//
//	return 0;

//	DHTNetworkID target("ba5d2db2408e1681967941526a54301716f01a6c");
//	DHTNetworkID x("ba5d2db2408e1681967941526a54301716f01a6c");
//	DHTNetworkID y("fcbc1b9256b67b5ab1f3c421064e5dcff78a794c");
//	if (target.isInInterval(x, y, false, true))
//		cout<<"true"<<endl;
//	else cout<<"false"<<endl;
//	return 0;

}


