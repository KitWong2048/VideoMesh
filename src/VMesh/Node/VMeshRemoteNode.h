#ifndef _VMESH_REMOTE_NODE_H
#define _VMESH_REMOTE_NODE_H

#include "../Common/VMeshCommon.h"
#include "../Node/VMeshLocalNode.h"
#include "../Util/SocketClient.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

/**	@file VMeshRemoteNode.h
 *	This file contains the definition of class VMeshRemoteNode
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshRemoteNode
 *	This class provides the abstract of a node on a remote machine
 */
class VMeshRemoteNode
{
public:
	/**	@fn VMeshRemoteNode(string _ip, int _port, bool _isBootstrap)
	 *	Constructor for a bootstrap node;
	 *	@param _ip ip address
	 *	@param _port port number
	 *	@param _isBootstrap true if the node is a bootstrap node
	 */
	VMeshRemoteNode(string _ip, int _port, bool _isBootstrap, int p_identity = -1);

	/**	@fn VMeshRemoteNode(string _ip, int _port, int _SegmentID)
	 *	Constructor for a peer node;
	 *	@param _ip ip address
	 *	@param _port port number
	 *	@param _SegmentID the segment ID of segment the node holding
	 */
	VMeshRemoteNode(string _ip, int _port, int _SegmentID, int p_identity = -1);

	/**	@fn VMeshRemoteNode(const VMeshLocalNode& oldNode)
	 *	Copy constructor from a VMeshLocalNode, in which they represent the same node;
	 *	@param oldNode VMeshLocalNode to be copied
	 */
	VMeshRemoteNode(const VMeshLocalNode& oldNode);

	VMeshRemoteNode(const VMeshRemoteNode& oldNode);

	/**	@fn VMeshRemoteNode(const DHT::Node& dhtNode)
	 *	Copy constructor from a DHT Node, in which they represent the same node;
	 *	@param dhtNode DHT Node to be copied
	 */
	VMeshRemoteNode(const DHT::Node& dhtNode);
	
	/**	@fn VMeshRemoteNode(const char* _serial, int _length)
	 *	Constructor using serialized char array;
	 *	@param _serial serialized char array
	 *	@param _length length of the array
	 */
	VMeshRemoteNode(const char* _serial, int _length);
	
	/**	@fn ~VMeshRemoteNode()
	 *	Default destructor;
	 */
	~VMeshRemoteNode();

	/**	@fn operator==(const VMeshRemoteNode& node) const
	 *	Compare the VMeshRemoteNode by their IP address, port and segment ID;
	 *	@return true if both VMeshRemoteNode has the same IP address, port and segment ID
	 */
	bool operator==(const VMeshRemoteNode& node) const;

	/**	@fn serialize(int& _length)
	 *	Creates a serialized char array for this object;
	 *	@param _length length of the array
	 *	@return serialized char array
	 */
	char* serialize(int& _length);

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

	/**	@fn setSegmentID(int segID)
	 *	set the segment ID, but must be used carefully;
	 *	@param segID the segment ID
	 */

	void setSegmentID(int segID);
	

	void setPingTime(int _myPingTime);

	/**	@fn isBootstrap() const
	 *	check if it is a bootstrap node;
	 *	@return true if it is a bootstrap node
	 */
	bool isBootstrap() const;

	/**	@fn getPingTime() const
	 *	get the ping time to this node (but no ping is done);
	 *	@return the ping time
	 */
	int getPingTime() const;

	int getControlSocketPingTime() const;
	/**	@fn getSocketClient()
	 *	get the SocketClient that can communicate to this node;
	 *	the SocketClient must be created by calling connect() before;
	 *	@return the SocketClient that can communicate to this node
	 *	@return NULL if previous connect() is failed
	 */
	SocketClient* getSocketClient();

	SocketClient* getControlSocketClient();
	/**	@fn ping()
	 *	send ping request to this node, and record the ping time;
	 *	the SocketClient must be created by calling connect() before;
	 *	@return the SocketClient that can communicate to this node
	 *	@return true if ping succeeds
	 *	@return false if ping lost of timeout or socket error
	 */
	bool ping();
	
	bool pingControlSocket();

	void closeControlSocket();
	/**	@fn toString(bool fullInfo = true)
	 *	get a string representation of this node;
	 *	@return string representation of this node
	 */
	std::string toString(bool fullInfo = true);

	/**	@fn connect()
	 *	esablish a connection to this node;
	 *	destruct the current SocketClient if exists and connect again;
	 *	once connecton is established, succeeding call of isSocketUsing() will return false;
	 *	@return true if the node is connected, false otherwise
	 */
	bool connect();

	bool connectControlSocket();
	/**	@fn isSocketUsing()
	 *	check if the SocketClient of this node is using already;
	 *	once a successful connecton is established, succeeding call of isSocketUsing() will return false;
	 *	@return true if the SocketClient of this node is using, false otherwise
	 */
	bool isSocketUsing();

	/**	@fn setSocketUsing()
	 *	set to use the SocketClient, which is mutex protected;
	 *	@return true if the socket can be set successfully
	 */
	bool setSocketUsing();

	/**	@fn clrSocketUsing()
	 *	clear to use the SocketClient, which is mutex protected;
	 */
	void clrSocketUsing();

	void setDHTRegistered();

	void addAccessCount();

	void reduceAccessCount();

	/** @fn getAccessCount()
	 *	get access count to check conection counter
	 */
	long getAccessCount();

	bool isRemovable();

private:
	string myIP;

	int myPort;

	int mySegmentID;

	// check if this node is a boostrap node
	bool ismeBootstrap;

	// represent the RTT of the remote node
	// will be used in sorting purpose
	int myPingTime;

	int ControlPingTime;
	SocketClient* mySocketClient;

	//
	SocketClient* ControlSocketClient;

	bool usingSocket;

	Mutex socketgrant;

	long accessCount;

	int peer_identity;
};
_VMESH_NAMESPACE_FOOTER
#endif

