#include "ChordManager.h"

#include "../chord/src/App/ChordApp.h"
#include "../chord/src/App/ChordAttr.h"

using namespace std;
using namespace App;

ChordManager::ChordManager(string _myIP, int _myPort) : myIP_(_myIP), myPort_(_myPort)
{
	// initialize the chord DHT
	// prepare attributes for chord, see ChordApp.h for ChordAttr description
	ChordAttr attr;
	//attr.isMultiThreaded = true;
	attr.listeningIP = myIP_;
	attr.listeningPort = myPort_+1;
	//attr.connectionTimeOut = DHT_CONNECTION_TIMEOUT;
	attr.connectionTimeOut = 5*1000;	// for faster termination
	attr.connectionNum = MAX_DHT_CONNECTION;
	attr.stabilizingInterval = STABILIZING_INTERVAL;
	attr.fixingInterval = FIXING_INTERVAL;
	attr.neighborVisitInterval = 60; //seconds //  NEIGHBOR_VISIT_INTERVAL;
	attr.maxSearchTime = 120;
	attr.neighborhoodSize = DEFAULT_NEIGHBOR_SIZE;
	// for logging level,0 LogNormal is for normal use, LogDebug prints out routing information. They can be combined together by LogNormal | LogDebug
	attr.logLevel = Util::LogNormal; //  LogNormal /*| LogDeepDebug | LogDebug | LogAll | LogDebug*/;
	attr.logPath = "";
	//attr.logPath = "DHTLog.txt";
	string message = attr.checkAndCorrect();
	//if (message!="")
		//LOG_DEBUG(message);

	// make a new chord instance
	DHTSocket::PoolSocketMgr::employ(25, 25, attr.listeningIP.c_str(), attr.listeningPort);
	myChord_ = new ChordApp(attr);
}

ChordManager::~ChordManager(void)
{
	SAFE_DELETE(myChord_);
}

int ChordManager::run()
{
	return myChord_->run();
}

int ChordManager::stop()
{
	return myChord_->stop();
}

int ChordManager::addNode(const NetworkID& _id)
{
	ChordNetworkID* chordId = (ChordNetworkID*)&(_id);
	return myChord_->addNode(chordId->genDHTNetworkID(), genAppValue().c_str());
}

int ChordManager::removeNode(const NetworkID& _id)
{
	ChordNetworkID* chordId = (ChordNetworkID*)&(_id);
	return myChord_->removeNode(chordId->genDHTNetworkID());
}

NetworkID* ChordManager::lookup(const NetworkID& _id, int& _hopCount)
{
	/*TODO*/
	return new ChordNetworkID();
	//return new ChordNetworkID(*myChord_->lookup(DHT::DHTNetworkID(_id.toBin()), _hopCount));
}

NetworkID* ChordManager::lookupFullPath(const NetworkID& _id, list<NetworkID*>& _searchPath, int& _hopCount)
{
	DHT::Node* tempSearchPath;
	// calculate the new node ID value
	string searchSubnet = _id.getIP().substr(0, _id.getIP().find('.', _id.getIP().find('.',0)+1)) + ".0.0";
	ChordNetworkID searchId(searchSubnet, _id.getPort(), _id.getSegment(), _id.getMediaHash());
	DHT::DHTNetworkID dht(searchId.genDHTNetworkID());
	DHT::Node* resultNode = myChord_->lookupFullPath(dht, tempSearchPath, _hopCount);
	if(!resultNode)
	{
		printf("LookupFull Path: Nothing Return from Chord\n");
		return 0;
	}
	const unsigned char* key = resultNode->getID();
	int segment;
	segment = key[MEDIAINFOHASH_LENGTH + 1];
	segment += key[MEDIAINFOHASH_LENGTH] * 256;
	string movieHash = resultNode->getIDHex();

	NetworkID* resultId = new ChordNetworkID(resultNode->getNodeIP(), resultNode->getNodePort(), segment, movieHash.substr(0, 2*MEDIAINFOHASH_LENGTH));
	list<NetworkID*> neighborList;
	if ((tempSearchPath != NULL) && (_hopCount>0))
	{
		for (int i = 0; i <= _hopCount; i++)
		{
			key = (tempSearchPath +i)->getID();
			segment = key[MEDIAINFOHASH_LENGTH + 1];
			segment += key[MEDIAINFOHASH_LENGTH] * 256;
			movieHash = (tempSearchPath +i)->getIDHex();
			NetworkID* temp = new ChordNetworkID((tempSearchPath +i)->getNodeIP(), (tempSearchPath +i)->getNodePort(), segment, movieHash.substr(0, 2*MEDIAINFOHASH_LENGTH));
			_searchPath.push_back(temp);
		}
		delete []tempSearchPath;
		tempSearchPath = NULL;
	}
	printf("[DEBUG MSG]ChordManager::Get %d Node in Search Path\n", _searchPath.size());
	return resultId;
}

list<NetworkID*> ChordManager::getNeighbor(const NetworkID& _id)
{
	ChordNetworkID* chordId = (ChordNetworkID*)&(_id);
	DHT::Node* node = new DHT::Node(chordId->genDHTNetworkID());
	DHT::Node** targetNeighbour = myChord_->getNeighbor(*node);


	list<NetworkID*> neighborList;
	for (int i=0; true; i++)
	{
		if (targetNeighbour==NULL || targetNeighbour[i]==NULL)
			break;

		const unsigned char* key = targetNeighbour[i]->getID();
		int segment;
		segment = key[MEDIAINFOHASH_LENGTH + 1];
		segment += key[MEDIAINFOHASH_LENGTH] * 256;
		string movieHash = targetNeighbour[i]->getIDHex();

		NetworkID* neighborID = new ChordNetworkID(targetNeighbour[i]->getNodeIP(), targetNeighbour[i]->getNodePort(), segment, movieHash.substr(0, 2*MEDIAINFOHASH_LENGTH));
		neighborList.push_back(neighborID);
		delete targetNeighbour[i];
	}
	printf("[DEBUG MSG]ChordManager::Get %d Neighbor\n", neighborList.size());
	return neighborList;
}

void ChordManager::print()
{
	return myChord_->print((Util::LogLevel)LogNormal);
}

bool ChordManager::isRunning()
{
	return myChord_->isDHTRunning();
}

int ChordManager::join(const NetworkID& _id, const string _hash)
{
	//// create an initial DHT key for accessing DHT
	ChordNetworkID* myTempNode = new ChordNetworkID(myIP_, myPort_, INITIAL_SEGMENTID, _hash);
	DHT::DHTNetworkID tempID = myTempNode->genDHTNetworkID();

	// bootstrap node initialization
	DHT::Node bootStrapNode;
	bootStrapNode.setNodeIP(_id.getIP().c_str());
	bootStrapNode.setNodePort(_id.getPort());
	bootStrapNode.setNodeID(tempID);
	string appValue = this->genAppValue();

	delete myTempNode;
	// add the initial node to the DHT
	return myChord_->addNode(tempID, appValue.c_str(), bootStrapNode) < 0;
}

string ChordManager::genAppValue()
{
	string app = "";
	const char* temp = myIP_.c_str();
	int IP_array[6];
	sscanf(temp, "%d %*c %d %*c %d %*c %d", &IP_array[0], &IP_array[1], &IP_array[2], &IP_array[3]);
	app += IP_array[0];
	app += IP_array[1];
	app += IP_array[2];
	app += IP_array[3];
	app += myPort_/256;
	app += myPort_%256;
	return app;
}
