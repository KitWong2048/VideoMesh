#pragma once
#include "afxwin.h"
#include "stdafx.h"
#include "VMesh Player.h"

// CURLDialog dialog

class CURLDialog : public CDialog
{
	DECLARE_DYNAMIC(CURLDialog)

public:
	CURLDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CURLDialog();

// Dialog Data
	enum { IDD = IDD_URLDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_URL;
	CString url;
	CButton m_OK;
	afx_msg void OnEnChangeUrl();
	CString GetURL(void);
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
