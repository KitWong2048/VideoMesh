#include "../Node/VMeshLocalNodeList.h"

using namespace std;

_VMESH_NAMESPACE_HEADER

VMeshLocalNodeList::VMeshLocalNodeList()
{
	// blank constructor
}

VMeshLocalNodeList::~VMeshLocalNodeList()
{
	// destruct all the VMeshLocalNode in the list
	for (vector<VMeshLocalNode*>::iterator i = myList.begin(); i!=myList.end(); i++)
	{
		if ((*i)!=NULL)
			delete (VMeshLocalNode*)(*i);
	}
}

int VMeshLocalNodeList::getSize()
{
	return (int)myList.size();
}

void VMeshLocalNodeList::addNode(VMeshLocalNode* _node)
{
	myList.push_back(_node);
}

VMeshLocalNode* VMeshLocalNodeList::getNodeBySegmentID(int _segmentID)
{
	for (vector<VMeshLocalNode*>::iterator i = myList.begin(); i!=myList.end(); i++)
	{
		if ((*i)!=NULL)
			if ((*i)->getSegmentID() == _segmentID || (*i)->getSegmentID() == BOOTSTRAP_SEGMENTID )
				return (VMeshLocalNode*)(*i);
	}
	return NULL;
}

VMeshLocalNode* VMeshLocalNodeList::getNode(int i)
{
	if ( i >= (int)myList.size())
		return NULL;
	else
		return myList[i];
}

_VMESH_NAMESPACE_FOOTER
