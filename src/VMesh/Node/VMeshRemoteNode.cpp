#include "../Node/VMeshRemoteNode.h"

using namespace std;
using DHT::Node;
using DHT::AbstractNodeValue;

_VMESH_NAMESPACE_HEADER

VMeshRemoteNode::VMeshRemoteNode(string _ip, int _port, bool _isBootstrap, int p_identity)
: socketgrant("SocketGrant")
{
	myIP = _ip;
	myPort = _port;
	mySegmentID = BOOTSTRAP_SEGMENTID;
	ismeBootstrap = _isBootstrap;
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;

	peer_identity = p_identity;
}


VMeshRemoteNode::VMeshRemoteNode(string _ip, int _port, int _SegmentID, int p_identity)
: socketgrant("SocketGrant")
{
	myIP = _ip;
	myPort = _port;
	mySegmentID = _SegmentID;
	ismeBootstrap = (mySegmentID == BOOTSTRAP_SEGMENTID);
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;

	peer_identity = p_identity;
}

VMeshRemoteNode::VMeshRemoteNode(const VMeshLocalNode& oldNode)
: socketgrant("SocketGrant")
{
	myIP = oldNode.getIP();
	myPort = oldNode.getPort();
	mySegmentID = oldNode.getSegmentID();
	ismeBootstrap = oldNode.isBootstrap();
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;

	peer_identity = oldNode.getPeerIdentity();
}

VMeshRemoteNode::VMeshRemoteNode(const VMeshRemoteNode& oldNode)
: socketgrant("SocketGrant")
{
	myIP = oldNode.getIP();
	myPort = oldNode.getPort();
	mySegmentID = oldNode.getSegmentID();
	ismeBootstrap = oldNode.isBootstrap();
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;

	peer_identity = oldNode.getPeerIdentity();
}



VMeshRemoteNode::VMeshRemoteNode(const Node& dhtNode)
: socketgrant("SocketGrant")
{
	char* appValue = NULL;

	AbstractNodeValue* abstractAppValue = dhtNode.getAppValue();
	if (abstractAppValue->toBin(appValue)!=6)
	{
		LOG_ERROR("VMeshRemoteNode:\t AppValue of Node must be of length 6");	
	}
	delete abstractAppValue;

	char tempIP[20];
	sprintf(tempIP, "%d.%d.%d.%d", (unsigned char)appValue[0], (unsigned char)appValue[1], (unsigned char)appValue[2], (unsigned char)appValue[3]);
	myIP = (string)(tempIP);
	myPort = 256*(int)(unsigned char)appValue[4] + (int)(unsigned char)appValue[5];
	
	delete [] appValue;

	// retrieve the Segment ID from DHT ID
	char* DHTid = dhtNode.getNodeID().toBin();
	mySegmentID = 256*(int)(unsigned char)DHTid[4] + (int)(unsigned char)DHTid[5];
	delete [] DHTid;

	ismeBootstrap = (mySegmentID == BOOTSTRAP_SEGMENTID);
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;
}

VMeshRemoteNode::VMeshRemoteNode(const char* _serial, int _length)
: socketgrant("SocketGrant")
{
	//if ((_length!=9)||(_serial[8]!=(_serial[0]^_serial[1]^_serial[2]^_serial[3]^_serial[4]^_serial[5]^_serial[6]^_serial[7])))

	// Jack's comment::add identity entire to VMeshRemoteNode
	if ((_length!=10)||(_serial[9]!=(_serial[0]^_serial[1]^_serial[2]^_serial[3]^_serial[4]^_serial[5]^_serial[6]^_serial[7]^_serial[8])))
	{
		//checksum failed
		myIP = "";
		myPort = 99999;
		mySegmentID = INITIAL_SEGMENTID;
		ismeBootstrap = false;
		myPingTime = PING_FAIL;
		ControlPingTime=PING_FAIL;
		mySocketClient = NULL;
		ControlSocketClient = NULL;
		usingSocket = false;
		peer_identity = -1;	// Jack's comment:: unknown indentity type
		return;
	}
	int ipint[4];
	for (int i=0; i<4; ++i)
		ipint[i] = (unsigned char) _serial[i];

	char buffer[50];
	sprintf(buffer, "%d.%d.%d.%d", ipint[0], ipint[1], ipint[2], ipint[3]);
	myIP = buffer;

	myPort = ((int)(unsigned char)(_serial[4])) * 256 + ((int)(unsigned char)(_serial[5]));
	mySegmentID = ((int)(unsigned char)(_serial[6])) * 256 + ((int)(unsigned char)(_serial[7]));
	peer_identity = ((int)(unsigned char)(_serial[8]));
	ismeBootstrap = (mySegmentID==BOOTSTRAP_SEGMENTID);
	
	
	
	myPingTime = PING_NOT_READY;
	ControlPingTime=PING_NOT_READY;
	mySocketClient = NULL;
	ControlSocketClient = NULL;
	usingSocket = false;
	accessCount = 0;
}


