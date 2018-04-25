// SimpleMediaPlayer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "CWMPPlayer4.h"
#include "CWMPControls3.h"
#include "CWMPMedia3.h"
#include "URLDialog.h"
#include "MainDialog.h"

// CSimpleMediaPlayerApp:
// See SimpleMediaPlayer.cpp for the implementation of this class
//

class CVMeshPlayerApp : public CWinApp
{
public:
	CVMeshPlayerApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	afx_msg void OnExit();
	afx_msg void OnFileOpenmedia();
	afx_msg void OnFileOpenurl();
	afx_msg void OnViewStretch();
};

extern CVMeshPlayerApp theApp;