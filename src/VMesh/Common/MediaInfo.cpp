#include "../Common/MediaInfo.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

MediaInfo::	MediaInfo(int mediaType, char* hash, int interval, int numIntervalsinSegment, 
	unsigned int* numPackets, std::vector<IntervalIndexEntry>* iIndexTable,
	string mediaName, unsigned int fileSize, char* _appData, int _appLen)
:type(mediaType), mediaHash(hash), interval(interval), numIntervalsinSegment(numIntervalsinSegment), 
numPackets(numPackets), intervalIndexTable(iIndexTable), mediaName(mediaName), fileSize(fileSize),
appData(_appData), appLen(_appLen) {

	int i=0;
	int j=0;
	
	numStreams = NUM_STREAM;	
	numIntervals = new int[2];
	numIntervals[0] = (int)intervalIndexTable[0].size();
	numIntervals[1] = (int)intervalIndexTable[1].size();

	// calculate the number of segments in the movie
	// e.g. if there are 101 minutes of movie and 5 minutes is one segment, then there are 20 + 1 segments
	// use the stream with more intervals
	if (numIntervals[0] > numIntervals[1]) {
		numSegments = numIntervals[0]/numIntervalsinSegment;
		if (numIntervals[0]%numIntervalsinSegment) numSegments++;
	}
	else {
		numSegments = numIntervals[1]/numIntervalsinSegment;
		if (numIntervals[1]%numIntervalsinSegment) numSegments++;
	}

	int k;
	packetIDTable = new int*[numStreams];
	for (i=0; i<numStreams; i++)
	{
		packetIDTable[i] = new int[numSegments];
		for (j=0; j<numSegments; j++)
		{
			for ( k = 0; k < numIntervals[i]; k++ )
				if ( j == intervalIndexTable[i][k].sid )
				{
					packetIDTable[i][j] = intervalIndexTable[i][k].pid;
					break;
				}
		}
	}

	return;
}
	
MediaInfo::MediaInfo(const char* serialData, int len){

//	LOG_DEBUG((string)"MediaInfo\t: Constructor: Length of serialData = " + itos(len));
	int i, j;
		
	int parsed=0;

	parsed+=unpackIntFromChar(appLen, serialData+parsed);
	appData = new char[appLen];
	memcpy(appData, serialData+parsed, appLen);
	parsed+=appLen;
	
	mediaHash = new char[MEDIAINFOHASH_LENGTH*2];
	for (i=0; i<MEDIAINFOHASH_LENGTH*2; i++){
		mediaHash[i] = serialData[i+parsed];
	}
	parsed+=i;
//	LOG_DEBUG((string)"MediaInfo\t: Constructor: Parsed = " + itos(parsed));

	parsed+=unpackIntFromChar(type, serialData+parsed);
	parsed+=unpackIntFromChar(numStreams, serialData+parsed);	
	parsed+=unpackIntFromChar(interval, serialData+parsed);
	parsed+=unpackIntFromChar(numIntervalsinSegment, serialData+parsed);			
	parsed+=unpackIntFromChar(numSegments, serialData+parsed);
//	LOG_DEBUG((string)"MediaInfo\t: Constructor: Parsed = " + itos(parsed));

	int mediaNameSize;
	parsed+=unpackIntFromChar(mediaNameSize, serialData+parsed);
	for (i=0; i<mediaNameSize; i++){
		mediaName += serialData[i+parsed];
	}
	parsed+=i;
	
	parsed+=unpackUIntFromChar(fileSize, serialData+parsed);
	
	numPackets = new unsigned int[numStreams];
	numIntervals = new int[numStreams];
	intervalIndexTable = new vector<IntervalIndexEntry>[numStreams];
	packetIDTable = new int*[numStreams];
	IntervalIndexEntry entry;
	int segmentCount = 0;
	for (i=0; i<numStreams; i++){
		packetIDTable[i] = new int[numSegments];
		parsed+=unpackUIntFromChar(numPackets[i], serialData+parsed);			
		parsed+=unpackIntFromChar(numIntervals[i], serialData+parsed);			
		for (j=0; j<numIntervals[i]; j++){
			parsed+=unpackUIntFromChar(entry.pid, serialData+parsed);
			parsed+=unpackIntFromChar(entry.numPackets, serialData+parsed);
			parsed+=unpackUIntFromChar(entry.timestamp, serialData+parsed);
//			LOG_DEBUG((string)"MediaInfo\t: Constructor: Parsed = " + itos(parsed));
			entry.ms = interval*j;
			entry.sid = j/numIntervalsinSegment;
			intervalIndexTable[i].push_back(entry);
			if ( segmentCount == entry.sid )
			{
				packetIDTable[i][segmentCount] = entry.pid;
				segmentCount++;
			}
		}
	}


}

