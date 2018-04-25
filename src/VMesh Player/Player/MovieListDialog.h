#pragma once
#include "afxwin.h"
#include "CExplorer1.h"

// CMovieListDialog dialog

class CMovieListDialog : public CDialog
{
	DECLARE_DYNAMIC(CMovieListDialog)

public:
	CMovieListDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMovieListDialog();

// Dialog Data
	enum { IDD = IDD_MOVIELISTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeMovielist();
	virtual BOOL OnInitDialog();
	CButton m_OKButton;
	CListBox m_MovieListCtrl;
	CStatic m_MovieText;
	CExplorer1 m_Browser;
	int GetMovieSelection(void);
	int m_MovieSelection;
	afx_msg void OnBnClickedOk();
	void SetMovieCount(int count);
	int m_MovieCount;
	CString* m_MovieList;
	CString* m_MovieDesc;
	int* m_Data;
	bool AddItem(CString name, CString desc, int data);
	int m_MaxMovieCount;
	CFont* m_Font;
	afx_msg void OnLbnDblclkMovielist();
	afx_msg void OnLbnSelcancelMovielist();
};
