
#include "CentralLookupNetworkID.h"

using namespace DHT;
using namespace std;

CentralLookupNetworkID::CentralLookupNetworkID(void){
}

CentralLookupNetworkID::~CentralLookupNetworkID(void)
{
}

CentralLookupNetworkID::CentralLookupNetworkID(const CentralLookupNetworkID& _ID) : NetworkID(_ID)
{
}

//CentralLookupNetworkID::CentralLookupNetworkID(std::string ip, int port, int segment, std::string media) : NetworkID(ip, port, segment, media)
CentralLookupNetworkID::CentralLookupNetworkID(std::string ip, int port, int segment, std::string media, int p_identity) : NetworkID(ip, port, segment, media, p_identity)
{
}

CentralLookupNetworkID::CentralLookupNetworkID(const char* _hexIDValue) :NetworkID(_hexIDValue)
{
}

DHTNetworkID CentralLookupNetworkID::genDHTNetworkID()
{
	DHTNetworkID ID;
	ID.setID((const unsigned char*)IDValue);
	return ID;
}
