// VMeshSettingDialog.cpp : implementation file
//

#include "stdafx.h"
//#include "Player.h"
#include <afxinet.h>
#include "resource.h"
#include "VMeshSettingDialog.h"


// CVMeshSettingDialog dialog

IMPLEMENT_DYNAMIC(CVMeshSettingDialog, CDialog)
CVMeshSettingDialog::CVMeshSettingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVMeshSettingDialog::IDD, pParent)
	, m_StrMovie(NULL)
	, m_StrServer(NULL)
	, m_StrPort(NULL)
{
}

CVMeshSettingDialog::~CVMeshSettingDialog()
{
	if ( m_StrMovie )
		free( m_StrMovie );
	if ( m_StrPort )
		free( m_StrPort );
	if ( m_StrServer )
		free( m_StrServer );
}

void CVMeshSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVER, m_Server);
	DDX_Control(pDX, IDC_PORT, m_Port);
	DDX_Control(pDX, IDC_MOVIE, m_Movie);
	DDX_Control(pDX, IDOK, m_OK);
}


BEGIN_MESSAGE_MAP(CVMeshSettingDialog, CDialog)
	ON_EN_CHANGE(IDC_SERVER, OnEnChangeServer)
	ON_EN_CHANGE(IDC_PORT, OnEnChangePort)
	ON_EN_CHANGE(IDC_MOVIE, OnEnChangeMovie)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CVMeshSettingDialog message handlers

BOOL CVMeshSettingDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	char hostname[260];
	WORD wVersionRequested;
	WSADATA wsaData;
	PHOSTENT hostinfo;
	wVersionRequested = MAKEWORD( 2, 0 );
	WSAStartup( wVersionRequested, &wsaData );
	gethostname(hostname,MAX_PATH);
	hostinfo = gethostbyname(hostname);
	strcpy(hostname, inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list));
	WSACleanup();

	m_Server.SetWindowText(hostname);
	m_Port.SetWindowText("9000");
	m_Port.LimitText(5);
	m_Movie.LimitText(8);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool CVMeshSettingDialog::CheckEmptyInput(void)
{
	CString str;
	m_Server.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	m_Port.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	m_Movie.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	return true;
}

void CVMeshSettingDialog::OnEnChangeServer()
{
	m_OK.EnableWindow(CheckEmptyInput());
}

void CVMeshSettingDialog::OnEnChangePort()
{
	m_OK.EnableWindow(CheckEmptyInput());
}

void CVMeshSettingDialog::OnEnChangeMovie()
{
	m_OK.EnableWindow(CheckEmptyInput());
}

void CVMeshSettingDialog::OnBnClickedOk()
{
	CString str;
	m_Port.GetWindowText(str);
	int port = atoi(str);
	if (port < 1 || port > 65535 )
	{
		::AfxMessageBox("Invalid port number");
		return;
	}
	m_Movie.GetWindowText(str);
	if ( str.GetLength() != 8 )
	{
		::AfxMessageBox("Invalid movie hash");
		return;
	}
	m_Server.GetWindowText(str);
	if ( str.IsEmpty()  )
	{
		::AfxMessageBox("Server address is required");
		return;
	}
	if ( str.Find(" ") > -1 )
	{
		::AfxMessageBox("Invalid server address");
		return;
	}

	m_StrPort = (char *) malloc( sizeof(char) * 10 );
	m_Port.GetWindowText(m_StrPort, 10);
	m_StrMovie = (char *) malloc( sizeof(char) * 10 );
	m_Movie.GetWindowText(m_StrMovie, 10);
	m_StrServer = (char *) malloc( sizeof(char) * 100 );
	m_Server.GetWindowText(m_StrServer, 100);
	
	OnOK();
}

const char* CVMeshSettingDialog::GetServerAddress(void)
{
	return m_StrServer;
}

const char* CVMeshSettingDialog::GetMovieHash(void)
{
	return m_StrMovie;
}

const char* CVMeshSettingDialog::GetPortNumber(void)
{
	return m_StrPort;
}
