#define ID_LENGTH 13 // bytes  12
#define ID_BIT_LENGTH	(ID_LENGTH * 8) // bits

#define MEDIAINFOHASH_LENGTH	4
#define SEGMENTID_LENGTH		2
//#define COORDINATE_LENGTH		6
#define COORDINATE_LENGTH		7

/*! @brief Segment ID macro for bootstrap node */
#define BOOTSTRAP_SEGMENTID		65535
/*! @brief Segment ID macro for unknown segment in DHT */
#define INITIAL_SEGMENTID		65534
/*! @brief Segment ID macro for Query segment in Server */
#define QUERY_SEGMENTID			65533


#define COMM_BUFFER_SIZE 102400
#define SMALL_BUFFER_SIZE 10
#define DEFAULT_LOOKUP_PORT 10080
#define DEFAULT_SERVER_IP "127.0.0.1"

#define REFRESH_SERVER_INTERVAL 10*60	// in second

#ifndef LOGLEVEL
#define LOGLEVEL

enum LogLevel{
		LogNone = 0,
		LogDebug = 1,
		LogNormal = 2,
		LogError = 4,
		LogRequest = 8,
		LogService = 16,
		LogStabilizer = 32,
		LogDeepDebug = 64,
		LogCommon = LogNormal | LogError,
		LogAll =  LogCommon | LogRequest | LogService | LogStabilizer| LogDeepDebug
	};	

#endif

#define SAFE_DELETE(X)		\
	if (X != NULL)			\
	{						\
		delete X;			\
		X = NULL;			\
	}