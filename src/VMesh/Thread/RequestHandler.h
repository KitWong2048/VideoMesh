#ifndef _REQUEST_HANDLER_H
#define _REQUEST_HANDLER_H

#include "../Common/VMeshCommon.h"
#include "../Common/MediaInfo.h"
#include "../Node/VMeshLocalNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Thread/StreamRequest.h"
#include "../Storage/Storage.h"
#include "../Util/Buffer.h"
#include "../Util/SocketClient.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"
#include "../Util/ExchangeTable.h"
//#include "../VivaldiAdaptor/VivaldiAdaptor.h"

/**	@file RequestHandler.h
 *	This file contains the definition of class RequestHandler
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class RequestHandler
 *	This class implements a thread that serving a remote node's requests
 */
class RequestHandler : public Thread {
public:
	/**	@fn RequestHandler (string _myIP, int _myPort, SocketClient* _client, MediaInfo* _info,
		VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest=NULL)
	 *	Constructor that requires some shared object
	 *	@param _myIP local IP address
	 *	@param _myPort local port number
	 *	@param _client SocketClient to be served
	 *	@param _info MediaInfo
	 *	@param _list VMeshLocalNodeList storing local nodes of completed segment
	 *	@param _storage Storage of segments
	 *	@param _buffer Buffer
	 *	@param _segment SegmentManager instance
	 *	@param _myStreamRequest StreamRequest threads
	 */
	RequestHandler (string _myIP, int _myPort, SocketClient* _client, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest=NULL, int p_identity = -1);

	/**	@fn getMyNode()
	 *	get a VMeshRemoteNode representing the remote SocketClient
	 *	@return VMeshRemoteNode
	 */
	VMeshRemoteNode* getMyNode();
	
	/**	@fn ~RequestHandler()
	 *	Destructor
	 */
	virtual ~RequestHandler();

	/**	@fn stop()
	 *	stop running the thread
	 */
	void stop();

	bool isRunning();
private:
	/**	@fn echo(char* msg, int length)
	 *	show what have received to the screen
	 *	@param msg request message received from Socketclient
	 *	@param length length of request message
	 */
	void echo(char* msg, int length);

	/**	@fn ping(char* msg, int length)
	 *	handle PING
	 *	@param msg PING request message received from Socketclient
	 *	@param length length of request message
	 */
	void ping(char* msg, int length);
	
	/**	@fn supply_LocalNodeList(char* msg, int length)
	 *	handle SUPPLY_LOCALNODELIST
	 *	@param msg SUPPLY_LOCALNODELIST request message received from Socketclient
	 *	@param length length of request message
	 */
	void supply_LocalNodeList(char* msg, int length);

	/**	@fn req_MediaInfo(char* msg, int length)
	 *	handle REQ_MEDIAINFO
	 *	@param msg REQ_MEDIAINFO request message received from Socketclient
	 *	@param length length of request message
	 */
	void req_MediaInfo(char* msg, int length);

	/**	@fn req_Packet(char* msg, int length)
	 *	handle REQ_PACKET
	 *	@param msg REQ_PACKET request message received from Socketclient
	 *	@param length length of request message
	 */
	void req_Packet(char* msg, int length);

	/**	@fn req_LocalNodeList(char* msg, int length)
	 *	handle REQ_LOCALNODELIST
	 *	@param msg REQ_LOCALNODELIST request message received from Socketclient
	 *	@param length length of request message
	 */
	void req_LocalNodeList(char* msg, int length);

	/**	@fn req_ParentList(char* msg, int length)
	 *	handle REQ_PARENTLIST
	 *	@param msg REQ_PARENTLIST request message received from Socketclient
	 *	@param length length of request message
	 */
	void req_ParentList(char* msg, int length);

protected:
	/**	@fn execute()
	 *	Override the execute() in Thread
	 */
	void* execute();

private:
	string myIP;
	int myPort;
	int peer_identity;

	VMeshRemoteNode* myNode;
	
	SocketClient* myClient;

	MediaInfo* myMediaInfo;

	VMeshLocalNodeList* myNodeList;

	Storage* myStorage;

	Buffer* myBuffer;

	//App::ChordApp* myChord;
	SegmentManager* mySegment_;
	
	StreamRequest* myStreamRequest;

	bool keepRunning;

	DWORD timeTaken[5], start;

	//VivaldiAdaptor* myVivaldiAdaptor;
};
_VMESH_NAMESPACE_FOOTER
#endif

