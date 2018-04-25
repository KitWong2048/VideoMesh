// ServerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VMesh Player.h"
#include "ServerDialog.h"
#include ".\serverdialog.h"


// CServerDialog dialog

IMPLEMENT_DYNAMIC(CServerDialog, CDialog)
CServerDialog::CServerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDialog::IDD, pParent)
	, m_Port(SERVER_PORT)
	, m_FileLocation(_T(""))
	, m_FromFile(false)
	, m_Title(_T(""))
	, m_PlaylistName(_T("Default"))
{
	m_IPAddress[0] = 127;
	m_IPAddress[1] = 0;
	m_IPAddress[2] = 0;
	m_IPAddress[3] = 1;
}

CServerDialog::~CServerDialog()
{
}

void CServerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BROWSE, m_Browse);
	DDX_Control(pDX, IDC_FILESELECTION, m_FileSelection);
	DDX_Control(pDX, IDC_SERVERSELECTION, m_ServerSelection);
	DDX_Control(pDX, IDC_FILELOCATION, m_FileLocationCtrl);
	DDX_Control(pDX, IDC_IPADDRESS, m_IPAddressCtrl);
	DDX_Control(pDX, IDC_PORT, m_PortCtrl);
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Control(pDX, IDC_PLAYLISTNAME, m_PlaylistNameCtrl);
	DDX_Control(pDX, IDC_SAVEBUTTON, m_SaveButton);
}


BEGIN_MESSAGE_MAP(CServerDialog, CDialog)
	ON_EN_CHANGE(IDC_FILELOCATION, OnEnChangeFilelocation)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS, OnIpnFieldchangedIpaddress)
	ON_EN_CHANGE(IDC_PORT, OnEnChangePort)
	ON_EN_CHANGE(IDC_IPADDRESS, OnEnChangeIPAddress) 
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_FILESELECTION, OnBnClickedFileselection)
	ON_BN_CLICKED(IDC_SERVERSELECTION, OnBnClickedServerselection)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_SAVEBUTTON, OnBnClickedSavebutton)
END_MESSAGE_MAP()


// CServerDialog message handlers

BOOL CServerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->SetWindowText(m_Title);
	m_IPAddressCtrl.SetAddress(m_IPAddress[0], m_IPAddress[1], m_IPAddress[2], m_IPAddress[3]);
	m_PortCtrl.SetLimitText(5);
	m_PlaylistNameCtrl.SetLimitText(0xffff);
	m_PlaylistNameCtrl.SetWindowText(m_PlaylistName);
	char port[6];
	m_PortCtrl.SetWindowText(itoa(m_Port, port, 10));
	if ( m_FromFile )
	{
		m_FileSelection.SetCheck(true);
		m_FileLocationCtrl.SetWindowText(m_FileLocation);
	}
	else
		m_ServerSelection.SetCheck(true);

	SetAllControls();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CServerDialog::SetAllControls(void)
{

	m_FileLocationCtrl.EnableWindow(m_FromFile);
	m_Browse.EnableWindow(m_FromFile);

	m_IPAddressCtrl.EnableWindow(!m_FromFile);
	m_PortCtrl.EnableWindow(!m_FromFile);
	m_PlaylistNameCtrl.EnableWindow(!m_FromFile);
	m_SaveButton.EnableWindow(!m_FromFile);

	SetOKButton();
}

void CServerDialog::SetOKButton(void)
{
	bool bEnable = false;
	CString szFileLocation;
	m_FileLocationCtrl.GetWindowText(szFileLocation);
	CString szPlaylistName;
	m_PlaylistNameCtrl.GetWindowText(szPlaylistName);
	if ( m_FromFile && !szFileLocation.IsEmpty() && !szPlaylistName.IsEmpty())
		bEnable = true;
	CString szPort;
	m_PortCtrl.GetWindowText(szPort);
	int port = atoi((LPCTSTR)szPort);
	DWORD ip;
	if ( !m_FromFile && m_IPAddressCtrl.GetAddress(ip) == 4 && port > 0 && port < 65535 )
		bEnable = true;
	m_OKButton.EnableWindow(bEnable);
	m_SaveButton.EnableWindow(!m_FromFile && bEnable);

}

void CServerDialog::OnEnChangeFilelocation()
{
	SetOKButton();
}

void CServerDialog::OnIpnFieldchangedIpaddress(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	SetOKButton();
	*pResult = 0;
}

void CServerDialog::OnEnChangePort()
{
	SetOKButton();
}

void CServerDialog::OnEnChangeIPAddress()
{
	SetOKButton();
}

void CServerDialog::OnBnClickedBrowse()
{
	CFileDialog dlg(true, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT  | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST , "Server Information (.server)|*.server||");
	dlg.m_pOFN->lpstrTitle = "Load Server Information";
	if (dlg.DoModal() == IDCANCEL)
		return;
	if ( CheckServerInformationFile(dlg.GetPathName()) == false)
	{
		::AfxMessageBox("Invalid File");
		return;
	}
	m_FileLocationCtrl.SetWindowText(dlg.GetPathName());
}

void CServerDialog::OnBnClickedFileselection()
{
	m_FromFile = true;
	SetAllControls();
}

void CServerDialog::OnBnClickedServerselection()
{
	m_FromFile = false;
	SetAllControls();
}

