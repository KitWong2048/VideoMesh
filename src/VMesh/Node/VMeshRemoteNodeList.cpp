#include "../Node/VMeshRemoteNodeList.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

VMeshRemoteNodeList::VMeshRemoteNodeList() //: myLock("VMeshRemoteNodeListLock")
{
	// blank constructor
}

VMeshRemoteNodeList::VMeshRemoteNodeList(VMeshLocalNodeList& oldList) //: myLock("VMeshRemoteNodeListLock")
{
	for (int i =0; i< oldList.getSize(); ++i)
	{
		if ((oldList.getNode(i)!=NULL) && (oldList.getNode(i)->isDHTRegistered()))
		{
			VMeshRemoteNode* nNode = new VMeshRemoteNode(*(oldList.getNode(i)));
			this->addNode(nNode);
		}
	}
}

VMeshRemoteNodeList::VMeshRemoteNodeList(const char* _serial, int _length) //: myLock("VMeshRemoteNodeListLock")
{
	// we know that length of serial() in VMeshRemoteNode is always 9
	/*int num = _length / 9;
	for (int i =0; i< num; i++)
	{
		VMeshRemoteNode *p = new VMeshRemoteNode(_serial+9*i, 9);
		myList.push_back(p);
	}*/

	// Jack's comment::add identity entire to VMeshRemoteNode, the length of node becomes 10
	int num = _length / 10;
	for (int i =0; i< num; i++)
	{
		VMeshRemoteNode *p = new VMeshRemoteNode(_serial+10*i, 10);
		myList.push_back(p);
	}
}

VMeshRemoteNodeList::~VMeshRemoteNodeList()
{
	// also destruct all nodes inside
	/*
	for (vector<VMeshRemoteNode*>::iterator i = myList.begin(); i!=myList.end(); i++)
	{
		if ((*i)!=NULL)
			delete (VMeshRemoteNode*)(*i);
	}
	*/
}

char* VMeshRemoteNodeList::serialize(int& _length, bool isKeepBoostrap)
{

	int dummy = 0;
	int tempLength;

	//myLock.lock();
	//removeNULL();
	// we know that length of serial() in VMeshRemoteNode is always 9
	//int tempLength = myList.size() * 9;

	// Jack's comment::add identity entire
	// we know that length of serial() in VMeshRemoteNode is always 10
	// consider the table exchange some cases dont exchange the bootstrap node
	if(isKeepBoostrap)
		tempLength = myList.size() * 10;
	else
		tempLength = (myList.size()-1)*10;

	char* data = new char[tempLength];
	char* ptrData = data;
	for (int i=0; i<myList.size(); i++)
	{
		if(!myList[i]->isBootstrap()||isKeepBoostrap)
		{
			char* temp = myList[i]->serialize(dummy);
			memcpy(ptrData, temp, 10);
			ptrData += 10;
			delete [] temp;
		}
	}
	_length = tempLength;
	//myLock.release();
	return data;
}

int VMeshRemoteNodeList::getSize()
{
	//myLock.lock();
	int x = myList.size();
	//myLock.release();
	return x;
}

void VMeshRemoteNodeList::clearall()
{
	//myLock.lock();
	for (int i=0; i<myList.size(); i++)
	{
		delete myList[i];
		myList[i] = NULL;
	}
	removeNULL();
	//myLock.release();
	
}


//add by shallow copy
void VMeshRemoteNodeList::addNode(VMeshRemoteNode* _node)
{
	if (_node==NULL)
		return;
	//myLock.lock();
	myList.push_back(_node);
	//myLock.release();
}


/*
*add a node to the remote list by a simple insert sort
*might use shell sort later
*add by sharrow copy
*/
void VMeshRemoteNodeList::addNodeByPingTime(VMeshRemoteNode* _node)
{

	//VMeshRemoteNode* newNode=new VMeshRemoteNode(_node->getIP(),_node->getPort(),_node->getSegmentID(),_node->getPeerIdentity());
	
	vector<VMeshRemoteNode*>::iterator it;
	
	for(it = myList.begin(); it!=myList.end();it++)
	{
		if((*it)->getPingTime()>_node->getPingTime())
			break;
	}

	myList.insert(it,_node);
}


bool VMeshRemoteNodeList::sharrowRemoveNode(VMeshRemoteNode* _node)
{
	if (_node==NULL)
		return false;

	bool deleted = false;
	for (int i=0; i<myList.size(); i++)
	{
		if (myList[i]!=NULL &&myList[i] ==_node)
		{	
			//delete myList[i];
			myList.erase(myList.begin()+i);
			deleted = true;
		}
	}
	return deleted;
}

