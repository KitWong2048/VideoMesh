// MainDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MainDialog.h"
#include "afxwin.h"
#include "..\Common\natupnp.h"
#include <vector>
#include ".\maindialog.h"
#define VMESH_PLAYER

#pragma comment(lib,"Version.lib")

const int MAX_RANGE = 1000000000;
const double ZERO = 0.01;


// CMainDialog dialog
#define PROGRAM_TITLE "VMesh Player"

const int PLAY_STATE_DELAY = 3000;  // ms
const int IS_PLAYING_STATE_TIMER = 1111;
bool CheckIsPortAvailableP(int port);
VMeshControllerThread* _vmMeshControllerThread;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	CAboutDlg(CString& version);
// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	CString m_Version;
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_WMPver;
	virtual BOOL OnInitDialog();
	CStatic m_Title;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	m_Version = "";
}

CAboutDlg::CAboutDlg(CString& version) : CDialog(CAboutDlg::IDD)
{
	m_Version = version;
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WMPVER, m_WMPver);
	DDX_Control(pDX, IDC_ABOUTTITLE, m_Title);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_WMPver.SetWindowText("Embedded WMP v" + m_Version);

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


int CMainDialog::appInstanceCount;

IMPLEMENT_DYNAMIC(CMainDialog, CDialog)
CMainDialog::CMainDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDialog::IDD, pParent)
	, currentURL(_T(""))
	, m_IsCreated(false)
	, m_ControllerHandle(NULL)
	, m_CurrentPosition(0)
	, m_IsPlaying(false)
	, m_AlwaysOnTop(false)
	, m_bReconnected(false)
	, m_Seeking(-1)
	, m_DataPort(0)
	, m_StartupTimerHandler(-1)
	, m_SeekingTimerHandler(-1)
	, m_SwitchMovieTimerHandler(-1)
	, m_FreezingTimerHandler(-1)
	, m_FromFile(false)
	, m_FirstConnection(true)
	, m_Port(0)
	, m_FileLocation(_T(""))
	, m_PlaylistCount(0)
	, VMInfo(NULL)
	, m_CurrentPlaying(-1)
	, m_ParentList(NULL)
	, m_NewSeekTime(0)
	, m_bSwitchingMovie(false)
	, i_playerState(NON_STARTED)
	, totalMovieCount(0)
	, numSeg(0)
	, showConsole(true)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMainDialog::~CMainDialog()
{
	
}

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLAYER, m_Player);
	DDX_Control(pDX, IDC_TIME, m_Time);
	DDX_Control(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_SLIDERTIME, m_SliderTime);
	DDX_Control(pDX, IDC_VOLUME, m_Volume);
	DDX_Control(pDX, IDC_PLAY, m_Play);
	DDX_Control(pDX, IDC_STOP, m_Stop);
	DDX_Control(pDX, IDC_PAUSE, m_Pause);
	DDX_Control(pDX, IDC_VOLLABEL, m_VolLabel);
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
	ON_BN_CLICKED(IDC_PAUSE, OnBnClickedPause)
	ON_BN_CLICKED(IDC_PLAY, OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_WM_TIMER()
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TIME, OnNMReleasedcaptureTime)
ON_WM_SYSCOMMAND()
ON_WM_SIZE()
ON_WM_HSCROLL()
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_VOLUME, OnNMReleasedcaptureVolume)
//ON_WM_CLOSE()
ON_COMMAND(ID_FILE_OPENVMESH, OnFileOpenvmesh)
ON_COMMAND(ID_FILE_OPENCONFIGFILE, OnFileOpenconfigfile)
ON_WM_QUERYDRAGICON()
ON_COMMAND(ID_ABOUT, OnAbout)
ON_COMMAND(ID_VIEW_ALWAYSONTOP, OnViewAlwaysontop)
ON_WM_SIZING()
ON_COMMAND(ID_FILE_CONNECTTOSERVER, OnFileConnecttoserver)
ON_COMMAND(ID_FILE_OPENSERVERPLAYLIST, OnFileOpenserverplaylist)
ON_COMMAND(ID_MOVIE_REFRESHPLAYLIST, OnMovieRefreshPlaylist)
ON_COMMAND(ID_MOVIE_CHOOSEFROMPLAYLIST, ChooseNewMovie)
ON_COMMAND(ID_FILE_CONFIG, OnConfiguration)
ON_COMMAND_RANGE(ID_MOVIE_BASE, ID_MOVIE_BASE + MAX_MOVIE_NUM, OnMovieSelected)
END_MESSAGE_MAP()


// CMainDialog message handlers

void CMainDialog::SetFileName(CString strFileName)
{	
	KillTimer(1);
	m_Control.stop();

	m_Network.put_bufferingTime(10000);		// 10000ms buffering time

	m_Player.put_URL(strFileName);

	m_Control.play();
	m_Media = m_Player.get_currentMedia();
	m_SliderTime.SetWindowText("");
	m_Time.SetPos(0);
	m_PositionOffset = 0;
	if ((strFileName.GetLength() > 4) && (strFileName.Left(4) == "http"))
	{
		currentURL.SetString(strFileName);
	
		m_Name.SetWindowText("Initializing... ");
	}
}

BOOL CMainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	Logging::getInstance("VMesh_Player")->write_msg(8, "VMesh Player Started");
	
	srand(time(NULL));
	EnableToolTips(true);

	m_TimeToolTip.Create(&m_Time);
	m_TimeToolTip.AddTool(&m_Time, TTS_ALWAYSTIP);
	m_TimeToolTip.SetDelayTime(10);

	m_IsCreated = true;
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_AOT, "Always on top");
		//pSysMenu->CheckMenuItem(IDM_AOT, MF_BYCOMMAND | MF_CHECKED);
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);		
	}

	CMenu* pMenu = GetMenu();
	//pSubMenu = pSubMenu->GetSubMenu(0);
	//pSubMenu = pSubMenu->GetSubMenu(1);
	//pMenu->RemoveMenu(ID_FILE_OPENMEDIA, MF_BYCOMMAND);
	//pMenu->RemoveMenu(ID_FILE_OPENURL, MF_BYCOMMAND);

	pMenu->EnableMenuItem(ID_MOVIE_REFRESHPLAYLIST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);

	m_DelayEnableThread = NULL;
	m_ResetControllerThread = NULL;

	m_Control = m_Player.get_controls();
	m_Network = m_Player.get_network();
	m_Setting = m_Player.get_settings();
	m_Player.EnableScrollBar(SB_BOTH, ESB_DISABLE_BOTH);

	m_Network.put_bufferingTime(10000);		// 1000ms buffering time

	m_Time.SetRangeMax(MAX_RANGE);
	m_Volume.SetRangeMax(100);
	m_Volume.SetPos(m_Setting.get_volume());
	currentURL = "";

#ifdef SIM_PLAYER
	m_Time.EnableWindow(false);
#endif

	ResizeWindow(555, 411);

	SetStretch();

	m_Time.EnableWindow(false);
	m_Play.EnableWindow(false);
	m_Stop.EnableWindow(false);
	m_Pause.EnableWindow(false);

	m_hReady = NULL;
	m_hNoPacket = NULL;
	m_hReset = NULL;
	m_hFailed = NULL;
	m_hReleaseMutex = NULL;
	m_hStart = CreateEvent(NULL, true, false, NULL);
	GetCurrentDirectory(MAX_PATH, m_CurrentDirectory);
	CString playlistDirectory = CString(m_CurrentDirectory) + "\\Playlist";
	CreateDirectory(playlistDirectory, 0);
	//LoadConfigFile();
	//SetAlwaysOnTop();
	//ConnectToServer();
	
	// auto loader server playlist
	//::AfxMessageBox("Loading server playlist");
	CFileFind finder;
	BOOL finding = finder.FindFile (CString(playlistDirectory) + CString ("\\*.lst"));
	while (finding)
	{
		finding = finder.FindNextFile();
		CString path = finder.GetFilePath();
		//::AfxMessageBox(CString("found 1 file: ") + CString(path));
		if ( path.IsEmpty() || (CServerPlaylistDialog::ReadPlaylistCount(path)<=0)) {
			//delete invalide file
			DeleteFile(path);
		} else {
			CString title = finder.GetFileTitle();
			//::AfxMessageBox(title);

			m_PlaylistName[m_PlaylistCount] = title.Mid(0, title.FindOneOf("@"));
			m_IPAddress[m_PlaylistCount] = title.Mid(title.FindOneOf("@")+1, title.ReverseFind('.')-title.FindOneOf("@")-1);
			m_ServerPort[m_PlaylistCount] = atoi(title.Mid(title.ReverseFind('.')+1));
			//::AfxMessageBox(m_PlaylistName[m_PlaylistCount]);
			//::AfxMessageBox(m_IPAddress[m_PlaylistCount]);
			//::AfxMessageBox(title.Mid(title.ReverseFind('.')+1));
			m_PlaylistCount++;
		}
	}
	finder.Close ();
	RefreshPlaylist(true);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDialog::OnBnClickedPause()
{
	// TODO: Add your control notification handler code here
	int state = m_Player.get_playState();
	if ( state == 2 ) // paused
		m_Control.play();
	else if ( state == 3 )
		m_Control.pause();
}