VMeshRemoteNode::~VMeshRemoteNode()
{
	if (mySocketClient!=NULL){
		delete mySocketClient;
		mySocketClient = NULL;
	}

	if(ControlSocketClient!=NULL)
	{
		delete ControlSocketClient;
		ControlSocketClient=NULL;
	}
}

bool VMeshRemoteNode::operator==(const VMeshRemoteNode& node) const
{
	if ((this->getIP() == node.getIP()) && (this->getPort() == node.getPort()) && (this->getSegmentID() == node.getSegmentID()))
		return true;
	else
		return false;
}

char* VMeshRemoteNode::serialize(int& _length)
{
	char buffer[100];
	const char* temp = myIP.c_str();
	int ipint[4];
	sscanf(temp, "%d%*c%d%*c%d%*c%d", &ipint[0], &ipint[1], &ipint[2], &ipint[3]);
	
	// Jack's comment:: Original Code
	//_length = sprintf(buffer, "%c%c%c%c%c%c%c%c ", ipint[0], ipint[1], ipint[2], ipint[3], (char)(myPort/256), (char)(myPort%256), (char)(mySegmentID/256), (char)(mySegmentID%256));

	// Jack's comment:: add identity entire to VMeshRemoteNode
	_length = sprintf(buffer, "%c%c%c%c%c%c%c%c%c ", ipint[0], ipint[1], ipint[2], ipint[3], (char)(myPort/256), (char)(myPort%256), (char)(mySegmentID/256), (char)(mySegmentID%256), (char)peer_identity);

	// Jack's comment:: Original Code
	// checksum byte
	//buffer[8] = buffer[0]^buffer[1]^buffer[2]^buffer[3]^buffer[4]^buffer[5]^buffer[6]^buffer[7];

	// Jack's comment:: add one more entire to VMeshRemoteNode
	buffer[9] = buffer[0]^buffer[1]^buffer[2]^buffer[3]^buffer[4]^buffer[5]^buffer[6]^buffer[7]^buffer[8];

	char* buffer2 = new char[_length];
	memcpy(buffer2, buffer, _length);
	return buffer2;
}

string VMeshRemoteNode::getIP() const
{
	return myIP;
}

int VMeshRemoteNode::getPort() const
{
	return myPort;
}

int VMeshRemoteNode::getSegmentID() const
{
	return mySegmentID;
}

int VMeshRemoteNode::getPeerIdentity() const
{
	return peer_identity;
}

void VMeshRemoteNode::setSegmentID(int segID)
{
	mySegmentID = segID;
}

void VMeshRemoteNode::setPingTime(int _myPingTime)
{
	myPingTime=_myPingTime;
}

bool VMeshRemoteNode::isBootstrap() const
{
	return ismeBootstrap;
}

int VMeshRemoteNode::getPingTime() const
{
	return myPingTime;
}

int VMeshRemoteNode::getControlSocketPingTime() const
{
	return ControlPingTime;
}

SocketClient* VMeshRemoteNode::getSocketClient()
{
	return mySocketClient;
}

SocketClient* VMeshRemoteNode::getControlSocketClient()
{
	return ControlSocketClient;
}

