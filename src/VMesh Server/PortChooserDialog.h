#pragma once
#include "afxwin.h"


// CPortChooserDialog dialog

class CPortChooserDialog : public CDialog
{
	DECLARE_DYNAMIC(CPortChooserDialog)

public:
	CPortChooserDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPortChooserDialog();

// Dialog Data
	enum { IDD = IDD_PORTCHOOSERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_PortCtrl;
	int CheckAvailablePort(int nStart);
	bool CheckIsPortAvailable(int nPort);
	afx_msg void OnBnClickedFindfreeportbutton();
	int GetPort(void);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	int m_Port;
	virtual BOOL OnInitDialog();
	CButton m_OKButton;
	afx_msg void OnEnChangePortctrl();
};
