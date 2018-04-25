/**	
* VMesh 
*
* definition of the VMesh Peer-to-Peer Network
* 
* @author Kelvin Chan, Philip Yang
*/


#ifndef _VMESH_H
#define _VMESH_H

#include "../Common/VMeshCommon.h"
#include "../Common/Publisher.h"
#include "../Common/Packet.h"
#include "../Common/MediaInfo.h"
#include "../Node/VMeshLocalNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Util/CircularBuffer.h"
#include "../Storage/Storage.h"
#include "../Storage/BootstrapStorage.h"
#include "../Storage/PeerStorage.h"
#include "../Thread/RequestListener.h"
#include "../Thread/StreamRequest.h"
#include "../Util/SocketClient.h"
#include "../Util/SocketServer.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"
#include "../Util/ExchangeTable.h"

// include Vivaldi Project
//#include "../../Vivaldi/Common/VivaldiCommon.h"
//#include "../../Vivaldi/Thread/VivaldiRequest.h"

_VMESH_NAMESPACE_HEADER

/**	@enum ConnectState
 *	An enum to store different state in the connection of a peer
 */
enum ConnectState
{
	DISCONNECTED,	/**< disconnected */
	UL,				/**< uploading to this peer */
	DL,				/**< downloading to this peer */
	BOTH,			/**< uploading and downloading to this peer */
	MYSELF			/**< this peer is local */
};

/**	@struct PeerInfo
 *	A struct to store a peer's connection information
 */
struct PeerInfo
{
	std::string ip;			/**< IP address */
	int port;				/**< port number */
	bool isBootstrap;		/**< true if the node is bootstrap, false otherwise */
	int segmentID;			/**< segment ID */
	int pingTime;			/**< ping time, RTT distance */
	int isConnected;		/**< ConnectState */
};

/**	@typedef PeerInfoList
 *	A list of PeerInfo
 */
typedef std::list<PeerInfo> PeerInfoList;

/**	@class VMesh::VMesh
 *	This class provides the mechanism for coordination within VMesh
 */
class VMesh 
{
	friend class VMeshServer;
	friend class VMeshClient;

	friend class VMeshProxy;		//add by jack 25-11-2010
	
private:
	/**	@fn VMesh (string _myIP, int _myPort)
	 *	Private constructor so that it can be constructed only by the friend class
	 *	@param _myIP the local IP address (may use getHostIP() function in Utility.h)
	 *	@param _myPort port number to be listened by VMesh (_myPort + 1 will be occupied by DHT)
	 *  @param _localStorageCapacity number of segments stored by each peer
	 */
	VMesh(string _myIP, int _myPort, int _localStorageCapacity = -1, int _identity = -1, int _exchangeNeighbor = -1);

public:
	/**	@fn ~VMesh()
	 *	Destructor
	 */
	~VMesh();

	/**	@fn publish(string _filename)
	 *	publish a movie
	 *	@param _filename the movie filename to be published
	 *	@return true if publish succeeds, false otherwise
	 */
	bool publish(string _filename);

	/**	@fn join(string _hash, string _remoteIP, int _remotePort)
	 *	join to watch a movie
	 *	@param _hash movie hash in hex
	 *	@param _remoteIP IP address of the remote machine
	 *	@param _remotePort port number of the remote machine
	 *	@return true if join succeeds, false otherwise
	 */
	bool join(string _hash, string _remoteIP, int _remotePort);

	/**	@fn start()
	 *	start running VMesh internal mechanism;
	 *	start() must be called after EITHER publish() OR join();
	 *	after VMesh is started, fetch and getPacket can be called
	 *	@return true if VMesh start properly, false otherwise
	 */
	bool start();

	/**	@fn stop()
	 *	stop running VMesh
	 */
	void stop();
	
	/**	@fn getMediaInfo()
	 *	get the MediaInfo associated in VMesh;
	 *	a movie is associated when start() is called
	 *	@return MediaInfo associated in VMesh
	 */
	MediaInfo* getMediaInfo();