bool VMeshRemoteNode::pingControlSocket()
{
	/*
	if (!setSocketUsing())
		return true;
	*/

	if (ControlSocketClient==NULL){
		ControlPingTime = PING_FAIL;
		return false;
	}

	char msg1[1];
	msg1[0] = (char)PING;
	char* msg2 = 0;

	struct timeval x, y, z;

	// ping
	int state = 0;
	bzero(&x, sizeof(x));

	gettimeofday(&x, NULL);
	
	if (ControlSocketClient->send(msg1, 1))
	{
		STAT_AGGREGATE(UP_CONTROL, 1);
		state = ControlSocketClient->receive(msg2, VMESH_PING_TIMEOUT);
		STAT_AGGREGATE(DOWN_CONTROL, state);
	}
	else
		state = 0;
	
	bzero(&y, sizeof(y));
	gettimeofday(&y, NULL);
	
	if (y.tv_usec < x.tv_usec){
		z.tv_usec = 1000000 + y.tv_usec - x.tv_usec;
		z.tv_sec = y.tv_sec - x.tv_sec - 1;
	}else{
		z.tv_usec = y.tv_usec - x.tv_usec;
		z.tv_sec = y.tv_sec - x.tv_sec;
	}

	// check ping result
	if (state==-1)
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping timed out");
		// timeout
		ControlPingTime = PING_TIMEOUT;
		return false;
	}
	else if (state==0 || msg2 == NULL )
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping failed");
		// socket error
		/*if (ControlSocketClient!=NULL){
			delete ControlSocketClient;
			ControlSocketClient = NULL;
		}*/
		ControlPingTime = PING_FAIL;
		return false;
	}
	else if ((state == 1) && (msg2[0]==(char)RPY_PING))
	{
		// ping succeed
		if (msg2 != 0){
			delete [] msg2;
			msg2 = 0;
		}
		ControlPingTime = ( z.tv_sec * 1000 + z.tv_usec / 1000 )/2;
		//LOG_DEBUG((string)"PingTime: " + itos(myPingTime));
		return true;
	}
	else
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping reply is unknown");
		// unknown reply
		if (msg2 != 0){
			delete [] msg2;
			msg2 = 0;
		}
		ControlPingTime = PING_FAIL;
		return false;
	}
}

bool VMeshRemoteNode::ping()
{
	/*
	if (!setSocketUsing())
		return true;
	*/

	if (mySocketClient==NULL){
		myPingTime = PING_FAIL;
		return false;
	}

	char msg1[1];
	msg1[0] = (char)PING;
	char* msg2 = 0;

	struct timeval x, y, z;

	// ping
	int state = 0;
	bzero(&x, sizeof(x));

	gettimeofday(&x, NULL);
	
	if (mySocketClient->send(msg1, 1))
	{
		STAT_AGGREGATE(UP_CONTROL, 1);
		state = mySocketClient->receive(msg2, VMESH_PING_TIMEOUT);
		STAT_AGGREGATE(DOWN_CONTROL, state);
	}
	else
		state = 0;
	
	bzero(&y, sizeof(y));
	gettimeofday(&y, NULL);
	
	if (y.tv_usec < x.tv_usec){
		z.tv_usec = 1000000 + y.tv_usec - x.tv_usec;
		z.tv_sec = y.tv_sec - x.tv_sec - 1;
	}else{
		z.tv_usec = y.tv_usec - x.tv_usec;
		z.tv_sec = y.tv_sec - x.tv_sec;
	}

	// check ping result
	if (state==-1)
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping timed out");
		// timeout
		myPingTime = PING_TIMEOUT;
		return false;
	}
	else if (state==0 || msg2 == NULL )
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping failed");
		// socket error
		if (mySocketClient!=NULL){
			delete mySocketClient;
			mySocketClient = NULL;
		}
		myPingTime = PING_FAIL;
		return false;
	}
	else if ((state == 1) && (msg2[0]==(char)RPY_PING))
	{
		// ping succeed
		if (msg2 != 0){
			delete [] msg2;
			msg2 = 0;
		}
		myPingTime = ( z.tv_sec * 1000 + z.tv_usec / 1000 )/2;
		//LOG_DEBUG((string)"PingTime: " + itos(myPingTime));
		return true;
	}
	else
	{
		LOG_DEBUG("VMeshRemoteNode\t: Ping reply is unknown");
		// unknown reply
		if (msg2 != 0){
			delete [] msg2;
			msg2 = 0;
		}
		myPingTime = PING_FAIL;
		return false;
	}
}



void VMeshRemoteNode::closeControlSocket()
{
	if(ControlSocketClient!=NULL)
	{
		delete ControlSocketClient;
		ControlSocketClient=NULL;
	}
}

