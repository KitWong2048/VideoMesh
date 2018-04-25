#pragma once
#include "networkid.h"
#include <string>
#include "../chord/src/DHT/DHTNetworkID.h"

class ChordNetworkID :
	public NetworkID
{
public:
	ChordNetworkID(void);
	ChordNetworkID(std::string ip, int port, int segment, std::string media);
	~ChordNetworkID(void);

	ChordNetworkID(const ChordNetworkID& _ID);

	virtual DHT::DHTNetworkID genDHTNetworkID();
};
