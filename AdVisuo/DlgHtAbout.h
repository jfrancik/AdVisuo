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
	virtual bool OutWaitMessage(AVLONG nWaitStage, AVULONG &nMsecs)	{ return true; }

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

	bool m_bQuitRequested;		// flag: user clicked the Cross icon
	bool m_bProgramStarting;	// flag: program is starting, splash displayed for the first time

public:
	CDlgHtSplash(UINT nIDTemplate = IDD, UINT nHtmlResID = IDH, CWnd* pParent = NULL) : CDlgHtOutText(nIDTemplate, nHtmlResID, pParent)		{ m_bQuitRequested = false; }
	virtual ~CDlgHtSplash()	{}

	// OutTextSink
	virtual void OutText(LPCTSTR lpszItem);
	virtual bool OutWaitMessage(AVLONG nWaitStage, AVULONG &nMsecs);

	// Decoration: full frame with Minimize box
	bool IsDecorated();
	void Decorate();
	void Undecorate();

// Dialog Data
	enum { IDD = IDD_ADV_SPLASH, IDH = IDR_HTML_SPLASH };

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
