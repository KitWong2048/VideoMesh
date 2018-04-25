// VMesh Server.cpp : 
//

#include "stdafx.h"
#include "VMesh Server.h"
#include "VMesh ServerDlg.h"
#include ".\vmesh server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LPCTSTR lpszUniqueClass = "VMesh_Server";

// CVMeshServerApp

BEGIN_MESSAGE_MAP(CVMeshServerApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVMeshServerApp

CVMeshServerApp::CVMeshServerApp()
{
	// TODO: 
}


CVMeshServerApp theApp;


BOOL CVMeshServerApp::InitInstance()
{
	// singleton of the program
	if ( IsAppRunning() )
	{
		CWnd* pWindow = CWnd::FindWindow(lpszUniqueClass, NULL);
		if (pWindow != NULL)
		{
			pWindow->ShowWindow(SW_SHOW);
			pWindow->SetForegroundWindow();
		}
		return false;
	} 

	// Register your unique class name that you wish to use
	WNDCLASS wc = {0};
	wc.style = CS_BYTEALIGNWINDOW|CS_SAVEBITS|CS_DBLCLKS;
	wc.lpfnWndProc = DefDlgProc;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(IDR_MAINFRAME);
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wc.lpszClassName = lpszUniqueClass;

	ATOM cls = RegisterClass(&wc);		// Register the class name, to be used by FindWindow by other instance
	InitCommonControls();
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	SetRegistryKey(_T("VMesh Server"));

	CVMeshServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	
	}
	else if (nResponse == IDCANCEL)
	{
	}

	return FALSE;
}

// check if there is already a running instance
// use this if we want the server program to be a singleton
bool CVMeshServerApp::IsAppRunning(void)
{
	HANDLE hMutex = NULL;
	
	// to make sure that we are running a singleton
	hMutex = CreateMutex( NULL, TRUE, TEXT("Global\\{760151EA-1919-4f38-9758-73BA26E3623B}"));
    if ( GetLastError() == ERROR_ALREADY_EXISTS )
    {
        CloseHandle ( hMutex );
        return TRUE;
    }
    return FALSE;
}
