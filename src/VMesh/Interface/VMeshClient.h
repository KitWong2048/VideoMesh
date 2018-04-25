#ifndef _VMESH_CLIENT_H
#define _VMESH_CLIENT_H

#include <string>

#include "../Common/VMeshCommon.h"
#include "../Interface/VMesh.h"
#include "../Util/Logger.h"

/**	@file VMeshClient.h
 *	This file contains the definition of class VMeshClient
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshClient
 *	This class provides the wrapping of VMesh for client node (only support single movie per instance)
 */
class VMeshClient 
{
public:
	/**	@fn VMeshClient(string _myIP, int _myPort, string _remoteIP, int _remotePort)
	 *	Constructor
	 *	@param _myIP the local IP address (may use getHostIP() function in Utility.h)
	 *	@param _myPort port number to be listened by VMesh (_myPort + 1 will be occupied by DHT)
	 *	@param _remoteIP the remote IP address of the bootstrap node
	 *	@param _remotePort port number of the remote bootstrap node
	 *	@param _localStorageCapacity number of segments stored by each peer
	 */
	VMeshClient(string _myIP, int _myPort, string _remoteIP, int _remotePort, int _localStorageCapacity = LOCAL_STORAGE_CAPACITY, int _clientExchangeNeighbor = -1);

	/**	@fn ~VMeshClient()
	 *	Destructor
	 */
	~VMeshClient();
	
	/**	@fn start()
	 *	start running
	 *	@return true if start succeeds, false otherwise
	 */
	bool start();

	/**	@fn stop()
	 *	stop running
	 */
	void stop();

	/**	@fn movieJoin(const char* _key)
	 *	join to watch a movie throught the bootstrap node;
	 *	after a movie is joined, Packets can be retrieved by getPacket()
	 *	@param _key the hash value of the movie
	 *	@return if join succeeds, false otherwise
	 */	
	bool movieJoin(const char* _key);

	/**	@fn movieLeave(const char* _key)
	 *	leave a joined movie
	 *	@param _key the hash value of the movie
	 *	@return if leave succeeds, false otherwise
	 */	
	bool movieLeave(const char* _key);

	/**	@fn getMediaInfo(const char* _key)
	 *	get the MediaInfo
	 *	@param _key the hash value of the movie
	 *	@return MediaInfo associated in VMesh
	 */
	MediaInfo* getMediaInfo(const char* _key);

	/**	@fn setTime(int _sec)
	 *	set the playing time of the movie, prefeching would be performed automatically
	 *	@param _sec playing time in second
	 */
	void setTime(int _sec);

	/**	@fn getPacket(int _type)
	 *	get the next Packet (should be free after use);
	 *	prefetching would be done automatically
	 *	@param _type stream of the Packet
	 *	@return Packet, which should be free after use
	 *	@return NULL pointer is the Packet is not in buffer
	 */
	Packet* getPacket(int _type);

	/**	@fn existPacket(int _type)
	 *	check if the next Packet is in the buffer
	 *	@param _type the stream of the packet
	 *	@return true if the Packet is in the buffer already, false otherwise
	 */
	bool existPacket(int _type);

private:
	VMesh myVMesh;
	std::string temp_remoteIP;
	int temp_remotePort;
	int currentTime;
	unsigned int nextAID;
	int fetchedAID;
	unsigned int nextVID;
	int fetchedVID;
	int prefetch;
};

_VMESH_NAMESPACE_FOOTER
#endif
