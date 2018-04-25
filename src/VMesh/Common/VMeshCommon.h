#ifndef _VMESH_COMMON_H
#define _VMESH_COMMON_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

/**	@file VMeshCommon.h
 *	This file contains the definition of system variables and parameters
 *	@author Kelvin Chan
 */

/*! @brief compiled with Windows version for VMesh */
#define _VMESH_WIN32_
/*! @brief compiled with Windows version for Chord */
#define DHT_WIN32

#include "../../chord/src/Util/SHA1Util.h"
#include "../../chord/src/Util/ConditionVariable.h"
#include "../../chord/src/Util/Common.h"
#include "../../chord/src/DHT/Chord.h"
#include "../../chord/src/App/ChordApp.h"

/* Include Node Lookup Segment Library*/
#include "../../libNode/ChordManager.h"
#include "../../libNode/ChordNetworkID.h"
#include "../../libNode/CentralLookupManager.h"
#include "../../libNode/CentralLookupNetworkID.h"
#include "../../libNode/TableExchangeLookupManager.h"
#include "../../libNode/TableExchangeLookupNetworkID.h"

/*! @brief VMesh namespace header macro */
#define _VMESH_NAMESPACE_HEADER namespace VMesh {
/*! @brief VMesh namespace footer macro */
#define _VMESH_NAMESPACE_FOOTER }
/*! @brief using VMesh namespace macro */
#define _USING_VMESH_NAMESPACE using namespace VMesh;

// for central lookup option
#define CENTRAL_LOOKUP
// for chord lookup option
//#define CHORD_LOOKUP	
// for neighbor table exchange
//#define TABLE_EXCH_LOOKUP

#define WIN32_LEAN_AND_MEAN

#ifdef _VMESH_WIN32_
	#pragma comment(lib, "ws2_32.lib")
	#pragma warning(disable:4267 4311 4018)
	#include <winsock2.h>
	#include <process.h>
	#include <time.h>
	#include "../Util/Win32Support.h"
#else
	#include <pthread.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/unistd.h>
	#include <dirent.h>
	#include <ifaddrs.h>
#endif

#include "../Util/Utility.h"
#include "../Util/MessageType.h"

/*! @brief Segment ID macro for bootstrap node */
#define BOOTSTRAP_SEGMENTID		65535
/*! @brief Segment ID macro for unknown segment in DHT */
#define INITIAL_SEGMENTID		65534

#define PROXY_SEGMENTID			65533

/*! @brief ping time macro for unknown ping */
#define PING_NOT_READY			9999997
/*! @brief ping time macro for ping timeout */
#define PING_TIMEOUT			9999998
/*! @brief ping time macro for failed ping */
#define PING_FAIL				9999999


/*! @brief macro for audio stream or stream 0 */
#define AUDIO_STREAM			0
/*! @brief macro for video stream or stream 1 */
#define VIDEO_STREAM			1

/*! @brief macro for seek resolution (1000ms = 1sec) */
#define SEEK_RESOLUTION			1000

/*! @brief number of bytes for movie hash in DHT key */
#define MEDIAINFOHASH_LENGTH			4
/*! @brief number of bytes for segment ID in DHT key */
#define SEGMENTID_LENGTH				2
/*! @brief number of bytes for IP address in DHT key */
//#define COORDINATE_LENGTH				6
#define COORDINATE_LENGTH				7
/*!< the total length must equal ID_LENGTH in chord/Common.h */

/*! number of segments to store locally */
#define LOCAL_STORAGE_CAPACITY		2
/*! path for putting temp file */
#define STORAGEFILE_PATH			"./temp/"
/*! the max number of parents of a single segmentID to reply */
#define RPY_PARENT_LIST_SIZE		10

/*! buffer size in byte */
#define BUFFER_SIZE					(20 * 1024 * 1024)
/*! prefetching buffer size in byte */
#define PREFETCH_SIZE				(0.7 * BUFFER_SIZE)

/*! upper bound of PeerStorage size for holding segments (5MBps)*/
#define PEERSTORAGE_SIZE			(LOCAL_STORAGE_CAPACITY * SEGMENT_LENGTH * 5 * 1024)

/*! number of retry for connection */
#define VMESH_CONNECT_RETRY			2
/*! timeout in ms for connection */
#define VMESH_CONNECT_TIMEOUT		5000
/*! timeout in ms for ping */
#define VMESH_PING_TIMEOUT			5000
/*! timeout in ms for idle connection */
#define VMESH_IDLE_TIMEOUT			(2 * 60 * 1000)
/*! timeout in ms for waiting reply */
#define VMESH_REPLY_TIMEOUT			5000
/*! number of retry for a Packet */
#define VMESH_PACKET_RETRY			30

/*! max downlink connection for bootstrap server */
//#define MAX_BOOTSTRAP_CONNECTION		5000
#define MAX_BOOTSTRAP_CONNECTION		20
/*! max downlink connection for normal peers */
//#define MAX_SERVE_CONNECTION			20
#define MAX_SERVE_CONNECTION			200
/*! max uplink connections */
//#define MAX_STREAM_PARENTS				5
#define MAX_STREAM_PARENTS				2
/*! max number of peers stored */
#define MAX_PEERS						1000

/*! stream from Bootstrap if avaliable peers <= threshold */
#define CONNECT_BOOTSTRAP_THRESHOLD		(MULTIPLE_BOOTSTRAP_CONNECTION)

/*! mutiple connections to the bootstrap for higher BW */
#define MULTIPLE_BOOTSTRAP_CONNECTION	(1)

/*! number of stream in a movie (max 2) */
#define NUM_STREAM						1

/*! period in sec of updating adjacent segment peers (max 360000) */
#define REFRESH_ADJACENT_LIST_INTERVAL				60
//#define REFRESH_ADJACENT_LIST_INTERVAL				90
/*! period in sec of logging streaming info (max 360000) */
#define CONSOLE_SHOW_PARENT_LIST_INTERVAL			10
/*! period in sec of looking up the same segment consecutively (max 360000) */
#define CLEAR_LOOKUP_HISTORY_INTERVAL				120
/*! period in sec of registering the segment again in DHT (max 360000) */
#define REG_SEGMENT_INTERVAL				600

/*! length of a segment in ms */
#define SEGMENT_LENGTH			(10*60*1000)

#define DEFAULT_MIN_TRANSFER_RATE			5120		// 5 KB
#define DEFAULT_MAX_TRANSFER_RATE			512000		// 500 KB
#define TRANSFER_RATE_STEP_SIZE				1024		// 1 KB
#define INCREASE_STEP						5
#define DECREASE_STEP						10
#define INACTIVE_COUNT_PERIOD				20			// 20 iterations

#define PROXY_IDENTITY						0			// Proxy identity
#define CLIENT_IDENTITY						1			// Client identity
#define SERVER_IDENTITY						2			// Server identity

/*
*added by hongjie 
*constant for table exchange
*/
#define maxParentPerSegment 5
#define numOfNeighbor	3


/*Total Bandwidth Setting*/
#define SERVER_TOTAL_BANDWIDTH						(5*1000000)  // in Mbps
#define PROXY_TOTAL_BANDWIDTH						(15*1000000)  // in Mbps
#define CLIENT_TOTAL_BANDWIDTH						(15*1000000)  // in Mbps

/*Consume Bandwidth setting*/
#define CONSUME_BANDWIDTH							(5*1000000)	  // in Mbps


#endif

