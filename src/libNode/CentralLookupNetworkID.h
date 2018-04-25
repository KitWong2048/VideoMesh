#pragma once
#include "networkid.h"
#include <string>
#include "../chord/src/DHT/DHTNetworkID.h"

class CentralLookupNetworkID :
	public NetworkID
{
public:
	CentralLookupNetworkID(void);
	~CentralLookupNetworkID(void);

	CentralLookupNetworkID::CentralLookupNetworkID(std::string ip, int port, int segment, std::string media, int p_identity);

	CentralLookupNetworkID(const char* _hexIDValue);

	CentralLookupNetworkID(const CentralLookupNetworkID& _ID);

	virtual DHT::DHTNetworkID genDHTNetworkID();
};
