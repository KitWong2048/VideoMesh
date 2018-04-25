// PlaylistDialog.cpp : implementation file
// TODO: 
//	1. Move all the GUI part to a central GUI project
//  2. Design an interface for the functions needed 

#include "stdafx.h"
#include "Shlwapi.h"
#include "..\VMesh\Interface\VMeshServer.h"
#include "VMesh Server.h"
#include "PlaylistDialog.h"
#include <iostream>
using std::string;

using namespace VMesh;
// CMovieListDialog dialog

IMPLEMENT_DYNAMIC(CPlaylistDialog, CDialog)
CPlaylistDialog::CPlaylistDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaylistDialog::IDD, pParent)
	, m_MovieCount(0)
	, m_CheckedRow(-1)
	, m_CurrentDirectory(_T(""))
{
}

CPlaylistDialog::~CPlaylistDialog()
{
}

void CPlaylistDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOVIELIST, m_MovieListCtrl);
	DDX_Control(pDX, IDC_LISTNAME, m_ListNameCtrl);
	DDX_Control(pDX, IDOK, m_SaveButton);
}


BEGIN_MESSAGE_MAP(CPlaylistDialog, CDialog)
	ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
	ON_BN_CLICKED(IDC_SELECTNONE, OnBnClickedSelectnone)
	ON_EN_CHANGE(IDC_LISTNAME, OnEnChangeListname)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CMovieListDialog message handlers

BOOL CPlaylistDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	TCHAR szDirectory[MAX_PATH];
	::GetModuleFileName(NULL, szDirectory, sizeof(szDirectory) - 1 );

	int i;
	for ( i = strlen(szDirectory) - 1; i >= 0; i-- )
		if ( szDirectory[i] == '/' || szDirectory[i] == '\\' )
			break;
	if ( i < 0 )
		i = 0;

	m_CurrentDirectory = CString(szDirectory).Left(i);

	CreateDirectory(m_CurrentDirectory + "\\Playlist", NULL);

	m_MovieListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT  | LVS_EX_CHECKBOXES | 0x00008000 );
	CRect rect;
	m_MovieListCtrl.GetClientRect(&rect);
	
	double nColInterval = (double) rect.Width() / 100.0;
	m_MovieListCtrl.InsertColumn(0, _T("Name"), LVCFMT_LEFT	, nColInterval * 80);
	m_MovieListCtrl.InsertColumn(1, _T("Port"),	LVCFMT_LEFT	, nColInterval * 20);

	for ( i = 0 ; i < m_MovieCount; i++ )
	{
		m_MovieListCtrl.InsertItem(i, m_MovieName[i]);
		m_MovieListCtrl.SetItemText(i, 1, m_MoviePort[i]);
		if ( m_bIsChecked[i] )
			m_MovieListCtrl.SetCheck( i , 1 );
	}	

	m_ListNameCtrl.SetWindowText(_T("Playlist"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPlaylistDialog::OnBnClickedSelectall()
{
	int i;
	for ( i = 0; i < m_MovieListCtrl.GetItemCount() ; i++ )
		m_MovieListCtrl.SetCheck( i , 1 );
}

void CPlaylistDialog::OnBnClickedSelectnone()
{
	int i;
	for ( i = 0; i < m_MovieListCtrl.GetItemCount() ; i++ )
		m_MovieListCtrl.SetCheck( i , 0 );
}

void CPlaylistDialog::OnEnChangeListname()
{
	CString str;
	m_ListNameCtrl.GetWindowText(str);
	m_SaveButton.EnableWindow(!str.IsEmpty());
}

void CPlaylistDialog::OnBnClickedOk()
{
	int i;
	unsigned char count = 0;

	CString filename;
	m_ListNameCtrl.GetWindowText(filename);
	if ( filename == "Default" )
	{
		::AfxMessageBox("Cannot save as default playlist");
		return;
	}
	for ( i = 0; i < m_MovieListCtrl.GetItemCount() ; i++ )
		if ( m_MovieListCtrl.GetCheck(i) )
			count++;
	if ( count == 0 )
	{
		::AfxMessageBox("No movie selected");
		return;
	}
	
	filename =  m_CurrentDirectory + "\\Playlist\\" + filename + ".lst";

	if ( PathFileExists(filename) == true)
	{
		if ( IDNO == MessageBox("Playlist with the same name exists\nDo you want to overwrite it?", "Playlist Name Conflict", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
			return;
	}

	unsigned int temp[4];
	unsigned char ip[4] = {0, 0, 0, 0};
	sscanf(getHostIP().c_str(), "%u.%u.%u.%u", &temp[0], &temp[1], &temp[2], &temp[3]);
	for ( i = 0; i < 4; i++ )
		ip[i] = static_cast<unsigned char>(temp[i]);
	unsigned char tempChar[2] = { count, 0 };
	CFile playlistFile;
	playlistFile.Open(filename, CFile::modeCreate | CFile::modeWrite);
	playlistFile.Write("SRPL", 4);
	playlistFile.Write(tempChar, 1);
	playlistFile.Write(ip, 4);
	for ( i = 0; i < m_MovieCount; i++ )
	{
		if ( m_MovieListCtrl.GetCheck(i) == false )
			continue;
		tempChar[0] = ( m_MovieName[i].GetLength() >> 8 ) & 0xff;
		tempChar[1] = m_MovieName[i].GetLength() & 0xff;
		playlistFile.Write(tempChar, 2);
		playlistFile.Write(m_MovieName[i], m_MovieName[i].GetLength());

		tempChar[0] = ( m_MovieDesc[i].GetLength() >> 8 ) & 0xff;
		tempChar[1] = m_MovieDesc[i].GetLength() & 0xff;
		playlistFile.Write(tempChar, 2);
		playlistFile.Write(m_MovieDesc[i], m_MovieDesc[i].GetLength());
		
		unsigned int port = atoi((LPCSTR)m_MoviePort[i]);
		tempChar[0] = ( port >> 8 ) & 0xff;
		tempChar[1] = port & 0xff;
		playlistFile.Write(tempChar, 2);
		
		playlistFile.Write(m_MovieHash[i], 8);
	}
	playlistFile.Close();
	OnOK();
}


void CPlaylistDialog::AddItem(CString name, CString port, CString desc, CString hash, bool check)
{
	if ( m_MovieCount >= MAX_MOVIE_NUM )
		return;
	m_MovieName[m_MovieCount] = name;
	m_MoviePort[m_MovieCount] = port;
	m_MovieHash[m_MovieCount] = hash;
	m_MovieDesc[m_MovieCount] = desc;
	m_bIsChecked[m_MovieCount] = check;
	m_MovieCount++;

}
