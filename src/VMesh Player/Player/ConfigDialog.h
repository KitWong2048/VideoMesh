#pragma once
#include "afxwin.h"

// CConfigDialog dialog

class CConfigDialog : public CDialog
{
	DECLARE_DYNAMIC(CConfigDialog)

public:
	CConfigDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigDialog();

// Dialog Data
	enum { IDD = IDD_CONFIGPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	void setValue(bool _showConsole, int _numSeg);
	void getValue(bool &_showConsole, int &_numSeg);
	CComboBox m_showConsole;
	CComboBox m_superNode;

	bool showConsole;
	int numSeg;
};
