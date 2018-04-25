#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CServerPlaylistDialog dialog
const int MAX_LIST = 100;

class CServerPlaylistDialog : public CDialog
{
	DECLARE_DYNAMIC(CServerPlaylistDialog)
private:
	

public:
	CServerPlaylistDialog(CString* , CString*, int*, int& ,CWnd* pParent = NULL);   
	virtual ~CServerPlaylistDialog();

// Dialog Data
	enum { IDD = IDD_SERVERLISTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	unsigned int m_TotalMovieCount;
	CListCtrl m_ServerListCtrl;
	int m_SelectedItem;
	int& m_PlaylistCount;
	CString m_CurrentDirectory;
	CString* m_PlaylistName;
	CString* m_IPAddress;
	int* m_ServerPort;
	unsigned int m_Count[MAX_LIST];

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAddbutton();
	afx_msg void OnNMClickServerlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedMoveupbutton();
	afx_msg void OnNMRclickServerlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedMovedownbutton();
	afx_msg void OnBnClickedRemovebutton();
	afx_msg void OnBnClickedOk();
	
	int GetPlaylistCount(void);
	bool AddPlaylistItem(CString playlistname, CString ipaddress, int port);
	void ControlButtons(void);
	void SetCurrentDirectory(TCHAR* directory);	
	void GetPlaylistInfo(int nItem, CString& playlistname, CString& ipaddress, int& port);
	unsigned int GetTotalMovieCount(void);
	static bool DownloadFile(CString src, CString dest);
	static int ReadPlaylistCount(CString filepath);
	
	CButton m_RemoveButton;
	CButton m_MoveUpButton;
	CButton m_MoveDownButton;
	afx_msg void OnBnClickedRefreshbutton();
	CButton m_SaveButton;
	afx_msg void OnBnClickedSavebutton();
};
