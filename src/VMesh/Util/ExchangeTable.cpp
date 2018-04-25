#include "../Util/ExchangeTable.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

ExchangeTable::ExchangeTable():myLock("ExchangeTable")
{
	neighborNodeList=new VMeshRemoteNodeList();
}


ExchangeTable::~ExchangeTable()
{
	neighborNodeList->clearall();
	/*
	for(int i=0;i<neighborNodeList->getSize();i++)
	{
		VMeshRemoteNode* node=neighborNodeList->getNode(i);
		delete node;
		node=NULL;
	}
	*/
}

VMeshRemoteNode* ExchangeTable::pop(int _segmentID)
{
	VMeshRemoteNodeList *result_list=NULL;
	VMeshRemoteNode* node=NULL;
	int foundNode = 0;
	//int maxAccessNum=3;
	myLock.lock();
	result_list=&(neighborMap[_segmentID]);
	for(int i=0;i<result_list->getSize();i++)
	{
		node=result_list->getNode(i);
		if(!node->isSocketUsing())
		{	
			foundNode=1;
			break;
		}
	}

	if(foundNode==0)
	{
		result_list=&(neighborMap[BOOTSTRAP_SEGMENTID]);
		for(int i=0;i<result_list->getSize();i++)
		{
			node=result_list->getNode(i);
			if(!node->isSocketUsing())
			{	
				foundNode=1;
				break;
			}
		}

	}

	if(foundNode)
	{
		node->setSocketUsing();
		node->addAccessCount();
	}
	myLock.release();

	if(foundNode==0)
		return NULL;
	
	else
		return node;
}


void ExchangeTable::push(VMeshRemoteNode *_node)
{
	if (_node==NULL)
		return;
	myLock.lock();
	_node->clrSocketUsing();
	_node->reduceAccessCount();
	myLock.release();

}

void ExchangeTable::insert(const char* _serial, int _length)
{
	VMeshRemoteNodeList* list=new VMeshRemoteNodeList(_serial,_length);
	
	myLock.lock();
	insert(list);
	myLock.release();

	list->clearall();
	delete list;
}


void ExchangeTable::insert(VMeshRemoteNode *_node)
{
	if (_node==NULL)
		return;
	
	int segID= _node->getSegmentID();
	int index=-1;

	myLock.lock();

	VMeshRemoteNodeList * parentList=&(neighborMap[segID]);
	index =parentList->exist(_node);

	if(index!=-1)
	{
		parentList->updatePingTime(index, _node);
	}
	else
	{
		if(parentList->getSize()<maxParentPerSegment)
		{
			//VMeshRemoteNode* newNode=new VMeshRemoteNode(_node->getIP(),_node->getPort(),_node->getSegmentID(),_node->getPeerIdentity());
			neighborNodeList->addNodeByPingTime(_node);
			//parentList->addNode(newNode);
			parentList->addNodeByPingTime(_node);
		}
	}


	myLock.release();
	
} 



void ExchangeTable::insert(VMeshRemoteNodeList *_list)
{
	if (_list==NULL)
		return;

	for (int i=0;i<_list->getSize();i++)
	{
		VMeshRemoteNode* node=_list->getNode(i);
		
		myLock.lock();
		insert(node);
		myLock.release();
	}

} 




VMeshRemoteNodeList *ExchangeTable::getParentBySegID(int _segmentID)
{
	VMeshRemoteNodeList *parentList=NULL;
	VMeshRemoteNodeList *returnList=new VMeshRemoteNodeList();;
	myLock.lock();

	parentList=&(neighborMap[_segmentID]);
	/*
	for(int i=0;i<parentList->getSize();i++)
	{
		VMeshRemoteNode* newNode=new VMeshRemoteNode(*(parentList->getNode(i)));
				//memcpy(tempRemoteNode,myParentList->getNode(i),sizeof(VMeshRemoteNode));
		returnList->addNode(newNode);		
	}
	*/
	myLock.release();
	
	return parentList;
}


