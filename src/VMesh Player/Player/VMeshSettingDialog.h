#pragma once
#include "afxwin.h"


// CVMeshSettingDialog dialog

class CVMeshSettingDialog : public CDialog
{
	DECLARE_DYNAMIC(CVMeshSettingDialog)

public:
	CVMeshSettingDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVMeshSettingDialog();

// Dialog Data
	enum { IDD = IDD_VMESHSETTINGDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_Server;
	CEdit m_Port;
	CEdit m_Movie;
	bool CheckEmptyInput(void);
	afx_msg void OnEnChangeServer();
	CButton m_OK;
	afx_msg void OnEnChangePort();
	afx_msg void OnEnChangeMovie();
	afx_msg void OnBnClickedOk();
	char* m_StrMovie;
	char* m_StrServer;
	char* m_StrPort;
	const char* GetServerAddress(void);
	const char* GetMovieHash(void);
	const char* GetPortNumber(void);
};