string VMeshRemoteNode::toString(bool fullInfo)
{
	string info;
	if (fullInfo)
        info = (string)"IP: ";
	info = info + getIP() + ":" + itos(getPort()) + "::" + itos(getPeerIdentity());

	info = info + " SEG: ";
	if (getSegmentID()==INITIAL_SEGMENTID)
		info = info + "IS";
	else if (getSegmentID()==BOOTSTRAP_SEGMENTID)
		info = info + "BS";
	else
		info = info + itos(getSegmentID());

	if (fullInfo)
	{
		info = info + "  ";
		if (getControlSocketPingTime()==PING_NOT_READY)
			info = info + "PING: NR ";
		else if (getControlSocketPingTime()==PING_TIMEOUT)
			info = info + "PING: TO ";
		else if (getControlSocketPingTime()==PING_FAIL)
			info = info + "PING: FAIL ";
		else
			info = info + "PING: " + itos(getControlSocketPingTime()) + " ";
		info = info + " BUSY: " + (isSocketUsing()?"yes":"no");
	}

	return info;
}

bool VMeshRemoteNode::connect()
{
	try
	{
		if (mySocketClient!=NULL)
		{
			delete mySocketClient;
			mySocketClient = NULL;	
		}
		
		SocketClient* cli = NULL;
		cli = new SocketClient(NULL, 0);
		if ((cli != NULL) && cli->connect(myIP.c_str(), myPort, VMESH_CONNECT_TIMEOUT))
		{
			LOG_DEBUG((string)"VMeshRemoteNode\t: Connect to " + myIP + ":" + itos(myPort) + " succeeds");
			mySocketClient = cli;
			return true;
		}
		else
		{
			LOG_DEBUG((string)"VMeshRemoteNode\t: Connect to " + myIP + ":" + itos(myPort) + " fails");
			mySocketClient = NULL;
			myPingTime = PING_FAIL;
			if (cli != NULL){
				delete cli;
				cli = NULL;
			}
			return false;
		}
	} catch(...)
	{
		LOG_DEBUG("VMeshRemoteNode\t: Exception catched in connect() of " + this->getIP() + ":" + itos(this->getPort()));
		return false;
	}
}

bool VMeshRemoteNode::connectControlSocket()
{
	try
	{
		if (ControlSocketClient!=NULL)
		{
			delete ControlSocketClient;
			ControlSocketClient = NULL;	
		}
		
		SocketClient* cli = NULL;
		cli = new SocketClient(NULL, 0);
		if ((cli != NULL) && cli->connect(myIP.c_str(), myPort, VMESH_CONNECT_TIMEOUT))
		{
			LOG_DEBUG((string)"VMeshRemoteNode\t: Connect to " + myIP + ":" + itos(myPort) + " succeeds");
			ControlSocketClient = cli;
			return true;
		}
		else
		{
			LOG_DEBUG((string)"VMeshRemoteNode\t: Connect to " + myIP + ":" + itos(myPort) + " fails");
			ControlSocketClient = NULL;
			myPingTime = PING_FAIL;
			if (cli != NULL){
				delete cli;
				cli = NULL;
			}
			return false;
		}
	} catch(...)
	{
		LOG_DEBUG("VMeshRemoteNode\t: Exception catched in connect() of " + this->getIP() + ":" + itos(this->getPort()));
		return false;
	}

}

bool VMeshRemoteNode::isSocketUsing()
{
	return usingSocket;
}

bool VMeshRemoteNode::setSocketUsing()
{
	socketgrant.lock();
	bool set_success = !usingSocket;
	usingSocket = true;
	//LOG_DEBUG("setSocketUsing() Set succeeds? " + itos(set_success));
	socketgrant.release();
	return set_success;
}

void VMeshRemoteNode::clrSocketUsing()
{
	socketgrant.lock();
	//LOG_DEBUG("clrSocketUsing() using? " + itos(usingSocket));
	usingSocket = false;
	socketgrant.release();
}

void VMeshRemoteNode::addAccessCount()
{
#ifdef _VMESH_WIN32_
	InterlockedIncrement(&accessCount);
#else
	accessCount++;
#endif
}

void VMeshRemoteNode::reduceAccessCount()
{
#ifdef _VMESH_WIN32_
	InterlockedDecrement(&accessCount);
#else
	accessCount--;
#endif
}

// Get access count for checking the conection counter
long VMeshRemoteNode::getAccessCount()
{
	return accessCount;
}

bool VMeshRemoteNode::isRemovable()
{
	if (accessCount > 0)
		return false;
	else
		return true;
}

_VMESH_NAMESPACE_FOOTER
