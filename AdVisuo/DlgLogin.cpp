// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgLogin.h"
#include "_version.h"

// CDlgLogin dialog

IMPLEMENT_DYNCREATE(CDlgLogin, CDHtmlDialog)

CDlgLogin::CDlgLogin(CString strServers, CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDlgLogin::IDD, CDlgLogin::IDH, pParent)
{
	m_state = INIT;
	m_strServers = strServers;
	int curPos = 0;
	m_strUrl.Format(L"http://%s/advsrv.asmx", m_strServers.Tokenize(_T(";"), curPos));
}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_ElementText(pDX, _T("idUserName"), DISPID_A_VALUE, m_strUsername); 
	DDX_DHtml_ElementText(pDX, _T("idPassword"), DISPID_A_VALUE, m_strPassword); 
}

BEGIN_MESSAGE_MAP(CDlgLogin, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgLogin)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("idLoginButton"), OnButtonLogin)
	DHTML_EVENT_ONCLICK(_T("idConnButton"), OnButtonConnection)
	DHTML_EVENT_ONCLICK(_T("idAfterFailConn"), OnButtonConnection)
	DHTML_EVENT_ONCLICK(_T("idProceedButton"), OnButtonProceed)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgLogin, CDHtmlDialog)
END_DISPATCH_MAP()

////////////////////////////////////////////////////////////////////////
// Javascript Bounding

void CDlgLogin::exec(CString function)
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

void CDlgLogin::GotoFailure(CString title, CString text)
{
	m_state = FAILURE;
	std::wstringstream str;
	str << L"GotoFailure(\"" << (LPCTSTR)title << "\", \"" << (LPCTSTR)text << "\")";
	exec(str.str().c_str());
}

void CDlgLogin::GotoFailure(int nVersionReq, CString strVerDate, CString strDownloadPath)
{
	std::wstringstream str;
	str << L"Version of AdVisuo you are using is outdated and not compatible<br />with the server software.<br /><br />";
	str << L"Currently used version: " << VERSION_MAJOR << L"." << VERSION_MINOR  << L"." << VERSION_REV << L" (" << __DATE__ << L")<br />";
	str << L"Version required:       " << nVersionReq/10000 << L"." << (nVersionReq%10000) / 100  << L"." << nVersionReq%100 << L" (" << (LPCTSTR)strVerDate << L")<br />";
	str << L"<a href='" << (LPCTSTR)strDownloadPath << "'>Download the latest version</a>";
	GotoFailure(L"VERSION MISMATCH", str.str().c_str());
}

void CDlgLogin::GotoFailure(_com_error &ce, CString url)
{
	std::wstringstream str;
	str << "System cannot access server side service at:<br />" << (LPCTSTR)url << ".<br/>&nbsp;<br/>";

	if ((wchar_t*)ce.Description())
		str << (LPCTSTR)ce.Description();
	else
		str << ce.ErrorMessage();

	CString s = str.str().c_str();
	s.TrimRight();
	GotoFailure(L"CONNECTION ERROR", s);
}

void CDlgLogin::GotoFailure(_xmlreq_error &xe, CString url)
{
	std::wstringstream str;
	str << L"HTTP error " << xe.status() << L": " << xe.msg() << L"<br />at " << (LPCTSTR)url << L".";
	GotoFailure(L"HTTP ERROR", str.str().c_str());
}

void CDlgLogin::GotoFailure(CString url)
{
	std::wstringstream str;
	str << L"Unidentified error while downloading from " << url;
	GotoFailure(L"CONNECTION ERROR", str.str().c_str());
}

void CDlgLogin::GotoLogin()
{
	CWaitCursor wait;

	// test the connection
	try
	{
		m_http.setURL((LPCTSTR)m_strUrl);
		std::wstring appname = m_http.AVGetAppName();
		int verreq = m_http.AVGetRequiredVersion();
		std::wstring date = m_http.AVGetRequiredVersionDate();
		std::wstring path = m_http.AVGetLatestVersionDownloadPath();

		if (VERSION < verreq)
			GotoFailure(verreq, date.c_str(), path.c_str());
		else
		{
			// the only successful way to the login!
			m_state = LOGIN;
			exec(L"GotoLogin()");
		}
	}
	catch (_com_error ce)
	{
		GotoFailure(ce, m_http.URL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		GotoFailure(xe, m_http.URL().c_str());
	}
	catch(...)
	{
		GotoFailure(m_http.URL().c_str());
	}
}

void CDlgLogin::GotoBadLogin()
{
	m_strUsername = m_strPassword = L"";
	UpdateData(FALSE);
	m_state = LOGIN;
	exec(L"GotoBadLogin()");
}

void CDlgLogin::GotoConnection()
{
	m_state = CONNECTION;
	std::wstringstream str;
	str << L"GotoConnection(\"" << (LPCTSTR)m_strServers << "\")";
	exec(str.str().c_str());
}

////////////////////////////////////////////////////////////////////////
// Button Handlers

void CDlgLogin::OnOK()
{
	CWaitCursor wait;
	switch (m_state)
	{
	case INIT:
	case FAILURE:
		EndDialog(IDCANCEL);
		break;
	case LOGIN:
		UpdateData();
		try
		{
			m_http.setURL((LPCTSTR)m_strUrl);
			if (m_http.AVLogin((LPCTSTR)m_strUsername, (LPCTSTR)m_strPassword))
				EndDialog(IDOK);
			else
				GotoBadLogin();
		}
		catch (_com_error ce)
		{
			GotoFailure(ce, m_http.URL().c_str());
		}
		catch (_xmlreq_error xe)
		{
			GotoFailure(xe, m_http.URL().c_str());
		}
		catch(...)
		{
			GotoFailure(m_http.URL().c_str());
		}
		break;
	case CONNECTION:
		OnButtonProceed(NULL);
		break;
	}
}

void CDlgLogin::OnCancel()
{
	EndDialog(IDCANCEL);
}

HRESULT CDlgLogin::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

HRESULT CDlgLogin::OnButtonLogin(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgLogin::OnButtonConnection(IHTMLElement* /*pElement*/)
{
	GotoConnection();
	return S_OK;
}

HRESULT CDlgLogin::OnButtonProceed(IHTMLElement* /*pElement*/)
{
	// the URL selected or typed by the user...
	CString newtoken = GetElementProperty(L"idUrlOther", DISPID_A_VALUE);

	// this is a new m_string sequence
	CString servers = newtoken;

	// tokenize existing m_servers, create a new one
	int curPos = 0;
	int nCount = 1;
	CString token = m_strServers.Tokenize(_T(";"), curPos);
	while (nCount < 5 && token != _T(""))
	{
		if (token != newtoken)
		{
			servers += ";";
			servers += token;
			nCount++;
		}
		token = m_strServers.Tokenize(_T(";"), curPos);
	};
	m_strServers = servers;
	m_strUrl.Format(L"http://%s/advsrv.asmx", newtoken);

	GotoLogin();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// Event Handlers & Virtual Functions

BOOL CDlgLogin::OnInitDialog()
{
	return CDHtmlDialog::OnInitDialog();
}

void CDlgLogin::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CWaitCursor wait;

	HRESULT h = S_OK;
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	if (m_state != INIT)
		return;

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

	// goto login - this includes the connection test test
	GotoLogin();
}
