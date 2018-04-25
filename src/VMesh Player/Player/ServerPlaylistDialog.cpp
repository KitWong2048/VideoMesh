// ServerPlaylistDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VMesh Player.h"
#include "ServerPlaylistDialog.h"
#include "ServerDialog.h"
#include ".\serverplaylistdialog.h"
#include <iostream>
using std::cout;

// CServerPlaylistDialog dialog

IMPLEMENT_DYNAMIC(CServerPlaylistDialog, CDialog)

CServerPlaylistDialog::CServerPlaylistDialog(CString* playlist, CString* ipaddress, int* port, int& count,  CWnd* pParent /*=NULL*/)
:CDialog(CServerPlaylistDialog::IDD, pParent)
	, m_SelectedItem(-1)
	, m_PlaylistCount(count)
	, m_CurrentDirectory(_T(""))
	, m_TotalMovieCount(0)
{
	m_PlaylistName = playlist;
	m_IPAddress = ipaddress;
	m_ServerPort = port;
}

CServerPlaylistDialog::~CServerPlaylistDialog()
{
}

void CServerPlaylistDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVERLIST, m_ServerListCtrl);
	DDX_Control(pDX, IDC_REMOVEBUTTON, m_RemoveButton);
	DDX_Control(pDX, IDC_MOVEUPBUTTON, m_MoveUpButton);
	DDX_Control(pDX, IDC_MOVEDOWNBUTTON, m_MoveDownButton);
	DDX_Control(pDX, IDC_SAVEBUTTON, m_SaveButton);
}


BEGIN_MESSAGE_MAP(CServerPlaylistDialog, CDialog)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnBnClickedAddbutton)
	ON_NOTIFY(NM_CLICK, IDC_SERVERLIST, OnNMClickServerlist)
	ON_BN_CLICKED(IDC_MOVEUPBUTTON, OnBnClickedMoveupbutton)
	ON_NOTIFY(NM_RCLICK, IDC_SERVERLIST, OnNMRclickServerlist)
	ON_BN_CLICKED(IDC_MOVEDOWNBUTTON, OnBnClickedMovedownbutton)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnBnClickedRemovebutton)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_SAVEBUTTON, OnBnClickedSavebutton)
END_MESSAGE_MAP()


// CServerPlaylistDialog message handlers

BOOL CServerPlaylistDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ServerListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	CRect rect;
	m_ServerListCtrl.GetClientRect(&rect);
	
	double nColInterval = (double) rect.Width() / 100.0;
	m_ServerListCtrl.InsertColumn(0, _T("Server"), LVCFMT_LEFT	, nColInterval * 40);
	m_ServerListCtrl.InsertColumn(1, _T("Playlist"),	LVCFMT_CENTER	, nColInterval * 40);
	m_ServerListCtrl.InsertColumn(2, _T("Count"),	LVCFMT_RIGHT	, nColInterval * 20);

	int i;
	CString filename;
	CString countStr;
	CString url;
	int count;
	m_TotalMovieCount = 0;
	for ( i = 0; i < m_PlaylistCount; i++ )
	{
		
		url.Format("http://%s:%d/%s.lst", m_IPAddress[i], m_ServerPort[i], m_PlaylistName[i]);
		filename.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, m_PlaylistName[i], m_IPAddress[i], m_ServerPort[i]);
		if ( DownloadFile(url, filename) == false )
			continue;
		count = ReadPlaylistCount(filename);
		if ( count <= 0 )
			continue;

		m_Count[i] = (unsigned int)count;
		m_TotalMovieCount += count;
		countStr.Format("%d", count);
		m_ServerListCtrl.InsertItem(i, m_IPAddress[i]);
		m_ServerListCtrl.SetItemText(i, 1, m_PlaylistName[i]);
		m_ServerListCtrl.SetItemText(i, 2, countStr);
		m_ServerListCtrl.SetItemData(i, m_ServerPort[i]);
	}
	ControlButtons();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CServerPlaylistDialog::OnNMClickServerlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
    LVHITTESTINFO ht;
    ht.pt = pNM->ptAction;
	m_ServerListCtrl.SubItemHitTest(&ht);
	m_SelectedItem = ht.iItem;
	ControlButtons();
	*pResult = 0;
}