	/**	@fn fetch(int _whichInterval, unsigned int& startAPID, int& Alength, unsigned int& startVPID, int& Vlength)
	 *	fetch Packets to the buffer;
	 *	Packets of a second would be fetch to buffer in batch;
	 *	a Packet must be fetched to the buffer before it can be retrieved by getPacket()
	 *	this call is non-blocking
	 *	@param _whichInterval which second in time of Packet to be fetched to the buffer
	 *	@param startAPID the starting Packet ID of AUDIO_STREAM in _whichInterval
	 *	@param Alength the number of Packet of AUDIO_STREAM in _whichInterval
	 *	@param startVPID the starting Packet ID of VIDEO_STREAM in _whichInterval
	 *	@param Vlength the number of Packet of VIDEO_STREAM in _whichInterval
	 *	@return false when MediaInfo or StreamRequest is not yet avaliable
	 *	@return true otherwise
	 */
	bool fetch(int _whichInterval, unsigned int& startAPID, int& Alength, unsigned int& startVPID, int& Vlength);

	/**	@fn clear_fetch()
	 *	clear all fetching packets in the queue previously called by fetch();
	 *	this may be called when a jump is requested, so fetching Packet would not be useful anymore
	 */
	void clear_fetch();
	
	/**	@fn getPacket(int _type, unsigned int _PacketID)
	 *	retrieved a Packet from the buffer;
	 *	the Packet must be fetched to the buffer in advance by calling fetch();
	 *	the call is non-blocking and the Packet returned is deep-copied
	 *	@param _type the stream of the packet
	 *	@param _PacketID the packet ID
	 *	@return Packet, which should be free after use
	 *	@return NULL pointer is the Packet is not in buffer
	 */
	Packet* getPacket(int _type, unsigned int _PacketID);

	/**	@fn existPacket(int _type, unsigned int _PacketID)
	 *	check if a Packet is in the buffer
	 *	@param _type the stream of the packet
	 *	@param _PacketID the packet ID
	 *	@return true if the Packet is in the buffer already, false otherwise
	 */
	bool existPacket(int _type, unsigned int _PacketID);

	/**	@fn getPeerInfo()
	 *	get the current peer information
	 *	@return list of current peer information
	 */
	PeerInfoList getPeerInfo();
	
	/**	@fn getSeekResolution()
	 *	get the seek resolution (1000 ms) (this function is of no use)
	 *	@return 1000
	 */
	int getSeekResolution();

private:
	// IP and port of the peer
	string myIP;
	int myPort;
	int identity;	// Proxy = 0, Client = 1, Server = 2
	int exchangeNeighbor; // node performing exchange table with number of neighbor 

	// capacity of local storage
	int localStorageCapacity;

	// boostrapping information
	string bootstrapIP;
	int  bootstrapPort;

	// maximum accpet connection
	int maxConnection;

	// hash data of the movie, keep "" for bootstrap
	std::string myHash;
	
	MediaInfo* myMediaInfo;
	
	// maintain the list of neighbor peers
	VMeshLocalNodeList* myNodeList;
	
	// permenate storage of the peer
	Storage* myStorage;
	
	// temporary storage of the peer
	Buffer* myBuffer;
	
	// use Chord for data retrieval
	App::ChordApp* myChord;

	// the overlay manager
	SegmentManager* mySegment_;

	// decide if the peer is a supernode
	bool isSuperNode;

	// the streaming server
	// a thread to listen to others' connections
	RequestListener* myListener;

	// the streaming client
	// a thread to ask other peers to stream the packets
	StreamRequest* myStreamRequest;

	// a thread to calculate Network Coordinate
	//Vivaldi::VivaldiRequest* myVivaldiRequest;
};

_VMESH_NAMESPACE_FOOTER
#endif
