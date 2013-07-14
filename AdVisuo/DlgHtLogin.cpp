// DlgHtLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtLogin.h"
#include "XMLRequest.h"
#include "VisProject.h"		// for _prj_error

// CDlgHtLogin dialog

IMPLEMENT_DYNCREATE(CDlgHtLogin, CDlgHtFailure)

CDlgHtLogin::CDlgHtLogin(CXMLRequest *pHttp, CString strServers, CWnd* pParent /*=NULL*/)
	: CDlgHtFailure()
{
	m_pHttp = pHttp;
	m_state = INIT;
	m_strServers = strServers;
	int curPos = 0;
	m_strUrl.Format(L"http://%s/advsrv.asmx", m_strServers.Tokenize(_T(";"), curPos));
	m_fnLoadComplHandle = [this] { GotoLogin(); };
}

CDlgHtLogin::~CDlgHtLogin()
{
}

void CDlgHtLogin::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtFailure::DoDataExchange(pDX);
	DDX_DHtml_ElementText(pDX, _T("idUserName"), DISPID_A_VALUE, m_strUsername); 
	DDX_DHtml_ElementText(pDX, _T("idPassword"), DISPID_A_VALUE, m_strPassword); 
}

BEGIN_MESSAGE_MAP(CDlgHtLogin, CDlgHtFailure)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtLogin)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("idLoginButton"), OnButtonLogin)
	DHTML_EVENT_ONCLICK(_T("idConnButton"), OnButtonConnection)
	DHTML_EVENT_ONCLICK(_T("idAfterFailConn"), OnButtonConnection)
	DHTML_EVENT_ONCLICK(_T("idProceedButton"), OnButtonProceed)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtLogin, CDlgHtFailure)
END_DISPATCH_MAP()

////////////////////////////////////////////////////////////////////////
// Javascript Bounding

void CDlgHtLogin::OnGotoFailure(CString title, CString text)
{
	m_state = FAILURE;
	std::wstringstream str;
	str << L"GotoFailureConn(\"" << (LPCTSTR)title << L"\", \"" << (LPCTSTR)text << L"\")";
	ExecJS(str.str().c_str());
	SetWindowText(CString(L"AdVisuo ") + title);
}

void CDlgHtLogin::GotoLogin()
{
	CWaitCursor wait;

	if (!m_pHttp) return;

	// test the connection
	try
	{
		m_pHttp->setURL((LPCTSTR)m_strUrl);
		std::wstring appname = m_pHttp->AVGetAppName();
		if (appname != L"AdVisuo")
			throw _prj_error(_prj_error::E_PRJ_INTERNAL);
		int verreq = m_pHttp->AVGetRequiredVersion();
		std::wstring date = m_pHttp->AVGetRequiredVersionDate();
		std::wstring path = m_pHttp->AVGetLatestVersionDownloadPath();

		if (VERSION < verreq)
			throw _version_error(verreq, date.c_str(), path.c_str());

		// the only successful way to the login!
		m_state = LOGIN;
		ExecJS(L"GotoLogin()");
		SetWindowText(L"AdVisuo Login");
	}
	catch (_prj_error pe)
	{
		GotoFailure(pe, m_pHttp->URL().c_str());
	}
	catch (_com_error ce)
	{
		GotoFailure(ce, m_pHttp->URL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		GotoFailure(xe, m_pHttp->URL().c_str());
	}
	catch (_version_error ve)
	{
		GotoFailure(ve, m_pHttp->URL().c_str());
	}
	catch(...)
	{
		GotoFailure(m_pHttp->URL().c_str());
	}
}

void CDlgHtLogin::GotoBadLogin()
{
	m_strUsername = m_strPassword = L"";
	UpdateData(FALSE);
	m_state = LOGIN;
	ExecJS(L"GotoBadLogin()");
}

void CDlgHtLogin::GotoConnection()
{
	m_state = CONNECTION;
	std::wstringstream str;
	str << L"GotoConnection(\"" << (LPCTSTR)m_strServers << "\")";
	ExecJS(str.str().c_str());
	SetWindowText(L"AdVisuo Server Connection");
}

////////////////////////////////////////////////////////////////////////
// Button Handlers

void CDlgHtLogin::OnOK()
{
	CWaitCursor wait;
	switch (m_state)
	{
	case INIT:
	case FAILURE:
		OnCancel();
		break;
	case LOGIN:
		if (!m_pHttp) return;
		UpdateData();
		try
		{
			m_pHttp->setURL((LPCTSTR)m_strUrl);
			if (m_pHttp->AVLogin((LPCTSTR)m_strUsername, (LPCTSTR)m_strPassword))
				CDlgHtFailure::OnOK();
			else
				GotoBadLogin();
		}
		catch (_com_error ce)
		{
			GotoFailure(ce, m_pHttp->URL().c_str());
		}
		catch (_xmlreq_error xe)
		{
			GotoFailure(xe, m_pHttp->URL().c_str());
		}
		catch (_version_error ve)
		{
			GotoFailure(ve, m_pHttp->URL().c_str());
		}
		catch(...)
		{
			GotoFailure(m_pHttp->URL().c_str());
		}
		break;
	case CONNECTION:
		OnButtonProceed(NULL);
		break;
	}
}

void CDlgHtLogin::OnCancel()
{
	CDlgHtFailure::OnCancel();
}

HRESULT CDlgHtLogin::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

HRESULT CDlgHtLogin::OnButtonLogin(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgHtLogin::OnButtonConnection(IHTMLElement* /*pElement*/)
{
	GotoConnection();
	return S_OK;
}

HRESULT CDlgHtLogin::OnButtonProceed(IHTMLElement* /*pElement*/)
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
