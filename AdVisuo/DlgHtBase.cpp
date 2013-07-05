// DlgHtBase.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtBase.h"
#include "VisProject.h"		// for _prj_error
#include "_version.h"

// CDlgHtBase/CDlgHtFailure dialog

IMPLEMENT_DYNCREATE(CDlgHtBase, CDHtmlDialog)

CDlgHtBase::CDlgHtBase(UINT nIDTemplate, UINT nHtmlResID, CWnd* pParent)
	: CDHtmlDialog(nIDTemplate, nHtmlResID, pParent)
{
	m_bShown = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_fnLoadComplHandle = [] { };
}

BEGIN_MESSAGE_MAP(CDlgHtBase, CDHtmlDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtBase)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtBase, CDHtmlDialog)
END_DISPATCH_MAP()

////////////////////////////////////////////////////////////////////////
// Javascript Bounding

void CDlgHtBase::ExecJS(CString function)
{
	CComPtr<IHTMLDocument2> pDoc;
	CComPtr<IHTMLWindow2> pWnd;
	
	HRESULT h = GetDHtmlDocument(&pDoc);
	if (FAILED(h) || !pDoc) return;

	h = pDoc->get_parentWindow(&pWnd);
	if (FAILED(h) || !pWnd) return;

	CString language = L"javascript";
	VARIANT vEmpty = {0};

	h = pWnd->execScript(function.AllocSysString(), language.AllocSysString(), &vEmpty);
}

void CDlgHtBase::DoNonModal(unsigned nTimeout)
{
	Create(m_lpszTemplateName);
	DWORD nTime = ::GetTickCount();

	DWORD nTimeStamp = ::GetTickCount();
	while (!m_bShown && ::GetTickCount() < nTimeStamp + nTimeout) 
	{ 
		MSG msg;
		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		if (!AfxGetApp()->PumpMessage()) 
		{ 
			::PostQuitMessage(0); 
			break; 
		}
	}
}


////////////////////////////////////////////////////////////////////////
// Button Handlers

void CDlgHtBase::OnOK()
{
	// enable being the application main window
	if (AfxGetApp()->m_pMainWnd == this)
		AfxGetApp()->m_pMainWnd = NULL;
	EndDialog(IDOK);
}

void CDlgHtBase::OnCancel()
{
	// enable being the application main window
	if (AfxGetApp()->m_pMainWnd == this)
		AfxGetApp()->m_pMainWnd = NULL;
	EndDialog(IDCANCEL);
}

HRESULT CDlgHtBase::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// Event Handlers & Virtual Functions

BOOL CDlgHtBase::OnInitDialog()
{
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	return CDHtmlDialog::OnInitDialog();
}

void CDlgHtBase::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CWaitCursor wait;

	HRESULT h = S_OK;
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	if (m_bShown)
		return;
	m_bShown = true;

	CenterWindow();
	ShowWindow(SW_SHOW);
	UpdateWindow();

	// wait to see the empty page
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
	{ 
		if (!AfxGetApp()->PumpMessage()) 
		{ 
			::PostQuitMessage(0); 
			break; 
		} 
	}

	if (m_fnLoadComplHandle)
		m_fnLoadComplHandle();
}


void CDlgHtBase::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

HCURSOR CDlgHtBase::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

////////////////////////////////////////////////////////////////////////
// CDlgHtFailure

IMPLEMENT_DYNCREATE(CDlgHtFailure, CDlgHtBase)

CDlgHtFailure::CDlgHtFailure() : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this]() { };
}

CDlgHtFailure::CDlgHtFailure(int nVersionReq, CString strVerDate, CString strDownloadPath) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, nVersionReq, strVerDate, strDownloadPath]() { GotoFailure(nVersionReq, strVerDate, strDownloadPath); };
}

CDlgHtFailure::CDlgHtFailure(_prj_error &pe, CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, pe, url]() { _prj_error _pe = pe; GotoFailure(_pe, url); };
}

CDlgHtFailure::CDlgHtFailure(_com_error &ce, CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, ce, url]() { _com_error _ce = ce; GotoFailure(_ce, url); };
}

CDlgHtFailure::CDlgHtFailure(_xmlreq_error &xe, CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, xe, url]() { _xmlreq_error _xe = xe; GotoFailure(_xe, url); };
}

CDlgHtFailure::CDlgHtFailure(dbtools::_value_error &ve, CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, ve, url]() { dbtools::_value_error _ve = ve; GotoFailure(_ve, url); };
}

CDlgHtFailure::CDlgHtFailure(CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, url]() { GotoFailure(url); };
}

BEGIN_MESSAGE_MAP(CDlgHtFailure, CDlgHtBase)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtFailure)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtFailure, CDlgHtBase)
END_DISPATCH_MAP()

void CDlgHtFailure::OnGotoFailure(CString title, CString text)
{
	std::wstringstream str;
	str << L"GotoFailure(\"" << (LPCTSTR)title << L"\", \"" << (LPCTSTR)text << L"\")";
	ExecJS(str.str().c_str());
	SetWindowText(CString(L"AdVisuo ") + title);
}

void CDlgHtFailure::GotoFailure(int nVersionReq, CString strVerDate, CString strDownloadPath)
{
	std::wstringstream str;
	str << L"Version of AdVisuo you are using is outdated and not compatible<br />with the server software.<br /><br />";
	str << L"Currently used version: " << VERSION_MAJOR << L"." << VERSION_MINOR  << L"." << VERSION_REV << L" (" << __DATE__ << L")<br />";
	str << L"Version required:       " << nVersionReq/10000 << L"." << (nVersionReq%10000) / 100  << L"." << nVersionReq%100 << L" (" << (LPCTSTR)strVerDate << L")<br />";
	str << L"<a href='" << (LPCTSTR)strDownloadPath << "'>Download the latest version</a>";
	OnGotoFailure(L"VERSION MISMATCH", str.str().c_str());
}

void CDlgHtFailure::GotoFailure(_prj_error &pe, CString url)
{
	std::wstringstream str;
	str << L"AdVisuo internal error:<br />" << pe.ErrorMessage() << L";<br />loading from " << (LPCTSTR)url << L".";
	OnGotoFailure(L"INTERNAL ERROR", str.str().c_str());
}

void CDlgHtFailure::GotoFailure(_com_error &ce, CString url)
{
	std::wstringstream str;
	str << "System cannot access server side service at:<br />" << (LPCTSTR)url << ".<br/>&nbsp;<br/>";

	if ((wchar_t*)ce.Description())
		str << (LPCTSTR)ce.Description();
	else
		str << ce.ErrorMessage();

	CString s = str.str().c_str();
	s.TrimRight();
	OnGotoFailure(L"CONNECTION ERROR", s);
}

void CDlgHtFailure::GotoFailure(_xmlreq_error &xe, CString url)
{
	std::wstringstream str;
	str << L"HTTP error " << xe.status() << L": " << xe.msg() << L"<br />at " << (LPCTSTR)url << L".";
	OnGotoFailure(L"HTTP ERROR", str.str().c_str());
}

void CDlgHtFailure::GotoFailure(dbtools::_value_error &ve, CString url)
{
	std::wstringstream str;
	str << L"AdVisuo internal error:<br />" << ve.ErrorMessage() << L";<br />loading from " << (LPCTSTR)url << L".";
	OnGotoFailure(L"INTERNAL ERROR", str.str().c_str());
}

void CDlgHtFailure::GotoFailure(CString url)
{
	std::wstringstream str;
	str << L"Unidentified error while downloading from " << url;
	OnGotoFailure(L"CONNECTION ERROR", str.str().c_str());
}

