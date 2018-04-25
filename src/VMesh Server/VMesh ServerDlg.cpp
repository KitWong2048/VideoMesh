// VMesh ServerDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "VMesh Server.h"
#include "VMesh ServerDlg.h"
#include "AddMovieDialog.h"
//#include "..\VMesh\Interface\VMeshServer.h"//Comment by Juan, Feb 24,2010: we dont need it
//#include "..\VMesh\Interface\VMeshServer.h"
#include "..\VMeshAPI\VMeshAPI.h"
//#include "Charlotte.h"//Comment by Juan, Feb 24,2010: we dont need it
#include "afxwin.h"
#include "PortChooserDialog.h"
#include ".\vmesh serverdlg.h"

#pragma comment(lib,"Version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROGRAM_TITLE "VMesh Server"
#define MIN_WIDTH 500
#define MIN_HEIGHT 300

using namespace VMesh;
// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_Title;
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_Title);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TCHAR szFullPath[MAX_PATH];
	DWORD dwVerInfoSize = 0;
	DWORD dwVerHnd;
	VS_FIXEDFILEINFO * pFileInfo;

	GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	WORD m_nProdVersion[2] = { 1, 0 };
	if (dwVerInfoSize)
	{
		// If we were able to get the information, process it:
		HANDLE  hMem;
		LPVOID  lpvMem;
		unsigned int uInfoSize = 0;
	
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);

		::VerQueryValue(lpvMem, (LPTSTR)_T("\\"), (void**)&pFileInfo, &uInfoSize);
			
		// Product version from the FILEVERSION of the version info resource 
		m_nProdVersion[0] = HIWORD(pFileInfo->dwProductVersionMS); 
		m_nProdVersion[1] = LOWORD(pFileInfo->dwProductVersionMS);
	}

	CString title;
	title.Format("%s v%d.%d", PROGRAM_TITLE, m_nProdVersion[0], m_nProdVersion[1]);
	m_Title.SetWindowText((LPCTSTR)_T(title));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVMeshServerDlg 對話方塊

int CVMeshServerDlg::m_MovieCount = 0;
int CVMeshServerDlg::m_TopNumCount = 0;
int CVMeshServerDlg::m_SelectedMovie = -1;
int CVMeshServerDlg::m_Port = PORT_BASE;
int CVMeshServerDlg::m_Index = -1;
int CVMeshServerDlg::m_PreparingMovieCount = 0;
int CVMeshServerDlg::m_Count = 0;
int CVMeshServerDlg::m_HttpPort = SERVER_PORT;

TCHAR CVMeshServerDlg::m_CurrentDirectory[MAX_PATH+1];

bool CVMeshServerDlg::m_Exiting = false;

CString CVMeshServerDlg::m_MovieName[MAX_MOVIE_NUM];
CString CVMeshServerDlg::m_MoviePath[MAX_MOVIE_NUM];
CString CVMeshServerDlg::m_MovieHash[MAX_MOVIE_NUM];
CString CVMeshServerDlg::m_MovieDesc[MAX_MOVIE_NUM];

int CVMeshServerDlg::m_MoviePort[MAX_MOVIE_NUM];
int CVMeshServerDlg::m_State[MAX_MOVIE_NUM];

HANDLE CVMeshServerDlg::m_Stop[MAX_MOVIE_NUM];
CWinThread* CVMeshServerDlg::m_VMeshServerThread[MAX_MOVIE_NUM];
HANDLE CVMeshServerDlg::m_CentralLookupServerProcess;

CListCtrl CVMeshServerDlg::m_MovieList;
CStatusBarCtrl CVMeshServerDlg::m_Status;

queue<int> CVMeshServerDlg::m_UsableIndexQueue;

CVMeshServerDlg::CVMeshServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVMeshServerDlg::IDD, pParent)
	, m_ColumnWidth(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	int i;
	for ( i = 0; i < MAX_MOVIE_NUM; i++ )
	{
		m_VMeshServerThread[i] = NULL;
		m_UsableIndexQueue.push(i);
		m_State[i] = STATE_NOTUSED;
	}
}

void CVMeshServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOVIELIST, m_MovieList);
}

