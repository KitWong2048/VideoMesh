#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CServerDialog dialog

class CServerDialog : public CDialog
{
	DECLARE_DYNAMIC(CServerDialog)

public:
	CServerDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerDialog();

// Dialog Data
	enum { IDD = IDD_SEVERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_Port;
	unsigned char m_IPAddress[4];
	CString m_FileLocation;
	bool m_FromFile;
	virtual BOOL OnInitDialog();
	CButton m_Browse;
	CButton m_FileSelection;
	CButton m_ServerSelection;
	CEdit m_FileLocationCtrl;
	CIPAddressCtrl m_IPAddressCtrl;
	CEdit m_PortCtrl;
	void SetAllControls(void);
	void SetOKButton(void);
	CButton m_OKButton;
	afx_msg void OnEnChangeFilelocation();
	afx_msg void OnIpnFieldchangedIpaddress(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangePort();
	afx_msg void OnEnChangeIPAddress();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedFileselection();
	afx_msg void OnBnClickedServerselection();
	afx_msg void OnBnClickedOk();
	bool CheckServerInformationFile(const char * filename);
	void SetServerInfo(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, int port, CString playlist = "Default");
	void SetFileLocation(const char* filelocation);
	void GetServerInfo(unsigned char& ip1, unsigned char& ip2, unsigned char& ip3, unsigned char& ip4, int& port, CString& playlist);
	void GetFileLocation(CString& filelocation);
	void GetServerInformationFromFile(const char* filename);
	bool GetSource();
	void SetDialogTitle(CString title);
	CString m_Title;
	CEdit m_PlaylistNameCtrl;
	CString m_PlaylistName;
	CButton m_SaveButton;
	afx_msg void OnBnClickedSavebutton();
};