bool VMeshRemoteNodeList::deepRemoveNode(VMeshRemoteNode* _node)
{
	if (_node==NULL)
		return false;

	bool deleted = false;
	for (int i=0; i<myList.size(); i++)
	{
		if (myList[i]!=NULL && myList[i]== _node)
		{
			delete myList[i];
			myList.erase(myList.begin()+i);
			deleted = true;
		}
	}
	return deleted;
}

VMeshRemoteNode* VMeshRemoteNodeList::getSegmentNode(int n)
{
	//myLock.lock();
	//removeNULL();
	VMeshRemoteNode* p = NULL;
	int foundNode = 0;

	// Jack's comment::checking the sequence before change the server position
	/*for (int i=0; i<myList.size(); i++)
	{
		std::cout << "VMeshRemoteNodeList::checking the sequence before change the server position::\t" << myList[i]->getPeerIdentity() << endl;
	}*/

	/*for (int i=0; i<myList.size(); i++)
	{
		if ( myList[i] == NULL )
			continue;

		// Jack's comment::if node is server identity, then push back to the vector
		// Jack's comment::if the list only contain one node(VMesh Server), we wont perform the priority change
		if (myList.size() > 1)
		{
			if (myList[i]->getPeerIdentity() == SERVER_IDENTITY)
			{
				VMeshRemoteNode* server = myList[i];

				delete myList[i];
				myList[i] = NULL;

				myList.erase(myList.begin()+i);
				myList.push_back(server);
				break;
			}
		}
	}*/

	for (int i=0; i<myList.size(); i++)
	{
		if ( myList[i] == NULL )
			continue;

		if (myList[i]->getSegmentID() == n)
		{
			std::cout << "VMeshRemoteNodeList::checking the sequence before change the server position::\t" << myList[i]->getPeerIdentity() << endl;
			foundNode++;
			if (!myList[i]->isSocketUsing())
			{
				std::cout << "VMeshRemoteNodeList::checking socket available for make connection::\t" << myList[i]->getPeerIdentity() << endl;
				p = myList[i];
				break;
			}
		}
	}

	// Jack's comment::checking the sequence before after the server position
	/*for (int i=0; i<myList.size(); i++)
	{
		std::cout << "VMeshRemoteNodeList::checking the sequence after change the server position::\t" << myList[i]->getPeerIdentity() << endl;
	}*/

	if ((n!=INITIAL_SEGMENTID) && (foundNode<=CONNECT_BOOTSTRAP_THRESHOLD) && (p==NULL))
	{
		for (int i=0; i<myList.size(); i++)
		{
			if ((myList[i]->isBootstrap()) && (!myList[i]->isSocketUsing()))
			{
				p = myList[i];
				break;
			}
		}
	}
	//myLock.release();
	return p;
}

VMeshRemoteNode* VMeshRemoteNodeList::getNode(int n)
{
	//myLock.lock();
	//removeNULL();
	VMeshRemoteNode* ptr = NULL;
	if (n<0 || n>=myList.size())
		ptr = NULL;
	else
		ptr = myList[n];
	//myLock.release();
	return ptr;
}

int VMeshRemoteNodeList::exist(VMeshRemoteNode* node)
{
	if (node==NULL)
	{
		return -1;
	}
	
	//myLock.lock();
	//removeNULL();
	for (int i=0; i<myList.size(); i++)
	{
		if ((*myList[i]) == (*node))
		{
			//myLock.release();
			return i;
		}
		else if (node->getSegmentID()==INITIAL_SEGMENTID)
		{
			if ((myList[i]->getIP() == node->getIP()) && (myList[i]->getPort() == node->getPort()))
			{
				//myLock.release();
				return i;			
			}
		}
	}
	//myLock.release();
	return -1;
}

void VMeshRemoteNodeList::updatePingTime(int index, VMeshRemoteNode* _node)
{
	myList[index]->setPingTime(_node->getPingTime());
}

// Jack's comment::This function has problem!!!
/*
void VMeshRemoteNodeList::merge(VMeshRemoteNodeList& _list)
{
	myLock.lock();
	removeNULL();
	myLock.release();
	while (!_list.myList.empty())
	{
		if (exist(_list.myList.back()))
			delete _list.myList.back();
		else
		{
			myLock.lock();
			this->myList.push_back(_list.myList.back());
			myLock.release();
		}
		_list.myList.pop_back();
	}
}
*/