BEGIN_MESSAGE_MAP(CVMeshServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_OPENFILE_ADDMOVIE, OnOpenfileAddmovie)
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_NOTIFY(NM_RCLICK, IDC_MOVIELIST, OnNMRclickMovielist)
	ON_COMMAND(ID_POPUP_ADDMOVIE, OnPopupAddmovie)
	ON_NOTIFY(NM_CLICK, IDC_MOVIELIST, OnNMClickMovielist)
	ON_COMMAND(ID_OPENFILE_REMOVEMOVIE, OnOpenfileRemovemovie)
	ON_COMMAND(ID_POPUP_REMOVEMOVIE, OnPopupRemovemovie)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_MOVIELIST, OnLvnEndlabeleditMovielist)
	ON_COMMAND(ID_POPUP_COPYTOCLIPBOARD, OnPopupCopytoclipboard)
	ON_COMMAND(ID_POPUP_SAVEASCONFFILE, OnPopupSaveasconffile)
	ON_COMMAND(ID_FILE_SAVEASSERVERPLAYLIST, OnFileSaveasmovielist)
	ON_COMMAND(ID_POPUP_SAVEASSERVERPLAYLIST, OnPopupSaveasmovielist)
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// CVMeshServerDlg 訊息處理常式

BOOL CVMeshServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	Logging::setLoggingInterval(60);
	Logging::getInstance("VMesh_Server")->write_msg(8, "VMesh Server Started");

	// ----------------------- UI Initialization -----------------------
	// 將 "關於..." 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示


	m_MovieList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	CRect rect;
	m_MovieList.GetClientRect(&rect);
	
	double nColInterval = (double) rect.Width() / 100.0;
	m_MovieList.InsertColumn(0, _T("Name"), LVCFMT_LEFT	, nColInterval * 30);
	m_MovieList.InsertColumn(1, _T("Port"),	LVCFMT_CENTER	, nColInterval * 10);
	m_MovieList.InsertColumn(2, _T("Movie Hash"), LVCFMT_CENTER	, nColInterval * 15);
	m_MovieList.InsertColumn(3, _T("Status"), LVCFMT_CENTER	, nColInterval * 15);

	m_ColumnWidth = nColInterval * 30 + nColInterval * 10 + nColInterval * 15 + nColInterval * 15;
	m_MovieList.InsertColumn(4, _T("Path"), LVCFMT_LEFT	, rect.Width() - m_ColumnWidth);

	int parts[3];
	RECT r;
	GetClientRect(&rect);
	r.left = r.top  = 50;
	r.right = r.bottom = rect.right;
	parts[0] = r.right / 5 + 20;
	parts[1] = r.right * 8 / 9 - 10;
	parts[2] = r.right;
	m_Status.Create(WS_VISIBLE | WS_CHILD, r, this, AFX_IDW_STATUS_BAR );
	// set parts and text of status bar
	m_Status.SetParts(3, parts);
	m_Status.SetText("No movie publishing", 0, 0);
	m_Status.SetText("", 1, 0);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,AFX_IDW_CONTROLBAR_FIRST);

	// ----------------------- UI Initialization End -----------------------

	// ----------------------- Winsock Initialization -----------------------

	WSADATA wd;

	if ( WSAStartup(MAKEWORD(1,1), &wd) != 0 )
	{
		::AfxMessageBox("Unable to initialize WinSock 1.1");
		return false;
	}

	// ----------------------- Winsock Initialization End  -----------------------

	::GetCurrentDirectory(MAX_PATH, m_CurrentDirectory);
	TCHAR playlistPath[MAX_PATH + 10];
	strcpy(playlistPath, m_CurrentDirectory);
	strcat(playlistPath, "\\Playlist");
	CreateDirectory(playlistPath, NULL);



	m_HttpServerThread = AfxBeginThread(StartHttpServer, this);

	m_LookupServerControlThread = AfxBeginThread(LookupServerManageThreadFunc, this);

	

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CVMeshServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，以便繪製圖示。
// 對於使用文件/檢視模式的 MFC 應用程式，框架會自動完成此作業。

void CVMeshServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//當使用者拖曳最小化視窗時，系統呼叫這個功能取得游標顯示。
HCURSOR CVMeshServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVMeshServerDlg::OnAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CVMeshServerDlg::OnExit()
{
	this->OnCancel();
}

