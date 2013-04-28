// Dialogs.h - a part of the AdVisuo Client Software

#pragma once

#include "resource.h"

class CProjectVis;

class CDlgDownload : public CDialog
{
	DECLARE_DYNAMIC(CDlgDownload)
private:
	CString m_servers;
	CString m_url;

public:
	CDlgDownload(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgDownload();

// Dialog Data
	enum { IDD = IDD_ADV_DOWNLOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetServers(CString servers)		{ m_servers = servers; }
	CString GetServers()					{ return m_servers; }
	CString GetURL()						{ return m_url; }

	CString m_server;
	CComboBox m_combo;
	int m_nSort;			// sorting info
	bool m_bAscending[3];

	std::vector<CProjectVis*> m_prjs;	// sims

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRefresh();
	CListCtrl m_list;
	afx_msg void OnSelchangeCombo();
	afx_msg void OnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_details;
	afx_msg void OnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetinfotipList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnDestroy();
};
#pragma once