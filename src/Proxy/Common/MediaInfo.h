/**
* MediaInfo
* 
* Store all the infomation necessary to describe a segment 
* @author Kelvin Chan, Philip Yang
*/
#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <string>
#include <vector>
#include "../Common/VMeshCommon.h"
#include "../Util/Logger.h"
#include "../Util/Mutex.h"

_VMESH_NAMESPACE_HEADER

/** @struct IntervalIndexEntry
 *  A struct to store the packet, time and segment information for each second
 */
struct IntervalIndexEntry
{
	IntervalIndexEntry()
	{
		ms = pid = sid = numPackets = streamType = timestamp = 0;
	}

	int ms;						/*!< the second (in ms) */
	unsigned int pid;			/*!< the packet ID of the first packet in this second */
	int sid;					/*!< the segment ID */
	int numPackets;				/*!< the number of packets in this second */
	int streamType;				/*!< the stream */
	unsigned int timestamp;		/*!< the exact timestamp for the first packet */
};

/**	@class MediaInfo
 *	This class provides indexing and general information of a movie
 */
class MediaInfo
{
public:
	/**	@fn MediaInfo(int mediaType, char* hash, int interval, int numIntervalsinSegment, 
			unsigned int* numPackets, std::vector<IntervalIndexEntry>* iIndexTable,
			string mediaName, unsigned int fileSize, char* _appData, int _appLen)
	 *	Constructor using mediaType, filename, hash, interval of each index entry as input;
	 *	Note 1:	pointer to hash, numPackets, tables, AppData are shallow copied, i.e. no memory is allocated;
	 *	Note 2: interval is in ms;
	 *	@param mediaType type of media (no use)
	 *	@param hash hashvalue of the movie in HEX, char[MEDIAINFOHASH_LENGTH*2]
	 *	@param interval 1000 (no use)
	 *	@param numIntervalsinSegment number of seconds in a segment
	 *	@param numPackets array of total number of packets in each stream
	 *	@param iIndexTable array of IntervalIndexEntry storing the first packet in that second
	 *	@param mediaName movie name
	 *	@param fileSize filesize
	 *	@param _appData application data
	 *	@param _appLen length of application data
	 */
	MediaInfo(int mediaType, char* hash, int interval, int numIntervalsinSegment, 
	unsigned int* numPackets, std::vector<IntervalIndexEntry>* iIndexTable,
	string mediaName, unsigned int fileSize, char* _appData, int _appLen);
	
	/**	@fn MediaInfo(const char* serialData, int len)
	 *	Constructor using serialized char array;
	 *	@param serialData serialized char array
	 *	@param len length of the array
	 */
	MediaInfo(const char* serialData, int len);
	
	/**	@fn MediaInfo(const MediaInfo& m)
	 *	Copy constructor;
	 *	@param m MediaInfo to be copied
	 */
	MediaInfo(const MediaInfo& m);
	
	/**	@fn ~MediaInfo()
	 *	Destructor
	 */
	~MediaInfo();
	
	/**	@fn serialize(char*& serialData, int& len)
	 *	Creates a serialized char array for this object;
	 *	@param serialData serialized char array
	 *	@param len length of the array
	 */
	void serialize(char*& serialData, int& len);
	
	/**	@fn isEqual(const MediaInfo& m) const
	 *	Compare two MediaInfo according to media hash;
	 *	@param m MedaiInfo to be compared
	 *	@return true if their hash value are the same
	 */
	bool isEqual(const MediaInfo& m) const;
	
	/**	@fn getMediaHash()
	 *	Return the media hash value (no memory allocated);
	 *	@return the media hash
	 */
	const char* getMediaHash();
	
	/**	@fn getSIDbyPID(unsigned int pid, int streamType)
	 *	Lookup the segment ID for a given Packet ID;
	 *	@param pid the packet ID
	 *	@param streamType the stream of the packet
	 *	@return the Segment ID of the segment that contains the packet with Packet ID
	 *	@return -1 if invalid packet
	 */
	int getSIDbyPID(unsigned int pid, int streamType);
	

