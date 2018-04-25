#include "TableExchangeLookupNetworkID.h"

using namespace DHT;
using namespace std;

TableExchangeLookupNetworkID::TableExchangeLookupNetworkID(void)
{
}

TableExchangeLookupNetworkID::~TableExchangeLookupNetworkID(void)
{
}

TableExchangeLookupNetworkID::TableExchangeLookupNetworkID(const TableExchangeLookupNetworkID& _ID) : NetworkID(_ID)
{
}

TableExchangeLookupNetworkID::TableExchangeLookupNetworkID(std::string ip, int port, int segment, std::string media) : NetworkID(ip, port, segment, media)
{
}

TableExchangeLookupNetworkID::TableExchangeLookupNetworkID(const char* _hexIDValue) :NetworkID(_hexIDValue)
{
}

DHTNetworkID TableExchangeLookupNetworkID::genDHTNetworkID()
{
	DHTNetworkID ID;
	ID.setID((const unsigned char*)IDValue);
	return ID;
}
