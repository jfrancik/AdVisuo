// DlgHtLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtLogin.h"
#include "XMLRequest.h"
#include "VisProject.h"		// for _prj_error
#include <WinCred.h>

// CDlgHtLogin dialog

IMPLEMENT_DYNCREATE(CDlgHtLogin, CDlgHtFailure)

CDlgHtLogin::CDlgHtLogin(CXMLRequest *pHttp, CString strServers, CWnd* pParent /*=NULL*/)
	: CDlgHtFailure()
{
	m_pHttp = pHttp;
	m_state = INIT;
	m_strServers = strServers;
	m_bAutoLogin = false;
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
	DDX_DHtml_CheckBox(pDX, _T("idRemember"), m_bRememberMe);
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

void CDlgHtLogin::OnGotoFailure(CString title, CString text, CString url)
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

	// display info
	std::wstringstream str;
	str << L"showVer(" << VERSION_MAJOR << L", " << VERSION_MINOR << L", " << VERSION_REV << L", \"" << (LPCTSTR)VERSION_DATE << L"\")";
	ExecJS(str.str().c_str());
	str.str(std::wstring());
	str << L"showUrl(\"" << (LPCTSTR)m_strUrl << L"\")";
	ExecJS(str.str().c_str());

	// produce stored credentials
	PCREDENTIAL pcred;
	if (CredRead((LPWSTR)(LPCTSTR)m_strUrl, CRED_TYPE_GENERIC, 0, &pcred))
	{
		m_bRememberMe = 1;
		m_strUsername = pcred->UserName;
		m_strPassword = (wchar_t*)pcred->CredentialBlob;
		CredFree(pcred);
	}
	else
	{
		m_bRememberMe = 0;
		m_strUsername = L"";
		m_strPassword = L"";
	}
	UpdateData(FALSE);

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

#ifdef _DEBUG
		if (!m_strPassword.IsEmpty() && m_bAutoLogin)
			OnOK();
#endif
	}
	catch (_prj_error pe)
	{
		GotoFailure(pe, m_pHttp->getURL().c_str());
	}
	catch (_com_error ce)
	{
		GotoFailure(ce, m_pHttp->getURL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		GotoFailure(xe, m_pHttp->getURL().c_str());
	}
	catch (_version_error ve)
	{
		GotoFailure(ve, m_pHttp->getURL().c_str());
	}
	catch(...)
	{
		GotoFailure(m_pHttp->getURL().c_str());
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
			{
				CDlgHtFailure::OnOK();

				// store credentials
				if (m_bRememberMe)
				{
					// store credentials
					CREDENTIAL cred = {0};
					cred.Type = CRED_TYPE_GENERIC;
					cred.TargetName = (LPWSTR)(LPCTSTR)m_strUrl;
					cred.CredentialBlobSize = 2 * (m_strPassword.GetLength() + 1);
					cred.CredentialBlob = (LPBYTE)m_strPassword.GetBuffer();
					cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
					cred.UserName = (LPWSTR)(LPCTSTR)m_strUsername;
					CredWrite(&cred, 0);
					m_strPassword.ReleaseBuffer();
				}
				else
					// destroy credentials if any stored
					CredDelete((LPWSTR)(LPCTSTR)m_strUrl, CRED_TYPE_GENERIC, 0);
			}
			else
				GotoBadLogin();
		}
		catch (_com_error ce)
		{
			GotoFailure(ce, m_pHttp->getURL().c_str());
		}
		catch (_xmlreq_error xe)
		{
			GotoFailure(xe, m_pHttp->getURL().c_str());
		}
		catch (_version_error ve)
		{
			GotoFailure(ve, m_pHttp->getURL().c_str());
		}
		catch(...)
		{
			GotoFailure(m_pHttp->getURL().c_str());
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
