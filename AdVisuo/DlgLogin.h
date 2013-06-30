#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "XMLRequest.h"

class CDlgLogin : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CDlgLogin)


public:
	CString m_strUsername;
	CString m_strPassword;
	CString m_strServers;
	CString m_strUrl;

	CXMLRequest m_http;

	enum STATE { INIT, FAILURE, LOGIN, CONNECTION } m_state;

public:
	CDlgLogin(CString strServers = L"", CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLogin();

	// Javascript Bounding
	void exec(CString function);
	
	void GotoFailure(CString title, CString text);
	void GotoFailure(int nVersionReq, CString strVerDate, CString strDownloadPath);
	void GotoFailure(_com_error &ce, CString url);
	void GotoFailure(_xmlreq_error &xe, CString url);
	void GotoFailure(CString url);
	void GotoFailure(CString title, CString line1, CString line2);
	
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

// Dialog Data
	enum { IDD = IDD_ADV_LOGIN, IDH = IDR_HTML_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
	{
		if (lpMsg && lpMsg-> message == WM_KEYDOWN) 
		{ 
			bool bCtrl = (0x80 == (0x80 & GetKeyState (VK_CONTROL)));
			if (bCtrl) return S_OK; 
			if (lpMsg-> wParam == VK_F5) return S_OK; 
		} 
		return CDHtmlDialog :: TranslateAccelerator (lpMsg, pguidCmdGroup, nCmdID); 
	}

	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
	{
		return S_OK;
	}

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()
public:
};
