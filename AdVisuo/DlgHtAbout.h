#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"

// CDlgHtOutText dialog	- generic dialog with IOutTextSink abstract implementation

class CDlgHtOutText : public CDlgHtBase, public IOutTextSink
{
public:
	CDlgHtOutText(UINT nIDTemplate, UINT nHtmlResID, CWnd* pParent = NULL) : CDlgHtBase(nIDTemplate, nHtmlResID, pParent)   { }
	virtual ~CDlgHtOutText()	{}

// Overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};

// CDlgHtAbout dialog

class CDlgHtAbout : public CDlgHtOutText
{
	DECLARE_DYNCREATE(CDlgHtAbout)

public:
	CDlgHtAbout(UINT nIDTemplate = IDD, UINT nHtmlResID = IDH, CWnd* pParent = NULL);
	virtual ~CDlgHtAbout()	{}

// OutTextSink
	virtual void OutText(LPCTSTR lpszItem);

	void ShowVersion(AVULONG nMajor, AVULONG nMinor, AVULONG nRel, CString date);

// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_ABOUT, IDH = IDR_HTML_ABOUT };

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};

// CDlgHtSplash dialog

class CDlgHtSplash : public CDlgHtOutText
{
	DECLARE_DYNCREATE(CDlgHtSplash)

public:
	CDlgHtSplash(UINT nIDTemplate = IDD, UINT nHtmlResID = IDH, CWnd* pParent = NULL) : CDlgHtOutText(nIDTemplate, nHtmlResID, pParent)		{}
	virtual ~CDlgHtSplash()	{}

// OutTextSink
	virtual void OutText(LPCTSTR lpszItem);

// Dialog Data
	enum { IDD = IDD_ADV_SPLASH, IDH = IDR_HTML_SPLASH };

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
