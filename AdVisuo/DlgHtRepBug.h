// DlgHtRepBug.h - a part of the AdVisuo Client Software

#pragma once

#include "resource.h"
#include "DlgHtBase.h"

class CDlgHtRepBug : public CDlgHtBase
{
	DECLARE_DYNAMIC(CDlgHtRepBug)

public:
	CDlgHtRepBug(CString msg = L"", CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHtRepBug();

// Dialog Data
	enum { IDD = IDD_ADV_REPORT_BUG, IDH = IDR_HTML_REP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	AVULONG m_nId;
	CString m_errormsg;
	CString m_diagnostic;
	CString m_path;

	CComboBox m_combo;
	CString m_desc;
	CString m_sys;


	virtual BOOL OnInitDialog();
	HRESULT OnButtonSend(IHTMLElement* /*pElement*/);
	HRESULT OnButtonCancel(IHTMLElement* /*pElement*/);
	virtual void OnOK();
	int m_cat;
};
#pragma once
