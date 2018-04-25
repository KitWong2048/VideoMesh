#pragma once
#include <winsock2.h>
#include <process.h>
#include <string>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
using std::string;

using boost::asio::ip::tcp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
//--------------------------------------------------------------
// manifest constants
//--------------------------------------------------------------
#define COMM_BUFFER_SIZE 10240
#define SMALL_BUFFER_SIZE 50
#define DEFAULT_PORT 10080
#define SEGMENT_SIZE 1000
#define NUMBER_OF_LOOKUP 5

#define NODE_TIMEOUT	30*60	// 30 Mins

//#define ID_LENGTH 12 // bytes
#define ID_LENGTH 13 // bytes
#define ID_BIT_LENGTH	(ID_LENGTH * 8) // bits

#define LOG_SERVER_INFO_LENGTH	8

#define MEDIAINFOHASH_LENGTH	4
#define SEGMENTID_LENGTH		2
//#define COORDINATE_LENGTH		6
#define COORDINATE_LENGTH		7

/*! @brief Segment ID macro for bootstrap node */
#define BOOTSTRAP_SEGMENTID		65535
/*! @brief Segment ID macro for unknown segment in DHT */
#define INITIAL_SEGMENTID		65534

#define MOVIE_RECORD_FILE		"mrecord.txt"

#define PROXY_IDENTITY						0			// Proxy identity
#define CLIENT_IDENTITY						1			// Client identity
#define SERVER_IDENTITY						2			// Server identity

//--------------------------------------------------------------
// structures
//--------------------------------------------------------------
struct LookupRequestHeader
{
	char method[SMALL_BUFFER_SIZE];
	char url[MAX_PATH];
	char filepathname[MAX_PATH];
	char httpversion[SMALL_BUFFER_SIZE];
	tcp::endpoint client_ip;
	char identity[5];		// For identity each role of VMesh(Server, Proxy, Client)
};

struct ClientInfo
{
	SOCKET client_socket;
	IN_ADDR client_ip;
};


//--------------------------------------------------------------
// prototypes
//--------------------------------------------------------------

// the main loop
void StartServer(void);
void HandleHTTPRequest( socket_ptr socket_ );
BOOL ParseLookupHeader(char *receivebuffer, LookupRequestHeader &requestheader, char* data);
void OutputHTTPRespond(socket_ptr, int statuscode );
void OutputScreenError(const char *errmsg);
void GetLogServerInfo(char *);
class SegmentDB;
