// AddMovieDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VMesh Server.h"
#include "AddMovieDialog.h"
#include "VMesh ServerDlg.h"
#include ".\addmoviedialog.h"

int CAddMovieDialog::m_InternalCount = 0;
int CAddMovieDialog::m_AvailablePort = PORT_BASE;
// CAddMovieDialog dialog

IMPLEMENT_DYNAMIC(CAddMovieDialog, CDialog)
CAddMovieDialog::CAddMovieDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAddMovieDialog::IDD, pParent)

{
}

CAddMovieDialog::~CAddMovieDialog()
{

}

void CAddMovieDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_PORT, m_Port);
	DDX_Control(pDX, IDC_PATH, m_Path);
	DDX_Control(pDX, IDC_DESC, m_Desc);
	DDX_Control(pDX, IDC_URL, m_URL);
	DDX_Control(pDX, IDOK, m_OK);
}


BEGIN_MESSAGE_MAP(CAddMovieDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_PORT, OnEnChangePort)
	ON_EN_CHANGE(IDC_NAME, OnEnChangeName)
	ON_EN_CHANGE(IDC_PATH, OnEnChangePath)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CAddMovieDialog message handlers

BOOL CAddMovieDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/*WSADATA wd;
	if ( WSAStartup(MAKEWORD(1,1), &wd) != 0 )
	{
		::AfxMessageBox("Winsock initialization error");
		this->OnCancel();
	} */


	CString name;
	name.Format("Movie %02d", m_InternalCount + 1);
	while (((CVMeshServerDlg*)this->GetParent())->CheckIsNameUnique(name) == false)
	{
		m_InternalCount++;
		name.Format("Movie %02d", m_InternalCount + 1);
	} 
	m_Name.SetWindowText(name);


	CString port;
	m_AvailablePort = CheckAvailablePort(PORT_BASE);
	if (m_AvailablePort == -1)
	{
		::AfxMessageBox("No available port");
		this->OnCancel();
	}
	port.Format("%d", m_AvailablePort);
	m_Port.SetWindowText(port);
	m_Port.SetLimitText(5);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CAddMovieDialog::CheckAvailablePort(int nStart)
{
	if (nStart <= 0 || nStart >= 65535)
		nStart = PORT_BASE;
	int port = nStart;
	bool result = false;
	

	do
	{
		if ( ((CVMeshServerDlg*)this->GetParent())->CheckIsPortUnique(port) )
			if ( CheckIsPortAvailable(port) )		
				return port;

		port++;
		
		if ( port >= 65535 )
			port = 1;
		

	} while ( port != nStart );

	return -1;
}

void CAddMovieDialog::OnBnClickedBrowse()
{
	CFileDialog fileDlg(true, 0, 0, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, "ASF File|*.asf||");
	if (fileDlg.DoModal() == IDCANCEL)
		return;
	m_Path.SetWindowText(fileDlg.GetPathName());
	int FileNameStart = fileDlg.GetPathName().ReverseFind('\\') + 1;
	int FileNameEnd = fileDlg.GetPathName().ReverseFind('.');
	m_Name.SetWindowText(fileDlg.GetPathName().Mid(FileNameStart, FileNameEnd - FileNameStart));
}

void CAddMovieDialog::OnEnChangePort()
{
	m_OK.EnableWindow(CheckEmptyControl());
}

void CAddMovieDialog::OnEnChangeName()
{
	m_OK.EnableWindow(CheckEmptyControl());
}

void CAddMovieDialog::OnEnChangePath()
{
	m_OK.EnableWindow(CheckEmptyControl());
}