void CServerPlaylistDialog::OnNMRclickServerlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
    LVHITTESTINFO ht;
    ht.pt = pNM->ptAction;
	m_ServerListCtrl.SubItemHitTest(&ht);
	m_SelectedItem = ht.iItem;
	ControlButtons();
	*pResult = 0;
}

void CServerPlaylistDialog::OnBnClickedMoveupbutton()
{
	m_ServerListCtrl.SetFocus();
	if ( m_SelectedItem < 1 )
		return;
	m_ServerListCtrl.InsertItem(m_SelectedItem - 1, m_ServerListCtrl.GetItemText(m_SelectedItem, 0));
	int i;
	for ( i = 1 ; i < m_ServerListCtrl.GetHeaderCtrl()->GetItemCount() ; i++ )
		m_ServerListCtrl.SetItemText(m_SelectedItem - 1, i, m_ServerListCtrl.GetItemText(m_SelectedItem + 1, i));
	m_ServerListCtrl.SetItemData(m_SelectedItem - 1, m_ServerListCtrl.GetItemData(m_SelectedItem + 1));
	m_ServerListCtrl.DeleteItem(m_SelectedItem + 1);
	m_ServerListCtrl.SetItemState(m_SelectedItem - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_SelectedItem--;
	ControlButtons();
}


void CServerPlaylistDialog::OnBnClickedMovedownbutton()
{
	m_ServerListCtrl.SetFocus();
	if ( m_SelectedItem >= m_ServerListCtrl.GetItemCount() - 1  || m_SelectedItem == -1 )
		return;
	m_ServerListCtrl.InsertItem(m_SelectedItem + 2, m_ServerListCtrl.GetItemText(m_SelectedItem, 0));
	int i;
	for ( i = 1 ; i <  m_ServerListCtrl.GetHeaderCtrl()->GetItemCount() ; i++ )
		m_ServerListCtrl.SetItemText(m_SelectedItem + 2, i, m_ServerListCtrl.GetItemText(m_SelectedItem, i));
	m_ServerListCtrl.SetItemData(m_SelectedItem + 2, m_ServerListCtrl.GetItemData(m_SelectedItem));
	m_ServerListCtrl.DeleteItem(m_SelectedItem);
	m_ServerListCtrl.SetItemState(m_SelectedItem + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_SelectedItem++;
	ControlButtons();
}

void CServerPlaylistDialog::OnBnClickedRemovebutton()
{
	m_ServerListCtrl.SetFocus();
	if ( m_SelectedItem == -1 )
		return;
	CString filename;
	CString ip = m_ServerListCtrl.GetItemText(m_SelectedItem, 0);
	CString playlistName = m_ServerListCtrl.GetItemText(m_SelectedItem, 1);
	int port = m_ServerListCtrl.GetItemData(m_SelectedItem);

	filename.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, playlistName, ip, port);
	remove(filename);
	m_ServerListCtrl.DeleteItem(m_SelectedItem);
	m_SelectedItem = -1;
	ControlButtons();
}

void CServerPlaylistDialog::OnBnClickedAddbutton()
{
	if ( m_ServerListCtrl.GetItemCount() == MAX_LIST )
	{
		::AfxMessageBox("Limit exceeded");
		return;
	}
	CServerDialog dlg;
	dlg.SetDialogTitle("Add a server playlist");
	if ( dlg.DoModal() == IDCANCEL )
		return;
	unsigned char ip[4];
	int port;
	int i;
	CString playlistname;
	CString url;
	CString ipstring;
	CString localfilelocation;

	dlg.GetServerInfo(ip[0], ip[1], ip[2], ip[3], port, playlistname);

	ipstring.Format("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

	for ( i = 0; i < m_ServerListCtrl.GetItemCount() ; i++ )
		if ( m_ServerListCtrl.GetItemText( i, 0 ) == ipstring && m_ServerListCtrl.GetItemText( i, 1 ) == playlistname && m_ServerListCtrl.GetItemData(i) == port )
		{
			::AfxMessageBox("Playlist has been added before");
			return;
		}

	url.Format("http://%u.%u.%u.%u:%d/%s.lst", ip[0], ip[1], ip[2], ip[3], port, playlistname);
	localfilelocation.Format("%s\\Playlist\\%s@%u.%u.%u.%u.%d.lst", m_CurrentDirectory, playlistname, ip[0], ip[1], ip[2], ip[3], port);
	if ( DownloadFile(url, localfilelocation) == false )
	{
		::AfxMessageBox("Unable to load the playlist from server");
		return;
	}

	int count = ReadPlaylistCount(localfilelocation);
	if ( count == 0 )
	{
		::AfxMessageBox("No movie in playlist");
		return;
	}
	if ( count < 0 )
	{
		::AfxMessageBox("Error reading playlist");
		return;
	}
	CString countStr;
	countStr.Format("%d", count);
	
	int pos = m_ServerListCtrl.GetItemCount();
	m_ServerListCtrl.InsertItem(pos, ipstring);
	m_ServerListCtrl.SetItemText(pos, 1, playlistname);
	m_ServerListCtrl.SetItemText(pos, 2, countStr);
	m_ServerListCtrl.SetItemData(pos, port);
	m_Count[pos] = (unsigned int) count;
	ControlButtons();
}
int CServerPlaylistDialog::GetPlaylistCount(void)
{
	return m_PlaylistCount;
}

void CServerPlaylistDialog::OnBnClickedOk()
{
	m_PlaylistCount = m_ServerListCtrl.GetItemCount();
	int i;
	m_TotalMovieCount = 0;
	for ( i = 0; i < m_PlaylistCount; i++ )
	{
		m_IPAddress[i] = m_ServerListCtrl.GetItemText(i, 0);
		m_PlaylistName[i] = m_ServerListCtrl.GetItemText(i, 1);
		m_ServerPort[i] = m_ServerListCtrl.GetItemData(i);
		m_TotalMovieCount += m_Count[i];
	}
	OnOK();
}

void CServerPlaylistDialog::GetPlaylistInfo(int nItem, CString& playlistname, CString& ipaddress, int& port)
{
	if ( nItem >= m_PlaylistCount || nItem < 0 )
	{
		playlistname = "";
		ipaddress = "";
		port = -1;
	}

	playlistname = m_PlaylistName[nItem];
	ipaddress = m_IPAddress[nItem];
	port = m_ServerPort[nItem];
}

bool CServerPlaylistDialog::DownloadFile(CString src, CString dest)
{
	CInternetSession sess;
	CHttpFile* fileGet;
	CString strSentence;
	try{
		fileGet=(CHttpFile*)sess.OpenURL(src, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
	}
	catch (CInternetException* e)
	{
		//::AfxMessageBox("Invalid Host or IP address");
		return false;
	}
	int byteCount;
	bool state = true;
	char data[4096];
	if(fileGet)
    {
        DWORD dwStatus;
        DWORD dwBuffLen = sizeof(dwStatus);
        BOOL bSuccess = fileGet->QueryInfo(HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwBuffLen);
        if( bSuccess && dwStatus>= 200&& dwStatus<300 ) 
        { 
            CFile fileWrite;
            if(fileWrite.Open(dest, CFile::modeWrite|CFile::modeCreate))
            { 
				while((byteCount = fileGet->Read(data, 4096) ))
                {
					fileWrite.Write(data, byteCount);
                }
                fileWrite.Close();
				
				//::AfxMessageBox("File " + dest + " finished download");
            }
            else
            {
                //::AfxMessageBox("Cannot open local file");
				state = false;
            }
        }
        else 
        {
			strSentence.Format("Error opening remote file. Error code: %d", dwStatus);
            //::AfxMessageBox(strSentence);
			state = false;
        }
        fileGet->Close();
        delete fileGet;
	}
    else
	{
        //::AfxMessageBox("Remote file cannot be retrieved");
		state = false;
	}
    sess.Close();
	return state;
}

void CServerPlaylistDialog::SetCurrentDirectory(TCHAR* directory)
{
	m_CurrentDirectory = directory;
}

bool CServerPlaylistDialog::AddPlaylistItem(CString playlistname, CString ipaddress, int port)
{
	if ( m_PlaylistCount == MAX_LIST )
		return false;
	if ( playlistname == "" || ipaddress == "" || port <= 0 || port >= 65535 )
		return false;
	m_PlaylistName[m_PlaylistCount] = playlistname;
	m_IPAddress[m_PlaylistCount] = ipaddress;
	m_ServerPort[m_PlaylistCount] = port;
	m_PlaylistCount++;
}

int CServerPlaylistDialog::ReadPlaylistCount(CString filepath)
{
	char id[5];
	TCHAR count;
	CFile file;
	if ( file.Open(filepath, CFile::modeRead ) == false )
		return -1;
	file.Read(id, 4);
	id[4] = '\0';
	if ( strcmp(id, "SRPL") != 0 )
	{
		file.Close();
		::AfxMessageBox("Invalid Playlist");
		return -1;
	}
	file.Read((void *)&count, 1);
	file.Close();
	if ( count == 0 )
		remove(filepath);
	return count;
}

unsigned int CServerPlaylistDialog::GetTotalMovieCount(void)
{
	return m_TotalMovieCount;
}

void CServerPlaylistDialog::ControlButtons(void)
{
	if ( m_SelectedItem == -1 )
	{
		m_SaveButton.EnableWindow(false);
		m_RemoveButton.EnableWindow(false);
		m_MoveUpButton.EnableWindow(false);
		m_MoveDownButton.EnableWindow(false);
		return;
	}
	m_SaveButton.EnableWindow(true);
	m_RemoveButton.EnableWindow(true);
	m_MoveUpButton.EnableWindow((m_SelectedItem > 0));
	m_MoveDownButton.EnableWindow((m_SelectedItem < (m_ServerListCtrl.GetItemCount() - 1)));

}

/*
void CServerPlaylistDialog::OnBnClickedRefreshbutton()
{
	if ( m_SelectedItem < 0 || m_SelectedItem >= m_ServerListCtrl.GetItemCount())
		return;
	m_ServerListCtrl.SetFocus();
	CString url;
	CString localfilelocation;
	url.Format("http://%s:%d/%s.lst", m_ServerListCtrl.GetItemText(m_SelectedItem, 0), m_ServerListCtrl.GetItemData(m_SelectedItem), m_ServerListCtrl.GetItemText(m_SelectedItem, 1));
	localfilelocation.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, m_ServerListCtrl.GetItemText(m_SelectedItem, 1), m_ServerListCtrl.GetItemText(m_SelectedItem, 0), m_ServerListCtrl.GetItemData(m_SelectedItem));
	if ( DownloadFile(url, localfilelocation) == false )
		return;

	int count = ReadPlaylistCount(localfilelocation);
	if ( count == 0 )
	{
		::AfxMessageBox("No movie in playlist");
		m_ServerListCtrl.DeleteItem(m_SelectedItem);
		return;
	}
	if ( count < 0 )
		return;

	CString countStr;
	countStr.Format("%d", count);
	
	m_ServerListCtrl.SetItemText(m_SelectedItem, 2, countStr);
	m_Count[m_SelectedItem] = (unsigned int) count;
	
}*/

void CServerPlaylistDialog::OnBnClickedSavebutton()
{
	if ( m_SelectedItem == -1 || m_SelectedItem >= m_ServerListCtrl.GetItemCount() )
		return;

	CFileDialog dlg(false, ".server", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, "Server Information (.server) |*.server||");
	dlg.m_pOFN->lpstrTitle = _T("Save as new file");
	if (dlg.DoModal() == IDCANCEL )
		return;

	int port = m_ServerListCtrl.GetItemData(m_SelectedItem);
	unsigned char portStr[2] = { (port >> 8) & 0xff, port & 0xff };

	unsigned char ip[4];
	unsigned int temp[4];
	sscanf(m_ServerListCtrl.GetItemText(m_SelectedItem, 0), "%d.%d.%d.%d", &temp[0], &temp[1], &temp[2], &temp[3]);
	int i;
	for ( i = 0 ; i < 4; i ++ )
		ip[i] = (unsigned char) temp[i];

	CString playlistName = m_ServerListCtrl.GetItemText(m_SelectedItem, 1);
	int length = playlistName.GetLength();
	char buffer[2] = { (length >> 8) & 0xff, length & 0xff };

	CFile serverFile;
	if ( serverFile.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite ) == false )
	{
		::AfxMessageBox("Cannot write to the destination file");
		return;
	}
	serverFile.Write("SRIF", 4);
	serverFile.Write(ip, 4);
	serverFile.Write(portStr, 2);
	serverFile.Write(buffer, 2);
	serverFile.Write( playlistName , playlistName.GetLength());
	serverFile.Close();
}