	/**	@fn getIntervalbyPID(unsigned int pid, int streamType)
	 *	Lookup the time for a given Packet ID;
	 *	@param pid the packet ID
	 *	@param streamType the stream of the packet
	 *	@return the second contains the packet with Packet ID
	 *	@return -1 if invalid packet
	 */
	int getIntervalbyPID(unsigned int pid, int streamType);
	
	/**	@fn getSIDbyInterval(int num)
	 *	Lookup the segment ID for a given time;
	 *	@param num the second
	 *	@return the segment ID the time belongs
	 *	@return -1 if invalid time
	 */
	int getSIDbyInterval(int num);
	

	/**	@fn getPIDbyInterval(int num, int streamType)
	 *	Lookup the Packet ID for a given time;
	 *	@param num the second
	 *	@param streamType the stream of the packet
	 *	@return the packet ID
	 *	@return (unsigned int) -1 if invalid time
	 */
	unsigned int getPIDbyInterval(int num, int streamType);
	
	/**	@fn getTimestampbyInterval(int num, int streamType)
	 *	Lookup the specific first packet timestamp for a given second;
	 *	@param num the second
	 *	@param streamType the stream of the packets
	 *	@return the specific first packet timestamp
	 */
	unsigned int getTimestampbyInterval(int num, int streamType);
	
	/**	@fn getNumSegments()
	 *	Return the number of segments in the movie;
	 *	@return the number of segments in the movie
	 */
	int getNumSegments();
	
	/**	@fn getNumPacketsinInterval(int num, int streamType)
	 *	Return the number of packets for a stream in a given second;
	 *	@param num the second
	 *	@param streamType the stream of the packets
	 *	@return the number of packets in the second in that stream
	 *	@return -1 if invalid time
	 */
	int getNumPacketsinInterval(int num, int streamType);
	
	/**	@fn getNumPacketsinSegment(int sid)
	 *	Return the number of packets in a segment;
	 *	@param sid the segment ID
	 *	@return the number of packets in the segment
	 *	@return -1 if invalid time
	 */
	int getNumPacketsinSegment(int sid);

	/**	@fn getStartingPIDbySID(int sid, int streamType)
	 *	Return the starting Packet ID in a segment;
	 *	@param sid the segment ID
	 *	@param streamType the stream of the packets
	 *	@return the starting Packet ID
	 */
	unsigned int getStartingPIDbySID(int sid, int streamType);

	/**	@fn getNumPacketsinSegment(int sid, int streamType)
	 *	Return the number of packets in a segment;
	 *	@param sid the segment ID
	 *	@param streamType the stream of the packets
	 *	@return the number of packets
	 */
	int getNumPacketsinSegment(int sid, int streamType);
	
	/**	@fn getMediaLength(int streamType)
	 *	Return the media length in ms, just a round up to second;
	 *	@param streamType the stream of the packets
	 *	@return the media length in ms
	 */
	int getMediaLength(int streamType);

	/**	@fn getMediaName()
	 *	Return the media name;
	 *	@return the media name
	 */
	std::string getMediaName();
	
	/**	@fn getFileSize()
	 *	Return the media file size;
	 *	@return the media file size
	 */
	unsigned int getFileSize();

	/**	@fn getAppData (const char*& data)
	 *	Shallow copy of the AppData to the pointer;
	 *	@param data the pointer to be pointing
	 *	@return the length of the AppData
	 */
	int getAppData (const char*& data);
	
	/**	@fn operator=(const MediaInfo& m)
	 *	Deep copy of the MediaInfo;
	 *	@return the node being copied
	 */
	MediaInfo& operator=(const MediaInfo& m);
		
	/**	@fn operator==(const MediaInfo& m) const
	 *	Compare the mediainfo by their mediaHash;
	 *	@return true if both MediaInfo has the same hash value
	 */
	bool operator==(const MediaInfo& m) const;

private:
	char* mediaHash;
	int type;
	int numStreams;
	int interval; // in ms
	int numIntervalsinSegment;
	int numSegments;
	unsigned int* numPackets;
	std::string mediaName;
	unsigned int fileSize;

	char* appData;
	int appLen;

	int* numIntervals; // numIntervals[numStreams]
	std::vector<IntervalIndexEntry>* intervalIndexTable; // interval[numStreams]
	int** packetIDTable;
};
_VMESH_NAMESPACE_FOOTER
#endif
