// MovieListDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VMesh Player.h"
#include "MovieListDialog.h"
#include ".\movielistdialog.h"

// CMovieListDialog dialog

IMPLEMENT_DYNAMIC(CMovieListDialog, CDialog)
CMovieListDialog::CMovieListDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieListDialog::IDD, pParent)
	, m_MovieSelection(-1)
	, m_MovieCount(0)
	, m_MovieList(NULL)
	, m_Data(NULL)
	, m_MovieDesc(NULL)
	, m_MaxMovieCount(0)
	, m_Font(NULL)
{
}

CMovieListDialog::~CMovieListDialog()
{
	if ( m_MovieList )
		delete[] m_MovieList;
	if ( m_Data )
		delete[] m_Data;
	if ( m_MovieDesc )
		delete[] m_MovieDesc;
	if ( m_Font )
		delete m_Font;
}

void CMovieListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Control(pDX, IDC_MOVIELIST, m_MovieListCtrl);
	DDX_Control(pDX, IDC_MOVIETEXT, m_MovieText);
	DDX_Control(pDX, IDC_IE, m_Browser);
}


BEGIN_MESSAGE_MAP(CMovieListDialog, CDialog)
	ON_LBN_SELCHANGE(IDC_MOVIELIST, OnLbnSelchangeMovielist)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_LBN_DBLCLK(IDC_MOVIELIST, OnLbnDblclkMovielist)
	ON_LBN_SELCANCEL(IDC_MOVIELIST, OnLbnSelcancelMovielist)
END_MESSAGE_MAP()


// CMovieListDialog message handlers

void CMovieListDialog::OnLbnSelchangeMovielist()
{
	if ( m_MovieListCtrl.GetCurSel() != LB_ERR && m_MovieListCtrl.GetItemData(m_MovieListCtrl.GetCurSel()) != -1 )
	{
		CString showText, showURL;
		int startPos = m_MovieDesc[m_MovieListCtrl.GetCurSel()].Find("<link>");
		int endPos = m_MovieDesc[m_MovieListCtrl.GetCurSel()].Find("<\\link>");
		showURL = m_MovieDesc[m_MovieListCtrl.GetCurSel()].Mid(startPos+6, endPos - startPos - 6);
		showText = m_MovieDesc[m_MovieListCtrl.GetCurSel()].Mid(endPos + 7);

		//m_Browser.EnableScrollBar(false);
		//m_Browser.Refresh();
		//::AfxMessageBox("Before Navigate()");
		CComVariant v;  // empty variant
		m_Browser.Navigate(showURL, &v, &v, &v, &v);
		//::AfxMessageBox("After Navigate()");
		m_MovieText.SetWindowTextA(showText);
		m_OKButton.EnableWindow(true);
	}
	else
	{
		CComVariant v;  // empty variant
		m_Browser.Navigate("about:blank", &v, &v, &v, &v);

		m_MovieText.SetWindowTextA("");
		m_OKButton.EnableWindow(false);
	}
}

BOOL CMovieListDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_Font = new CFont;
	m_Font->CreateFont(14,0,0,0,700,0,0,0,0,0,0,0,DEFAULT_PITCH|FF_DONTCARE,"Arial");
	m_MovieListCtrl.SetFont(m_Font);
	int i;
	for ( i = 0; i < m_MovieCount; i++ )
	{
		m_MovieListCtrl.AddString(m_MovieList[i]);
		m_MovieListCtrl.SetItemData(i, m_Data[i]);
	}
	CFont *m_f = new CFont;
	m_f->CreateFont(15,0,0,0,700,0,0,0,0,0,0,0,DEFAULT_PITCH|FF_DONTCARE,"Arial");
	m_MovieText.SetFont(m_f);
	m_OKButton.EnableWindow(false);
m_Browser.EnableScrollBar(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CMovieListDialog::GetMovieSelection(void)
{
	return m_MovieSelection;
}

void CMovieListDialog::OnBnClickedOk()
{
	if ( m_MovieListCtrl.GetCurSel() == LB_ERR || m_Data[m_MovieListCtrl.GetCurSel()] == -1 )
		return;
	m_MovieSelection = m_MovieListCtrl.GetItemData(m_MovieListCtrl.GetCurSel());
	OnOK();
}

void CMovieListDialog::SetMovieCount(int count)
{
	m_MaxMovieCount = count;
	if ( m_MovieList )
		delete[] m_MovieList;
	if ( m_Data )
		delete[] m_Data;
	if ( m_MovieDesc )
		delete[] m_MovieDesc;
	m_MovieList = new CString[count];
	m_Data = new int[count];
	m_MovieDesc = new CString[count];
}

bool CMovieListDialog::AddItem(CString name, CString desc, int data)
{
	if ( m_MovieCount == m_MaxMovieCount )
		return false;
	if ( data == -1 )
		m_MovieList[m_MovieCount] = "#####  " + name + "  #####";
	else
		m_MovieList[m_MovieCount] = "    " + name;
	m_Data[m_MovieCount] = data;
	m_MovieDesc[m_MovieCount] = desc;
	m_MovieCount++;
	return true;
}

void CMovieListDialog::OnLbnDblclkMovielist()
{
	OnBnClickedOk();
}

void CMovieListDialog::OnLbnSelcancelMovielist()
{
	m_OKButton.EnableWindow(false);
}