void CMainDialog::OnBnClickedPlay()
{
	// TODO: Add your control notification handler code here
	if ((m_Network.get_sourceProtocol() == "http") && (m_Player.get_playState() != 3) && (m_Player.get_playState() != 2))
		SeekThroughHTTP(SliderCurrentPositionToTime());
	m_Control.play();
	m_IsPlaying = true;
}

void CMainDialog::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	m_IsPlaying = false;
	m_Control.stop();
	
	m_PositionOffset = 0;
	m_CurrentPosition = 0;
	m_Time.SetPos(0);
	m_SliderTime.SetWindowText("00:00");
}
BEGIN_EVENTSINK_MAP(CMainDialog, CDialog)
	ON_EVENT(CMainDialog, IDC_PLAYER, 5101, PlayStateChangePlayer, VTS_I4)
	ON_EVENT(CMainDialog, IDC_PLAYER, 6505, ClickPlayer, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMainDialog, IDC_PLAYER, 6506, DoubleClickPlayer, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


CString TimeConvertToString(int nSec){
	CString strTime;
	
	int hr = nSec / 3600;
	int min = nSec / 60 % 60;
	int sec = nSec % 60;
	if (hr > 0)
		strTime.Format("%02d:%02d:%02d", hr, min, sec);
	else
		strTime.Format("%02d:%02d", min, sec);
	return strTime;

}

void CMainDialog::OnTimer(UINT nIDEvent)
{
	bool reset = false;
	bool reconnect = false;

	if (nIDEvent == 1){
		double timePos =  GetCurrentPosition();
		timePos = timePos / m_Media.get_duration();
		timePos*= MAX_RANGE;
		
		KillTimer(5);
		if (::GetCapture() != m_Time.GetSafeHwnd()){ // slider moves if mouse is not dragging
			if (m_Media.get_duration() > ZERO)
				m_Time.SetPos((int)timePos);
			else
				m_Time.SetPos(MAX_RANGE);

			// When playing the movie to the end, then restart to play the clip
			int c = (int)GetCurrentPosition();
			int d = (int)m_Media.get_duration();
			if ( c == d )
				reset = true;
		}
		if ( ::WaitForSingleObject(m_hNoPacket, 0) == WAIT_OBJECT_0 )
			reconnect = true;

		//	m_Player.get_playState() return different states of player
		//	1 = stopped
		//	2 = paused
		//	3 = clip playing
		//	4 = scanning stream forward
		//	5 = scanning stream reverse
		//	6 = buffering media
		//	7 = waiting for streaming data
		//	8 = reached the end of the media
		//	9 = preparing new media
		//	10 = ready to begin playback

		if ( (3 == m_Player.get_playState()) &&  (( (int)GetCurrentPosition() ) > 0) )
			m_CurrentPosition = (int)GetCurrentPosition();
		double curPos = m_Time.GetPos();
		curPos = curPos / MAX_RANGE;
		if (m_Media.get_duration() > ZERO)
			curPos *= m_Media.get_duration();
		else
			curPos *= GetCurrentPosition();
		CString curPosStr = TimeConvertToString((int)curPos);
		CString durationStr = m_Media.get_durationString();
		if (durationStr == "00:00")
		{
			KillTimer(4);
			KillTimer(5);
			StartVMeshClient(m_sHash, m_sPort, m_sServer);
		}
		m_SliderTime.SetWindowText(curPosStr + " / " + durationStr);
	}
	else if ( nIDEvent == 2 )
	{
		if ( ::WaitForSingleObject(m_hNoPacket, 0) == WAIT_OBJECT_0 )
			if ( m_Player.get_playState() == 6 ) // still buffering
				reconnect = true;
		KillTimer(2);

	}
	else if ( nIDEvent == 3 )
	{
		if ( WaitForSingleObject(m_hStart, 0) == WAIT_OBJECT_0 )
		{
			SetFileName("http://0.0.0.1/");

			Sleep(1000);
			CString localAddress;
			localAddress.Format("http://127.0.0.1:%d/?set_time=%d", m_DataPort, m_NewSeekTime);
			//m_NewSeekTime = 0;
			SetFileName(localAddress);
			KillTimer(3);
		}
		return;
	}
	else if ( nIDEvent == 4 )
	{
		reconnect = true;
		KillTimer(4);

	}
	else if ( (nIDEvent == 5) || (nIDEvent == 6))
	{
		KillTimer(4);
		KillTimer(5);
		KillTimer(6);
		if ( (m_CurrentPosition == (int)m_Media.get_duration()) || ((m_CurrentPosition+1) == (int)m_Media.get_duration()) )
		{
			StartVMeshClient(m_sHash, m_sPort, m_sServer);
		}
		else
		{
			int currentPosition = m_CurrentPosition;
			if (!m_NewSeekTime)
				m_NewSeekTime = currentPosition;
			StartVMeshClient(m_sHash, m_sPort, m_sServer);
		}
		return;
	}
	else if ( nIDEvent == 7 )
	{
		//update buffering state
		if (m_Player.get_playState() == 6)
			PlayStateChangePlayer(6);
		else 
			KillTimer(7);
		return;
	}
	else if ( nIDEvent == IS_PLAYING_STATE_TIMER )
	{
		// Stop the Startup Timer if it is started
		int offset = 0 - PLAY_STATE_DELAY / 1000;
		if (m_StartupTimerHandler != -1)
		{
			STOP_TIMER(m_StartupTimerHandler, "SD", offset)
			m_StartupTimerHandler = -1;
			i_playerState = PLAYING;
		}

		// Stop the Seeking Timer if it is started
		if (m_SeekingTimerHandler != -1)
		{
			STOP_TIMER(m_SeekingTimerHandler, "JFD", offset)
			m_SeekingTimerHandler= -1;
			i_playerState = PLAYING;
		}

		if ((m_SwitchMovieTimerHandler != -1) && (m_bSwitchingMovie == false))
		{
			STOP_TIMER(m_SwitchMovieTimerHandler, "SMD", offset)
			m_SwitchMovieTimerHandler = -1;
			i_playerState = PLAYING;
		}

		if (i_playerState == FREEZE)
		{
			int duration = 0;
			STOP_TIMER(m_FreezingTimerHandler, "FZ", duration)
			i_playerState = PLAYING;
		}
		m_Time.EnableWindow(true);
		return;
	}
	CDialog::OnTimer(nIDEvent);
	if ( reset && !m_bReconnected)
	{	
		SeekThroughHTTP(0);
		m_Name.SetWindowText("Reached end / Starting from beginning");
		cout<<"reach end/starting from beginning Juan";
		//SetTimer(5, 50000, 0);
		m_bReconnected = true;
	}
	else if ( reconnect && !m_bReconnected )
	{
		if ( (m_CurrentPosition == (int)m_Media.get_duration()) || ((m_CurrentPosition+1) == (int)m_Media.get_duration()) )
		{
			SeekThroughHTTP(0);
		}
		else
			SeekThroughHTTP(m_CurrentPosition);
		ResetEvent(m_hNoPacket);
		m_Name.SetWindowText("Reconnecting");
		//SetTimer(5, 50000, 0);
		m_bReconnected = true;
	}

}

void CMainDialog::PlayStateChangePlayer(long NewState)
{
	
	KillTimer(IS_PLAYING_STATE_TIMER);

	if ( (NewState != 2) && (NewState != 3))
		KillTimer(1);
	if ( (NewState == 1) || (NewState == 2) || (NewState == 3) || (NewState == 6))
	{
		KillTimer(4);
		KillTimer(6);
	}
	if ( (NewState == 1) || (NewState == 2) || (NewState == 3))
	{
		m_bReconnected = false;
		KillTimer(5);
	}
	
	if (NewState == 1)	// stopped 
	{
		if (m_IsPlaying)
		{
			Sleep(1000);
			if ( m_CurrentPosition == (int)m_Media.get_duration() || (m_CurrentPosition+1) == (int)m_Media.get_duration() )
			{
				SeekThroughHTTP(0);
				m_Control.play();
			}
			else
				SeekThroughHTTP(m_CurrentPosition);
			
		}
		m_Name.SetWindowText("Stopped");
	}
	if (NewState == 2)	// paused 
	{
		m_Name.SetWindowText("Paused");
	}


	if (NewState == 3) // playing
	{
	
		m_Name.SetWindowText("Playing Content");
		SetTimer(1, 200, 0);
		SetTimer(5, 300000, 0);
		m_NewSeekTime = 0;
		m_IsPlaying = true;
		m_Play.EnableWindow(true);
		m_Stop.EnableWindow(true);
		m_Pause.EnableWindow(true);
		SetTimer(IS_PLAYING_STATE_TIMER, PLAY_STATE_DELAY, 0);
		return;
	}

	if (NewState == 6) // buffering
	{
		if (i_playerState == PLAYING)
		{
			if (m_FreezingTimerHandler != -1)
			{
				int duration = 0;
				STOP_TIMER(m_FreezingTimerHandler, NULL, duration)
			}
			GET_TIMER(m_FreezingTimerHandler);
			i_playerState = FREEZE;
		}
		m_Seeking = -1;
		m_IsPlaying = true;
		CString str;
		str.Format("[%d%%]", m_Network.get_lostPackets());
		m_Name.SetWindowText((CString)"Buffering... " + str);
		SetTimer(7, 1000, 0); // for refreshing
		
		KillTimer(2);
		//SetTimer(2, 30000, 0); // Avoid endless pause, activated after 30 seconds in buffering state
		
		//if (m_DelayEnableThread)
		//	TerminateThread(m_DelayEnableThread->m_hThread, 0);
		//m_DelayEnableThread = ::AfxBeginThread(EnableSeekControl, this);
	}
	else {
		KillTimer(2);
	}
	if (NewState == 7) // waiting
	{
		m_Name.SetWindowText("Waiting...");
		//SetTimer(4, 60000, 0);
	}
	if (NewState == 8) // end of media
	{
		SeekThroughHTTP(0);
		m_Name.SetWindowText("Reached end / Starting from beginning");
		//SetTimer(4, 60000, 0);
	}
	if (NewState == 9) // preparing
	{
		m_Name.SetWindowText("Preparing");
		if ((m_Media.get_duration() != 0) && (m_CurrentPosition > 0))
			if ( (m_CurrentPosition == (int)m_Media.get_duration()) || ((m_CurrentPosition+1) == (int)m_Media.get_duration()) )
			{
				SeekThroughHTTP(0);
				m_Name.SetWindowText("Reached end / Starting from beginning");
				return;
			}
			else;
		else
			return;
		//SetTimer(4, 60000, 0);
	
	}
	if (NewState == 10) // media ready
	{
		m_Name.SetWindowText("Ready");
		//SetTimer(6, 30000, 0);
		if (m_IsPlaying)
		{
			if ((m_CurrentPosition == (int)m_Media.get_duration()) || ((m_CurrentPosition+1) == (int)m_Media.get_duration()) )
			{
					//m_Control.stop();
					m_Name.SetWindowText("Reached end / Starting from beginning");
					m_CurrentPosition = 0;
					
			}
			SeekThroughHTTP(m_CurrentPosition);
		}
		//SetTimer(4, 60000, 0);
	}
	if (NewState == 11)
		m_Name.SetWindowText("Trying to reconnect");
}


void CMainDialog::OnNMReleasedcaptureTime(NMHDR *pNMHDR, LRESULT *pResult)
{

	Jump();
	*pResult = 0;
}

double CMainDialog::GetCurrentPosition(void)
{
	int playstate = m_Player.get_playState();
	if ( 1 == playstate)
		return 0;
	int dotPos = m_Player.get_versionInfo().Find(".");
	CString verNum = m_Player.get_versionInfo().Left(dotPos);
	int ver = _ttoi(verNum);
	double curPos = m_Control.get_currentPosition();
	if (ver <= 9)
		curPos += m_PositionOffset;
	
	return curPos;
}

CString CMainDialog::GetCurrentPositionString(void)
{
	return TimeConvertToString(GetCurrentPosition());
}

void CMainDialog::SeekThroughHTTP(double seek_time_in_sec)
{
	
	i_playerState = SEEKING;
	//if ( m_Seeking == seek_time_in_sec )
	//	return;

	m_Seeking = seek_time_in_sec;
	CString str;
	CString newURL;

	m_IsPlaying = false;
	//m_Player.put_URL("http://0.0.0.1/");	// dummy

	str.Format("Requesting seeking to %d s", (int)seek_time_in_sec);

	newURL.Format("http://127.0.0.1:%d/?set_time=%d", m_DataPort, (int)seek_time_in_sec);

	m_Network.put_bufferingTime(10000);		// 1000ms buffering time
	
	m_Player.put_URL(newURL);
	m_PositionOffset = seek_time_in_sec;
	m_CurrentPosition = seek_time_in_sec;
	str.Format("Requesting seeking to %d s", (int)seek_time_in_sec);
	m_Name.SetWindowText(str);

}

void CMainDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Add your message handler code here and/or call default

	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout(m_Player.get_versionInfo());
		dlgAbout.DoModal();
	} 
	else if ((nID & 0xFFF0) == IDM_AOT)
	{
		SetAlwaysOnTop();
	}
	else if ((nID & 0xFFF0) == SC_CLOSE)
        //if user clicked the "X"
        EndDialog(IDOK);   //Close the dialog with IDOK (or IDCANCEL)
        //---end of code you have added    
    else
        CDialog::OnSysCommand(nID, lParam);
}

