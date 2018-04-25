#ifndef _VMESH_SERVER_H
#define _VMESH_SERVER_H

#include <string>

#include "../Common/VMeshCommon.h"
#include "../Interface/VMesh.h"
#include "../Util/Logger.h"

/**	@file VMeshServer.h
 *	This file contains the definition of class VMeshServer
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class VMeshServer
 *	This class provides the wrapping of VMesh for server node (only support single movie per instance)
 */
class VMeshServer 
{
public:
	/**	@fn VMeshServer (string _myIP, int _myPort)
	 *	Constructor
	 *	@param _myIP the local IP address (may use getHostIP() function in Utility.h)
	 *	@param _myPort port number to be listened by VMesh (_myPort + 1 will be occupied by DHT)
	 */
	VMeshServer (string _myIP, int _myPort, int _localStorageCapacity = -1);

	/**	@fn VMeshServer(string _myIP, int _myPort, string _remoteIP, int _remotePort)
	 *	Constructor through another bootstrap node
	 *	@param _myIP the local IP address (may use getHostIP() function in Utility.h)
	 *	@param _myPort port number to be listened by VMesh (_myPort + 1 will be occupied by DHT)
	 *	@param _remoteIP the remote IP address of the bootstrap node
	 *	@param _remotePort port number of the remote bootstrap node
	 */
	VMeshServer(string _myIP, int _myPort, string _remoteIP, int _remotePort);

	/**	@fn ~VMeshServer()
	 *	Destructor
	 */
	~VMeshServer();
	
	/**	@fn start()
	 *	start running
	 *	@return true if start succeeds, false otherwise
	 */
	bool start();
	
	/**	@fn stop()
	 *	stop running
	 */
	void stop();

	/**	@fn moviePublish(const char* _key, string _filename)
	 *	publish a movie
	 *	@param _key the hash value of the movie
	 *	@param _filename the movie filename to be published
	 *	@return if publish succeeds, false otherwise
	 */	
	bool moviePublish(const char* _key, string _filename);

	/**	@fn movieUnPublish(const char* _key)
	 *	Unpublish a published movie
	 *	@param _key the hash value of the movie
	 *	@return if unpublish succeeds, false otherwise
	 */	
	bool movieUnPublish(const char* _key);

	/**	@fn getMediaInfo(const char* _key)
	 *	get the MediaInfo
	 *	@param _key the hash value of the movie
	 *	@return MediaInfo associated in VMesh
	 */
	MediaInfo* getMediaInfo(const char* _key);

private:
	VMesh myVMesh;
	std::string temp_remoteIP;
	int temp_remotePort;
};

_VMESH_NAMESPACE_FOOTER
#endif