void CServerDialog::OnBnClickedOk()
{

	if ( m_FromFile )
	{
		CString szFileLocation;
		m_FileLocationCtrl.GetWindowText(szFileLocation);
		if ( szFileLocation.IsEmpty() )
		{
			::AfxMessageBox("File location cannot be empty");
			return;
		}

		if ( CheckServerInformationFile(szFileLocation) == false )
		{
			::AfxMessageBox("Invalid File");
			return;
		}

		GetServerInformationFromFile(szFileLocation);

		m_FileLocation = szFileLocation;
	}
	else
	{
		DWORD ip;
		if ( !m_FromFile && m_IPAddressCtrl.GetAddress(ip) < 4 )
		{
			::AfxMessageBox("IP address cannot be blank");
			return;
		}

		CString szPort;
		m_PortCtrl.GetWindowText(szPort);
		int port = atoi((LPCTSTR)szPort);
		if ( !m_FromFile && (port <= 0 || port >= 65535) )
		{
			::AfxMessageBox("Invalid port number");
			return;
		}

		m_IPAddressCtrl.GetAddress(m_IPAddress[0], m_IPAddress[1], m_IPAddress[2], m_IPAddress[3]);
		m_Port = port;
		m_PlaylistNameCtrl.GetWindowText(m_PlaylistName);

	}
	OnOK();
}

bool CServerDialog::CheckServerInformationFile(const char * filename)
{
	FILE* file;
	file = fopen(filename, "rb");
	if ( file == NULL )
		return false;
	bool result = false;
	char id[5];
	char ip[4];
	char port[2];
	char len[2];
	do
	{
		if ( feof(file) || ferror(file) )
			break;
		fread(id, sizeof(char), 4, file);

		id[4] = '\0';

		if ( strcmp(id, "SRIF") != 0 || feof(file) || ferror(file) )	// check file identifier
			break;
		fread(ip, sizeof(char), 4, file);

		if ( ( !ip[0] && !ip[1] && !ip[2] && !ip[3] ) || feof(file) || ferror(file) )	// check ip = 0.0.0.0
			break;
		fread(port, sizeof(char), 2, file);

		if ( ( !port[0] && !port[1] ) ) 	// check port = 0
			break;

		fread(len, sizeof(char), 2, file);

		if ( ( !len[0] && !len[1] ) ) 		// check length = 0
			break;

		if ( feof(file) || ferror(file) )
			break;

		result = true;
	} while (false);
	fclose(file);
	return result;
}

void CServerDialog::GetServerInformationFromFile(const char* filename)
{
	FILE* file;
	file = fopen(filename, "rb");
	if ( file == NULL )
		return;

	char id[5];
	fread(id, sizeof(char), 4, file);

	fread(m_IPAddress, sizeof(char), 4, file);

	m_Port = (unsigned char)fgetc(file);
	m_Port <<= 8;
	m_Port |= (unsigned char)fgetc(file);

	unsigned int length = (unsigned char)fgetc(file);
	length <<= 8;
	length |= (unsigned char)fgetc(file);

	char* playlistname = new char[length + 1];
	fread(playlistname, sizeof(char), length, file);
	playlistname[length] = '\0';
	m_PlaylistName = playlistname;

	delete []playlistname;
	fclose(file);
}

void CServerDialog::SetServerInfo(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, int port, CString playlist)
{
	m_IPAddress[0] = ip1;
	m_IPAddress[1] = ip2;
	m_IPAddress[2] = ip3;
	m_IPAddress[3] = ip4;
	if ( port <= 0 || port >= 65535 )
		port = SERVER_PORT;
	m_Port = port;
	m_FromFile = false;
	m_PlaylistName = playlist;
}

void CServerDialog::SetFileLocation(const char* filelocation)
{
	m_FileLocation = filelocation;
	m_FromFile = true;
}

void CServerDialog::GetServerInfo(unsigned char& ip1, unsigned char& ip2, unsigned char& ip3, unsigned char& ip4, int& port, CString& playlist)
{
	ip1 = m_IPAddress[0];
	ip2 = m_IPAddress[1];
	ip3 = m_IPAddress[2];
	ip4 = m_IPAddress[3];
	port = m_Port;
	playlist = m_PlaylistName;
}

void CServerDialog::GetFileLocation(CString& filelocation)
{
	filelocation = m_FileLocation;
}

bool CServerDialog::GetSource()
{
	return m_FromFile;
}

void CServerDialog::SetDialogTitle(CString title)
{
	m_Title = title;
}

void CServerDialog::OnBnClickedSavebutton()
{
	CFileDialog dlg(false, ".server", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, "Server Information (.server) |*.server||");
	dlg.m_pOFN->lpstrTitle = _T("Save as new file");
	if (dlg.DoModal() == IDCANCEL )
		return;

	CString szPort;
	m_PortCtrl.GetWindowText(szPort);
	int port = atoi((LPCTSTR)szPort);
	unsigned char portStr[2] = { (port >> 8) & 0xff, port & 0xff };

	unsigned char ip[4];
	m_IPAddressCtrl.GetAddress(ip[0], ip[1], ip[2], ip[3]);

	CString playlistName;
	m_PlaylistNameCtrl.GetWindowText(playlistName);
	int length = playlistName.GetLength();
	char buffer[2] = { (length >> 8) & 0xff, length & 0xff };

	CFile serverFile;
	serverFile.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite );
	serverFile.Write("SRIF", 4);
	serverFile.Write(ip, 4);
	serverFile.Write(portStr, 2);
	serverFile.Write(buffer, 2);
	serverFile.Write( playlistName , playlistName.GetLength());
	serverFile.Close();
}
