#pragma once


#include "cwmpplayer4.h"
#include "cwmpcontrols3.h"
#include "cwmpmedia3.h"
#include "cwmpnetwork.h"
#include "cwmpsettings20.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "VMesh Player.h"
#include "VMeshSettingDialog.h"
#include "ServerDialog.h"
#include "ServerPlaylistDialog.h"
#include "MovieListDialog.h"
#include "NiceSlider.h"
#include "ConfigDialog.h"
#include "..\Common\Common.h"
//#include "..\VMesh Controller\VMeshController.h"
#include "..\VMesh Controller\VMeshControllerThread.h"
// CMainDialog dialog

#define MAX_PATH 1024
extern const int MAX_LIST;
#define MAX_MOVIE_NUM					10000
extern const char* szClassName;
extern int is_ControllerRun; //Add by Juan

struct VMeshMovieInfo
{
	CString IP_Address;
	CString Movie_Hash;
	CString Movie_Desc;
	int Port;
};

class CMainDialog : public CDialog
{
	DECLARE_DYNAMIC(CMainDialog)

protected:

	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CMainDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainDialog();

// Dialog Data
	enum { IDD = IDD_MAINDIALOG };
	

	char m_sHash[10];
	char m_sPort[6];
	char m_sServer[30];

	CWMPPlayer4 m_Player;
	CWMPControls3 m_Control;
	CWMPMedia3 m_Media;
	CWMPNetwork m_Network;
	CWMPSettings20 m_Setting;
	CString currentURL;

	CToolTipCtrl m_TimeToolTip;
	CNiceSliderCtrl m_Time;
	CNiceSliderCtrl m_Volume;
	CEdit m_Name;
	CEdit m_SliderTime;
	CButton m_Play;
	CButton m_Stop;
	CButton m_Pause;
	CVMeshSettingDialog vsDlg;
	double m_PositionOffset;
	int m_NewSeekTime;

	void CheckControllerThreadFunction();
	double GetCurrentPosition(void);
	CString GetCurrentPositionString(void);
	void SeekThroughHTTP(double seek_time_in_sec);
	void SetFileName(CString strFileName);
	void SetStretch(void);
	void CleanUp(void);
	virtual BOOL DestroyWindow();
	CString GetPreviousURL(void);
	
	void StartVMeshProcess(ControllerInfo *VCInfo);

	CWinThread* m_DelayEnableThread;
	CWinThread* m_ResetControllerThread;


	virtual BOOL OnInitDialog();
	void PlayStateChangePlayer(long NewState);
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMReleasedcaptureVolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureTime(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMovieSelected( UINT nID );
	afx_msg void OnHScroll(UINT nSBCode,UINT nPos, CScrollBar *pScrollBar);

	int ResizeWindow(int cx, int cy);
	
	void ClickPlayer(short nButton, short nShiftState, long fX, long fY);
	void DoubleClickPlayer(short nButton, short nShiftState, long fX, long fY);
	double SliderCurrentPositionToTime(void);

	bool CMainDialog::TerminateController(HANDLE& handler);

//	afx_msg void OnClose();
	
	TCHAR m_CurrentDirectory[MAX_PATH];
	bool m_IsCreated;
	afx_msg void OnFileOpenvmesh();
	HANDLE m_ControllerHandle;
	afx_msg void OnFileOpenconfigfile();
	void LoadConfigFile();
	void StartVMeshClient(const char* hash, const char* port, const char* server);
	int m_CurrentPosition;
	bool m_IsPlaying;
	CStatic m_VolLabel;
	HANDLE m_hReady;
	HANDLE m_hNoPacket;
	HANDLE m_hStart;
	HANDLE m_hReset;
	HANDLE m_hFailed;
	HANDLE m_hMutex;
	HANDLE m_hReleaseMutex;
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAbout();
	bool m_AlwaysOnTop;
	void SetAlwaysOnTop(void);
	afx_msg void OnViewAlwaysontop();
	void EnableControls(bool);
	bool m_bReconnected;

	DECLARE_EVENTSINK_MAP()
	HANDLE m_hPaused;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	int m_Seeking;
	int m_DataPort;

	afx_msg void OnFileConnecttoserver();
	bool m_FromFile;
	bool m_FirstConnection;
	void ConnectToServer();
	
	int m_Port;
	CString m_FileLocation;
	afx_msg void OnFileOpenserverplaylist();
	int m_PlaylistCount;
	CString m_PlaylistName[MAX_LIST];
	CString m_IPAddress[MAX_LIST];
	int m_ServerPort[MAX_LIST];
	VMeshMovieInfo* VMInfo;
	int* m_ParentList;
	int m_CurrentPlaying;
	void StopPlaying(void);
	static bool CALLBACK EnumWindowsProc(HWND hWnd, long lParam);
	static int appInstanceCount;
	void OnConfiguration();
	bool showConsole;
	int numSeg;

	afx_msg void OnMovieRefreshPlaylist();
	int RefreshPlaylist(bool nDownload);
	void ChooseNewMovie();
	void OpenSelectedMovie(int nIndex);
	void RemoveAllFilesInSubDir(CString subdir);
	void RemoveUPnPPortMappings();
	

	bool m_bSwitchingMovie;

	// For state Logging
	enum {
		NON_STARTED	= 0,
		STARTING	= 1,
		PLAYING		= 2,
		FREEZE		= 3,
		SWITCHING	= 4,
		SEEKING		= 5
	};
	int i_playerState;
	int m_StartupTimerHandler;
	int m_SeekingTimerHandler;
	int m_SwitchMovieTimerHandler;
	int m_FreezingTimerHandler;
	void Jump(void);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	int totalMovieCount;
	//Add by Juan
	ControllerInfo *VCInfo;
	//End by Juan
	unsigned threadID;
};
