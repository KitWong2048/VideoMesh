#include "../Common/Publisher.h"

using namespace std;
using Util::sha1HashHex;

_VMESH_NAMESPACE_HEADER

bool Publisher::packetizeMovie(string filename){
	packetize(filename.c_str(), filename.substr(0, filename.find_first_of('.')).c_str(), 1000);
    return true;	
}

bool Publisher::publish(BootstrapStorage*& b, MediaInfo*& m, string filename, int mediaType, int interval, int numIntervalsinSegment){
	b=0;
	m=0;
	
	char t;
	unsigned int j;
	int i;
	PacketIndexEntry pie;
	IntervalIndexEntry iie;
	unsigned int* totalNumPackets;
	
	vector<PacketIndexEntry>* pIndexTable;
	vector<IntervalIndexEntry>* iIndexTable;

	pIndexTable = new vector<PacketIndexEntry>[2];
	iIndexTable = new vector<IntervalIndexEntry>[2];

	totalNumPackets = new unsigned int[2];
	
	unsigned int fileSize = 0;
	unsigned int begin;
	unsigned int end;
	
	fstream fin, fs;

	LOG_INFO("Publisher\t: Start publishing, please wait...");

	
	LOG_INFO("Publisher\t: Checking file existence...");
	bool fileExists = true;
	if (fileExists)
	{
		fs.open((filename+".aud").c_str(),fstream::in|fstream::binary);
		if(!fs.is_open())
			fileExists = false;
		fs.close();
	}
	if (fileExists)
	{
		fs.open((filename+".vid").c_str(),fstream::in|fstream::binary);
		if(!fs.is_open())
			fileExists = false;
		fs.close();
	}
	if (fileExists)
	{
		fs.open((filename+".ain").c_str(),fstream::in|fstream::binary);
		if(!fs.is_open())
			fileExists = false;
		fs.close();
	}
	if (fileExists)
	{
		fs.open((filename+".vin").c_str(),fstream::in|fstream::binary);
		if(!fs.is_open())
			fileExists = false;
		fs.close();
	}
	if (!fileExists)
	{
		LOG_INFO("Publisher\t: Packetizing " + filename);
		packetizeMovie(filename + ".asf");
	}




	//////////////// parse .aud : create pIndexTable[0] ////////////////////////
	fin.open((filename+".aud").c_str(), fstream::in|fstream::binary);
	if (!fin.is_open()){
		LOG_ERROR("Publisher::publish\t: cannot open \""+filename+".aud\"");
		delete[] pIndexTable;
		delete[] iIndexTable;
		return false;
	}

	fin.seekg (0, ios::beg);
	begin = fin.tellg();
	fin.seekg (0, ios::end);
	end = fin.tellg();
	fin.seekg (0, ios::beg);
	fileSize += (end-begin);

	fin.clear();

	i=0;
	
	
	LOG_DEBUG("Publisher\t: Processing \"" + filename + ".aud\"\tFile Size: " + itos(end-begin));
	
	fin.peek();
	while(fin.good()){
		pie.packetSize = 0;
		for (j=0; j<4; j++){
			t = fin.get();
			pie.packetSize = (pie.packetSize<<8)|(unsigned char)t;
		}
		
		if (pie.packetSize == 0){
			LOG_ERROR("Publisher\t: \"" + filename + ".aud\" is corrupted");
			delete[] pIndexTable;
			delete[] iIndexTable;
			return false;				
		}
		
		pie.packetID = i++;
		pie.packetLocation = fin.tellg();
		
		double percentage = (double)pie.packetLocation / (double)(end-begin);
		percentage*= 100;
		/*
		if (pie.packetID % 1000==0)
			LOG_DEBUG("finished .... " + itos((int)percentage) +"%");
		*/
		pIndexTable[0].push_back(pie);
		
		//fin.ignore(pie.packetSize);
		fin.seekg ((int) fin.tellg() + pie.packetSize);
		fin.peek();
	}
	fin.close();
	fin.clear();
	totalNumPackets[0] = i;
	//LOG_DEBUG("Publisher\t: Finished processing \"" + filename + ".aud\"");
	
	//////////////// parse .vid : create pIndexTable[1] ////////////////////////
	fin.open((filename+".vid").c_str(), fstream::in|fstream::binary);
	if (!fin.is_open()){
		LOG_ERROR("Publisher\t: cannot open \"" + filename + ".vid\"");
		delete[] pIndexTable;
		delete[] iIndexTable;
		return false;
	}
	
	begin = fin.tellg();
	fin.seekg (0, ios::end);
	end = fin.tellg();
	fin.seekg (0, ios::beg);
	fileSize += (end-begin);
	fin.clear();
	
	i=0;
	LOG_DEBUG("Publisher\t: Processing \"" + filename + ".vid\"");
	
	fin.peek();
	while(fin.good()){
		pie.packetSize = 0;
		for (j=0; j<4; j++){
			t = fin.get();
			pie.packetSize = (pie.packetSize<<8)|(unsigned char)t;
		}
		if (pie.packetSize == 0){
			LOG_ERROR("Publisher\t: \"" + filename + ".vid\" is corrupted");
			delete[] pIndexTable;
			delete[] iIndexTable;
			return false;				
		}

		pie.packetID = i++;
		pie.packetLocation = fin.tellg();
		double percentage = (double)pie.packetLocation / (double)(end-begin);
		percentage*= 100;
		/*
		if (pie.packetID % 1000==0)
			LOG_DEBUG("finished .... " + itos((int)percentage) + "%");
		*/
		pIndexTable[1].push_back(pie);
		
		//fin.ignore(pie.packetSize);
		fin.seekg ((int) fin.tellg() + pie.packetSize);
		fin.peek();
	}
	fin.close();
	fin.clear();
	totalNumPackets[1] = i;
	
	//LOG_DEBUG("Publisher\t: Finished processing \"" + filename + ".vid\"");
	
	//////////////// parse .ain : create iIndexTable[0] ////////////////////////
	fin.open((filename+".ain").c_str(), fstream::in|fstream::binary);
	if (!fin.is_open()){
		LOG_ERROR("Publisher\t: cannot open \"" + filename + ".ain\"");
		delete[] pIndexTable;
		delete[] iIndexTable;
		return false;
	}
	
	i=0;
	
	LOG_DEBUG("Publisher\t: Processing \"" + filename + ".ain\"");
	
	fin.peek();
	while (fin.good()){
		iie.pid = 0;
		iie.timestamp = 0;
		for(j=0; j<4; j++){
			t = fin.get();
			iie.pid = (iie.pid<<8)|(unsigned char)t;
		}
		
		fin.ignore(2);

		for(j=0; j<4; j++){
			t = fin.get();
			iie.timestamp = (iie.timestamp<<8)|(unsigned char)t;
		}
		
		fin.ignore(4);	

		iie.ms = i*interval;
		iie.sid = i/numIntervalsinSegment;
		iIndexTable[0].push_back(iie);
		//LOG_DEBUG("PID: " + itos(iie.pid) + "\ttimestamp: " + itos(iie.timestamp));
		i++;
		fin.peek();	
	}
	fin.close();
	fin.clear();

	//LOG_DEBUG("Publisher\t: Finished processing \"" + filename + ".ain\"");
	
	for (i=0; i<(iIndexTable[0].size()-1); i++){
		iIndexTable[0][i].numPackets = iIndexTable[0][i+1].pid - iIndexTable[0][i].pid;			
	}
	iIndexTable[0][i].numPackets = totalNumPackets[0] - iIndexTable[0][i].pid;

	//////////////// parse .vin : create iIndexTable[1] ////////////////////////
	fin.open((filename+".vin").c_str(), fstream::in|fstream::binary);
	if (!fin.is_open()){
		LOG_ERROR("Publisher::publish\t: cannot open "+filename+".vin");
		delete[] pIndexTable;
		delete[] iIndexTable;
		return false;
	}
	
	i=0;
	
	LOG_DEBUG("Publisher\t: Processing \"" + filename + ".vin\"");
	
	fin.peek();
	while (fin.good()){
		iie.pid = 0;
		iie.timestamp = 0;
		for(j=0; j<4; j++){
			t = fin.get();
			iie.pid = (iie.pid<<8)|(unsigned char)t;
		}
		
		fin.ignore(2);

		for(j=0; j<4; j++){
			t = fin.get();
			iie.timestamp = (iie.timestamp<<8)|(unsigned char)t;
		}
		
		fin.ignore(4);	
		
		iie.ms = i*interval;
		iie.sid = i/numIntervalsinSegment;
		iIndexTable[1].push_back(iie);

		i++;
		fin.peek();	
	}
	fin.close();
	fin.clear();
	
	//LOG_DEBUG("Publisher\t: Finished processing \"" + filename + ".vin\"");
	
	for (i=0; i<((int)iIndexTable[1].size()-1); i++){
		iIndexTable[1][i].numPackets = iIndexTable[1][i+1].pid - iIndexTable[1][i].pid;
	}
	if (iIndexTable[1].size() > 0)
		iIndexTable[1][i].numPackets = totalNumPackets[1] - iIndexTable[1][i].pid;
	
	//////////////// finished parsing: pIndexTable and iIndexTable complete ////////////////////////
	
	//////////////// start parsing AppData ////////////////////////
	char* appData = 0;
	int appLen = 0;
	fin.open((filename+".data").c_str(), fstream::in|fstream::binary);
	if (!fin.is_open()){
		LOG_DEBUG("Publisher\t: No AppData is loaded");
	}
	else
	{
		begin = fin.tellg();
		fin.seekg (0, ios::end);
		end = fin.tellg();
		fin.seekg (0, ios::beg);
		fin.clear();
		appLen = (end-begin);
		fileSize += appLen;
		
		appData = new char[appLen];
		
		fin.read(appData, appLen);

		fin.close();
		LOG_DEBUG("Publisher\t: " + itos(appLen) + " bytes of AppData is loaded");
	}
	
	//////////////// finished parsing AppData ////////////////////////

	
	unsigned int* numPackets = new unsigned int[2];
	numPackets[0] = totalNumPackets[0];
	numPackets[1] = totalNumPackets[1];
	
	// crop the file name without the path
	string shortfilename = filename;
	int lastdash = (int)filename.find_last_of('/');
	if (lastdash!=string::npos)
	{
		shortfilename = filename.substr(lastdash+1);
	}	
	
	// calculate hash - number of packets in video stream, audio stream and filename is used
	char hashKey[65];
	char* hash;

	bzero(hashKey, sizeof(hashKey));
	
	if (shortfilename.length() > 30){
		sprintf(hashKey, "%d%d%s", numPackets[0], numPackets[1], (shortfilename.substr(0, 30)).c_str());
	
	} else{
		sprintf(hashKey, "%d%d%s", numPackets[0], numPackets[1], shortfilename.c_str());
	}
	
	hash = sha1HashHex(hashKey);
	
	char* mediaHash = new char[MEDIAINFOHASH_LENGTH*2 + 1];
	memcpy(mediaHash, hash, MEDIAINFOHASH_LENGTH*2);
	mediaHash[MEDIAINFOHASH_LENGTH*2] = '\0';
	delete[] hash;


	m = new MediaInfo(mediaType, mediaHash, interval, numIntervalsinSegment, numPackets, iIndexTable, shortfilename, fileSize, appData, appLen);
	if (m==0) {
		LOG_ERROR("Publisher\t: MediaInfo cannot be created");
		delete[] pIndexTable;
		delete[] iIndexTable;
		return false;
	}
	LOG_DEBUG("Publisher\t: MediaInfo is created");
	
	b = new BootstrapStorage(2, m->getNumSegments(), pIndexTable, totalNumPackets, filename);
	if (b==0) {
		LOG_ERROR("Publisher\t: BootstrapStorage cannot be created");
		delete[] pIndexTable;
		delete[] iIndexTable;
		delete m;
		return false;
	}
	LOG_DEBUG("Publisher\t: BootstrapStorage is created");

	return true;
}

_VMESH_NAMESPACE_FOOTER