const LONG MIN_WIDTH  = 300; // Minimum width
const LONG MIN_HEIGHT = 300; // Minimum Height

void CMainDialog::OnSizing(UINT fwSide, LPRECT pRect)
{	
	// Set the minimum size if width going to be less than minimum width
	if ((pRect->right  - pRect->left) <= MIN_WIDTH)
		pRect->right = pRect->left+MIN_WIDTH; 

	// Set the minimum size if height going to be less than minimum height
	if ((pRect->bottom - pRect->top)<= MIN_HEIGHT)
		pRect->bottom = pRect->top+MIN_HEIGHT;

	CDialog::OnSizing(fwSide, pRect);
}

void CMainDialog::OnSize(UINT nType, int cx, int cy)
{

	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here+
	
	if (m_IsCreated)
		ResizeWindow(cx, cy);

	RedrawWindow();
}

void CMainDialog::OnNMReleasedcaptureVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	m_Setting.put_volume(m_Volume.GetPos());
	*pResult = 0;
}

int CMainDialog::ResizeWindow(int cx, int cy)
{

	m_Player.SetWindowPos(NULL, 0, 0, cx - 25, cy - 80, SWP_NOMOVE | SWP_NOZORDER);
	
	if ( cx > 400 )
	{
		
		m_Volume.SetWindowPos(		NULL,	 cx - 128,	cy - 30,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_VolLabel.SetWindowPos(	NULL,	 cx - 145,	cy - 25,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_Time.SetWindowPos(		NULL,	       10,	cy - 65,	 cx - 160, 30,				SWP_NOZORDER);
		m_SliderTime.SetWindowPos(	NULL,	 cx - 140,	cy - 55,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_Pause.SetWindowPos(		NULL, cx / 2 - 75,	cy - 30,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_Play.SetWindowPos(		NULL, cx / 2 - 20,	cy - 30,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_Stop.SetWindowPos(		NULL, cx / 2 + 35,	cy - 30,			0,	0, SWP_NOSIZE | SWP_NOZORDER);
		m_Name.SetWindowPos(		NULL,		   20,	cy - 25, cx / 2 - 100, 25,				SWP_NOZORDER);
		m_Volume.ShowWindow(true);
		m_SliderTime.ShowWindow(true);
	}
	else
	{
		m_Pause.SetWindowPos(NULL,	cx / 2 - 30, cy - 30,			0,  0, SWP_NOSIZE | SWP_NOZORDER);
		m_Play.SetWindowPos( NULL,	cx / 2 + 25, cy - 30,			0,  0, SWP_NOSIZE | SWP_NOZORDER);
		m_Stop.SetWindowPos( NULL,	cx / 2 + 80, cy - 30,			0,  0, SWP_NOSIZE | SWP_NOZORDER);
		m_Time.SetWindowPos( NULL,			 10, cy - 65,	  cx - 20, 30,				SWP_NOZORDER);
		m_Name.SetWindowPos( NULL,		     20, cy - 33, cx / 2 - 55, 25,				SWP_NOZORDER);
		m_Volume.ShowWindow(false);
		m_SliderTime.ShowWindow(false);
	}
	if ( cx > 450 )
		m_VolLabel.ShowWindow(true);
	else
		m_VolLabel.ShowWindow(false);
	return 0;
}


void CMainDialog::SetStretch(void)
{
	m_Player.put_stretchToFit(!m_Player.get_stretchToFit());
	CMenu* theMenu;
	theMenu = GetMenu();
	theMenu = theMenu->GetSubMenu(2);
	if (m_Player.get_stretchToFit() == 0)
		theMenu->CheckMenuItem(ID_VIEW_STRETCH, MF_BYCOMMAND | MF_UNCHECKED);
	else
		theMenu->CheckMenuItem(ID_VIEW_STRETCH, MF_BYCOMMAND | MF_CHECKED);	
}

void CMainDialog::ClickPlayer(short nButton, short nShiftState, long fX, long fY)
{
	// TODO: Add your message handler code here
	// middle button for toggle fullscreen
	if (nButton & 0x0004) {
		//m_Player.put_uiMode("mini");
		//m_Player.put_enabled(0);
		m_Player.put_fullScreen(!m_Player.get_fullScreen());
		//CWnd* fullScreen = this->GetForegroundWindow();
		//fullScreen->SetParent(&m_Player);
		//fullScreen->EnableWindow(false);
		//this->SetCapture();
		return;
	}

	// toggle play pause
	if ((nButton & 0x0001) == 0)
		return;
	int state = m_Player.get_playState();
	if ( state == 2 ) // paused
		m_Control.play();
	else if ( state == 3 ) //playing
		m_Control.pause();
}

void CMainDialog::DoubleClickPlayer(short nButton, short nShiftState, long fX, long fY)
{
	// TODO: Add your message handler code here
	// toggle fullscreen
	if ((nButton & 0x0001) == 0)
		return;
	m_Player.put_fullScreen(!m_Player.get_fullScreen());

	// toggle play pause
	int state = m_Player.get_playState();
	if ( state == 2 ) // paused
		m_Control.play();
	else if ( state == 3 ) //playing
		m_Control.pause();
}

double CMainDialog::SliderCurrentPositionToTime(void)
{
	double timePos = m_Time.GetPos();
	timePos = timePos / MAX_RANGE;
	if (m_Media.get_duration() > ZERO)
		timePos *= m_Media.get_duration();
	else 
		timePos*= GetCurrentPosition();
	
	return timePos;
}

void CMainDialog::CleanUp(void)
{

	KillTimer(1);
	KillTimer(2);
	//TerminateController(m_ControllerHandle);

	if(_vmMeshControllerThread != NULL)
	{
		if(_vmMeshControllerThread->getJoinableStatus())
		{
			//TerminateController(m_ControllerHandle);
			_vmMeshControllerThread->stop();
			_vmMeshControllerThread->join();

			//m_Control.stop();
			//m_Name.SetWindowText("Switching movie");
		}
		delete _vmMeshControllerThread;
		_vmMeshControllerThread = NULL;
	}
	
	if ( m_hReady != NULL )
		CloseHandle(m_hReady);
	if ( m_hNoPacket != NULL )
		CloseHandle(m_hNoPacket);
	if ( m_hReleaseMutex != NULL )
		CloseHandle(m_hReleaseMutex);
	if ( m_hReset != NULL )
		CloseHandle(m_hReset);
	if ( m_hFailed != NULL )
		CloseHandle(m_hFailed);
	
	int retry = 0;
	if ( m_DataPort )
		do
		{
			Sleep(500);
			retry++;
		} while ( !CheckIsPortAvailableP(m_DataPort) && (retry <= 10) );

	// Remove all files inside temp and playlist folder
	RemoveAllFilesInSubDir("temp");

	/*appInstanceCount = 0;

	HANDLE hMutex = NULL;
	
	hMutex = CreateMutex( NULL, TRUE, TEXT("Global\\{8AA1E2D9-3F75-434e-8ED7-A71608E30422}"));

	WaitForSingleObject(hMutex, INFINITE);
	EnumWindows((WNDENUMPROC)EnumWindowsProc, 0);

	// remove all playlists saved before ( last instance of player )
	if ( appInstanceCount == 1 )*/
		//RemoveAllFilesInSubDir("Playlist");

	if ( VMInfo )
		delete [] VMInfo;
	VMInfo = NULL;
	if ( m_ParentList )
		delete [] m_ParentList;
	m_ParentList = NULL;
	
	RemoveUPnPPortMappings();
	//DEBUG_MSG("VMesh Player Stopped");
	
}

BOOL CMainDialog::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(4);
	KillTimer(5);
	CleanUp();
	return CDialog::DestroyWindow();
}

CString CMainDialog::GetPreviousURL(void)
{
	//::AfxMessageBox(currentURL);
	return currentURL;
}

void CMainDialog::OnFileOpenvmesh()
{
	
	if (vsDlg.DoModal() == IDCANCEL)
		return;

	if ( m_CurrentPlaying >= 0 )
	{
		CMenu* pMenu = GetMenu();
		pMenu->CheckMenuItem(m_CurrentPlaying + ID_MOVIE_BASE, MF_UNCHECKED | MF_BYCOMMAND );
	}
	m_CurrentPlaying = -2;

	StartVMeshClient(vsDlg.GetMovieHash(), vsDlg.GetPortNumber(), vsDlg.GetServerAddress());
}

void CMainDialog::OnFileOpenconfigfile()
{
	LoadConfigFile();
}

void CMainDialog::LoadConfigFile()
{
	CFileDialog dlg(true, 0, 0, OFN_HIDEREADONLY |OFN_OVERWRITEPROMPT  | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST , "Configure File(.conf)|*.conf||");
	dlg.m_pOFN->lpstrTitle = "Load VMesh Config File";
	if (dlg.DoModal() == IDCANCEL)
		return;
	CString filename = dlg.GetPathName();
	FILE* file;
	file = fopen(filename, "r");
	char hash[10];
	char port[6];
	char server[30];
	fscanf(file, "%s %s %s", hash, port, server);
	fclose(file);

	if ( m_CurrentPlaying >= 0 )
	{
		CMenu* pMenu = GetMenu();
		pMenu->CheckMenuItem(m_CurrentPlaying + ID_MOVIE_BASE, MF_UNCHECKED | MF_BYCOMMAND );
	}
	m_CurrentPlaying = -2;

	StartVMeshClient(hash, port, server);
}


UINT WaitController(PVOID lpParam);
//char command[100];

// initiate the VMesh client
void CMainDialog::StartVMeshClient(const char* hash, const char* port, const char* server)
{
	if (i_playerState == NON_STARTED)
	{
		GET_TIMER(m_StartupTimerHandler)
		i_playerState = STARTING;
	}
	CreateDirectory("temp" , NULL);

	strcpy(m_sHash, hash);
	strcpy(m_sPort, port);
	strcpy(m_sServer, server);

	m_IsPlaying = false;
	CString localAddress;
	char msg[20];

	//if (m_ControllerHandle)
	if(_vmMeshControllerThread != NULL)
	{
		if(_vmMeshControllerThread->getJoinableStatus())
		{
			//TerminateController(m_ControllerHandle);
			_vmMeshControllerThread->stop();
			_vmMeshControllerThread->join();

			//m_Control.stop();
			//m_Name.SetWindowText("Switching movie");
		}
		delete _vmMeshControllerThread;
		_vmMeshControllerThread = NULL;
	}

	if (!m_DataPort)
		m_DataPort = DATA_PORT + ( rand() % 1000 ) ;

	m_hMutex = CreateMutex(NULL, false, "VMPlayer_Mutex");
	WaitForSingleObject(m_hMutex, INFINITE);

	while ( (m_DataPort < 65535) && (CheckIsPortAvailableP(m_DataPort) == false) )
		m_DataPort++;
	if ( m_DataPort == 65535 )
	{
		m_DataPort = 1;
		while ( (m_DataPort < DATA_PORT) && (CheckIsPortAvailableP(m_DataPort) == false) )
			m_DataPort++;
		if ( m_DataPort == DATA_PORT )
		{
			::AfxMessageBox("No available port to use");
			return;
		}
	}

	if ( m_hReady != NULL )
		CloseHandle(m_hReady);
	if ( m_hNoPacket != NULL )
		CloseHandle(m_hNoPacket);
	if ( m_hReleaseMutex != NULL )
		CloseHandle(m_hReleaseMutex);
	if ( m_hReset != NULL )
		CloseHandle(m_hReset);
	if ( m_hFailed != NULL )
		CloseHandle(m_hFailed);

	SECURITY_ATTRIBUTES eventAttributes;
	eventAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	eventAttributes.bInheritHandle = true;
	eventAttributes.lpSecurityDescriptor = NULL;

	sprintf(msg, "Ready%d", m_DataPort);
	m_hReady = CreateEvent(&eventAttributes, true, false, msg);

	sprintf(msg, "Num Packets%d", m_DataPort);
	m_hNoPacket = CreateEvent(&eventAttributes, true, false, msg);

	sprintf(msg, "Release Mutex%d", m_DataPort);
	m_hReleaseMutex = CreateEvent(&eventAttributes, true, false, msg);

	m_hReset = CreateEvent(&eventAttributes, true, false, "Reset");
	m_hFailed = CreateEvent(&eventAttributes, true, false, "VMesh Failed");

	is_ControllerRun = 0; //Add by Juan, initialization

	//sprintf(command, "%s %s %d %s %d", hash, port, m_DataPort, server, numSeg );

	//Add by Juan
	// start the VMesh process
	//StartVMeshProcess();//Comment by Juan
	this->VCInfo = new ControllerInfo;
	this->VCInfo->hash = (char *)hash;
	cout<<hash<<endl;
	this->VCInfo->port = (char *)port;
	cout<<port<<endl;
	this->VCInfo->numSegment = numSeg;
	cout<<numSeg<<endl;
	this->VCInfo->dataport = m_DataPort;
	cout<<m_DataPort<<endl;

	this->VCInfo->server = (char *)server;
	cout<<server<<endl;
	
	StartVMeshProcess(VCInfo);


	while ((WaitForSingleObject(m_hReleaseMutex, 100) != WAIT_OBJECT_0)
		&& (is_ControllerRun==0))
		Sleep(100);

	ReleaseMutex(m_hMutex);
}
//Edit by Juan
bool CheckIsPortAvailableP(int port)
{
	if (port < 1 || port > 65535 )
		return false;
	AfxSocketInit();
	CSocket testSocket;
	testSocket.Create(port); 
	bool result = testSocket.Listen();
	testSocket.Close(); 
	return result;
}

UINT CheckController(PVOID lpParam)
{
	CMainDialog* thisDlg = (CMainDialog*) lpParam;
	//::AfxMessageBox("Controller!!!!");
	thisDlg->CheckControllerThreadFunction();
	return 0;
}

void CMainDialog::CheckControllerThreadFunction()
{
	while (1)
	{
		//::AfxMessageBox("Controller!!!!");
		if (WaitForSingleObject(m_hFailed, 100) == WAIT_OBJECT_0)
		{
			KillTimer(4);
			KillTimer(5);
			//if (WaitForSingleObject(m_ControllerHandle, 0) == WAIT_TIMEOUT )
			if (is_ControllerRun == 0)
			{
				//TerminateController(m_ControllerHandle);
				_vmMeshControllerThread->stop();

				//TerminateProcess((HANDLE)m_ControllerHandle, 0);
				//m_Control.stop();
				m_Name.SetWindowText("Connection Failed");
				Sleep(2000);
				m_Name.SetWindowText("Reconnecting...");
			}
			m_ResetControllerThread = NULL;
			ResetEvent(m_hFailed);
			return;
		}

		if (WaitForSingleObject(m_hReset, 100) == WAIT_OBJECT_0)
		{
			KillTimer(4);
			KillTimer(5);
			m_Name.SetWindowText("Reconnecting...");
			ResetEvent(m_hReset);
			Sleep(1000);
			break;
		}
		
		Sleep(1000);
	}

	if ( (m_CurrentPosition == (int)m_Media.get_duration()) || ((m_CurrentPosition+1) == (int)m_Media.get_duration()) )
	{
		StartVMeshClient(m_sHash, m_sPort, m_sServer);
	}
	else
	{
		int currentPosition = m_CurrentPosition;
		if (!m_NewSeekTime)
			m_NewSeekTime = currentPosition;
		StartVMeshClient(m_sHash, m_sPort, m_sServer);
	}

	m_ResetControllerThread = NULL;
}

UINT WaitController(PVOID lpParam)
{
	CMainDialog* thisDlg = (CMainDialog*) lpParam;

	thisDlg->EnableControls(false);
	//::AfxMessageBox("VMeshController Start");
	while (WaitForSingleObject(thisDlg->m_hReady, 100) != WAIT_OBJECT_0
		//&& WaitForSingleObject(thisDlg->m_ControllerHandle, 100) != WAIT_OBJECT_0)
		&& (is_ControllerRun==0))
		Sleep(100);
	ResetEvent(thisDlg->m_hReady);
	
	//::AfxMessageBox("VMeshController Start");
	if (is_ControllerRun==1){
		for(int i=0;i<6000;i++){
			if(is_ControllerRun==2)
				break;
			else
				sleep(10);
		}
	}
	if(is_ControllerRun == 1)
	{
		::AfxMessageBox("Movie does not exist"); //add by Juan
		thisDlg->KillTimer(4);
		thisDlg->KillTimer(5);
		thisDlg->GetMenu()->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
		thisDlg->DrawMenuBar();
		int currentPosition = thisDlg->m_CurrentPosition;
		if (0 == (int)(thisDlg->m_NewSeekTime))
			thisDlg->m_NewSeekTime = currentPosition;
		CString str;
		str.Format("Requesting seeking to %d s", (int)thisDlg->m_NewSeekTime);
		thisDlg->m_Name.SetWindowText(str);
		//TerminateProcess((HANDLE)thisDlg->m_ControllerHandle, 0);
		//thisDlg->TerminateController((HANDLE)thisDlg->m_ControllerHandle);
		_vmMeshControllerThread->stop();
		thisDlg->KillTimer(6);
		thisDlg->SetTimer(6, 30000, 0);
		return 0;
	}

	SetEvent(thisDlg->m_hStart);

	thisDlg->EnableControls(true);

	return 0;
}

void CMainDialog::StartVMeshProcess(ControllerInfo *VCInfo)
{
	Sleep(100);
	STARTUPINFO startupinfo = { sizeof(STARTUPINFO) };
	ZeroMemory( &startupinfo, sizeof( STARTUPINFO) );
	PROCESS_INFORMATION process_information;

	if (!showConsole) 
	{
		startupinfo.dwFlags |= STARTF_USESHOWWINDOW;
		startupinfo.wShowWindow = false;
	}
	startupinfo.dwFlags |= STARTF_FORCEOFFFEEDBACK;

	m_CurrentPosition = 0;

	CString controllerFilePath = m_CurrentDirectory;

	//#if (ConfigurationName == Debug)
	//	controllerFilePath += "\\VMeshControllerD.exe";
	//	if(!CreateProcess(controllerFilePath, command, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupinfo, &process_information))
	//	{
	//		AfxMessageBox("Cannot start VMesh Controller");
	//		printf("Cannot start VMesh Controller\n");
	//		return;
	//	}
	//#else
	/*coment by Juan
		controllerFilePath += "\\VMeshController.exe";
		if(!CreateProcess(controllerFilePath, command, NULL, NULL, true, NORMAL_PRIORITY_CLASS , NULL, NULL, &startupinfo, &process_information))
		{
			AfxMessageBox("VMesh Initialization Error", MB_ICONSTOP | MB_OK);
			return;
		}
	//#endif
	end comment by Juan
	*/
	
	//m_ControllerHandle = process_information.hProcess;StartVMeshContoller

	//start VMeshController Thread			[Modify by jack 5-08-2010]
	//m_ControllerHandle = (HANDLE)_beginthreadex(NULL,0,&StartVMeshContoller,(void *)VCInfo,0,&threadID);

	_vmMeshControllerThread = new VMeshControllerThread((void *)VCInfo);

	//cout << "Before everything start, check dataport: " << VCInfo->dataport << endl;
	//cout << "Before everything start, check server ip: " << VCInfo->server << endl;
	//m_ControllerHandle = (HANDLE)_vmMeshControllerThread->getThreadHandler();

	if (_vmMeshControllerThread == NULL)
		cout << "Before everything start, check _vmMeshControllerThread is NULL~"<< endl;
	else
		_vmMeshControllerThread->run();		//For test

	KillTimer(6);
	m_Name.SetWindowText("Initializing... ");
	SetTimer(6, 180000, 0);
	::AfxBeginThread(WaitController, this);
	if (m_ResetControllerThread)
		TerminateThread(m_ResetControllerThread->m_hThread, 0);
	m_ResetControllerThread = ::AfxBeginThread(CheckController, this);
	if (showConsole) {
		Sleep(500);
		AfxGetMainWnd()->SetForegroundWindow();
		AfxGetMainWnd()->SetFocus();
	}
	SetTimer(3, 100, 0);
}

HCURSOR CMainDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDialog::OnAbout()
{
	CAboutDlg dlgAbout(m_Player.get_versionInfo());
	dlgAbout.DoModal();
}

void CMainDialog::SetAlwaysOnTop(void)
{
	CMenu* pMenu = GetMenu();
	CMenu* pSysMenu = GetSystemMenu(false);
	if (m_AlwaysOnTop)
	{
		pMenu->CheckMenuItem(ID_VIEW_ALWAYSONTOP, MF_BYCOMMAND | MF_UNCHECKED);
		pSysMenu->CheckMenuItem(IDM_AOT, MF_BYCOMMAND | MF_UNCHECKED);
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
	else
	{
		pMenu->CheckMenuItem(ID_VIEW_ALWAYSONTOP, MF_BYCOMMAND | MF_CHECKED);
		pSysMenu->CheckMenuItem(IDM_AOT, MF_BYCOMMAND | MF_CHECKED);
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
	m_AlwaysOnTop = !m_AlwaysOnTop;
}

void CMainDialog::OnViewAlwaysontop()
{
	SetAlwaysOnTop();
}

void CMainDialog::EnableControls(bool bEnable)
{
	m_Time.EnableWindow(bEnable);
	m_Play.EnableWindow(bEnable);
	m_Stop.EnableWindow(bEnable);
	m_Pause.EnableWindow(bEnable);
	if (bEnable)
		GetMenu()->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
	else
		GetMenu()->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	DrawMenuBar();
}

void CMainDialog::ConnectToServer()
{
	return;
}	

void CMainDialog::OnFileConnecttoserver()
{
	ConnectToServer();
}


void CMainDialog::OnFileOpenserverplaylist()
{
	int i, j;
	VMeshMovieInfo currentPlayingVMInfo;
	CString currentPlayingMovieName;
	CString currentPlaylistName;
	CString currentPlaylistServer;
	int currentPlaylistPort;
	bool playing = false;
	CServerPlaylistDialog dlg(m_PlaylistName, m_IPAddress, m_ServerPort, m_PlaylistCount );
	dlg.SetCurrentDirectory(m_CurrentDirectory);
	CMenu* pMenu= GetMenu();

	if ( m_CurrentPlaying >= 0 )
	{
		playing = true;
		currentPlayingVMInfo = VMInfo[m_CurrentPlaying];
		currentPlaylistName = m_PlaylistName[m_ParentList[m_CurrentPlaying]];
		currentPlaylistServer = m_IPAddress[m_ParentList[m_CurrentPlaying]];
		currentPlaylistPort = m_ServerPort[m_ParentList[m_CurrentPlaying]];
		pMenu->GetMenuString(ID_MOVIE_BASE + m_CurrentPlaying,  currentPlayingMovieName, MF_BYCOMMAND);

	}

	if (dlg.DoModal() == IDCANCEL )
		return;
	
	if ( m_CurrentPlaying >= 0 )
		m_CurrentPlaying = -1;

	totalMovieCount = RefreshPlaylist(false);

	for ( i = 0; i < totalMovieCount; i++ )
	{
		CString movieName;
		pMenu->GetMenuString(i + ID_MOVIE_BASE, movieName, MF_BYCOMMAND);
		if ( (currentPlayingVMInfo.IP_Address == VMInfo[i].IP_Address) &&
			(currentPlayingVMInfo.Movie_Hash == VMInfo[i].Movie_Hash) &&
			(currentPlayingVMInfo.Port == VMInfo[i].Port) &&
			(currentPlayingMovieName ==  movieName) &&
			//(currentPlaylistName == m_PlaylistName[m_ParentList[i]]) &&
			(currentPlaylistServer == m_IPAddress[m_ParentList[i]]) &&
			(currentPlaylistPort == m_ServerPort[m_ParentList[i]]) )
		{
			pMenu->CheckMenuItem(i + ID_MOVIE_BASE, MF_CHECKED | MF_BYCOMMAND );
			m_CurrentPlaying = i;
			return;
		}
			
	}

	if ( m_CurrentPlaying == -1 )
		ChooseNewMovie();
}

void CMainDialog::OnMovieSelected( UINT nID )
{
	int index = nID - ID_MOVIE_BASE;
	GET_TIMER(m_SwitchMovieTimerHandler)
	m_bSwitchingMovie = true;
	i_playerState = SWITCHING;
	OpenSelectedMovie(index);
}

void CMainDialog::StopPlaying(void)
{
	m_Control.stop();
	m_Time.SetPos(0);
	m_SliderTime.SetWindowText("00:00");
	m_Play.EnableWindow(false);
	m_Stop.EnableWindow(false);
	m_Pause.EnableWindow(false);
}

bool CALLBACK CMainDialog::EnumWindowsProc(HWND hWnd, long lParam)
{
	char className[256];
	GetClassName(hWnd, className, 255);
	if ( strcmp(className, szClassName) == 0 )
		appInstanceCount++;
	return true;
}

int CMainDialog::RefreshPlaylist(bool nDownload)
{
	CMenu* pMenu = GetMenu();
	CMenu* pSubMenu;
	CFile file;
	int i, j;
	int length;
	totalMovieCount = 0;
	CString url, filepath;
	CString menuString;
	CString movieName;
	unsigned char movieCount;
	unsigned char ip[4];
	char id[5];
	unsigned char len[2];
	char* strBuffer;

	pMenu->DeleteMenu(1, MF_BYPOSITION);
	pMenu->InsertMenu(1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT) pMenu->m_hMenu, "Movie");
	pMenu = pMenu->GetSubMenu(1);
	
	for ( i = m_PlaylistCount-1; i >=0 ; i-- )
	{
		filepath.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, m_PlaylistName[i], m_IPAddress[i], m_ServerPort[i]);
		
		if (nDownload)
		{
			url.Format("http://%s:%d/%s.lst", m_IPAddress[i], m_ServerPort[i], m_PlaylistName[i]);
			if ( CServerPlaylistDialog::DownloadFile(url, filepath) == false )	
			{
				//download fails
				//::AfxMessageBox((CString)"Cannot download playlist from server: " + m_IPAddress[i]);
				remove(filepath);
				m_PlaylistCount--;
				for (int j = i; j < m_PlaylistCount; ++j) {
					m_PlaylistName[j] = m_PlaylistName[j+1];
					m_IPAddress[j] = m_IPAddress[j+1];
					m_ServerPort[j] = m_ServerPort[j+1];
				}
				continue;
			}
		}
		int movieCount = CServerPlaylistDialog::ReadPlaylistCount(filepath);
		if (movieCount > 0) {
			//::AfxMessageBox((CString)"Downloaded playlist from server: " + m_IPAddress[i]);
			totalMovieCount += movieCount;
		} else {
			//playlist fails
			//::AfxMessageBox((CString)"Corrupted playlist from server: " + m_IPAddress[i]);
			remove(filepath);
			m_PlaylistCount--;
			for (int j = i; j < m_PlaylistCount; ++j) {
				m_PlaylistName[j] = m_PlaylistName[j+1];
				m_IPAddress[j] = m_IPAddress[j+1];
				m_ServerPort[j] = m_ServerPort[j+1];
			}
			continue;
		}
	}

	if ( VMInfo )
		delete [] VMInfo;
	if ( m_ParentList )
		delete [] m_ParentList;

	VMInfo = new VMeshMovieInfo[totalMovieCount];
	m_ParentList = new int[totalMovieCount];
	totalMovieCount = 0;
	for ( i = 0; i < m_PlaylistCount; i++ )
	{
		filepath.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, m_PlaylistName[i], m_IPAddress[i], m_ServerPort[i]);
		//::AfxMessageBox((CString)("filepath: ") + filepath);
		if ( file.Open(filepath, CFile::modeRead ) == false )
			continue;
		file.Read(id, 4);
		id[4] = '\0';
		if ( strcmp(id, "SRPL") != 0 )
		{
			file.Close();
			//::AfxMessageBox("Invalid Playlist");
			continue;
		}

		menuString.Format("%s (%s)", m_PlaylistName[i], m_IPAddress[i]);
		pMenu->AppendMenu(MF_STRING | MF_POPUP, (UINT) pMenu->m_hMenu, menuString);
		pSubMenu = pMenu->GetSubMenu(i);
		file.Read((void *)&movieCount, 1);
		file.Read(ip, 4);
		for ( j = 0; j < movieCount; j++ )
		{
			file.Read(len, 2);
			length = ( len[0] << 8 ) | len[1];
			if ( length < 8 )
				strBuffer = new char[9];
			else
				strBuffer = new char[length+1];
			file.Read(strBuffer, length);
			strBuffer[length] = '\0';
			movieName = strBuffer;
			delete []strBuffer;

			file.Read(len, 2);
			length = ( len[0] << 8 ) | len[1];
			strBuffer = new char[length+1];
			file.Read(strBuffer, length);
			strBuffer[length] = '\0';
			VMInfo[totalMovieCount].Movie_Desc = strBuffer;
			VMInfo[totalMovieCount].Movie_Desc.Replace("\\\\", "\n");

			file.Read(len, 2);
			VMInfo[totalMovieCount].Port = ( len[0] << 8 ) | len[1]; 
			file.Read(strBuffer, 8);
			strBuffer[8] = '\0';
			VMInfo[totalMovieCount].Movie_Hash = strBuffer;
			
			VMInfo[totalMovieCount].IP_Address.Format("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
			m_ParentList[totalMovieCount] = i;
			pSubMenu->AppendMenu(MF_STRING, ID_MOVIE_BASE + totalMovieCount, movieName);

			delete []strBuffer;
			totalMovieCount++;
		}
		file.Close();
	}

	if ( totalMovieCount > 0 )
		pMenu->AppendMenu(MF_SEPARATOR);
	pMenu->AppendMenu(MF_STRING, ID_MOVIE_CHOOSEFROMPLAYLIST, "Movie List");
	pMenu->AppendMenu(MF_STRING, ID_MOVIE_REFRESHPLAYLIST, "Refresh Playlist");
	if ( totalMovieCount == 0 )
		pMenu->EnableMenuItem(ID_MOVIE_REFRESHPLAYLIST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	return totalMovieCount;
}

void CMainDialog::ChooseNewMovie()
{
	CMenu* pMenu = GetMenu();
	TerminateController(m_ControllerHandle);
	//StopPlaying();
	//m_Name.SetWindowText("No movie selected");

	if (  totalMovieCount == 0 )
		return;
	CMovieListDialog dlg;
	dlg.SetMovieCount(totalMovieCount + m_PlaylistCount);
	
	//char a[20];
	//sprintf( a, "%d", totalMovieCount);
	//::AfxMessageBox(a);
	/*
	CMenu* pSubMenu = GetMenu()->GetSubMenu(1);
	CMenu* pPlaylistMenu;
	CString name;
	int count = 0;
	int i, j;
	for ( i = 0; i < pSubMenu->GetMenuItemCount() - 3 ; i++ )
	{
		pPlaylistMenu = pSubMenu->GetSubMenu(i);
		dlg.AddItem(m_PlaylistName[i] + " (" + m_IPAddress[i] + ")", -1);
		for ( j = 0 ; j < pPlaylistMenu->GetMenuItemCount() ; j++ )
		{
			pMenu->GetMenuString(ID_MOVIE_BASE + count, name, MF_BYCOMMAND);
			dlg.AddItem(name, count);
			//char a[50];
			//sprintf( a, "%s %d", name, count);
			//::AfxMessageBox(a);
			count++;
		}
	}*/
	int count = 0;
	int i, j;
	CString filepath;
	CFile file;
	CString movieName;
	unsigned char movieCount;
	unsigned char  ip[4];
	char id[5];
	unsigned char len[2];
	int length;
	char* strBuffer;

	for ( i = 0; i < m_PlaylistCount; i++ )
	{
		dlg.AddItem(m_PlaylistName[i] + " (" + m_IPAddress[i] + ")", m_PlaylistName[i] + " (" + m_IPAddress[i] + ")", -1);

		filepath.Format("%s\\Playlist\\%s@%s.%d.lst", m_CurrentDirectory, m_PlaylistName[i], m_IPAddress[i], m_ServerPort[i]);
		//::AfxMessageBox((CString)("filepath: ") + filepath);
		if ( file.Open(filepath, CFile::modeRead ) == false )
			continue;
		file.Read(id, 4);
		id[4] = '\0';
		if ( strcmp(id, "SRPL") != 0 )
		{
			file.Close();
			//::AfxMessageBox("Invalid Playlist");
			continue;
		}

		file.Read((void *)&movieCount, 1);
		file.Read(ip, 4);
		for ( j = 0; j < movieCount; j++ )
		{
			file.Read(len, 2);
			length = ( len[0] << 8 ) | len[1];
			if ( length < 8 )
				strBuffer = new char[9];
			else
				strBuffer = new char[length+1];
			file.Read(strBuffer, length);
			strBuffer[length] = '\0';
			movieName = strBuffer;
			delete []strBuffer;

			file.Read(len, 2);
			length = ( len[0] << 8 ) | len[1];
			strBuffer = new char[length+1];
			file.Read(strBuffer, length);
			strBuffer[length] = '\0';
			CString Movie_Desc = strBuffer;
			Movie_Desc.Replace("\\\\", "\n");
			dlg.AddItem(movieName, Movie_Desc, count);

			file.Read(len, 2);
			//VMInfo[totalMovieCount].Port = ( len[0] << 8 ) | len[1]; 
			file.Read(strBuffer, 8);
			//strBuffer[8] = '\0';
			//VMInfo[totalMovieCount].Movie_Hash = strBuffer;
			
			//VMInfo[totalMovieCount].IP_Address.Format("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
			//m_ParentList[totalMovieCount] = i;

			//delete []strBuffer;
			count++;
		}
		file.Close();
	}

	if ( dlg.DoModal() == IDCANCEL )
		return;

	this->RefreshPlaylist(false);
	if ( dlg.GetMovieSelection() > -1 )
	{
		CString temp;
		temp.Format("%d", VMInfo[dlg.GetMovieSelection()].Port);
		StartVMeshClient(VMInfo[dlg.GetMovieSelection()].Movie_Hash, temp, VMInfo[dlg.GetMovieSelection()].IP_Address);
		pMenu->CheckMenuItem(ID_MOVIE_BASE + dlg.GetMovieSelection() , MF_CHECKED | MF_BYCOMMAND);
		m_CurrentPlaying = dlg.GetMovieSelection() ;
	}
}

void CMainDialog::OnMovieRefreshPlaylist()
{
	OpenSelectedMovie(-1);
}

void CMainDialog::OnConfiguration()
{
	CConfigDialog dlg;
	dlg.setValue(this->showConsole, this->numSeg);
	if ( dlg.DoModal() == IDCANCEL )
		return;
	dlg.getValue(this->showConsole, this->numSeg);
}

void CMainDialog::OpenSelectedMovie(int nIndex)
{
	int index = nIndex;
	int i;

	if ( (index >= 0) && (index == m_CurrentPlaying) )
	{
		int duration = 0;
		STOP_TIMER(m_SwitchMovieTimerHandler, NULL, duration)
		m_SwitchMovieTimerHandler = -1;
		m_bSwitchingMovie = false;
		i_playerState = PLAYING;
		return;
	}
	
	CString temp;
	CString targetMovieName;
	CMenu* pMenu = GetMenu();
	if ( index >= 0 )
	{
		pMenu->GetMenuString(index + ID_MOVIE_BASE, targetMovieName, MF_BYCOMMAND);
		temp.Format("Switching to \"%s\"\nDo you want to continue?", targetMovieName);
		if ( IDNO  == MessageBox(temp, "Movie switching", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) )
		{
			int duration = 0;
			STOP_TIMER(m_SwitchMovieTimerHandler, NULL, duration)
			m_SwitchMovieTimerHandler = -1;
			m_bSwitchingMovie = false;
			i_playerState = PLAYING;
			return;
		}
	}

	
	VMeshMovieInfo targetVMInfo;
	CString targetPlaylistName, targetPlaylistServer;
	int targetPlaylistPort;

	VMeshMovieInfo currentPlayingVMInfo;
	CString currentPlayingMovieName, currentPlaylistName, currentPlaylistServer;
	int currentPlaylistPort;
	bool playingPlaylistItem = false;

	if ( index >= 0 )
	{
		targetVMInfo = VMInfo[index];

		targetPlaylistName = m_PlaylistName[m_ParentList[index]];
		targetPlaylistServer = m_IPAddress[m_ParentList[index]];
		targetPlaylistPort = m_ServerPort[m_ParentList[index]];
	}
	if ( m_CurrentPlaying >= 0 )
	{
		playingPlaylistItem = true;
		currentPlayingVMInfo = VMInfo[m_CurrentPlaying];
		currentPlaylistName = m_PlaylistName[m_ParentList[m_CurrentPlaying]];
		currentPlaylistServer = m_IPAddress[m_ParentList[m_CurrentPlaying]];
		currentPlaylistPort = m_ServerPort[m_ParentList[m_CurrentPlaying]];
		pMenu->GetMenuString(ID_MOVIE_BASE + m_CurrentPlaying,  currentPlayingMovieName, MF_BYCOMMAND);
	}

	totalMovieCount = RefreshPlaylist(true);

	if ( index >= 0 )

		for ( i = 0; i < totalMovieCount; i++ )
		{
			CString movieName;
			pMenu->GetMenuString(i + ID_MOVIE_BASE, movieName, MF_BYCOMMAND);
			if ( (targetVMInfo.IP_Address == VMInfo[i].IP_Address) &&
				(targetVMInfo.Movie_Hash == VMInfo[i].Movie_Hash) &&
				(targetVMInfo.Port == VMInfo[i].Port) &&
				//targetMovieName == movieName &&
				(targetPlaylistName == m_PlaylistName[m_ParentList[i]])  &&
				(targetPlaylistServer == m_IPAddress[m_ParentList[i]]) &&
				(targetPlaylistPort == m_ServerPort[m_ParentList[i]]) )
			{
				pMenu->CheckMenuItem(i + ID_MOVIE_BASE, MF_CHECKED | MF_BYCOMMAND );
				m_CurrentPlaying = i;
				temp.Format("%d", VMInfo[i].Port);
				StartVMeshClient(VMInfo[i].Movie_Hash, temp, VMInfo[i].IP_Address);
				m_bSwitchingMovie = false;
				return;
			}
				
		}

	if ( playingPlaylistItem )
	{
		m_CurrentPlaying = -1;
		for ( i = 0; i < totalMovieCount; i++ )
		{
			CString movieName;
			pMenu->GetMenuString(i + ID_MOVIE_BASE, movieName, MF_BYCOMMAND);
			if ( (currentPlayingVMInfo.IP_Address == VMInfo[i].IP_Address) &&
				(currentPlayingVMInfo.Movie_Hash == VMInfo[i].Movie_Hash) &&
				(currentPlayingVMInfo.Port == VMInfo[i].Port) &&
				//currentPlayingMovieName ==  movieName &&
				(currentPlaylistName == m_PlaylistName[m_ParentList[i]])  &&
				(currentPlaylistServer == m_IPAddress[m_ParentList[i]]) &&
				(currentPlaylistPort == m_ServerPort[m_ParentList[i]]) )
			{
				pMenu->CheckMenuItem(i + ID_MOVIE_BASE, MF_CHECKED | MF_BYCOMMAND );
				m_CurrentPlaying = i;
				break;
			}
		}
	}

	if ( index >= 0 )
		::AfxMessageBox("The movie selected is not in server playlist");
	if ( m_CurrentPlaying == -1 )
		ChooseNewMovie();
}

void CMainDialog::RemoveAllFilesInSubDir(CString subdir)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(CString(m_CurrentDirectory) + "\\" + subdir + "\\*.*");
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		remove(finder.GetFilePath());
	}
}

void CMainDialog::RemoveUPnPPortMappings(void)
{
	bool bContinue = true;

	if ( FAILED( CoInitialize(NULL) ) )
		return;
	
	IUPnPNAT* piNAT = NULL;
	IStaticPortMappingCollection* piPortMappingCollection = NULL;	
	IUnknown* piUnk = NULL;
	IEnumVARIANT* piEnumerator = NULL;
	long cMappings = 0;
	HRESULT hr = S_OK;
	
	if ( !bContinue || FAILED( CoCreateInstance(__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void **)&piNAT) ) || ( piNAT==NULL ) )
		bContinue = false;
	
	if ( !bContinue || FAILED( piNAT->get_StaticPortMappingCollection(&piPortMappingCollection) ) || (piPortMappingCollection==NULL ) )
		bContinue = false;

	if ( !bContinue || FAILED( piPortMappingCollection->get__NewEnum( &piUnk ) ) || piUnk==NULL )
		bContinue = false;

	if ( !bContinue || FAILED( piUnk->QueryInterface(IID_IEnumVARIANT, (void **)&piEnumerator) ) || piEnumerator==NULL )
		bContinue = false;
	
	if ( !bContinue || FAILED( piEnumerator->Reset() ) )
		bContinue = false;

	if ( !bContinue || FAILED( piPortMappingCollection->get_Count( &cMappings ) ) )
		bContinue = false;

	WSADATA wsaData;
	char name[255];
	PHOSTENT hostinfo;
	char *ip;
	bool bWSACleanup = true;

	if (  WSAStartup(MAKEWORD(1,1), &wsaData) != 0 )
	{
		bWSACleanup = false;
		bContinue = false;
	}

	if( gethostname ( name, sizeof(name)) != 0)
		bContinue = false;
	
	if ((hostinfo = gethostbyname(name)) == NULL)
		bContinue = false;
		
	std::vector<long> lPortToBeRemoved;
	while ( bContinue )
	{
		VARIANT varCurMapping;
		VariantInit(&varCurMapping);
		
		hr = piEnumerator->Next( 1, &varCurMapping, NULL);
		if( FAILED(hr) )
			break;
		if ( varCurMapping.vt == VT_EMPTY )
			break;
		
		IStaticPortMapping* piMapping = NULL;
		IDispatch* piDispMap = V_DISPATCH(&varCurMapping);
		hr = piDispMap->QueryInterface(IID_IStaticPortMapping, (void **)&piMapping);
		if( FAILED(hr) )
			break;

		CComBSTR szDescription;
		CComBSTR szInternalClient;

		piMapping->get_Description(&szDescription);
		piMapping->get_InternalClient(&szInternalClient);

		int nCount = 0;
		if ( (szDescription == CComBSTR("VMesh Port")) || (szDescription == CComBSTR("DHT Port")) )
		{
			while( hostinfo->h_addr_list[nCount] )
			{
				ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[nCount]);
				if ( szInternalClient == CComBSTR(ip) )
				{
					long lExternalPort;
					piMapping->get_ExternalPort(&lExternalPort);
					lPortToBeRemoved.push_back(lExternalPort);
					break;
				}
				nCount++;
			}
		}

		piMapping->Release();
		piMapping = NULL;
	
		VariantClear( &varCurMapping );
	}
	
	std::vector<long>::const_iterator it;
	for ( it = lPortToBeRemoved.begin(); it < lPortToBeRemoved.end(); it++ )
		piPortMappingCollection->Remove(*it, CComBSTR("TCP"));

	// release COM objects and de-initialize COM
	
	if ( piEnumerator != NULL )
	{
		piEnumerator->Release();
		piEnumerator = NULL;
	}
	
	if ( piPortMappingCollection != NULL )
	{
		piPortMappingCollection->Release();
		piPortMappingCollection = NULL;
	}
	
	if ( piNAT != NULL )
	{
		piNAT->Release();
		piNAT = NULL;
	}
	
	if ( bWSACleanup )
		WSACleanup();
	CoUninitialize();
}

 void CMainDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)  
 {
	CDialog::OnHScroll(nSBCode,nPos,pScrollBar);
	if ( pScrollBar == NULL )
		return;

	if ( (m_Time.GetSafeHwnd() != NULL) && (&m_Time == (CNiceSliderCtrl*) pScrollBar) )
	{
		if ( nSBCode == TB_THUMBPOSITION )
			Jump();
		if ( nSBCode == TB_THUMBTRACK )
		{
			CRect rc;
			m_Time.GetChannelRect(&rc);
			m_Time.ClientToScreen(&rc);
			this->ScreenToClient(&rc);
			CPoint point;
			::GetCursorPos(&point);
			this->ScreenToClient(&point);

			double time;
			const int nMax = MAX_RANGE+1;
			time = (double)(point.x - rc.left)/(rc.right - rc.left);
			if ( time < 0.0 )
				time = 0.0;
			if ( time > 1.0 )
				time = 1.0;
			time *= m_Media.get_duration();
			m_TimeToolTip.UpdateTipText(TimeConvertToString(time) , &m_Time);
		}
	}
	if ( (m_Volume.GetSafeHwnd() != NULL) && (&m_Volume == (CNiceSliderCtrl*) pScrollBar) )
	{
		if ( nSBCode == TB_THUMBPOSITION )
		{
			m_Setting.put_volume(m_Volume.GetPos());
		}
	}
 }

 void CMainDialog::Jump(void)
 {
	 m_Time.EnableWindow(false);
	double timePos = SliderCurrentPositionToTime();
	if (m_Network.get_sourceProtocol() == "file"){
		m_Control.pause();
		m_Control.put_currentPosition(timePos);
	}
	else
	{
		GET_TIMER(m_SeekingTimerHandler);
		CString szTime;
		m_SliderTime.SetWindowText(TimeConvertToString((int)timePos) + " / " + m_Media.get_durationString());
		SeekThroughHTTP(timePos);
	}
	m_Control.play();
 }

 BOOL CMainDialog::PreTranslateMessage(MSG* pMsg)
 {
	 if ( m_TimeToolTip.m_hWnd )
		 m_TimeToolTip.RelayEvent(pMsg);

	 return CDialog::PreTranslateMessage(pMsg);
 }

bool CMainDialog::TerminateController(HANDLE& handler)
{
	// Notify running controller to terminate
	CString str;
	CString newURL;

	m_IsPlaying = false;
	//m_Player.put_URL("http://0.0.0.1/");	// dummy

	str.Format("Requesting Teminate");

	newURL.Format("http://127.0.0.1:%d/?command=teminate", m_DataPort);
	
	m_Player.put_URL(newURL);

	m_Control.stop();
	
	if (handler)
		TerminateProcess(handler, 0);
	//Modified by Juan, Mar. 31
	if ( handler != NULL )
		CloseHandle(handler);
	//End by Juan
	return true;
}