MediaInfo::MediaInfo(const MediaInfo& m)
:type(m.type), numStreams(m.numStreams),
interval(m.interval), numIntervalsinSegment(m.numIntervalsinSegment),
numSegments(m.numSegments), mediaName(m.mediaName), fileSize(m.fileSize)
{
	int i;
	
	mediaHash = new char[MEDIAINFOHASH_LENGTH*2];	
	for (i=0; i<MEDIAINFOHASH_LENGTH*2; i++) mediaHash[i] = m.mediaHash[i];
	
	numPackets = new unsigned int[numStreams];
	numIntervals = new int[numStreams];	
	intervalIndexTable = new vector<IntervalIndexEntry>[numStreams];
	packetIDTable = new int*[numStreams];
	for (i=0; i<numStreams; i++){
		numPackets[i] = m.numPackets[i];
		numIntervals[i] = m.numIntervals[i];
		intervalIndexTable[i] = m.intervalIndexTable[i];
		int j;
		packetIDTable[i] = new int[numSegments];
		for  ( j = 0; j < numSegments; j++ )
		{
			packetIDTable[i][j] = m.packetIDTable[i][j];
		}
	}
	
	appLen = m.appLen;
	appData = new char[appLen];
	memcpy(appData, m.appData, appLen);

	return;
}

MediaInfo::~MediaInfo(){
	int i;
	if ( packetIDTable )
	{
		for ( i = 0; i < numStreams; i++ )
			if ( packetIDTable[i] )
				delete [] packetIDTable[i];
		delete []packetIDTable;
	}
	

	if (mediaHash){
		delete[] mediaHash;
		mediaHash=0;
	}

	if (numPackets){
		delete[] numPackets;
		numPackets=0;
	}
	if (numIntervals){
		delete[] numIntervals;
		numIntervals=0;
	}
	
	if (intervalIndexTable){
		delete[] intervalIndexTable;
		intervalIndexTable=0;
	}

	if (appData){
		delete[] appData;
		appData = 0;
		appLen = 0;
	}

}