VMeshRemoteNode* ExchangeTable::getOneParentBySegID(int _segmentID,VMeshRemoteNodeList* failNodes)
{
	VMeshRemoteNodeList *result_list=NULL;
	VMeshRemoteNode* node=NULL;
	int foundNode = 0;
	//int maxAccessNum=3;

	myLock.lock();
	result_list=&(neighborMap[_segmentID]);
	
	for(int i=0;i<result_list->getSize();i++)
	{
		node=result_list->getNode(i);
		if(!node->isSocketUsing())
		{	
			foundNode=1;

			if(failNodes==NULL)
				break;
			else if(failNodes->getSize()==0)
				break;
			else
			{
				for(int j=0;j<failNodes->getSize();j++)
				{
					if(*(failNodes->getNode(j))==*node)
					{
						foundNode=0;
					}
				}
				if(foundNode==1)
					break;
			}

		}
	}

	if(foundNode==0)
	{
		result_list=&(neighborMap[BOOTSTRAP_SEGMENTID]);
		for(int i=0;i<result_list->getSize();i++)
		{
			node=result_list->getNode(i);
			if(!node->isSocketUsing())
			{	
				foundNode=1;
				break;
			}
		}

	}

	//node->addAccessCount();
	myLock.release();

	if(foundNode==0)
		return NULL;
	
	else
		return node;
}


void ExchangeTable::addAccessCount(VMeshRemoteNode* _node)
{

	if(_node==NULL)
		return;
	myLock.lock();
	_node->addAccessCount();
	myLock.release();

}

void ExchangeTable::reduceAccessCount(VMeshRemoteNode* _node)
{
	if(_node==NULL)
		return;
	myLock.lock();
	_node->reduceAccessCount();
	myLock.release();

}


void ExchangeTable::removeNode(VMeshRemoteNode* _node)
{
	if(_node==NULL)
		return;
	int segID= _node->getSegmentID();

	myLock.lock();	
	if(_node->getAccessCount()<1)
	{
		
		VMeshRemoteNodeList* parentList=&(neighborMap[segID]);
		parentList->sharrowRemoveNode(_node);
		neighborNodeList->deepRemoveNode(_node);
	}
	myLock.release();
}

int ExchangeTable::getSize()
{
	int size=0;
	myLock.lock();
	size=neighborNodeList->getSize();
	myLock.release();
	return size;
}


char* ExchangeTable::serialize(int& _length)
{

	char* result=NULL;
	myLock.lock();
	result=neighborNodeList->serialize(_length,false);
	myLock.release();
	return result;
}


VMeshRemoteNodeList* ExchangeTable::extractNeighborList(int segID, int maxNum)
{
	VMeshRemoteNodeList* result=NULL;
	myLock.lock();
	result=neighborNodeList->extractNeighborList(segID,maxNum);
	myLock.release();
	return result;
}


VMeshRemoteNodeList* ExchangeTable::getNearestNeighbor(int maxNum)
{
	VMeshRemoteNodeList* result=NULL;
	myLock.lock();
	result=neighborNodeList->getNearestNeighbor(maxNum);
	myLock.release();
	return result;
}

void ExchangeTable::showAllByPingTime()
{	
	myLock.lock();
	neighborNodeList->showall();
	myLock.release();
}

void ExchangeTable::showAllBySegmentID()
{
	VMeshRemoteNodeList* list=NULL;
	map<int,VMeshRemoteNodeList>::iterator it;
	myLock.lock();
	for(it=neighborMap.begin();it!=neighborMap.end();++it)
	{
		list=&(it->second);
		list->showall();
	}
	myLock.release();
}

VMeshRemoteNode* ExchangeTable::getNodeByIndex(int index)
{
	VMeshRemoteNode * anode=NULL;
	myLock.lock();
	anode=neighborNodeList->getNode(index);
	myLock.release();
	return anode;
}

_VMESH_NAMESPACE_FOOTER