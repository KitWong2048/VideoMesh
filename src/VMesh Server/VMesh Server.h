// VMesh Server.h : PROJECT_NAME 
//

#pragma once

#ifndef __AFXWIN_H__
	#error 'stdafx.h'
#endif

#include "resource.h"		// I don't ready Chinese :-)


// CVMeshServerApp:
// VMesh Server.cpp
//
#define MAX_MOVIE_NUM 100

class CVMeshServerApp : public CWinApp
{
public:
	CVMeshServerApp();
	public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
private:
	bool IsAppRunning(void);
};

extern CVMeshServerApp theApp;