void MediaInfo::serialize(char*& serialData, int& len){
	int i, j;
	
	len = sizeof(int) + appLen + MEDIAINFOHASH_LENGTH*2 + (7+numStreams*2)*sizeof(int) + (numIntervals[0]+numIntervals[1])*3*sizeof(int) + (int)mediaName.size();

	serialData = new char[len];
	
	int filled=0;

	filled+=packIntIntoChar(appLen, serialData+filled);
	memcpy(serialData+filled, appData, appLen);
	filled+=appLen;

		
	for (i=0; i<MEDIAINFOHASH_LENGTH*2; i++){
		serialData[i+filled] = mediaHash[i];
	}
	filled+=i;

	filled+=packIntIntoChar(type, serialData+filled);
	filled+=packIntIntoChar(numStreams, serialData+filled);
	filled+=packIntIntoChar(interval, serialData+filled);
	filled+=packIntIntoChar(numIntervalsinSegment, serialData+filled);
	filled+=packIntIntoChar(numSegments, serialData+filled);
	
	filled+=packIntIntoChar((int)mediaName.size(), serialData+filled);
	
	for(i=0; i < (int)mediaName.size(); i++){
		serialData[i+filled] = mediaName[i];
	}
	filled+=i;
	
	filled+=packUIntIntoChar(fileSize, serialData+filled);

	for (i=0; i<numStreams; i++){
		filled+=packUIntIntoChar(numPackets[i], serialData+filled);
		filled+=packIntIntoChar(numIntervals[i], serialData+filled);
		for (j=0; j<numIntervals[i]; j++){
			filled+=packUIntIntoChar(intervalIndexTable[i][j].pid, serialData+filled);
			filled+=packIntIntoChar(intervalIndexTable[i][j].numPackets, serialData+filled);
			filled+=packUIntIntoChar(intervalIndexTable[i][j].timestamp, serialData+filled);
		}
	}

//	LOG_DEBUG((string)"MediaInfo\t: Serialize: Length of serialData = " + itos(len));
//	LOG_DEBUG((string)"MediaInfo\t: Serialize: Serialized = " + itos(filled));


}
	
bool MediaInfo::isEqual(const MediaInfo& m) const{
	for (int i=0; i<MEDIAINFOHASH_LENGTH*2; i++){
		if (mediaHash[i]!=m.mediaHash[i]) return false;		
	}
	return true;	
}

const char* MediaInfo::getMediaHash(){
	return mediaHash;
}
	
int MediaInfo::getSIDbyPID(unsigned int pid, int streamType){
	if (streamType < 0 || streamType >= numStreams){
		//LOG_DEBUG("MediaInfo::getSIDbyPID\t: streamType is invalid (" + itos(streamType) + ")");
		return -1;
	}
	if (pid < 0 || pid >= numPackets[streamType]){
		//LOG_DEBUG("MediaInfo::getSIDbyPID\t: pid is invalid (" + itos(pid) + ")");
		return -1;
	}
	
	if (pid ==0) return 0; // first segment id
	
	int i;
	
	/*
	// slow sequential search =.=
	for (i=0; i<numIntervals[streamType]; i++){
		if (intervalIndexTable[streamType][i].pid > pid) break;
	}
	
	if (i==numIntervals[streamType]) return numSegments-1; // last segment id */

	if ( packetIDTable == NULL || packetIDTable[streamType] == NULL )
		return 0;
	for ( i = 0; i < numSegments; i++)
		if ( pid  < packetIDTable[streamType][i] )
			return i - 1;

	return numSegments - 1;
	//return intervalIndexTable[streamType][i-1].sid;
}

int MediaInfo::getIntervalbyPID(unsigned int pid, int streamType){
	if (streamType < 0 || streamType >= numStreams){
		//LOG_DEBUG("MediaInfo::getIntervalbyPID\t: streamType is invalid (" + itos(streamType) + ")");
		return -1;
	}
	if (pid < 0 || pid >= numPackets[streamType]){
		//LOG_DEBUG("MediaInfo::getIntervalbyPID\t: pid is invalid (" + itos(pid) + ")");
		return -1;
	}
	
	if (pid ==0) return 0; // first segment id
	
	int i;
	
	// slow sequential search =.=
	for (i=0; i<numIntervals[streamType]; i++){
		if (intervalIndexTable[streamType][i].pid > pid) break;
	}
	
	return i-1;
}
	
int MediaInfo::getSIDbyInterval(int num){
	if (num < 0) return -1;
	if (num/numIntervalsinSegment >= numSegments) return -1;
	return num/numIntervalsinSegment;
}
	
unsigned int MediaInfo::getPIDbyInterval(int num, int streamType){
	if (streamType >= numStreams) return (unsigned int)-1;
	if (num<0 || num>=numIntervals[streamType]) return (unsigned int)-1;
	return intervalIndexTable[streamType][num].pid;
}

