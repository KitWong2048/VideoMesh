// VMesh PlayerDlg.h : header file
//

#pragma once

#include "VMesh Player.h"

// CVMeshPlayerDlg dialog
class CVMeshPlayerDlg : public CDialog
{
// Construction
public:
	CVMeshPlayerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VMESHPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
