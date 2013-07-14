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
#include "_version.h"
#include "afxwinappex.h"
#include "AdVisuo.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "AdVisuoDoc.h"
#include "AdVisuoView.h"

#include "DlgRepBug.h"
#include "DlgDownload.h"
#include "DlgHtLogin.h"
#include "DlgHtAbout.h"

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
	m_pOutList = NULL;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CAdVisuoApp object

CAdVisuoApp theApp;

// Authorisatipon infrastructure

void CAdVisuoApp::Authorise(CString url, CString username, CString ticket)
{
	m_http.setURL((LPCTSTR)url);
	m_http.authorise((LPCTSTR)username, (LPCTSTR)ticket);
}

void CAdVisuoApp::RefreshAuthorisation()
{
	m_http.AVExtendAuthorisation();
}

void CAdVisuoApp::GetAuthorisation(CXMLRequest *pHttp)
{
	pHttp->authorise_from(m_http);
}



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
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);
	if (reg.Open(GetRegSectionPath(L"URL")))
		reg.Read(L"servers", m_servers); 
	if (m_servers.IsEmpty())
		m_servers = L"217.33.230.53:8081;adsimulo.mylb.eu:8081";

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_pAVDocTemplate = new CMultiDocTemplate(IDR_AdVisuoTYPE,
		RUNTIME_CLASS(CAdVisuoDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CAdVisuoView));
	if (!m_pAVDocTemplate)
		return FALSE;
	AddDocTemplate(m_pAVDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	m_pMainWnd = pMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// load a project (from the command line or interactively)
	try
	{
		// initialise HTTP Request object
		m_http.create();

		// Process advisuo: URL link...
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen &&
		   (cmdInfo.m_strFileName.Left(8).Compare(L"advisuo:") == 0 || cmdInfo.m_strFileName.Left(5).Compare(L"http:") == 0))
		{
			Debug(L"AdVisuo module started - a part of AdSimulo system.");
			Debug(L"Version %d.%d.%d (%ls)", VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_DATE);

			// open document
			CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(cmdInfo.m_strFileName);
			if (!pDoc) return FALSE;

			m_url = m_http.URL().c_str();
			if (pDoc) pDoc->ResetTitle();
		}
		else
		{
			// store configuration
			m_url = L"http://217.33.230.53:8081/advsrv.asmx";
			m_servers = L"217.33.230.53:8081";
			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);
			if (reg.CreateKey(GetRegSectionPath(L"URL")))
				reg.Write(L"servers", m_servers);

			// Download available projects
			std::vector<CProjectVis*> prjs;
			std::wstring response;
				
			m_http.setURL((LPCTSTR)(m_url));
			m_http.authorise(L"dummy", L"01234");

			if (m_http.AVIsAuthorised() <= 0)
				throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);
			
			std::wstring appname = m_http.AVGetAppName();
			int verreq = m_http.AVGetRequiredVersion();
			std::wstring date = m_http.AVGetRequiredVersionDate();
			std::wstring path = m_http.AVGetLatestVersionDownloadPath();
			if (VERSION < verreq)
				throw _version_error(verreq, date.c_str(), path.c_str());
			
			m_http.AVIndex();
			m_http.get_response(response);
			CProjectVis::LoadIndexFromBuf(response.c_str(), prjs);
			
			// show Index Dialog
			CDlgDownload dlg(prjs);
			if (dlg.DoModal() ==IDOK)
			{
				CWaitCursor wait;

				// prepare "debug" info
				Debug(L"AdVisuo module started - a part of AdSimulo system.");
				Debug(L"Version %d.%d.%d (%ls)", VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_DATE);
			
				CString url;
				url.Format(L"%s?request=%d&userid=%s&ticket=%s", m_url, dlg.GetProjectId(), m_http.get_username().c_str(), m_http.get_ticket().c_str());
				
				CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(url);
				// if (pDoc) pDoc->ResetTitle();

				if (!pDoc) return FALSE;
			}
			else
				return FALSE;
		}
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
		CDlgHtFailure dlg(ve, m_http.URL().c_str());
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

	// report session id
	m_nSessionId = rand() * (RAND_MAX + 1) + rand();

	// report to site
//	CDlgReportBug::Report(1);
	
	return TRUE;
}

int CAdVisuoApp::ExitInstance()
{
	//CDlgReportBug::Report(2);
	return CWinAppEx::ExitInstance();
}




