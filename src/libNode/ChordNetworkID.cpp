#include "ChordNetworkID.h"

using namespace DHT;
using namespace std;

ChordNetworkID::ChordNetworkID(void){
}

ChordNetworkID::~ChordNetworkID(void)
{
}

ChordNetworkID::ChordNetworkID(std::string ip, int port, int segment, std::string media) : NetworkID(ip, port, segment, media)
{
}

ChordNetworkID::ChordNetworkID(const ChordNetworkID& _ID) : NetworkID(_ID)
{
}

DHTNetworkID ChordNetworkID::genDHTNetworkID()
{
	DHTNetworkID ID;
	ID.setID((const unsigned char*)IDValue);
	return ID;
}