void CVMeshServerDlg::OnNMClickMovielist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
    LVHITTESTINFO ht;
    ht.pt = pNM->ptAction;
	m_MovieList.SubItemHitTest(&ht);

	if (ht.iItem >= 0 && ht.iItem < m_MovieList.GetItemCount())
		m_MovieList.SetItemState(ht.iItem, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	
	if (ht.iItem == m_SelectedMovie && ht.iSubItem == 0)
		m_MovieList.EditLabel(ht.iItem);

	m_SelectedMovie = ht.iItem;
	*pResult = 0;
}

void CVMeshServerDlg::OnNMRclickMovielist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
    LVHITTESTINFO ht;
    ht.pt = pNM->ptAction;
	m_MovieList.SubItemHitTest(&ht);
	if (ht.iItem >= 0 && ht.iItem < m_MovieList.GetItemCount())
		m_MovieList.SetItemState(ht.iItem, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	m_SelectedMovie = ht.iItem;

	CMenu* pSubMenu;
	CMenu  pMenu;
	pMenu.LoadMenu(IDR_RCLICKMENU);
	pSubMenu = pMenu.GetSubMenu(0);
	if ( m_MovieCount == 0 )
		pSubMenu->EnableMenuItem(ID_POPUP_SAVEASSERVERPLAYLIST, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	if(m_SelectedMovie == -1 || m_State[m_MovieList.GetItemData(m_SelectedMovie)] == STATE_REMOVING )
	{
		pSubMenu->EnableMenuItem(ID_POPUP_REMOVEMOVIE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		pSubMenu->EnableMenuItem(ID_POPUP_PROPERTIES, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		pSubMenu->EnableMenuItem(ID_POPUP_COPYTOCLIPBOARD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		pSubMenu->EnableMenuItem(ID_POPUP_SAVEASCONFFILE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}
	else if ( m_State[m_MovieList.GetItemData(m_SelectedMovie)] == STATE_INIT )
	{
		pSubMenu->EnableMenuItem(ID_POPUP_COPYTOCLIPBOARD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		pSubMenu->EnableMenuItem(ID_POPUP_SAVEASCONFFILE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}
	if ( m_MovieList.GetSelectedCount() > 1 )
	{
		pSubMenu->EnableMenuItem(ID_POPUP_COPYTOCLIPBOARD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		pSubMenu->EnableMenuItem(ID_POPUP_SAVEASCONFFILE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}

	/*if ( m_SelectedMovie != -1 )
	{
		CString str;
		str.Format("Index: %d   State: %d", m_MovieList.GetItemData(m_SelectedMovie), m_State[m_MovieList.GetItemData(m_SelectedMovie)]);
		pSubMenu->InsertMenu(0, MF_BYPOSITION | MF_SEPARATOR);
		pSubMenu->InsertMenu(0, MF_BYPOSITION | MF_STRING, 0, str);
		pSubMenu->SetDefaultItem(0);
	}*/
	CPoint oPoint;
    GetCursorPos( &oPoint );
	pSubMenu->TrackPopupMenu (TPM_LEFTALIGN, oPoint.x, oPoint.y, this); 
	*pResult = 0;
}

void CVMeshServerDlg::OnPopupAddmovie()
{
	AddNewMovie();
}


void CVMeshServerDlg::OnOpenfileAddmovie()
{
	AddNewMovie();
}

void CVMeshServerDlg::AddNewMovie(void)
{
	if ( m_MovieCount == MAX_MOVIE_NUM )
	{
		::AfxMessageBox("The number of movie exceeds the limit");
		return;
	}

	CString name;
	CString pathname;
	CString strPort;
	CString desc;

	CAddMovieDialog addMovieDialog(this);
	if (addMovieDialog.DoModal() == IDCANCEL)
		return;
	name = addMovieDialog.GetMovieName();
	pathname = addMovieDialog.GetMoviePath();
	strPort = addMovieDialog.GetPortNumber();
	desc = addMovieDialog.GetDesc();

	m_Port = atoi((LPCSTR)strPort);

	int index = 0;
	int i;

	for ( i = 0; i < m_MovieList.GetItemCount() ; i++ )
	{
		if ( pathname == m_MovieList.GetItemText(i, 4) )
		{
			int nResult = AfxMessageBox("This movie is being published, continue?",  MB_YESNO | MB_ICONQUESTION);
			if ( nResult != IDYES )
				return;
			break;
		}
	}

	index = m_UsableIndexQueue.front();
	m_UsableIndexQueue.pop();

	int count = m_MovieList.GetItemCount();
	int nIndex = m_MovieList.InsertItem( LVIF_TEXT | LVIF_STATE, count,  name, 0 , 0 , 0 , 0 );
	m_MovieList.SetItemData(nIndex, index);
	strPort.Format("%d", m_Port);
	m_MovieList.SetItemText(nIndex, 1 , strPort);
	m_MovieList.SetItemText(nIndex, 3 , "Initializing");
	m_MovieList.SetItemText(nIndex, 4 , pathname);
	m_MoviePort[index] = m_Port;
	m_MoviePath[index] = pathname;
	m_MovieName[index] = name;
	m_MovieDesc[index] = desc;
	m_Stop[index] = CreateEvent(0, 1, 0, 0);

	m_Index = index;
	m_SelectedMovie = nIndex;
	m_MovieList.SetItemState(nIndex, LVIS_SELECTED , LVIS_SELECTED | LVIS_FOCUSED); 
	m_MovieList.SetFocus();
	m_VMeshServerThread[index] = AfxBeginThread(StartVMeshServer, this);
}


UINT CVMeshServerDlg::StartVMeshServer(PVOID lpParam)
{
	int i;
	CString title;
	int itemPos = m_SelectedMovie;

	// get selected movie index from dialog
	int index = m_MovieList.GetItemData(itemPos);

	CString port = m_MovieList.GetItemText(itemPos, 1 );
	if (index >= MAX_MOVIE_NUM || itemPos >= MAX_MOVIE_NUM )
	{
		::AfxMessageBox("Error Occur");
		return 0;
	}

	m_PreparingMovieCount++;
	m_MovieCount++;

	m_State[index] = STATE_INIT;
	if (m_PreparingMovieCount == 1)
		title.Format("%d movie initializing", m_PreparingMovieCount);
	else
		title.Format("%d movies initializing", m_PreparingMovieCount);
	m_Status.SetText( title, 1, 0 );

	int pos;

	string filename;
	pos = m_MoviePath[index].Find(".asf");
	filename = (LPCSTR)m_MoviePath[index].Left(pos);
	   /***************Comment by Juan Feb 23, 2010
	VMeshServer* vmServer = new VMeshServer(getHostIP(), m_MoviePort[index]);
	vmServer->start();
	bool publishedSuccess = vmServer->moviePublish("", filename);
    *******************************************/
	//End by Juan


	//Add by Juan,Feb. 23, 2010:use VMeshAPI to start VMesh Server
	//VMesh Server publishs the movie
	VMeshServerAPI* vmServer = new VMeshServerAPI(getHostIP(), m_MoviePort[index]);
	vmServer->StartVServer();
	bool publishedSuccess = vmServer->MoviePublish("", filename);
	//End by Juan

	// wait in moviePublish
	itemPos = FindItemPosition(port);
	
	if (publishedSuccess && itemPos != -1 && m_State[index] != STATE_REMOVING )
	{
		m_State[index] = STATE_NORMAL;
		 //Comment by Juan, Feb 23, 2010, to support dll
		//m_MovieHash[index] = CString(vmServer->getMediaInfo("")->getMediaHash());
		//End by Juan
		 m_MovieHash[index] = CString(vmServer->GetMovieHash(""));
		//if (m_MovieHash[index].GetLength() > 8)
		//	m_MovieHash[index] = m_MovieHash[index].Left(8);
		printf("VMesh Server: Published Successfully\n");

		m_MovieList.SetItemText(itemPos, 2 , m_MovieHash[index]);
		m_MovieList.SetItemText(itemPos, 3 , "Published");
		
		
		m_PreparingMovieCount--;
		if (m_PreparingMovieCount == 1)
			title.Format("%d movie initializing", m_PreparingMovieCount);
		else if (m_PreparingMovieCount > 1)
			title.Format("%d movies initializing", m_PreparingMovieCount);
		else
			title.Format("");
		m_Status.SetText( title, 1, 0 );

		if (m_MovieCount == 1)
			title.Format("%d movie publishing", m_MovieCount - m_PreparingMovieCount);
		else
			title.Format("%d movies publishing", m_MovieCount - m_PreparingMovieCount);
		m_Status.SetText( title, 0, 0 );

		UpdateDefaultPlaylist();

		::WaitForSingleObject( m_Stop[index],  INFINITE ) ;

	}
	else if ( !publishedSuccess )
		::AfxMessageBox("VMesh Server: Published Failed");

	Sleep(2000);
	//vmServer->stop();
	delete vmServer;

	if ( m_Exiting == false )
	{
		itemPos = FindItemPosition(port);
		if ( itemPos != -1 )
			m_MovieList.DeleteItem(itemPos);
	}
	m_UsableIndexQueue.push(index);
	m_State[index] = STATE_NOTUSED;
	m_VMeshServerThread[index] = NULL;
	return 0;
}

void CVMeshServerDlg::OnOpenfileRemovemovie()
{
	RemoveSelectedMovies();
}

void CVMeshServerDlg::OnPopupRemovemovie()
{
	RemoveSelectedMovies();
}


void CVMeshServerDlg::RemoveSelectedMovies(void)
{
	if (m_Exiting == false && m_MovieCount == 0 )
	{
		::AfxMessageBox("No movie in the list");
		return;
	}
	POSITION pos = m_MovieList.GetFirstSelectedItemPosition();
	if (m_Exiting == false && pos == NULL)
	{
		::AfxMessageBox("No movie is selected");
		return;
	}

	int selectedCount = m_MovieList.GetSelectedCount();
	CString str;
	
	if ( selectedCount > 1 )
		str.Format("Do you want to remove these %d movies?", selectedCount);
	else
		str.Format("Do you want to remove this movie?");

	if ( m_Exiting == false )
		if ( IDNO  == MessageBox(str, "Movie Removal", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) )
			return;
	int *posInList = NULL;
	if ( selectedCount > 0 )
		posInList = new int[selectedCount];
	int i = 0;
	while (pos)
	{
		int nItem = m_MovieList.GetNextSelectedItem(pos);
		posInList[i] = nItem;
		i++;
	}
	for ( i = 0; i < selectedCount; i++ )
	{
		RemoveMovie(posInList[i]);
		m_MovieList.SetItemState(posInList[i], 0 , LVIS_SELECTED); 
	}
	
	if ( posInList )
		delete[] posInList;

	UpdateDefaultPlaylist();
}

bool CVMeshServerDlg::RemoveMovie(int selectedMovie)
{
	if ( selectedMovie < 0 || selectedMovie >= MAX_MOVIE_NUM )
		return false;
	int index = m_MovieList.GetItemData(selectedMovie);;
	if ( index < 0 || index >= MAX_MOVIE_NUM )
		return false;

	CString title;
	if ( m_State[index] == STATE_INIT )
	{
		m_PreparingMovieCount--;
		if (m_PreparingMovieCount == 1)
			title.Format("%d movie initializing", m_PreparingMovieCount);
		else if (m_PreparingMovieCount > 1)
			title.Format("%d movies initializing", m_PreparingMovieCount);
		else
			title.Format("");
		m_Status.SetText( title, 1, 0 );
	}

	SetEvent(m_Stop[index]);
	m_MovieList.SetItemText(selectedMovie, 3, "Removing");
	m_MovieCount--;
	if ( (m_MovieCount - m_PreparingMovieCount) == 1)
		title.Format("%d movie publishing", m_MovieCount - m_PreparingMovieCount);
	else if (m_MovieCount > 1)
		title.Format("%d movies publishing", m_MovieCount - m_PreparingMovieCount);
	else
		title.Format("no movie publishing");
	m_Status.SetText( title, 0, 0 );

	 m_State[index] = STATE_REMOVING;
	return true;
}

void CVMeshServerDlg::OnCancel()
{
	
	CString strStatus, strStatus2;
	 m_Count = m_MovieList.GetItemCount();
	 if ( m_Count > 0 )
		 if ( IDNO == MessageBox("One or more movies are publishing\n\nQuit anyway?", "Exit", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) )
			 return;
	m_Exiting = true;
	m_Status.SetText("Exiting", 0, 0 );
	int i;
	for ( i = 0; i < m_Count; i++ )
	{	
		if ( m_State[m_MovieList.GetItemData(i)] == STATE_REMOVING )
			continue;
		strStatus.Format("Removing %s", m_MovieName[m_MovieList.GetItemData(i)]);
		m_Status.SetText(strStatus , 1, 0 );
		
		int index = m_MovieList.GetItemData(i);
		//RemoveMovie(i);
		m_MovieList.SetItemText(i, 3, "Removing");
		m_MovieList.SetItemState(i, LVIS_SELECTED , LVIS_SELECTED | LVIS_FOCUSED); 
		if (i == 0)
			strStatus2.Format("Removed %s", m_MovieName[index]);
		else
			strStatus2 = strStatus2 + " / " + m_MovieName[index];
		m_Status.SetText(strStatus2 , 1, 0 );
		m_Status.SetText("Exiting", 0, 0 );
	}
	RemoveSelectedMovies();
	UpdateWindow();
	HANDLE threadHandle;
	for ( i = 0; i < MAX_MOVIE_NUM; i++)
		if ( m_VMeshServerThread[i] != NULL)
		{
			threadHandle = m_VMeshServerThread[i]->m_hThread;
			if ( WaitForSingleObject(threadHandle, 20000) == WAIT_TIMEOUT )
				TerminateThread(threadHandle, 0);
		}
	
	TerminateThread(m_HttpServerThread->m_hThread, 0);
	WSACleanup();

	TerminateProcess(m_CentralLookupServerProcess, 0);

	CDialog::OnCancel();
}

bool CVMeshServerDlg::CheckIsNameUnique(CString name)
{
	for (int i = 0; i < m_MovieList.GetItemCount(); i++ )
		if (m_MovieName[m_MovieList.GetItemData(i)] == name )	
			return false;
	return true;
}

bool CVMeshServerDlg::CheckIsPortUnique(int port)
{
	for (int i = 0; i < m_MovieList.GetItemCount(); i++ )
		if (m_MoviePort[m_MovieList.GetItemData(i)] == port )	
			return false;
	return true;
}

void CVMeshServerDlg::OnLvnEndlabeleditMovielist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if( pDispInfo->item.iItem != -1 && pDispInfo->item.pszText && strcmp(pDispInfo->item.pszText, "") != 0 )
	{
		int i;
		
		if ( CheckIsNameUnique(pDispInfo->item.pszText) == false )
		{
			::AfxMessageBox("Movie name already exists");
			return;
		}
		m_MovieList.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
		m_MovieName[pDispInfo->item.iItem] = pDispInfo->item.pszText;
		UpdateDefaultPlaylist();
	}
	*pResult = 0;
}

void CVMeshServerDlg::OnPopupCopytoclipboard()
{
	if ( m_SelectedMovie == -1 )
		return;
	if ( m_MovieList.GetItemText(m_SelectedMovie, 2) == "" )
		return;
	
	CString data = GetConfigFileContent();

	if (OpenClipboard())
	{
		
		EmptyClipboard();

		
		HGLOBAL hClipboardData;
		hClipboardData = GlobalAlloc(GMEM_DDESHARE, data.GetLength()+1);
	
		char * pchData;
		pchData = (char*)GlobalLock(hClipboardData);
					
		strcpy(pchData, LPCSTR(data));
					
		GlobalUnlock(hClipboardData);
					
		SetClipboardData(CF_TEXT,hClipboardData);

		CloseClipboard();
	}
}

void CVMeshServerDlg::OnPopupSaveasconffile()
{
	if ( m_SelectedMovie == -1 )
		return;
	if ( m_MovieList.GetItemText(m_SelectedMovie, 2) == "" )
		return;

	CString data = GetConfigFileContent();

	CString defaultName;
	defaultName.Format("%s_%s", m_MovieName[m_SelectedMovie], m_MovieList.GetItemText(m_SelectedMovie, 1));
	CFileDialog saveDialog(false, ".conf", defaultName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST , "VMesh Configuration File (*.conf) |*.conf||");
	if ( saveDialog.DoModal() == IDCANCEL )
		return;

	CFile newConfFile((LPCSTR)saveDialog.GetPathName(), CFile::modeWrite | CFile::modeCreate);
	newConfFile.Write((LPCSTR)data, data.GetLength());
	newConfFile.Close();

}


CString CVMeshServerDlg::GetConfigFileContent(void)
{
	CString strData;
	strData.Format("%s %s %s", m_MovieList.GetItemText(m_SelectedMovie, 2), m_MovieList.GetItemText(m_SelectedMovie, 1), getHostIP().c_str());
	return strData;
	
}

void CVMeshServerDlg::OnFileSaveasmovielist()
{

	SaveAsMovieList();
}

void CVMeshServerDlg::SaveAsMovieList()
{
	if ( m_MovieList.GetItemCount() == 0 )
	{
		::AfxMessageBox("No movie in list");
		return;
	}

	POSITION pos = m_MovieList.GetFirstSelectedItemPosition();
	CPlaylistDialog dlg;
	int i;
	int nextSelectedItem = -1;
	if ( pos != NULL )
		nextSelectedItem = m_MovieList.GetNextSelectedItem(pos);
	for ( i = 0; i < m_MovieList.GetItemCount() ; i++ )
		if ( m_State[m_MovieList.GetItemData(i)] == STATE_NORMAL )
			if ( i == nextSelectedItem )
			{
				CString temp;
				temp.Format("%d", m_MoviePort[i]);
				dlg.AddItem( m_MovieName[i], temp, m_MovieDesc[i], m_MovieHash[i], true);
				//dlg.AddItem( m_MovieList.GetItemText(i, 0), m_MovieList.GetItemText(i, 1), m_MovieList.GetItemText(i, 2), true);
				if (pos != NULL )
					nextSelectedItem = m_MovieList.GetNextSelectedItem(pos);
			}
			else
			{
				CString temp;
				temp.Format("%d", m_MoviePort[i]);
				dlg.AddItem( m_MovieName[i], temp, m_MovieDesc[i], m_MovieHash[i]);
				//dlg.AddItem( m_MovieList.GetItemText(i, 0), m_MovieList.GetItemText(i, 1), m_MovieList.GetItemText(i, 2));
			}
	if ( dlg.DoModal() == IDCANCEL )
		return;
}

void CVMeshServerDlg::OnPopupSaveasmovielist()
{
	SaveAsMovieList();
}

UINT CVMeshServerDlg::StartHttpServer(PVOID lpParam)
{
	TCHAR root[MAX_PATH + 10];
	CPortChooserDialog dlg;
	dlg.DoModal();
	while ( 1 )
	{
		m_HttpPort = dlg.GetPort();
		if ( m_HttpPort == -1 )
			break;
		CString portStr;
		portStr.Format("Port %d", m_HttpPort);
		m_Status.SetText(portStr, 2, 0);

		strcpy(root, m_CurrentDirectory);
//		strcat(root, "\\Ad");
		//Comment by Juan, Feb. 20, 2010, to use Vesh DLL
		//Start(root, m_HttpPort);
		SWebServer m_webServer(root, m_HttpPort);  //start web server
		//End by Juan
		
		m_Status.SetText("Restarting", 2, 0);
		Sleep(1000);
	}
	m_Status.SetText("Not Started", 2, 0);
	return(0);
}

int CVMeshServerDlg::FindItemPosition(CString port)
{
	int i;
	for( i = 0 ; i < m_MovieList.GetItemCount(); i++ )
		if ( port == m_MovieList.GetItemText(i, 1) )
			break;

	if ( i != m_MovieList.GetItemCount() )
		return i;
	return -1;

}

void CVMeshServerDlg::UpdateDefaultPlaylist(void)
{
	int count = 0;
	int i;
	CFile playlistFile;
	CString filename = m_CurrentDirectory;

	filename += "\\Playlist\\Default.lst";
	for ( i = 0; i < m_MovieList.GetItemCount() ; i++ )
		if ( m_State[m_MovieList.GetItemData(i)] == STATE_NORMAL )
			count++;
	remove(filename);
	//CString str;
	//str.Format("%d", count);
	//::AfxMessageBox(str);
	if ( count == 0 )
		return;

	unsigned int temp[4];
	unsigned char ip[4] = {0, 0, 0, 0};
	sscanf(getHostIP().c_str(), "%u.%u.%u.%u", &temp[0], &temp[1], &temp[2], &temp[3]);
	for ( i = 0; i < 4; i++ )
		ip[i] = static_cast<unsigned char>(temp[i]);
	unsigned char tempChar[2] = { count, 0 };

	
	playlistFile.Open(filename, CFile::modeCreate | CFile::modeWrite);
	playlistFile.Write("SRPL", 4);
	playlistFile.Write(tempChar, 1);
	playlistFile.Write(ip, 4);
	int index;
	for ( i = 0; i < m_MovieList.GetItemCount(); i++ )
	{
		index = m_MovieList.GetItemData(i);
		if ( m_State[index] != STATE_NORMAL )
			continue;
		tempChar[0] = ( m_MovieName[index].GetLength() >> 8 ) & 0xff;
		tempChar[1] = m_MovieName[index].GetLength() & 0xff;
		playlistFile.Write(tempChar, 2);
		playlistFile.Write(m_MovieName[index], m_MovieName[index].GetLength());

		tempChar[0] = ( m_MovieDesc[index].GetLength() >> 8 ) & 0xff;
		tempChar[1] = m_MovieDesc[index].GetLength() & 0xff;
		playlistFile.Write(tempChar, 2);
		playlistFile.Write(m_MovieDesc[index], m_MovieDesc[index].GetLength());

		tempChar[0] = ( m_MoviePort[index] >> 8 ) & 0xff;
		tempChar[1] = m_MoviePort[index] & 0xff;
		playlistFile.Write(tempChar, 2);
		
		playlistFile.Write(m_MovieHash[index], 8);
	}
	playlistFile.Close();
}

void CVMeshServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if ( m_MovieList.m_hWnd == NULL )
		return;

	int parts[3];
	RECT rect;
	int i;
	m_ColumnWidth = 0;
	for ( i = 0; i < 4; i ++ )
		m_ColumnWidth += m_MovieList.GetColumnWidth(i);
	m_MovieList.SetWindowPos(0, 0, 0, cx - 25, cy - 36, SWP_NOMOVE | SWP_NOZORDER);
	m_MovieList.GetClientRect(&rect);
	m_MovieList.SetColumnWidth(4, rect.right - m_ColumnWidth);

	GetClientRect(&rect);
	parts[0] = rect.right / 5 + 20;
	parts[1] = rect.right * 8 / 9 - 10;
	parts[2] = rect.right;
	m_Status.SetParts(3, parts);
	m_Status.SetWindowPos(0, cy - 32, 0, cx, cy, SWP_NOZORDER);

	
}

void CVMeshServerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
		// Set the minimum size if width going to be less than minimum width
	if ((pRect->right  - pRect->left) <= MIN_WIDTH)
		pRect->right = pRect->left+MIN_WIDTH; 

	// Set the minimum size if height going to be less than minimum height
	if ((pRect->bottom - pRect->top)<= MIN_HEIGHT)
		pRect->bottom = pRect->top+MIN_HEIGHT;

	CDialog::OnSizing(fwSide, pRect);
}

// execute the centralLookupServer.exe in CurrentDirectory
UINT CVMeshServerDlg::LookupServerManageThreadFunc(PVOID lpParam)
{
	m_CentralLookupServerProcess = NULL;

	STARTUPINFO startupinfo = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION process_information;

	CString commandName = m_CurrentDirectory;	
#ifdef _DEBUG
	commandName	+= "\\centralLookupServerD.exe";
#else
	commandName	+= "\\centralLookupServer.exe";
#endif
	
	// check execute the centralLookupServer.exe success or not
	if( CreateProcess(commandName, NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupinfo, &process_information) )
		m_CentralLookupServerProcess = process_information.hProcess;

	while (1)
	{
		if ( m_CentralLookupServerProcess)
			WaitForSingleObject(m_CentralLookupServerProcess, INFINITE);

		if ( m_Exiting )
			break;

		Sleep(100);

		if( CreateProcess(commandName, NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupinfo, &process_information) )
			m_CentralLookupServerProcess = process_information.hProcess;

		Sleep(500);
	}
	return(0);
}