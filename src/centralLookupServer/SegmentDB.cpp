#include "SegmentDB.h"

SegmentDB::SegmentDB(void)
{
}

SegmentDB::~SegmentDB(void)
{
}

std::string SegmentDB::Query(const char* command, const char* para, const char* data, int identity)
{
	printf("##################################\n");
	printf("SegmentDB is handling %s command\n",command);
	printf("##################################\n");
	
	
	std::string retChar = "NOT IMPLEMENTED";
	if(strlen((const char*)para) != ID_LENGTH*2)
	{
		printf("ERROR HashKey(%d|%d)", strlen((const char*)para), ID_LENGTH*2);
		retChar = "ERROR HashKey";
		return retChar;
	}
	Node nodeInfo(para);
	if(strcmp(command, "ADDNODE") == 0)
	{
		addNode(nodeInfo, identity);
		retChar = "ADD NODE";
		return retChar;
	}
	if(strcmp(command, "REMOVENODE") == 0)
	{
		removeNode(nodeInfo, identity);
		retChar = "REMOVE NODE SUCCESS";
		return retChar;
	}
	if(strcmp(command, "CREATEMOVIE") == 0)
	{
		createMovie(nodeInfo);
		retChar = "CREATE MOVIE SUCCESS";
		return retChar;
	}
	if(strcmp(command, "DELETEMOVIE") == 0)
	{
		deleteMovie(nodeInfo);
		retChar = "DELETE MOVIE SUCCESS";
		return retChar;
	}
	if(strcmp(command, "NODELOOKUP") == 0)
	{
		char* mydata = new char[strlen(data)];
		strcpy(mydata, data);
		retChar = nodeLookup(nodeInfo, identity);
		char* pos = strtok(mydata,"\n");
		while ( pos != NULL )
		{
			Node appNode((const char*)pos);
			addNode(appNode, identity);
			pos = strtok(NULL,"\n");
		}
		return retChar;
	}

	printf("NOT IMPLEMENTED: %s\n", command);
	return retChar;
}

int SegmentDB::addNode(Node node,int identity)
{
	// return error if the Movie existed
	if(moviesDB_.find(node.genAppKey()) == moviesDB_.end())
	{
		printf("Add New Node(Fail: Movie not exited): (%s--%d)%s:%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort());
		return -1;
	}
	int results = moviesDB_[node.genAppKey()]->addNode(node,identity);
	if (results==0)
		printf("New Node Added: (%s--%d)%s:%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort());
	else
		printf("Node Updated: (%s--%d)%s:%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort());
	return 0;
}

int SegmentDB::removeNode(Node node, int identity)
{
	// return error if the Movie existed
	if(moviesDB_.find(node.genAppKey()) == moviesDB_.end())
	{
		printf("Remove Node(Fail: Movie not exited): (%s)%s:%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort());
		return -1;
	}
	moviesDB_[node.genAppKey()]->removeNode(node);
	printf("Remove Node: (%s)%s:%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort());
	return 0;
}

int SegmentDB::createMovie(Node node)
{
	// return error if the Movie existed
	if(moviesDB_.find(node.genAppKey()) != moviesDB_.end())
	{
		printf("New Movie Added(Fail: Movie Existed): (%s)%s:%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort());
		return -1;
	}
	moviesDB_[node.genAppKey()] = new PeerList(node.getMediaHash());
	addNode(node, SERVER_IDENTITY);
	printf("New Movie Added: (%s)%s:%d::%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort(), node.getPeerIdentity());
	return 0;
}

int SegmentDB::deleteMovie(Node node)
{
	// return error if the Movie not existed
	std::map<std::string, PeerList*>::iterator it = moviesDB_.find(node.genAppKey());
	if(it == moviesDB_.end())
	{
		printf("Movie Deleted(Not Existed: (%s)%s:%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort());
	}
	else
	{
		delete moviesDB_[node.genAppKey()];
		moviesDB_.erase(node.genAppKey());
		printf("Movie Deleted: (%s)%s:%d\n", node.genAppKey().c_str(), node.getIP().c_str(), node.getPort());
	}
	return 0;
}

std::string SegmentDB::nodeLookup(Node node, int identity)
{
	std::string ret = "";
	// return error if the Movie existed
	if(moviesDB_.find(node.genAppKey()) == moviesDB_.end())
	{
		printf("Lookup Node(Fail: Movie not exited): (%s--%d)%s:%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort());
		return "";
	}

	int count = NUMBER_OF_LOOKUP;
	Node* nodeList = moviesDB_[node.genAppKey()]->lookup(node.getSegment(), count, identity);
	//printf("Lookup Node: (%s--%d)%s:%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort());
	printf("Lookup Node: (%s--%d)%s:%d::%d\n", node.genAppKey().c_str(), node.getSegment(), node.getIP().c_str(), node.getPort(), node.getPeerIdentity());
	printf("Node List:\n");
	for(int i=0; i< count; i++)
	{
		ret = ret + (nodeList+i)->genHashKey() + '\n';
		//printf("(%s--%d)%s:%d\n", (*(nodeList+i)).genAppKey().c_str(), (*(nodeList+i)).getSegment(), (*(nodeList+i)).getIP().c_str(), (*(nodeList+i)).getPort());
		printf("(%s--%d)%s:%d::%d\n", (*(nodeList+i)).genAppKey().c_str(), (*(nodeList+i)).getSegment(), (*(nodeList+i)).getIP().c_str(), (*(nodeList+i)).getPort(),(*(nodeList+i)).getPeerIdentity());
	}
	return ret;
}