unsigned int MediaInfo::getTimestampbyInterval(int num, int streamType){
	if (streamType >= numStreams) return (unsigned int)-1;
	if (num<0 || num>=numIntervals[streamType]) return (unsigned int)-1;
	return intervalIndexTable[streamType][num].timestamp;
}
	
int MediaInfo::getNumSegments(){
	return numSegments;
}

int MediaInfo::getNumPacketsinInterval(int num, int streamType){
	if (streamType >= numStreams) return -1;
	if (num<0 || num>=numIntervals[streamType]) return -1;
	return intervalIndexTable[streamType][num].numPackets;
	
}

int MediaInfo::getNumPacketsinSegment(int sid){
	if (sid < 0 || sid >= numSegments) return -1;
	int i, j, n;
	n=0;

	for (j=0; j<numStreams; j++){
		for (i=sid*numIntervalsinSegment; (i<numIntervals[j]) && (i<(sid+1)*numIntervalsinSegment); i++){
			n+=intervalIndexTable[j][i].numPackets;
		}		
	}
	return n;
}

unsigned int MediaInfo::getStartingPIDbySID(int sid, int streamType){
	if (streamType >= numStreams) return (unsigned int)-1;
	if (sid < 0 || sid >= numSegments) return (unsigned int)-1;
	
	return intervalIndexTable[streamType][numIntervalsinSegment*sid].pid;
}

int MediaInfo::getNumPacketsinSegment(int sid, int streamType){
	if (streamType >= numStreams) return (unsigned int)-1;
	if (sid < 0 || sid >= numSegments) return (unsigned int)-1;

	int n=0;
	
	for (int i=sid*numIntervalsinSegment; (i<numIntervals[streamType]) && (i<(sid+1)*numIntervalsinSegment); i++){
			n+=intervalIndexTable[streamType][i].numPackets;		
	}
	
	return n;		
}

int MediaInfo::getMediaLength(int streamType){
	if (streamType >= numStreams) return -1;
	return numIntervals[streamType]*interval;
}

string MediaInfo::getMediaName(){
	return mediaName;
}

unsigned int MediaInfo::getFileSize(){
	return fileSize;
}

int MediaInfo::getAppData (const char*& data)
{
	data = appData;
	return appLen;
}

MediaInfo& MediaInfo::operator=(const MediaInfo& m){
	if (this != &m){

		this->~MediaInfo();

		type=m.type;
		numStreams=m.numStreams;
		interval=m.interval;
		numIntervalsinSegment=m.numIntervalsinSegment;
		numSegments=m.numSegments;
			
		int i;
		
		mediaHash = new char[MEDIAINFOHASH_LENGTH*2];		
		for (i=0; i<MEDIAINFOHASH_LENGTH*2; i++) mediaHash[i] = m.mediaHash[i];
		
		numPackets = new unsigned int[numStreams];
		numIntervals = new int[numStreams];	
		intervalIndexTable = new vector<IntervalIndexEntry>[numStreams];
		packetIDTable = new int*[numStreams];
		for (i=0; i<numStreams; i++){
			numPackets[i] = m.numPackets[i];
			numIntervals[i] = m.numIntervals[i];
			intervalIndexTable[i] = m.intervalIndexTable[i];
			int j;
			packetIDTable[i] = new int[numSegments];
			for  ( j = 0; j < numSegments; j++ )
			{
				packetIDTable[i][j] = m.packetIDTable[i][j];
			}
		}
			
	}
	
	return *this;
	
}

bool MediaInfo::operator==(const MediaInfo& m) const{
	for (int i=0; i<MEDIAINFOHASH_LENGTH*2; i++){
		if (mediaHash[i]!=m.mediaHash[i]) return false;		
	}
	return true;
}

_VMESH_NAMESPACE_FOOTER
