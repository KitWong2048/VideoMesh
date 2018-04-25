#include "../Node/VMeshLocalNode.h"

using namespace std;
using namespace DHT;

_VMESH_NAMESPACE_HEADER

VMeshLocalNode::VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, bool _isbootstrap, int p_identity)
{
	myIP = _ip;
	myPort = _port;
	ismeBootstrap = _isbootstrap;
	mySegmentID = BOOTSTRAP_SEGMENTID;
	myHash = myMediaInfo->getMediaHash();
	isINDHT = true;

	peer_identity = p_identity;
}

VMeshLocalNode::VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, int _segmentID, int p_identity)
{
	myIP = _ip;
	myPort = _port;
	ismeBootstrap = false;
	mySegmentID = _segmentID;
	myHash = myMediaInfo->getMediaHash();
	isINDHT = false;

	peer_identity = p_identity;
}

VMeshLocalNode::VMeshLocalNode(string _hash, string _ip, int _port, int _segmentID)
{
	myIP = _ip;
	myPort = _port;
	ismeBootstrap = false;
	mySegmentID = _segmentID;
	myHash = _hash;
	isINDHT = false;
}

VMeshLocalNode::~VMeshLocalNode()
{
}

string VMeshLocalNode::getIP() const
{
	return myIP;
}

int VMeshLocalNode::getPort() const
{
	return myPort;
}

int VMeshLocalNode::getSegmentID() const
{
	return mySegmentID;
}

int VMeshLocalNode::getPeerIdentity() const
{
	return peer_identity;
}

bool VMeshLocalNode::isBootstrap() const
{
	return ismeBootstrap;
}

DHTNetworkID VMeshLocalNode::genDHTNetworkID()
{
	unsigned char key[MEDIAINFOHASH_LENGTH + SEGMENTID_LENGTH + COORDINATE_LENGTH];	//13

	string _hash = "00000000";
	_hash = myHash;

	for (int i=0; i<2*MEDIAINFOHASH_LENGTH; i++)
		if (_hash[i]<='9')
			_hash[i] -= '0';
		else
			_hash[i] = _hash[i] - 'a' + 10;

	for (int i=0; i<MEDIAINFOHASH_LENGTH; i++)
		key[i] = _hash[2*i]*16 + _hash[2*i + 1];

	key[MEDIAINFOHASH_LENGTH] = mySegmentID / 256;
	key[MEDIAINFOHASH_LENGTH + 1] = mySegmentID % 256;
	
	const char* temp = myIP.c_str();
	int IP_array[COORDINATE_LENGTH];
	sscanf(temp, "%d %*c %d %*c %d %*c %d", &IP_array[0], &IP_array[1], &IP_array[2], &IP_array[3]);
	key[MEDIAINFOHASH_LENGTH + 2] = IP_array[0];
	key[MEDIAINFOHASH_LENGTH + 3] = IP_array[1];
	key[MEDIAINFOHASH_LENGTH + 4] = IP_array[2];
	key[MEDIAINFOHASH_LENGTH + 5] = IP_array[3];
	key[MEDIAINFOHASH_LENGTH + 6] = this->getPort()/256;
	key[MEDIAINFOHASH_LENGTH + 7] = this->getPort()%256;
	key[MEDIAINFOHASH_LENGTH + 8] = peer_identity;
	DHTNetworkID ID;
	ID.setID((unsigned char*)key);
	return ID;
}

char* VMeshLocalNode::genAppValue()
{
	//char* app = new char[6];
	char* app = new char[7];
	const char* temp = myIP.c_str();
	int IP_array[COORDINATE_LENGTH];
	sscanf(temp, "%d %*c %d %*c %d %*c %d", &IP_array[0], &IP_array[1], &IP_array[2], &IP_array[3]);
	app[0] = IP_array[0];
	app[1] = IP_array[1];
	app[2] = IP_array[2];
	app[3] = IP_array[3];
	app[4] = myPort/256;
	app[5] = myPort%256;
	app[6] = peer_identity;
	return app;
}

bool VMeshLocalNode::isDHTRegistered() const
{
	return isINDHT;
}

void VMeshLocalNode::setDHTRegistered()
{
	isINDHT = true;
}

_VMESH_NAMESPACE_FOOTER

