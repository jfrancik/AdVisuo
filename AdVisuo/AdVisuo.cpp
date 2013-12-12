// AdVisuo.cpp - a part of the AdVisuo Client Software

// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// AdVisuo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "AdVisuo.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "AdVisuoDoc.h"
#include "AdVisuoView.h"

#include "DlgHtRepBug.h"
#include "DlgHtLogin.h"
#include "DlgHtAbout.h"
#include "DlgHtSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAdVisuoApp

BEGIN_MESSAGE_MAP(CAdVisuoApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CAdVisuoApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_FILE_DOWNLOAD, &CAdVisuoApp::OnFileDownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_DOWNLOAD, &CAdVisuoApp::OnUpdateFileDownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CAdVisuoApp::OnUpdateFileOpen)
	ON_COMMAND(ID_REPORT_BUG, &CAdVisuoApp::OnReportBug)
END_MESSAGE_MAP()


// CAdVisuoApp construction

CAdVisuoApp::CAdVisuoApp()
{
	m_bHiColorIcons = TRUE;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CAdVisuoApp object

CAdVisuoApp theApp;

// CAdVisuoApp initialization

std::wstring _stdPathModels;

BOOL CAdVisuoApp::InitInstance()
{
	srand((unsigned)time(NULL));

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Resolve default paths
	// chdir
	wchar_t path_buffer[_MAX_PATH];
	wchar_t drive1[_MAX_DRIVE];
	wchar_t drive2[_MAX_DRIVE];
	wchar_t dir1[_MAX_DIR];
	wchar_t dir2[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	
	/* modified from deprecated _splitpath & _makepath in VS2008 */
	_wsplitpath_s(m_pszHelpFilePath, drive1, dir1, fname, ext);
	if (wcslen(dir1) > 0 && (dir1[wcslen(dir1)-1] == '\\' || dir1[wcslen(dir1)-1] == '/'))
	{
		dir1[wcslen(dir1)-1] = '\0';
		_wsplitpath_s(dir1, drive2, dir2, fname, ext);
	}
	else
		wcscpy_s(dir2, _MAX_DIR, dir1);
	_wmakepath_s(path_buffer, drive1, dir2, L"", L"");
	_stdPathModels = CString(path_buffer) + L"models\\";


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("LerchBates"));
	LoadStdProfileSettings(8);  // Load standard INI file options

	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// read server configuration
	LoadRemoteServerConfig(L"adsimulo.mylb.eu:8081");

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_pAVDocTemplate = new CMultiDocTemplate(IDR_AdVisuoTYPE,
		RUNTIME_CLASS(CAdVisuoDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CAdVisuoView));
	if (!m_pAVDocTemplate)
		return FALSE;
	AddDocTemplate(m_pAVDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// load a simulation (from the command line or interactively)
	try
	{
		// initialise HTTP Request object
		m_http.create();

		// for debug start mode
#ifdef _DEBUG
		//cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
		//cmdInfo.m_strFileName = "http://adsimulodev.mylb.eu:8081/advsrv.asmx?request=51&userid=jarekf&ticket=fh4qrOXaydjk0FGraUSaWN3UrSc=";
		//cmdInfo.m_strFileName = "http://localhost:5204/advsrv.asmx?request=51&userid=jarekf&ticket=tl1GVsQwvZwsZeSCKhPG5bv3i5w=";
		//cmdInfo.m_strFileName = "c:\\Users\\jarek\\Desktop\\Low Tower.avx";
#endif

		// Process advisuo: URL link or ask through Login...
		CString url;
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen && (cmdInfo.m_strFileName.Left(8).Compare(L"advisuo:") == 0 || cmdInfo.m_strFileName.Left(5).Compare(L"http:") == 0))
		{
			url = cmdInfo.m_strFileName;
			//AfxMessageBox(url);
			if (url.Find(L"phase=16&") >= 0)
				return false;
			AddRemoteServer(url);
			SaveRemoteServerConfig();
		}
		else
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
		{
			url = cmdInfo.m_strFileName;
			AfxMessageBox(L"Loading from a local file this application will have limited functionality!");
		}
		else
		{
			if (!AskLogin()) return false;
			SaveRemoteServerConfig();
			AVULONG nSimulationId = SelectSimulation(m_url);
			if (!nSimulationId) return false;
			url = URLFromSimulationId(nSimulationId);
		}


		// create main MDI Frame window
		CMainFrame *pMainframe = new CMainFrame; 
		if (!pMainframe || !pMainframe->LoadFrame(IDR_MAINFRAME))
		{
			delete pMainframe;
			return FALSE;
		}
		m_pMainWnd = pMainframe;
		m_pMainWnd->DragAcceptFiles();

		// Load the simulation with all decorations (splash windows, debug info etc...)
		if (!InitSimulation(url)) return false;
	}
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, m_url);
		dlg.DoModal();
		return false;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, m_url);
		dlg.DoModal();
		return false;
	}
	catch (_xmlreq_error xe)
	{
		CDlgHtFailure dlg(xe, m_url);
		dlg.DoModal();
		return false;
	}
	catch (_version_error ve)
	{
		CDlgHtFailure dlg(ve, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, m_url);
		dlg.DoModal();
		return false;
	}
	catch(...)
	{
		CDlgHtFailure dlg(m_url);
		dlg.DoModal();
		return false;
	}

	return true;
}

