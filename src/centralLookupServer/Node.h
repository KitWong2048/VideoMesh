#pragma once
#include "Common.h"
#include <string>

class Node
{
public:
	Node(void);

	/* from Bin */
	Node(const unsigned char* _IDValue);

	/* from Hex */
	Node(const char* _hexIDValue);

	Node(std::string hash, int segment, std::string ip, int port, int p_identity);

	Node(const Node& _ID);

	~Node(void);

	int setIDHex(const char* _hexIDValue);

	int setInfo(std::string hash, int segment, std::string ip, int port, int p_identity);

	/**add identity to node's info
	 * @param _identity: identify each node identity(eg: server, proxy, or client)
	 */
	void setPeerIdentity(int _identity);
	
	/**get identity from Node
     */
	int getPeerIdentity();

	std::string genHashKey();
	std::string genAppKey();
	int getPort();
	std::string getIP();
	int getSegment();
	std::string getMediaHash();

private:
	unsigned char IDValue[ID_LENGTH];
	int peer_identity;

};
