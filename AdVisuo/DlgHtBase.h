#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "FailureStr.h"
#include <functional>

class _prj_error;
class _xmlreq_error;
namespace dbtools { class _value_error; }

class CDlgHtBase: public CDHtmlDialog
{
	DECLARE_DYNCREATE(CDlgHtBase)
public:
	HICON m_hIcon;
	bool m_bShown;
	std::function<void ()> m_fnLoadComplHandle;

public:
	CDlgHtBase(UINT nIDTemplate = 0, UINT nHtmlResID = 0, CWnd* pParent = NULL);			// standard constructor
	virtual ~CDlgHtBase()		{ }

	// Javascript Bounding
	void ExecJS(CString function);

	// 
	void WriteDoc(LPCTSTR pBuf);
	void WriteDoc(BSTR bstr);

	// Non-modal display
	void DoNonModal(unsigned nTimeout = 10000);

	// Sleep
	void Sleep(ULONG nTime);
	
	// Button Handlers
	virtual void OnOK();
	virtual void OnCancel();
	HRESULT OnButtonCancel(IHTMLElement *pElement);

protected:
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
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual void OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
};

class CDlgHtFailure: public CDlgHtBase
{
	DECLARE_DYNCREATE(CDlgHtFailure)

public:
	CDlgHtFailure();
	CDlgHtFailure(CString strTitle, CString strFailure, CString url);
	CDlgHtFailure(_version_error &ve, CString url);
	CDlgHtFailure(_prj_error &pe, CString url);
	CDlgHtFailure(_com_error &ce, CString url);
	CDlgHtFailure(_xmlreq_error &xe, CString url);
	CDlgHtFailure(dbtools::_value_error &ve, CString url);
	CDlgHtFailure(CString url);
	~CDlgHtFailure()	{ }

	// Built-in Failure Messages
	virtual void OnGotoFailure(CString title, CString text, CString url);	// generic & virtual...
	
	void GotoFailure(_version_error &ve, CString url)			{ OnGotoFailure(GetFailureTitle(ve), GetFailureString(ve, url), url); }
	void GotoFailure(_prj_error &pe, CString url)				{ OnGotoFailure(GetFailureTitle(pe), GetFailureString(pe, url), url); }
	void GotoFailure(_com_error &ce, CString url)				{ OnGotoFailure(GetFailureTitle(ce), GetFailureString(ce, url), url); }
	void GotoFailure(_xmlreq_error &xe, CString url)			{ OnGotoFailure(GetFailureTitle(xe), GetFailureString(xe, url), url); }
	void GotoFailure(dbtools::_value_error &ve, CString url)	{ OnGotoFailure(GetFailureTitle(ve), GetFailureString(ve, url), url); }
	void GotoFailure(CString url)								{ OnGotoFailure(GetFailureTitle(  ), GetFailureString(url), url); }

// Dialog Data
	enum { IDD = IDD_ADV_LOGIN, IDH = IDR_HTML_LOGIN };

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()
};
