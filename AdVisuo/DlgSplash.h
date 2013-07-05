#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"

// CDlgSplash dialog

class CDlgSplash : public CDlgHtBase
{
	DECLARE_DYNCREATE(CDlgSplash)

public:
	CDlgSplash(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSplash();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_SPLASH, IDH = IDR_HTML_DLGSPLASH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