bool CAdVisuoApp::AskLogin()
{
	// Display the Login dialog box
	CDlgHtLogin dlgLogin(&m_http, m_servers);
	auto pPrevFrame = m_pMainWnd;
	m_pMainWnd = &dlgLogin;
#ifdef _DEBUG
	dlgLogin.m_strUsername = "jarekf";
	dlgLogin.m_strPassword = "sv_penguin125";
	//dlgLogin.m_bAutoLogin = true;
#endif
	if (dlgLogin.DoModal() != IDOK)
		return false;
	m_pMainWnd = pPrevFrame;
	
	// store configuration
	m_url = dlgLogin.m_strUrl;
	m_servers = dlgLogin.m_strServers;

	return true;
}

AVULONG CAdVisuoApp::SelectSimulation(CString url, AVULONG nProjectId, AVULONG nSimulationId, bool bGotoSimulations)
{
	CWaitCursor wait;
	try
	{
		// Download available projects
	
		m_http.setURL((LPCTSTR)m_url);
		if (m_http.AVIsAuthorised() <= 0)
			throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);

		// show Select Dialog
		CDlgHtSelect dlgSelect((LPCTSTR)url, nProjectId, nSimulationId, bGotoSimulations);
		dlgSelect.Load(m_http);

		auto pPrevFrame = m_pMainWnd;
		if (!m_pMainWnd || !m_pMainWnd->IsWindowVisible())
			m_pMainWnd = &dlgSelect;
		int nRes = dlgSelect.DoModal();
		m_pMainWnd = pPrevFrame;
		if (nRes != IDOK)
			return 0;
		else
			return dlgSelect.GetSimulationId();
	}			
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, m_url);
		dlg.DoModal();
		return 0;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, m_url);
		dlg.DoModal();
		return 0;
	}
	catch (_xmlreq_error xe)
	{
		CDlgHtFailure dlg(xe, m_url);
		dlg.DoModal();
		return 0;
	}
	catch (_version_error ve)
	{
		CDlgHtFailure dlg(ve, m_http.getURL().c_str());
		dlg.DoModal();
		return 0;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, m_url);
		dlg.DoModal();
		return 0;
	}
	catch(...)
	{
		CDlgHtFailure dlg(m_url);
		dlg.DoModal();
		return 0;
	}
}

CString CAdVisuoApp::URLFromSimulationId(AVULONG nSimulationId)
{
	std::wstring strUsername;
	std::wstring strTicket;
	m_http.get_authorisation_data(strUsername, strTicket);
	CString url;
	url.Format(L"%s?request=%d&userid=%s&ticket=%s", m_url, nSimulationId, strUsername.c_str(), strTicket.c_str());
	return url;
}

bool CAdVisuoApp::InitSimulation(CString url)
{
	CWaitCursor wait;

	// Show Splash Window
	CDlgHtSplash *pSplash = new CDlgHtSplash;
	pSplash->DoNonModal(1000);
	pSplash->Sleep(400);

	// Report
	Report(0, (LPOLESTR)(LPCOLESTR)url, 0, L"", L"OK", L"");

	// prepare "debug" info
	for (int i = 0; i < 10; i++) pSplash->OutText(L"");
	if (!m_pMainWnd->IsWindowVisible())
	{
		OutText(L"AdVisuo module started - a part of AdSimulo system.");
		OutText(L"Version %d.%d.%d (%ls)", VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_DATE);
	}
			
	// open document
	CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(url);
	if (!pDoc) return FALSE;

	m_url = m_http.getURL().c_str();
	if (pDoc) pDoc->ResetTitle();

	// wait a moment
	pSplash->Sleep(250);

	if (!m_pMainWnd->IsWindowVisible())
	{
		// If the main window hasn't been shown yet (starting program!), show and update it
		m_pMainWnd->ShowWindow(SW_RESTORE);
		m_pMainWnd->ShowWindow(SW_MAXIMIZE);
		m_pMainWnd->UpdateWindow();

		// wait a moment
		pSplash->Sleep(500);
	}

	// close the splash window
	pSplash->OnOK();
	delete pSplash;

	AVGetMainWnd()->SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return true;
}

bool CAdVisuoApp::LoadSimulation(AVULONG nProjectId, AVULONG nSimulationId, bool bGotoSimulations)
{
	nSimulationId = SelectSimulation(m_url, nProjectId, nSimulationId, bGotoSimulations);
	if (!nSimulationId) return false;
	return InitSimulation(URLFromSimulationId(nSimulationId));
}

