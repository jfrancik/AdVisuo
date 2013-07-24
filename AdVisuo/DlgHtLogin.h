#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"

class CXMLRequest;

class CDlgHtLogin : public CDlgHtFailure
{
	DECLARE_DYNCREATE(CDlgHtLogin)
	enum STATE { INIT, FAILURE, LOGIN, CONNECTION } m_state;

public:
	CString m_strUsername;
	CString m_strPassword;
	CString m_strServers;
	CString m_strUrl;
	bool m_bAutoLogin;		// automatic login for speed-up in the debugging mode only

	CXMLRequest *m_pHttp;

public:
	CDlgHtLogin(CXMLRequest *pHttp = NULL, CString strServers = L"", CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHtLogin();

	virtual void OnGotoFailure(CString title, CString text);

	void GotoLogin();
	void GotoBadLogin();
	void GotoConnection();

	// Button Handlers
	virtual void OnOK();
	virtual void OnCancel();
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonLogin(IHTMLElement *pElement);
	HRESULT OnButtonConnection(IHTMLElement *pElement);
	HRESULT OnButtonProceed(IHTMLElement *pElement);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()
public:
};
