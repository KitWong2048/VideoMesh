// ConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ConfigDialog.h"

// CConfigDialog dialog

IMPLEMENT_DYNAMIC(CConfigDialog, CDialog)

CConfigDialog::CConfigDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDialog::IDD, pParent)
{
	showConsole = true;
	numSeg = 0;
}

CConfigDialog::~CConfigDialog()
{
}

BOOL CConfigDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	this->m_showConsole.ResetContent();
	this->m_showConsole.AddString("Yes");
	this->m_showConsole.AddString("No");
	this->m_showConsole.SetCurSel(showConsole?0:1);

	this->m_superNode.ResetContent();
	
	this->m_superNode.AddString("Default (Normal Peer)");
	for (int i = 1; i < 21; ++i) {
		CString t;
		t.Format("%d", i);
		this->m_superNode.AddString(t);
	}
	this->m_superNode.AddString("999");
	if (numSeg==999)
		this->m_superNode.SetCurSel(this->m_superNode.GetCount()-1);
	else
		this->m_superNode.SetCurSel(numSeg);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SHOWCONSOLE, m_showConsole);
	DDX_Control(pDX, IDC_COMBO_SUPERNODE, m_superNode);
}

BEGIN_MESSAGE_MAP(CConfigDialog, CDialog)
	ON_BN_CLICKED(ID_OK, &CConfigDialog::OnBnClickedOk)
	ON_BN_CLICKED(ID_CANCEL, &CConfigDialog::OnBnClickedCancel)
END_MESSAGE_MAP()

void CConfigDialog::setValue(bool _showConsole, int _numSeg)
{
	showConsole = _showConsole;

	if (((_numSeg >= 0) && (_numSeg <= 20)) || (_numSeg == 999)) {
		numSeg = _numSeg;
	}
}

void CConfigDialog::getValue(bool &_showConsole, int &_numSeg)
{
	_showConsole = showConsole;
	_numSeg = numSeg;
}

void CConfigDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	showConsole = (this->m_showConsole.GetCurSel()==0);
	CString t;
	this->m_superNode.GetLBText(this->m_superNode.GetCurSel(), t);
	numSeg = atoi(t);

	this->OnOK();
}

void CConfigDialog::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	this->OnCancel();
}
