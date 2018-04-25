#pragma once
#include "networkid.h"
#include <string>
#include "../chord/src/DHT/DHTNetworkID.h"

class TableExchangeLookupNetworkID :
	public NetworkID
{
public:
	TableExchangeLookupNetworkID(void);
	~TableExchangeLookupNetworkID(void);

	TableExchangeLookupNetworkID::TableExchangeLookupNetworkID(std::string ip, int port, int segment, std::string media);

	TableExchangeLookupNetworkID(const char* _hexIDValue);

	TableExchangeLookupNetworkID(const TableExchangeLookupNetworkID& _ID);

	virtual DHT::DHTNetworkID genDHTNetworkID();
};
