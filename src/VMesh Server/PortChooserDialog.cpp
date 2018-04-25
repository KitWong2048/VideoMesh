// PortChooserDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VMesh Server.h"
#include "VMesh ServerDlg.h"
#include "PortChooserDialog.h"
#include ".\portchooserdialog.h"


// CPortChooserDialog dialog

IMPLEMENT_DYNAMIC(CPortChooserDialog, CDialog)
CPortChooserDialog::CPortChooserDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPortChooserDialog::IDD, pParent)
	, m_Port(0)
{
	srand(time(NULL));
}

CPortChooserDialog::~CPortChooserDialog()
{
}

void CPortChooserDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_PortCtrl);
	DDX_Control(pDX, IDOK, m_OKButton);
}


BEGIN_MESSAGE_MAP(CPortChooserDialog, CDialog)
	ON_BN_CLICKED(IDC_FINDFREEPORTBUTTON, OnBnClickedFindfreeportbutton)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_PORTCTRL, OnEnChangePortctrl)
END_MESSAGE_MAP()


// CPortChooserDialog message handlers

BOOL CPortChooserDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_PortCtrl.SetLimitText(5);

	int port = SERVER_PORT;
	port = CheckAvailablePort(port);
	CString str;
	str.Format("%d", port);
	m_PortCtrl.SetWindowText(str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CPortChooserDialog::CheckAvailablePort(int nStart)
{
	if (nStart <= 0 || nStart >= 65535)
		nStart = SERVER_PORT;
	int port = nStart;
	do
	{	
		if ( CheckIsPortAvailable(port) )		
			return port;

		port++;
		
		if ( port >= 65535 )
			port = 1;
		
	} while ( port != nStart );
	return -1;
}

bool CPortChooserDialog::CheckIsPortAvailable(int nPort)
{
	if ( nPort < 1 || nPort > 65535 )
		return false;
	SOCKET test_socket = socket(AF_INET,SOCK_STREAM,0);
	if ( test_socket == INVALID_SOCKET )
		return false;

	SOCKADDR_IN si;

	si.sin_family = AF_INET;
	si.sin_port = htons(nPort);
	si.sin_addr.s_addr = htonl(INADDR_ANY);

	int l = 1;
    setsockopt(test_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));

	if ( bind(test_socket,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		closesocket(test_socket);
		return false;
	}

	closesocket(test_socket);
	return true;
}
void CPortChooserDialog::OnBnClickedFindfreeportbutton()
{
	int random_port = rand() % 65535 + 1;
	random_port = CheckAvailablePort(random_port);
	if ( random_port == -1 )
	{
		::AfxMessageBox("No available port");
		return;
	}
	CString str;
	str.Format("%d", random_port);
	m_PortCtrl.SetWindowText(str);
}

int CPortChooserDialog::GetPort(void)
{
	return CheckAvailablePort(m_Port);
}

void CPortChooserDialog::OnBnClickedOk()
{
	CString str;
	m_PortCtrl.GetWindowText(str);
	int port = atoi(str);
	if ( port < 1 || port > 65534 )
	{
		::AfxMessageBox("Invalid port number");
		return;
	}
	if ( CheckIsPortAvailable(port) == false )
	{
		::AfxMessageBox("Port is in use");
		return;
	}
	m_Port = port;
	OnOK();
}

void CPortChooserDialog::OnBnClickedCancel()
{
	m_Port = CheckAvailablePort(SERVER_PORT);
	OnCancel();
}


void CPortChooserDialog::OnEnChangePortctrl()
{
	CString str;
	m_PortCtrl.GetWindowText(str);
	int port = atoi(str);
	if ( port < 1 || port > 65534 )
		m_OKButton.EnableWindow(false);
	else 
		m_OKButton.EnableWindow(true);
}
