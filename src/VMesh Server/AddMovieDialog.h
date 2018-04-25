#pragma once
#include "afxwin.h"


// CAddMovieDialog dialog
#define PORT_BASE 9000
class CAddMovieDialog : public CDialog
{
	DECLARE_DYNAMIC(CAddMovieDialog)

public:
	CAddMovieDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddMovieDialog();

// Dialog Data
	enum { IDD = IDD_VMESHSERVER_ADDNEWMOVIEDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	DECLARE_MESSAGE_MAP()
public:
	static int m_InternalCount;
	static int m_AvailablePort;

	CEdit m_Name;
	CEdit m_Port;
	CEdit m_Path;
	CEdit m_Desc;
	CEdit m_URL;
	CString m_StrName;
	CString m_StrPath;
	CString m_StrPort;
	CString m_StrDesc;
	CString m_StrURL;
	CButton m_OK;

	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedBrowse();
	afx_msg void OnEnChangePort();
	afx_msg void OnEnChangeName();
	afx_msg void OnEnChangePath();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	bool CheckEmptyControl(void);
	bool CheckFileExistence(CString path);

	CString GetMovieName(void);
	CString GetMoviePath(void);
	CString GetPortNumber(void);
	CString GetDesc(void);

	int CheckAvailablePort(int nStart);
	static bool CheckIsPortAvailable(int port);
	
};