// App command to run the dialog
void CAdVisuoApp::OnAppAbout()
{
	if (((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen()) return;
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

void CAdVisuoApp::LoadCustomState()
{
	{
		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, TRUE);
		if (reg.Open(GetRegSectionPath(L"AdVisuo")))
		{
			reg.Read(L"NavigationMode", m_nWalkMode);
			reg.Read(L"ColouringMode", m_nColouringMode);
		}
	}
m_nWalkMode = 2;	// ghost mode only now...
	{
		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, TRUE);
		if (reg.Open(GetRegSectionPath(L"URL")))
		{
			reg.Read(L"servers", m_servers); 
		}
		else
		{
			m_servers = L"217.33.230.52:8081;adsimulo.mylb.eu:8081;localhost:7984";
		}
	}
}

void CAdVisuoApp::SaveCustomState()
{
	{
		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);
		if (reg.CreateKey(GetRegSectionPath(L"AdVisuo")))
		{
			reg.Write(L"NavigationMode", m_nWalkMode);
			reg.Write(L"ColouringMode", m_nColouringMode);
		}
	}
	{
		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);
		if (reg.CreateKey(GetRegSectionPath(L"URL")))
		{
			reg.Write(L"servers", m_servers);
		}
	}
}

// CAdVisuoApp message handlers



BOOL CAdVisuoApp::OnIdle(LONG lCount)
{
	return CWinAppEx::OnIdle(lCount);
}

void CAdVisuoApp::OnFileDownload()
{
	// Download available projects
	std::vector<CProjectVis*> prjs;
	std::wstring response;
	try
	{
		CWaitCursor wait;
		m_http.setURL((LPCTSTR)m_url);
		m_http.AVIndex();
		m_http.get_response(response);
		CProjectVis::LoadIndexFromBuf(response.c_str(), prjs);
	}
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, m_url);
		dlg.DoModal();
		return;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, m_url);
		dlg.DoModal();
		return;
	}
	catch (_xmlreq_error xe)
	{
		CDlgHtFailure dlg(xe, m_url);
		dlg.DoModal();
		return;
	}
	catch (_version_error ve)
	{
		CDlgHtFailure dlg(ve, m_http.URL().c_str());
		dlg.DoModal();
		return;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, m_url);
		dlg.DoModal();
		return;
	}
	catch(...)
	{
		CDlgHtFailure dlg(m_url);
		dlg.DoModal();
		return;
	}

			
			
	CDlgDownload dlg(prjs);

	if (dlg.DoModal() ==IDOK)
	{
		CWaitCursor wait;
		
//		static CDlgSplash splash;
//		splash.DoNonModal(1000);
//		splash.Sleep(400);

//		// prepare "debug" info
//		m_pOutList = (CListBox*)(splash.GetDlgItem(IDC_DEBUG));
//		for (int i = 0; i < 10; i++) OutDebugText(L"");
//		Debug(L"AdVisuo module started - a part of AdSimulo system.");
//		Debug(L"Version %d.%d.%d (%ls)", VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_DATE);
			
		CString url;
		url.Format(L"%s?request=%d&userid=%s&ticket=%s", m_url, dlg.GetProjectId(), m_http.get_username().c_str(), m_http.get_ticket().c_str());
				
		CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(url);
		// if (pDoc) pDoc->ResetTitle();

		if (!pDoc) return;

		// wait a moment
//		splash.Sleep(750);

		// close the splash window
//		m_pOutList = NULL;
//		splash.EndDialog(IDOK);

//		AfxGetMainWnd()->SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}


void CAdVisuoApp::OnUpdateFileDownload(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
}


void CAdVisuoApp::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
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
	CDlgReportBug dlg;
	dlg.DoModal();
}

BOOL CAdVisuoApp::LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd)
{
	BOOL b = CWinAppEx::LoadWindowPlacement(rectNormalPosition, nFflags, nShowCmd);
//	nShowCmd = SW_HIDE;
	return b;
}

void CAdVisuoApp::OutDebugText(LPCTSTR lpszItem)
{
	if (!m_pOutList) return;
	
	m_pOutList->AddString(lpszItem);
	CRect rect; m_pOutList->GetClientRect(rect);
	int n = rect.Height() / m_pOutList->GetItemHeight(0) - 2;
	n = m_pOutList->GetCount() - n;
	m_pOutList->SetTopIndex(n);

	m_pOutList->UpdateWindow();
}
