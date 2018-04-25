#include "Node.h"


Node::Node(void)
{
	memset(this->IDValue, 0, ID_LENGTH);
}

Node::Node(const unsigned char* _IDValue){
	memmove(this->IDValue, _IDValue, ID_LENGTH);
}

Node::Node(const char* _hexIDValue){ 
	setIDHex(_hexIDValue);
}

//Node::Node(std::string hash, int segment, std::string ip, int port)
Node::Node(std::string hash, int segment, std::string ip, int port, int p_identity)
{
	for (int i=0; i<MEDIAINFOHASH_LENGTH; i++)
	{
		if (hash[2*i]<='9')
			IDValue[i] = hash[2*i] - '0';
		else
			IDValue[i] = hash[2*i] - 'a' + 10;
		IDValue[i] *= 16;
		if (hash[2*i+1]<='9')
			IDValue[i] += hash[2*i+1] - '0';
		else
			IDValue[i] += hash[2*i+1] - 'a' + 10;
	}

	IDValue[MEDIAINFOHASH_LENGTH] = segment / 256;
	IDValue[MEDIAINFOHASH_LENGTH + 1] = segment % 256;
	
	const char* temp = ip.c_str();
	int IP_array[COORDINATE_LENGTH];
	sscanf(temp, "%d %*c %d %*c %d %*c %d", &IP_array[0], &IP_array[1], &IP_array[2], &IP_array[3]);
	IDValue[MEDIAINFOHASH_LENGTH + 2] = IP_array[0];
	IDValue[MEDIAINFOHASH_LENGTH + 3] = IP_array[1];
	IDValue[MEDIAINFOHASH_LENGTH + 4] = IP_array[2];
	IDValue[MEDIAINFOHASH_LENGTH + 5] = IP_array[3];
	IDValue[MEDIAINFOHASH_LENGTH + 6] = port/256;
	IDValue[MEDIAINFOHASH_LENGTH + 7] = port%256;
	IDValue[MEDIAINFOHASH_LENGTH + 8] = p_identity;
}

Node::Node(const Node& _ID){
	memset(this->IDValue, 0, ID_LENGTH);
	memmove(this->IDValue, _ID.IDValue, ID_LENGTH);
}

Node::~Node(void)
{
}

int Node::setIDHex(const char* _hexIDValue){
	unsigned int value[ID_LENGTH];

	for (int i = 0; i < ID_LENGTH; i++){
		sscanf(_hexIDValue + i * 2, "%02x", &value[i]);
		this->IDValue[i] = value[i];
	}

	return 0;
}

//int Node::setInfo(std::string hash, int segment, std::string ip, int port)
int Node::setInfo(std::string hash, int segment, std::string ip, int port, int p_identity)
{
	for (int i=0; i<MEDIAINFOHASH_LENGTH; i++)
	{
		if (hash[2*i]<='9')
			IDValue[i] = hash[2*i] - '0';
		else
			IDValue[i] = hash[2*i] - 'a' + 10;
		IDValue[i] *= 16;
		if (hash[2*i+1]<='9')
			IDValue[i] += hash[2*i+1] - '0';
		else
			IDValue[i] += hash[2*i+1] - 'a' + 10;
	}

	IDValue[MEDIAINFOHASH_LENGTH] = segment / 256;
	IDValue[MEDIAINFOHASH_LENGTH + 1] = segment % 256;
	
	const char* temp = ip.c_str();
	int IP_array[COORDINATE_LENGTH];
	sscanf(temp, "%d %*c %d %*c %d %*c %d", &IP_array[0], &IP_array[1], &IP_array[2], &IP_array[3]);
	IDValue[MEDIAINFOHASH_LENGTH + 2] = IP_array[0];
	IDValue[MEDIAINFOHASH_LENGTH + 3] = IP_array[1];
	IDValue[MEDIAINFOHASH_LENGTH + 4] = IP_array[2];
	IDValue[MEDIAINFOHASH_LENGTH + 5] = IP_array[3];
	IDValue[MEDIAINFOHASH_LENGTH + 6] = port/256;
	IDValue[MEDIAINFOHASH_LENGTH + 7] = port%256;
	IDValue[MEDIAINFOHASH_LENGTH + 8] = p_identity;
	return 0;
}

std::string Node::genHashKey()
{
	char hexOutput[ID_LENGTH * 2 + 1];

	for (int i = 0; i < ID_LENGTH; i++)
		sprintf(hexOutput + i * 2, "%02x", (unsigned char) IDValue[i]);

	return std::string(hexOutput);
}

std::string Node::genAppKey()
{
	/*char hexOutput[ID_LENGTH * 2 + 1];

	for (int i = 0; i < ID_LENGTH; i++)
		sprintf(hexOutput + i * 2, "%02x", (unsigned char) IDValue[i]);

	hexOutput[MEDIAINFOHASH_LENGTH*2] = 'f';
	hexOutput[MEDIAINFOHASH_LENGTH*2 + 1] = 'f';
	hexOutput[MEDIAINFOHASH_LENGTH*2 + 2] = 'f';
	hexOutput[MEDIAINFOHASH_LENGTH*2 + 3] = 'f';

	return std::string(hexOutput);*/
	return getMediaHash();
}

std::string Node::getMediaHash()
{
	char temp[MEDIAINFOHASH_LENGTH*2+1];

	sprintf(temp, "%02x%02x%02x%02x", IDValue[0], IDValue[1], IDValue[2], IDValue[3]);
	
	return temp;
}

int Node::getSegment()
{
	return IDValue[MEDIAINFOHASH_LENGTH]*256 + IDValue[MEDIAINFOHASH_LENGTH + 1];
}

std::string Node::getIP()
{
	char temp[100];
	sprintf(temp, "%d.%d.%d.%d", IDValue[MEDIAINFOHASH_LENGTH + 2], IDValue[MEDIAINFOHASH_LENGTH + 3], IDValue[MEDIAINFOHASH_LENGTH + 4], IDValue[MEDIAINFOHASH_LENGTH + 5]);
	;
	return temp;
}

int Node::getPort()
{
	return IDValue[MEDIAINFOHASH_LENGTH + 6] * 256 + IDValue[MEDIAINFOHASH_LENGTH + 7];
}

void Node::setPeerIdentity(int _identity)
{
	peer_identity = _identity;
}

int Node::getPeerIdentity()
{
	return IDValue[MEDIAINFOHASH_LENGTH + 8];
}