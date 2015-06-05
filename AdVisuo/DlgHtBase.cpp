// DlgHtBase.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtBase.h"
#include "VisProject.h"		// for _prj_error

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

void CDlgHtBase::WriteDoc(LPCTSTR pBuf)
{
	WriteDoc(SysAllocString(pBuf));
}

void CDlgHtBase::WriteDoc(BSTR bstr)
{
	CComPtr<IHTMLDocument2> pDoc;
	HRESULT h = GetDHtmlDocument(&pDoc);
	if (FAILED(h) || !pDoc) return;

	// Creates a new one-dimensional array
	SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (psaStrings == NULL) return;
	VARIANT *param;
	h = SafeArrayAccessData(psaStrings, (LPVOID*)&param);
	param->vt = VT_BSTR;
	param->bstrVal = bstr;
	h = SafeArrayUnaccessData(psaStrings);

	IDispatch *pDispatch = NULL;
	h = pDoc->open(CComBSTR("text/html"), CComVariant("replace"), CComVariant(), CComVariant(), &pDispatch);
	if (SUCCEEDED(h))
	{
		//pDoc2->Release();
		IHTMLDocument2 *pDoc2 = NULL;
		pDispatch->QueryInterface(&pDoc2);
		pDispatch->Release();
		pDoc2->put_charset(CComBSTR("windows-1250"));
		h = pDoc2->write(psaStrings);
		pDoc2->close();
	}

	SafeArrayDestroy(psaStrings);
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
			return; 
		}
	}
}

void CDlgHtBase::Sleep(ULONG nTime)
{
	MSG msg;
	DWORD t = ::GetTickCount();

	// first, consume all posted messages
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		if (!AfxGetApp()->PumpMessage()) 
		{ 
			::PostQuitMessage(0); 
			return; 
		}

	// now, idle for the time given...	
	while (::GetTickCount() <= t + nTime)
	{
		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		if (!AfxGetApp()->PumpMessage()) 
		{ 
			::PostQuitMessage(0); 
			return; 
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
	CDHtmlDialog::OnOK();
}

void CDlgHtBase::OnCancel()
{
	// enable being the application main window
	if (AfxGetApp()->m_pMainWnd == this)
		AfxGetApp()->m_pMainWnd = NULL;
	CDHtmlDialog::OnCancel();
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
	TRACE(L"OnInitDialog 1\n");
	return CDHtmlDialog::OnInitDialog();
	TRACE(L"OnInitDialog 2\n");
}

void CDlgHtBase::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	TRACE(L"OnDocumentComplete: %ls\n", szUrl);
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
	DWORD t = ::GetTickCount();
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && ::GetTickCount() <= t + 2000) 
	{ 
		if (msg.message == WM_PAINT)
			break;

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

CDlgHtFailure::CDlgHtFailure(CString strTitle, CString strFailure, CString strUrl) : CDlgHtBase(IDD, IDH)
{
	m_fnLoadComplHandle = [this, strTitle, strFailure, strUrl]() { OnGotoFailure(strTitle, strFailure, strUrl); };
}

CDlgHtFailure::CDlgHtFailure(_version_error &ve, CString url) : CDlgHtBase(IDD, IDH)
{ 
	m_fnLoadComplHandle = [this, ve, url]() { _version_error _ve = ve; GotoFailure(_ve, url); };
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
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtFailure, CDlgHtBase)
END_DISPATCH_MAP()

void CDlgHtFailure::OnGotoFailure(CString title, CString text, CString url)
{
	// display info
	std::wstringstream str;
	str << L"showVer(" << VERSION_MAJOR << L", " << VERSION_MINOR << L", " << VERSION_REV << L", \"" << (LPCTSTR)VERSION_DATE << L"\")";
	ExecJS(str.str().c_str());
	str.str(std::wstring());
	str << L"showUrl(\"" << (LPCTSTR)url << L"\")";
	ExecJS(str.str().c_str());

	str.str(L"");
	str << L"GotoFailure(\"" << (LPCTSTR)title << L"\", \"" << (LPCTSTR)text << L"\")";
	ExecJS(str.str().c_str());
	SetWindowText(CString(L"AdVisuo ") + title);
}

void CDlgHtBase::OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	TRACE(L"OnBeforeNavigate: %ls\n", szUrl);
	//CenterWindow();
	//ShowWindow(SW_SHOW);
	//UpdateWindow();

	CDHtmlDialog::OnBeforeNavigate(pDisp, szUrl);
}


void CDlgHtBase::OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	TRACE(L"OnNavigateComplete: %ls\n", szUrl);

	CDHtmlDialog::OnNavigateComplete(pDisp, szUrl);

	// TODO: Add your specialized code here and/or call the base class
}