void CAdVisuoApp::ExtendAuthorisation()
{
	if (!m_http.logged())
		return;
	try
	{
		m_http.AVExtendAuthorisation();
	}
	catch(...)
	{
		CDlgHtFailure dlg(L"unspecified source.");
		dlg.DoModal();
	}
}

bool CAdVisuoApp::Report(AVULONG nSimulationId, AVSTRING strPath, AVULONG nCat, AVSTRING strUserDesc, AVSTRING strDiagnostic, AVSTRING strErrorMsg)
{
	CXMLRequest http;
	http.create();
	http.setURL(L"http://adsimulo.mylb.eu:8081/advsrv.asmx");

	std::wstring strUsername, strTicket;
	m_http.get_authorisation_data(strUsername, strTicket);

	try
	{
		std::wstringstream s;
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		s << strDiagnostic << L" (winver " << osvi.dwMajorVersion << L"." << osvi.dwMinorVersion << L"." << osvi.dwBuildNumber << L"." << osvi.dwPlatformId << L" " << osvi.szCSDVersion << L")";

		http.AVReportIssue(m_http.getURL(), strUsername, strTicket, VERSION, nSimulationId, strPath, nCat, strUserDesc, s.str(), strErrorMsg);
		std::wstring response;
		http.get_response(response);
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (_xmlreq_error xe)
	{
		CDlgHtFailure dlg(xe, http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch(...)
	{
		CDlgHtFailure dlg(http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	return true;
}

int CAdVisuoApp::ExitInstance()
{
	return CWinAppEx::ExitInstance();
}

// App command to run the dialog
void CAdVisuoApp::OnAppAbout()
{
	if (AVGetMainWnd()->IsFullScreen()) return;
	CDlgHtAbout aboutDlg;
	aboutDlg.DoModal();
}

// CAdVisuoApp customization load/save methods

void CAdVisuoApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CAdVisuoApp::LoadRemoteServerConfig(CString strDefault)
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);
	if (reg.Open(GetRegSectionPath(L"URL")))
		reg.Read(L"servers", m_servers); 
	if (m_servers.IsEmpty())
		m_servers = strDefault;
}

void CAdVisuoApp::AddRemoteServer(CString url)
{
	int i;

	i = url.Find(L"://");
	if (i > 0) url = url.Mid(i + 3);

	i = url.Find(L'/');
	if (i > 0) url = url.Left(i);
	
	int curPos = 0;
	int nCount = 1;
	CString servers = url;
	CString token = m_servers.Tokenize(_T(";"), curPos);
	while (nCount < 5 && token != _T(""))
	{
		if (token != url)
		{
			servers += ";";
			servers += token;
			nCount++;
		}
		token = m_servers.Tokenize(_T(";"), curPos);
	};
	m_servers = servers;

}

void CAdVisuoApp::SaveRemoteServerConfig()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);
	if (reg.CreateKey(GetRegSectionPath(L"URL")))
		reg.Write(L"servers", m_servers);
}

void CAdVisuoApp::LoadCustomState()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);
	if (reg.Open(GetRegSectionPath(L"AdVisuo")))
	{
		reg.Read(L"NavigationMode", m_nWalkMode);
		reg.Read(L"ColouringMode", m_nColouringMode);
	}
	m_nWalkMode = 2;	// ghost mode only now...
}

void CAdVisuoApp::SaveCustomState()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);
	if (reg.CreateKey(GetRegSectionPath(L"AdVisuo")))
	{
		reg.Write(L"NavigationMode", m_nWalkMode);
		reg.Write(L"ColouringMode", m_nColouringMode);
	}
}

// CAdVisuoApp message handlers

BOOL CAdVisuoApp::OnIdle(LONG lCount)
{
	return CWinAppEx::OnIdle(lCount);
}

void CAdVisuoApp::OnFileDownload()
{
	LoadSimulation();
}


void CAdVisuoApp::OnUpdateFileDownload(CCmdUI *pCmdUI)
{
	if (IsLogged())
		pCmdUI->Enable(!AVGetMainWnd()->IsFullScreen());
	else
		pCmdUI->Enable(FALSE);
}


void CAdVisuoApp::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!AVGetMainWnd()->IsFullScreen());
}


void CAdVisuoApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	if (wcsncmp(lpszPathName, L"advisuo:", 8) != 0)
		CWinAppEx::AddToRecentFileList(lpszPathName);
	else
	{
		// add advisuo:// path to MRU
		ENSURE_ARG(lpszPathName != NULL);
		ASSERT(AfxIsValidString(lpszPathName));

		if (m_pRecentFileList != NULL)
		{
			// add code in the future to process advisuo paths --- currently does nothing
			// m_pRecentFileList->Add(lpszPathName);
		}
	}
}

void CAdVisuoApp::OnReportBug()
{
	CDlgHtRepBug dlg;
	dlg.DoModal();
}

BOOL CAdVisuoApp::LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd)
{
	BOOL b = CWinAppEx::LoadWindowPlacement(rectNormalPosition, nFflags, nShowCmd);
	nShowCmd = SW_HIDE;
	return b;
}

