// DlgRepBug.h - a part of the AdVisuo Client Software

#pragma once

#include "resource.h"

class CDlgReportBug : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgReportBug)

public:
	CDlgReportBug(CString msg = L"", CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgReportBug();

	static void Report(int nReason, CString message = L"");

// Dialog Data
	enum { IDD = IDD_ADV_REPORT_BUG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_msg;
	CString m_name;
	CString m_email;
	int m_cat;
	CString m_desc;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CComboBox m_combo;
	CString m_system;
};
#pragma once
