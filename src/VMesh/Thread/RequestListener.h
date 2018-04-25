#ifndef _REQUEST_LISTENER_H
#define _REQUEST_LISTENER_H

#include "../Common/VMeshCommon.h"
#include "../Common/MediaInfo.h"
#include "../Node/VMeshLocalNode.h"
#include "../Node/VMeshLocalNodeList.h"
#include "../Storage/Storage.h"
#include "../Util/Buffer.h"
#include "../Thread/RequestHandler.h"
#include "../Util/SocketClient.h"
#include "../Util/SocketServer.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

/**	@file RequestListener.h
 *	This file contains the definition of class RequestListener
 *	@author Kelvin Chan
 */
_VMESH_NAMESPACE_HEADER

/**	@class RequestListener
 *	This class implements a thread that listens to others' connections 
	and creates a RequestHandler thread to serve each of them
 */
class RequestListener : public Thread {
public:
	/**	@fn RequestListener (string _myIP, int _myPort, int _numHandler, MediaInfo* _info, 
		VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest=NULL)
	 *	Constructor that requires some shared object
	 *	@param _myIP local IP address
	 *	@param _myPort local port number
	 *	@param _numHandler max number of RequestHandler
	 *	@param _info MediaInfo
	 *	@param _list VMeshLocalNodeList storing local nodes of completed segment
	 *	@param _storage Storage of segments
	 *	@param _buffer Buffer
	 *	@param _segment SegmentManager instance
	 *	@param _myStreamRequest StreamRequest thread
	 */
	RequestListener (string _myIP, int _myPort, int _numHandler, MediaInfo* _info, VMeshLocalNodeList* _list, Storage* _storage, Buffer* _buffer, SegmentManager* _segment, StreamRequest* _myStreamRequest=NULL, int p_identity = -1);
	
	/**	@fn ~RequestListener()
	 *	Destructor
	 */
	virtual ~RequestListener();
	
	/**	@fn getConnectedNodeList()
	 *	allocate memory to make the node list;
	 *	but note that the node inside cannot be deleted (may use clearall())
	 *	@return VMeshRemoteNodeList storing all VMeshRemoteNode it is serving
	 */
	VMeshRemoteNodeList* getConnectedNodeList();

	/**	@fn stop()
	 *	stop running the thread
	 */
	void stop();

private:
	/**	@fn listenRequest()
	 *	start listening to others' requests
	 *	@return true if listen succeeds, false otherwise
	 */
	bool listenRequest();

protected:
	/**	@fn execute()
	 *	Override the execute() in Thread
	 */
	void* execute();

private:
	string myIP;
	int myPort;
	int peer_identity;

	SocketServer myServer;

	int numHandler;

	MediaInfo* myMediaInfo;

	VMeshLocalNodeList* myNodeList;

	Storage* myStorage;

	Buffer* myBuffer;

	//App::ChordApp* myChord;
	SegmentManager* mySegment_;

	RequestHandler** myHandlerList;

	StreamRequest* myStreamRequest;

	bool keepRunning;

	long handlerCounter;
};
_VMESH_NAMESPACE_FOOTER
#endif

