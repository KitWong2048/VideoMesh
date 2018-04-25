// VMesh ServerDlg.h : 標頭檔 
//

#pragma once
#include "afxcmn.h"
#include "PlaylistDialog.h"
#include "..\VMesh Player\Common\Common.h"
#include <queue>
using namespace std;

#define PORT_BASE 9000

#define STATE_NORMAL	0
#define STATE_INIT		1
#define STATE_REMOVING	2
#define STATE_NOTUSED	3

// CVMeshServerDlg 對話方塊
class CVMeshServerDlg : public CDialog
{
// 建構
public:
	CVMeshServerDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_VMESHSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenfileAddmovie();
	afx_msg void OnAbout();
	afx_msg void OnExit();	
	afx_msg void OnNMRclickMovielist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPopupAddmovie();
	
	void AddNewMovie(void);

	static CString m_MovieName[MAX_MOVIE_NUM];
	static CString m_MoviePath[MAX_MOVIE_NUM];
	static CString m_MovieHash[MAX_MOVIE_NUM];
	static CString m_MovieDesc[MAX_MOVIE_NUM];
	static int m_MoviePort[MAX_MOVIE_NUM];
	static int m_State[MAX_MOVIE_NUM];

	static HANDLE m_Stop[MAX_MOVIE_NUM];
	static CWinThread* m_VMeshServerThread[MAX_MOVIE_NUM];

	static int m_Port;
	static int m_Index;	
	static int m_MovieCount;
	static int m_SelectedMovie;
	static int m_TopNumCount;
	static int m_PreparingMovieCount;
	static int m_Count;
	static int m_HttpPort;

	static TCHAR m_CurrentDirectory[MAX_PATH+1];

	static CListCtrl m_MovieList;
	static CStatusBarCtrl m_Status;
	
	CWinThread* m_HttpServerThread;
	static HANDLE m_CentralLookupServerProcess;
	CWinThread* m_LookupServerControlThread;

	static UINT StartVMeshServer(PVOID lpParam);
	afx_msg void OnNMClickMovielist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOpenfileRemovemovie();
	afx_msg void OnPopupRemovemovie();
	bool RemoveMovie(int);

	bool CheckIsNameUnique(CString name);
	bool CheckIsPortUnique(int port);
	afx_msg void OnLvnEndlabeleditMovielist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPopupCopytoclipboard();
	afx_msg void OnPopupSaveasconffile();
	CString GetConfigFileContent(void);
	afx_msg void OnFileSaveasmovielist();
	void SaveAsMovieList();
	afx_msg void OnPopupSaveasmovielist();
	static UINT StartHttpServer(PVOID lpParam);
	static UINT LookupServerManageThreadFunc(PVOID lpParam);

	void RemoveSelectedMovies(void);
	static int FindItemPosition(CString port);
	static queue<int> m_UsableIndexQueue;
	static bool m_Exiting;
	
	static void UpdateDefaultPlaylist(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	int m_ColumnWidth;

protected:
	virtual void OnCancel();
};
