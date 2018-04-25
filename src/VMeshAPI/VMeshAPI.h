#ifndef VMESH_API_H
#define VMESH_API_H
//@Introduction: This document is used for Generate VMesh General API 
//@Author: Juan
//@Time: Feb 22, 2010
#include "..\VMesh\Interface\VMeshServer.h"
#include "..\VMesh\Util\Utility.h"
#include "Charlotte.h"
#include <string.h>
using namespace VMesh;

#define VMESH_API_MACRO
#define VMesh_API __declspec(dllexport)
#define EXPIMP_TEMPLATE(cmd) extern cmd




/********
 *@brief VMeshServerAPI is used for VMeshServer
 *Afer Objection Creation, bind it and start it for receive imcomming request
 */
class VMesh_API VMeshServerAPI
{
 public:
  VMeshServerAPI();
  ~VMeshServerAPI();
  /************************************
   *@brbif This is used for construct a VMeshServer accroding to the given
   *       server Port and IP address
   *@Param[in] m_ServeIp: server's IP address
   *@Param[in] m_ServerPort: Server's Port
   **********************************/
  VMeshServerAPI(string m_ServerIP, int m_ServerPort);	
  /************************************
   *@brief publish the given movie
   *@return true if the movie is published successfully
   ****/
  bool MoviePublish(const char *m_Key, string filename);
  /***************************************
   *@brief get the Movie's Hash Value when finish publishing
   *@Param m_Key: get the movie default Key
   *@return get the string
   *************************************/
  const char* GetMovieHash(const char *m_Key);
     
  /************************************
   *@brief just start the server
   */
  void StartVServer();
 private:
  string m_ServerIP;
  int m_ServerPort;
  VMeshServer *m_Server;
};

/********************************
 *@brief VMeshServer's WebServer
 *@detail to handle the movie list request
 ******************************/
class VMesh_API SWebServer{
 public:
  SWebServer();
  ~SWebServer();
  /**************************
   *@brief get the current directory and the port
   *@param root: the current directory
   *@param port: the current webserver port
   *************************/
  SWebServer(const TCHAR* root, unsigned int port);
  
  /**************************
   *@brief start the webserver
   *************************/
  void StartWServer(const TCHAR* root, unsigned int port);

};

/********************************
 *@brief CommonFunc for used for common function
 *@detail to handle the movie list request
 ******************************/
class VMesh_API CommonFunc{
public:
	CommonFunc();
	~CommonFunc();
	string GetHostIP();
	

};


#endif
