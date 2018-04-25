#pragma once
#include "LookUpCommon.h"
#include <iostream>
#include <string>

class NetworkID
{
public:
	/** 
	 * @fn DHTNetworkID()
	 *	A constructor.
	 */
	NetworkID(void);
	virtual ~NetworkID(void);

	NetworkID(const unsigned char* _IDValue);

	NetworkID(const char* _hexIDValue);

	/** 
	 * @fn DHTNetworkID(const DHTNetworkID& _ID)
	 *	A copy constructor.
	 * @param _ID: the DHT Network ID to be copied.
	 */
	NetworkID(const NetworkID& _ID);

	//NetworkID(std::string ip, int port, int segment = INITIAL_SEGMENTID, std::string media = "00000000");
	NetworkID(std::string ip, int port, int segment = INITIAL_SEGMENTID, std::string media = "00000000",int p_identity=-1);

	int setIDHex(const char* _hexIDValue);

	///////////////////////////////////
	// Encoding
	///////////////////////////////////
	// encode the information into a hashed string
	//int setInfo(std::string hash, int segment, std::string ip, int port);
	int setInfo(std::string hash, int segment, std::string ip, int port, int p_identity);

	///////////////////////////////////
	// Decoding
	///////////////////////////////////
	std::string getIP(void) const;
	int getPort(void) const;
	int getSegment(void) const;
	std::string getMediaHash(void) const;

	int getPeerIdentity() const;

	bool isBootstrap() const;

	/** @fn friend ostream& operator<<(ostream& _out, const DHTNetworkID& _ID)
	 *	output stream operator for this class
	 * @param _out: the stream to output.
	 * @param _ID: the DHTNetworkID to be output.
	 */
	//friend std::ostream& operator<<(std::ostream& _out, const NetworkID& _ID);

protected:

	/**	@var IDValue
	 * Placeholder for storing  ID value.
	 */
	unsigned char IDValue[ID_LENGTH];
};
