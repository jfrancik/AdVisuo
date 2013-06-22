#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CDlgLogin dialog

class CDlgLogin : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CDlgLogin)

public:
	CString m_strUsername;
	CString m_strPassword;

public:
	CDlgLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLogin();
// Overrides
	HRESULT OnButtonLogin(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonConnection(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_LOGIN, IDH = IDR_HTML_DLGLOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()

public:
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
};
