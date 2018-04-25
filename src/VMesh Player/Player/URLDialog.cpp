// URLDialog.cpp : implementation file
//
#include "stdafx.h"
#include "URLDialog.h"


// CURLDialog dialog

IMPLEMENT_DYNAMIC(CURLDialog, CDialog)
CURLDialog::CURLDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CURLDialog::IDD, pParent)
	, url(_T(""))
{
}

CURLDialog::~CURLDialog()
{
}

void CURLDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_URL);
	DDX_Control(pDX, IDOK, m_OK);
}


BEGIN_MESSAGE_MAP(CURLDialog, CDialog)
	ON_EN_CHANGE(IDC_URL, OnEnChangeUrl)
ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CURLDialog message handlers



void CURLDialog::OnEnChangeUrl()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString url;
	m_URL.GetWindowText(url);
	if (url.IsEmpty())
		m_OK.EnableWindow(FALSE);
	else m_OK.EnableWindow(TRUE);
}

CString CURLDialog::GetURL(void)
{
	return url;
}

void CURLDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_URL.GetWindowText(url);

	OnOK();
}


BOOL CURLDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//::AfxMessageBox(url);
	m_URL.SetWindowText(url);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