void VMeshRemoteNodeList::merge(VMeshRemoteNodeList* _list)
{
	//removeNULL();
	if(_list==NULL)
		return;
	for(int i=0;i<_list->getSize();i++)
	{
		VMeshRemoteNode* newNode=new VMeshRemoteNode(_list->getNode(i)->getIP(),_list->getNode(i)->getPort(),_list->getNode(i)->getSegmentID(),_list->getNode(i)->getPeerIdentity());
				//memcpy(tempRemoteNode,myParentList->getNode(i),sizeof(VMeshRemoteNode));
		this->addNode(newNode);		
	}

}

VMeshRemoteNodeList* VMeshRemoteNodeList::extractNeighborList(int segID, int maxNum)
{
	
	sortPingTime();
	VMeshRemoteNodeList *list = new VMeshRemoteNodeList();
	int cntNum[3] = {0,0,0};	// counter of number of nodes of each segment ID
	//myLock.lock();

	//removeNULL();
	for (int i=0; (cntNum[0]+cntNum[1]+cntNum[2]<3*maxNum) && (i<myList.size()); i++)
	{
		if (cntNum[0]<maxNum && myList[i]->getSegmentID()==segID-1)
		{
			//VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID());
			VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID(), myList[i]->getPeerIdentity());
			list->addNode(p);
			cntNum[0]++;
		}
		if (cntNum[1]<maxNum && myList[i]->getSegmentID()==segID)
		{
			//VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID());
			VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID(), myList[i]->getPeerIdentity());
			list->addNode(p);
			cntNum[1]++;
		}
		else if (cntNum[2]<maxNum && myList[i]->getSegmentID()==segID+1)
		{
			//VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID());
			VMeshRemoteNode* p = new VMeshRemoteNode(myList[i]->getIP(), myList[i]->getPort(), myList[i]->getSegmentID(), myList[i]->getPeerIdentity());
			list->addNode(p);
			cntNum[2]++;
		}
	}
	//myLock.release();

	return list;
}

VMeshRemoteNodeList* VMeshRemoteNodeList::getNearestNeighbor(int maxNum)
{
	VMeshRemoteNodeList * returnList=new VMeshRemoteNodeList();

	//begin randomly choose a peer to exchange routing table
	srand((unsigned int)time(0));
	int index= rand() % (myList.size());
	returnList->addNode(myList[index]);
	//end


	vector<VMeshRemoteNode*>::iterator it;
	
	for(it = myList.begin(); it!=myList.end()&&returnList->getSize()<maxNum;it++)
	{
		//returnList->addNode(*it);
		bool add=true;
		for(int i=0;i<returnList->getSize();i++)
		{
			if((*it)->getIP()==returnList->getNode(i)->getIP()&&(*it)->getPort()==returnList->getNode(i)->getPort())
				add=false;
			
		}
		if ((*it)->isBootstrap())
				add=false;
		if(add)
		{
			//VMeshRemoteNode *newNode=new VMeshRemoteNode((*it)->getIP(),(*it)->getPort(),(*it)->getSegmentID(),(*it)->getPeerIdentity());
			(*it)->addAccessCount();
			returnList->addNode(*it);
		}
	}

	return returnList;
}


void VMeshRemoteNodeList::sortSegmentID()
{

	//myLock.lock();

	//removeNULL();
	// a stupid sort
	for (int i=0; i<myList.size(); i++)
		for (int j=0; j<myList.size() - 1; j++)
			if (myList[j]->getSegmentID() > myList[j+1]->getSegmentID())
			{
				VMeshRemoteNode *p = myList[j];
				myList[j] = myList[j+1];
				myList[j+1] = p;
			}
	//myLock.release();
}

// Jack's comment::base on ping time to sort remote node list
void VMeshRemoteNodeList::sortPingTime()
{
	
	//myLock.lock();
	//removeNULL();

	// a stupid sort
	for (int i=0; i<myList.size(); i++)
		for (int j=0; j<myList.size() - 1; j++)
			if (myList[j]->getPingTime() > myList[j+1]->getPingTime())
			{
				VMeshRemoteNode *p = myList[j];
				myList[j] = myList[j+1];
				myList[j+1] = p;
			}
	//myLock.release();
}

void VMeshRemoteNodeList::showall()
{
	for (int i=0; i<myList.size(); i++)
	{
		LOG_INFO((string)"Node " + itos(i+1) + " " + myList[i]->toString());
	}
}

void VMeshRemoteNodeList::removeNULL()
{
	//myLock.lock();
	for (vector<VMeshRemoteNode*>::iterator i = myList.begin(); i!=myList.end(); )
	{
		if ((*i)==NULL)
		{
			myList.erase(i);
			i = myList.begin();
		}
		else
		{
			i++;
		}
	}
	//myLock.release();
}

_VMESH_NAMESPACE_FOOTER
