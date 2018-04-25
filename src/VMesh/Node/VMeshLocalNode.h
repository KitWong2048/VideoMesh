#ifndef _VMESH_LOCAL_NODE_H
#define _VMESH_LOCAL_NODE_H

#include "../Common/VMeshCommon.h"
#include "../Common/MediaInfo.h"
#include "../Util/Logger.h"

/**	@file VMeshLocalNode.h
 *	This file contains the definition of class VMeshLocalNode
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshLocalNode
 *	This class provides the abstract of a node on local machine
 *  Each segment in this case is considered as a "node"
 */
class VMeshLocalNode
{
public:
	/**	@fn VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, bool _isbootstrap = true)
	 *	Constructor for a bootstrap node;
	 *	@param myMediaInfo the media info associated with
	 *	@param _ip ip address
	 *	@param _port port number
	 *	@param _isbootstrap true if the node is a bootstrap node
	 */
	VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, bool _isbootstrap = true, int p_identity = -1);
	
	/**	@fn VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, int _segmentID)
	 *	Constructor for a peer node;
	 *	@param myMediaInfo the media info associated with
	 *	@param _ip ip address
	 *	@param _port port number
	 *	@param _segmentID the segment ID of segment the node holding
	 */
	VMeshLocalNode(MediaInfo* myMediaInfo, string _ip, int _port, int _segmentID, int p_identity = -1);
	
	/**	@fn VMeshLocalNode(string _hash, string _ip, int _port, int _segmentID)
	 *	Constructor for a peer node (instead of MediaInfo, only the media hash is needed);
	 *	@param _hash the media hash of the movie associated with
	 *	@param _ip ip address
	 *	@param _port port number
	 *	@param _segmentID the segment ID of segment the node holding
	 */
	VMeshLocalNode(string _hash, string _ip, int _port, int _segmentID);

	/**	@fn ~VMeshLocalNode()
	 *	Default destructor;
	 */
	~VMeshLocalNode();

	/**	@fn getIP() const
	 *	get the ip address;
	 *	@return the ip adress
	 */
	string getIP() const;

	/**	@fn getPort() const
	 *	get the port number;
	 *	@return the port number
	 */
	int getPort() const;

	/**	@fn getSegmentID() const
	 *	get the segment ID;
	 *	@return the segment ID
	 */
	int getSegmentID() const;

	/** @fn getPeerIdentity() const
	 * get the peer identity
	 * @return the peer identity
	 */
	int getPeerIdentity() const;

	/**	@fn isBootstrap() const
	 *	check if it is a bootstrap node;
	 *	@return true if it is a bootstrap node
	 */
	bool isBootstrap() const;

	/**	@fn genDHTNetworkID()
	 *	generate the DHTNetworkID used by chord;
	 *	@return the DHTNetworkID of the node
	 */
	DHT::DHTNetworkID genDHTNetworkID();

	/**	@fn genAppValue()
	 *	generate and allocate memory for the AppValue of Chord when adding a key;
	 *	the AppValue is 6 bytes, contains the IP and port for this VMeshLocalNode;
	 *	@return the AppValue of the node
	 */
	char* genAppValue();

	/**	@fn isDHTRegistered() const
	 *	check if the node is registered in DHT;
	 *	oririnally the node is not registered;
	 *	@return true if the node is registered in DHT
	 */
	bool isDHTRegistered() const;
	
	/**	@fn setDHTRegistered()
	 *	set the node to be registered in DHT;
	 */
	void setDHTRegistered();
	
private:
	string myIP;
	int myPort;
	int peer_identity;
	bool ismeBootstrap;
	int mySegmentID; 
	string myHash;
	bool isINDHT;

};
_VMESH_NAMESPACE_FOOTER
#endif