bool CAddMovieDialog::CheckEmptyControl(void)
{
	CString str;
	m_Name.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	m_Port.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	m_Path.GetWindowText(str);
	if (str.IsEmpty())
		return false;
	return true;
}
void CAddMovieDialog::OnBnClickedOk()
{
	CString name;
	m_Name.GetWindowText(name);
	if (((CVMeshServerDlg*)this->GetParent())->CheckIsNameUnique(name) == false)
	{
		::AfxMessageBox("Name Conflict");
		return;
	}

	CString strPort;
	m_Port.GetWindowText(strPort);
	int port = atoi((LPCTSTR)strPort);
	if (port <= 0 || port >= 65535 )
	{
		::AfxMessageBox("Invalid Port");
		return;
	}

	if ( CheckIsPortAvailable(port) == false )
	{
		::AfxMessageBox("Port is in used");
		return;
	}

	CString portStr;
	portStr.Format("%d", port);
	if (  ((CVMeshServerDlg*)this->GetParent())->FindItemPosition(portStr) != -1 )
	{
		::AfxMessageBox("Port is used for other movie");
		return;
	}

	CString path;
	m_Path.GetWindowText(path);
	if ( CheckFileExistence(path) == false )
	{
		::AfxMessageBox("File does not exist");
		return;
	}
	
	CString strTempName;
	strTempName.Format("Movie %02d", m_InternalCount + 1);
	if (((CVMeshServerDlg*)this->GetParent())->CheckIsNameUnique(strTempName) == false)
		do
		{
			m_InternalCount++;
			strTempName.Format("Movie %02d", m_InternalCount + 1);
		} while (((CVMeshServerDlg*)this->GetParent())->CheckIsNameUnique(strTempName) == false || strTempName == name);

	m_AvailablePort = CheckAvailablePort(port);
	m_Name.GetWindowText(m_StrName);
	m_Path.GetWindowText(m_StrPath);
	m_Port.GetWindowText(m_StrPort);
	m_Desc.GetWindowText(m_StrDesc);
	m_URL.GetWindowText(m_StrURL);

	//WSACleanup();

	OnOK();
}

bool CAddMovieDialog::CheckIsPortAvailable(int port)
{
	if ( port < 1 || port > 65535 )
		return false;
	SOCKET test_socket = socket(AF_INET,SOCK_STREAM,0);
	if ( test_socket == INVALID_SOCKET )
		return false;

	SOCKADDR_IN si;

	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.s_addr = htonl(INADDR_ANY);

	int l = 1;
    setsockopt(test_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));

	if ( bind(test_socket,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		closesocket(test_socket);
		return false;
	}

	closesocket(test_socket);

	port++;

	test_socket = socket(AF_INET,SOCK_STREAM,0);
	if ( test_socket == INVALID_SOCKET )
		return false;

	si.sin_port = htons(port);

    setsockopt(test_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&l, sizeof(l));
	if ( bind(test_socket,(struct sockaddr *) &si,sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
	{
		closesocket(test_socket);
		return false;
	}

	closesocket(test_socket);
	return true;
}

bool CAddMovieDialog::CheckFileExistence(CString path)
{
	CFile file;
	if (!file.Open(path, CFile::modeRead))
		return false;
	file.Close();
	return true;
}

CString CAddMovieDialog::GetMovieName(void)
{
	return m_StrName;
}

CString CAddMovieDialog::GetMoviePath(void)
{
	return m_StrPath;
}

CString CAddMovieDialog::GetPortNumber(void)
{
	return m_StrPort;
}

CString CAddMovieDialog::GetDesc(void)
{
	CFile file;
	file.Open(m_StrPath, CFile::modeRead);
	int fileSize = file.GetLength()/1048576;
	CString fileSizeStr;
	fileSizeStr.Format("%d", fileSize);
	CString returnStr;
	returnStr.Format( "<link>%s<\\link>Movie: %s\nFile Size: %s MBytes\nDescription:\n%s", m_StrURL, m_StrName, fileSizeStr, m_StrDesc);
	returnStr.Replace("\n","\\\\");
	return returnStr;
}

void CAddMovieDialog::OnBnClickedCancel()
{
	//WSACleanup();
	OnCancel();
}
