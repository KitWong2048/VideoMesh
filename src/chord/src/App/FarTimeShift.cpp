/*
FarTimeShift.cpp

Copyright (c) 2007 Hong Kong University of Science and Technology ("HKUST")
This source code is an intellectual property owned by HKUST and funded by
Innovation and Technology Fund (Ref No. GHP/045/05)

Permission is hereby granted, to any person and party obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the
Software with the rights to use, copy, modify and merge, subject to the
following conditions:

1. Commercial use of this source code or any derivatives works from this source
 code in any form is not allowed

2. Redistribution of this source code or any derivatives works from this source
 code in any form is not allowed

3. Any of these conditions can be waived if you get permission from the
copyright holder

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../App/FarTimeShift.h"
#define PRINT_SEARCH_DETAILS 1

using namespace DHTSocket;
using namespace Util;
using namespace DHT;
using namespace std;
using namespace App;

namespace App{
	FarTimeShift::FTSAsyncHandle::FTSAsyncHandle(ChordApp* app) : AsyncHandle(app){
		isLookupDone = false;
	}

	void FarTimeShift::FTSAsyncHandle::setPrefixConstraint(const char* prefix){
		if (prefix != NULL)
			memmove(ftsID, prefix, FTS_ID_LENGTH);
		else bzero(ftsID, FTS_ID_LENGTH);
	}

	bool FarTimeShift::FTSAsyncHandle::isResultReady(){
		return isLookupDone;
	}

	void FarTimeShift::FTSAsyncHandle::performAction(){
		Node **neighbor;
		Node* reqTarget;
		int i;

		if (opType == AsyncLookup){
			reqTarget = NULL;
			neighbor = NULL;

			// print search details if necessary
			if (PRINT_SEARCH_DETAILS == 1){
				cout<<"Hop Count = "<<hopCount<<endl;
				cout<<"search path = ";
				if (searchPath != NULL)
					for (i = 0; i < hopCount; i++)
						cout<<endl<<i<<".) "<<searchPath[i];
				else cout<<"NULL"<<endl;

				cout<<endl;

				if (dest == NULL)
					cout<<"dest = NULL"<<endl;
				else cout<<"dest = "<<*dest<<endl;
			}

			// add the search path and destination to the result set
			// and set a target to request its neighbor
			for (i = 0; i < hopCount; i++)
				if (this->addResult(searchPath[i]))
					reqTarget = &(searchPath[i]);
			if (dest != NULL)
				if (this->addResult(*dest))
					reqTarget = dest;

			// request for a peer's target
			if (reqTarget != NULL){
				neighbor = app->getNeighbor(*reqTarget);
				i = 0;
				if (neighbor != NULL){
					while (neighbor[i] != NULL){
						if (this->addResult(*(neighbor[i])))
							cout<<*neighbor[i]<<endl;
						delete neighbor[i];
						i++;
					}
					delete []neighbor;
				}
			}

			// set the flag to indicate search has completed
			isLookupDone = true;
		}
	}

	bool FarTimeShift::FTSAsyncHandle::addResult(const Node& inputNode){
		if (!memcmp(inputNode.getID(), ftsID, FTS_ID_LENGTH)){
			this->resultSet.insert(inputNode);
			return true;
		}else return false;
	}

	int FarTimeShift::FTSAsyncHandle::getResult(InetAddress* addrList, int len){
		int counter = 0;
		set<Node>::iterator l;
		char* binAddr;
		int i;

		l = this->resultSet.begin();
		while ((l != this->resultSet.end()) && (counter < len)){
			i = l->toBinApp(binAddr);
			if (i == APP_VALUE_LEN)
				addrList[counter].setAddress(binAddr);
			delete []binAddr;
			counter++;
		}

		return counter;
	}

	FarTimeShiftID::FarTimeShiftID(){
	}

	FarTimeShiftID::FarTimeShiftID(const FarTimeShiftID& _ID){
		memmove(ID, _ID.ID, FTS_ID_LENGTH);
	}

	FarTimeShiftID::FarTimeShiftID(const char* charID){
		memmove(ID, charID, FTS_ID_LENGTH);
	}

	FarTimeShiftID& FarTimeShiftID::operator=(const FarTimeShiftID& _ID){
		memmove(ID, _ID.ID, FTS_ID_LENGTH);
		return *this;
	}

	bool FarTimeShiftID::operator==(const FarTimeShiftID& _ID) const{
		return !memcmp(ID, _ID.ID, FTS_ID_LENGTH);
	}

	bool FarTimeShiftID::operator<(const FarTimeShiftID& _ID) const{
		return (memcmp(ID, _ID.ID, FTS_ID_LENGTH) < 0 ? true : false);
	}

	FarTimeShift::FarTimeShift(){
		this->chord = NULL;
		this->resultList = NULL;
		this->asyncLookupHandler = NULL;
		bzero(this->localAddr, ADDRESS_LENGTH);
	}

	FarTimeShift::~FarTimeShift(){
		if (this->chord != NULL){
			delete this->chord;
			PoolSocketMgr::dismiss();
		}

		if (this->resultList != NULL)
			delete []this->resultList;
	}

	void FarTimeShift::init(InetAddress& startUpAddr){
		char* tmp;
		tmp = startUpAddr.getAddress();
		memmove(this->localAddr, tmp, ADDRESS_LENGTH);
		delete []tmp;

		// prepare attributes for chord, see ChordApp.h for ChordAttr description
		ChordAttr attr;
		attr.listeningIP = startUpAddr.getIP();
		attr.listeningPort = startUpAddr.getPort();
		attr.isMultiThreaded = false;
		PoolSocketMgr::employ(25, 25, attr.listeningIP.c_str(), attr.listeningPort);

		// make a new chord instance
		chord = new ChordApp(attr);
		if (chord->run() < 0){
			cout<<"Failed to new a DHT instance."<<endl;
			delete chord;
			chord = NULL;
			PoolSocketMgr::dismiss();
		}else{	// wait for the chord to start
			int sleepingTime = 0;
			while ((sleepingTime < 60) && (!chord->isDHTRunning())){
				sleep(5);
				sleepingTime += 5;
			}

			if (!chord->isDHTRunning()){
				cout<<"Failed to start a DHT."<<endl;
				delete chord;
				chord = NULL;
				PoolSocketMgr::dismiss();
			}
		}

	}

	void FarTimeShift::add(const char* ftsID, InetAddress& serverAddress, const InetAddress* bootstrap){
		AsyncHandle* asyncHandler;

		if (ftsIDs.find(ftsID) != ftsIDs.end()){
			cout<<"The ID has been added before."<<endl;
			return ;
		}else if (chord == NULL){
			cout<<"DHT is not ready."<<endl;
			return ;
		}

		// node ID = fts ID + address
		char ID[FTS_ID_LENGTH + ADDRESS_LENGTH];
		memmove(ID, ftsID, FTS_ID_LENGTH);
		memmove(ID + FTS_ID_LENGTH, localAddr, ADDRESS_LENGTH);
		DHT::DHTNetworkID dhtID;
		dhtID.setID((unsigned char*) ID);

		// node value = application address
		char* binAddress;
		binAddress = serverAddress.getAddress();

		asyncHandler = new AsyncHandle(chord);
		if (bootstrap == NULL)
			asyncHandler->setAddNodeArg(dhtID, binAddress);
		else{
			Node bootstrapNode;
			bootstrapNode = dhtID;
			bootstrapNode.setNodeIP(bootstrap->getIP().c_str());
			bootstrapNode.setNodePort(bootstrap->getPort());
			asyncHandler->setAddNodeArg(dhtID, binAddress, &bootstrapNode);
		}
		delete []binAddress;
		chord->performAsyncAction(asyncHandler);
		ftsIDs.insert(ftsID);
	}

	void FarTimeShift::remove(const char* ftsID){
		AsyncHandle* asyncHandler;

		if (ftsIDs.find(ftsID) != ftsIDs.end()){
			char ID[FTS_ID_LENGTH + ADDRESS_LENGTH];
			memmove(ID, ftsID, FTS_ID_LENGTH);
			memmove(ID + FTS_ID_LENGTH, localAddr, ADDRESS_LENGTH);

			asyncHandler = new AsyncHandle(chord);
			asyncHandler->setRemoveNodeArg((unsigned char*) ID);
			chord->performAsyncAction(asyncHandler);
			ftsIDs.erase(ftsID);
		}else{
			cout<<"No such ID!"<<endl;
		}
	}

	int FarTimeShift::shift(const char* shiftID, InetAddress* addrList, int len){
		return 0;
	}

	void FarTimeShift::asyncShift(const char* shiftID){
		if (ftsIDs.begin() != ftsIDs.end()){
			DHTNetworkID dhtSearchID;
			unsigned char searchID[FTS_ID_LENGTH + ADDRESS_LENGTH];

			memmove(searchID, shiftID, FTS_ID_LENGTH);
			memmove(searchID + FTS_ID_LENGTH, localAddr, ADDRESS_LENGTH);
			dhtSearchID.setID(searchID);
			asyncLookupHandler = new FTSAsyncHandle(chord);
			asyncLookupHandler->setLookupArg(dhtSearchID);
			asyncLookupHandler->setPrefixConstraint(shiftID);
			chord->performAsyncAction(asyncLookupHandler);
		}
	}

	int FarTimeShift::getAsyncResult(InetAddress* addrList, int len){
		if (asyncLookupHandler != NULL)
			if (asyncLookupHandler->isResultReady())
				return asyncLookupHandler->getResult(addrList, len);
		return 0;
	}

	bool FarTimeShift::isResultReady(){
		if (this->asyncLookupHandler == NULL)
			return false;
		else return this->asyncLookupHandler->isResultReady();
	}

	bool FarTimeShift::isSearchReady(){
		if (chord == NULL)
			return false;
		else if ((chord->isDHTRunning()) && (ftsIDs.begin() != ftsIDs.end()))
			return true;

		return false;
	}

	void FarTimeShift::destroy(){
		if (chord != NULL)
			chord->stop();
	}

	void FarTimeShift::clean(){
		if (chord != NULL){
			chord->cleanAsyncList();
		}
	}

	void FarTimeShift::print(){
		if (chord->isDHTRunning())
			chord->print();
		else cerr<<"DHT has not been started yet."<<endl;
	}
	
	bool FarTimeShift::isDHTRunning(){
		return chord->isDHTRunning();	
	}
}
