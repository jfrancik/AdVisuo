#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"

// CDlgHtAbout dialog

class CDlgHtAbout : public CDlgHtBase
{
	DECLARE_DYNCREATE(CDlgHtAbout)

public:
	CDlgHtAbout(UINT nIDTemplate = IDD, UINT nHtmlResID = IDH, CWnd* pParent = NULL) : CDlgHtBase(nIDTemplate, nHtmlResID, pParent)		{}
	virtual ~CDlgHtAbout()	{}

// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_ABOUT, IDH = IDR_HTML_DLGSPLASH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};

// CDlgHtSplash dialog

class CDlgHtSplash : public CDlgHtAbout
{
	DECLARE_DYNCREATE(CDlgHtSplash)

public:
	CDlgHtSplash(UINT nIDTemplate = IDD, UINT nHtmlResID = IDH, CWnd* pParent = NULL) : CDlgHtAbout(nIDTemplate, nHtmlResID, pParent)		{}
	virtual ~CDlgHtSplash()	{}

// Dialog Data
	enum { IDD = IDD_ADV_SPLASH, IDH = IDR_HTML_DLGSPLASH };

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
