#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CPlaylistDialog dialog

class CPlaylistDialog : public CDialog
{
	DECLARE_DYNAMIC(CPlaylistDialog)

public:
	CPlaylistDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlaylistDialog();

// Dialog Data
	enum { IDD = IDD_PLAYLISTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_MovieListCtrl;
	CEdit m_ListNameCtrl;
	afx_msg void OnBnClickedSelectall();
	afx_msg void OnBnClickedSelectnone();
	afx_msg void OnEnChangeListname();
	CButton m_SaveButton;
	afx_msg void OnBnClickedOk();
	CString m_MovieHash[MAX_MOVIE_NUM];
	void AddItem(CString name, CString port, CString desc, CString hash, bool check = false);
	int m_MovieCount;
	int m_CheckedRow;
	CString m_MovieName[MAX_MOVIE_NUM];
	CString m_MoviePort[MAX_MOVIE_NUM];
	CString m_MovieDesc[MAX_MOVIE_NUM];
	CString m_CurrentDirectory;
	bool m_bIsChecked[MAX_MOVIE_NUM];
};
