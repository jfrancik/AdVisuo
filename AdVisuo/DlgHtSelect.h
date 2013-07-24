#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"
class CProjectVis;

class CDlgHtSelect : public CDlgHtBase
{
	DECLARE_DYNCREATE(CDlgHtSelect)

public:
	std::vector<CProjectVis*> *m_pPrjs;		// projects
	ULONG m_nProjectId;						// selection

public:
	CDlgHtSelect(std::vector<CProjectVis*> *pPrjs = NULL, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHtSelect();

	ULONG GetProjectId()			{ return m_nProjectId; }

	// Button Handlers
	virtual void OnOK();
	virtual void OnCancel();
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonOpen(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_SELECT, IDH = IDR_HTML_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()
public:
};
