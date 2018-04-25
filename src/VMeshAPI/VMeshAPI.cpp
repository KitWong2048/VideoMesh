#include "VMeshAPI.h"



/********************
@brief default construction, no actions
*********************/
VMeshServerAPI::VMeshServerAPI(){

}

/************************************
@brief start a VMeshServer accord to the given IP and Port
************************************/
VMeshServerAPI::VMeshServerAPI(string m_ServerIP, int m_ServerPort){
  m_Server = new VMeshServer(m_ServerIP, m_ServerPort);
  
}

bool VMeshServerAPI::MoviePublish(const char *m_Key, string m_filename){

  return m_Server->moviePublish(m_Key, m_filename);
}

void VMeshServerAPI::StartVServer(){
  m_Server->start();
}

const char* VMeshServerAPI::GetMovieHash(const char *m_Key){
  return m_Server->getMediaInfo(m_Key)->getMediaHash();
}

VMeshServerAPI::~VMeshServerAPI(){
  delete m_Server;
}


/***************************************************************************
SWServer Class
***********************************************************************/
SWebServer::SWebServer(){
}

SWebServer::SWebServer(const TCHAR* root, unsigned int port){
	this->StartWServer(root,port);
}

void SWebServer::StartWServer(const TCHAR* root, unsigned int port){
  //start a charlotte server
  Start(root, port);
}

SWebServer::~SWebServer(){
 
}

/**********************************
Common Function for VMesh userspace
***********************************/
string CommonFunc::GetHostIP(){
	return getHostIP();
}
CommonFunc::CommonFunc(){

}
CommonFunc::~CommonFunc(){

}