// SimpleMediaPlayer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VMesh Player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern const char* szClassName = "VMesh_Player";
// CVMeshPlayerApp

BEGIN_MESSAGE_MAP(CVMeshPlayerApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_COMMAND(ID_FILE_OPENMEDIA, OnFileOpenmedia)
	ON_COMMAND(ID_FILE_OPENURL, OnFileOpenurl)
	ON_COMMAND(ID_VIEW_STRETCH, OnViewStretch)
END_MESSAGE_MAP()


// cVMeshPlayerApp construction

CVMeshPlayerApp::CVMeshPlayerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CVMeshPlayerApp object

CVMeshPlayerApp theApp;


// CVMeshPlayerApp initialization

BOOL CVMeshPlayerApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	WNDCLASS wc = {0};
	wc.style = CS_BYTEALIGNWINDOW|CS_SAVEBITS|CS_DBLCLKS;
	wc.lpfnWndProc = DefDlgProc;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(IDR_MAINFRAME);
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wc.lpszClassName = szClassName;

	ATOM cls = RegisterClass(&wc);		// Register the class name, to be used by FindWindow by other instance

	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CMainDialog dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CVMeshPlayerApp::OnExit()
{
	// TODO: Add your command handler code here
	((CMainDialog*)m_pMainWnd)->DestroyWindow();
	exit(0);
}

void CVMeshPlayerApp::OnFileOpenmedia()
{
	// TODO: Add your command handler code here
	CString strFileName;

	CFile f;
	FILE* mediafile;

	CFileDialog FileDlg(TRUE);

	if( FileDlg.DoModal() == IDOK )
	{
		if( (mediafile = fopen((LPCTSTR)FileDlg.GetFileName(), "rb")) == NULL )
		{
			::AfxMessageBox("File cannot be accessed");
			return;
		}
		strFileName.SetString(FileDlg.GetPathName());
		
	}
	else return;
	f.Close();
	//m_pMainWnd->SetWindowText(FileDlg.GetPathName());
	((CMainDialog*)m_pMainWnd)->SetFileName(strFileName);
}

void CVMeshPlayerApp::OnFileOpenurl()
{
	// TODO: Add your command handler code here
	CString strURL;
	CURLDialog UrlDlg;
	
	if (((CMainDialog*)m_pMainWnd)->GetPreviousURL().GetLength() > 4 && ((CMainDialog*)m_pMainWnd)->GetPreviousURL().Left(4) == "http")
		UrlDlg.url.SetString(((CMainDialog*)m_pMainWnd)->GetPreviousURL());
	
	if( UrlDlg.DoModal() == IDOK )
	{
		if( UrlDlg.GetURL().IsEmpty() )
			return;
		strURL.SetString(UrlDlg.GetURL());
	}
	else return;
	((CMainDialog*)m_pMainWnd)->SetFileName(strURL);
}

void CVMeshPlayerApp::OnViewStretch()
{
	// TODO: Add your command handler code here
	((CMainDialog*)m_pMainWnd)->SetStretch();
}
