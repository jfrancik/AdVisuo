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

#include "DlgRepBug.h"
#include "DlgDownload.h"
#include "DlgLogin.h"

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
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
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

//	if (__argc >= 3)
//		m_strSimPathName = __targv[2];

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Process advisuo: URL link...
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen &&
	   (cmdInfo.m_strFileName.Left(8).Compare(L"advisuo:") == 0 || cmdInfo.m_strFileName.Left(5).Compare(L"http:") == 0))
	{
		CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(cmdInfo.m_strFileName);
		if (pDoc) pDoc->ResetTitle();
	}
	else
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	// report session id
	m_nSessionId = rand() * (RAND_MAX + 1) + rand();

	// report to site
//	CDlgReportBug::Report(1);
	
	return TRUE;
}

int CAdVisuoApp::ExitInstance()
{
	CDlgReportBug::Report(2);
	return CWinAppEx::ExitInstance();
}




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CAdVisuoApp::OnAppAbout()
{
	if (((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen()) return;
	CAboutDlg aboutDlg;
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
	CDlgLogin dlgLogin;

	dlgLogin.m_strUsername = L"jarekf";
	dlgLogin.m_strPassword = L"sv_penguin125";

	if (dlgLogin.DoModal() ==IDOK)
	{
		AfxMessageBox(dlgLogin.m_strUsername);
		AfxMessageBox(dlgLogin.m_strPassword);
	}
	else
		return;

	CDlgDownload dlg;

	dlg.SetServers(m_servers);

	if (dlg.DoModal() ==IDOK)
	{
		m_servers = dlg.GetServers();
		CString url = dlg.GetURL();

		CAdVisuoDoc *pDoc = (CAdVisuoDoc*)m_pAVDocTemplate->OpenDocumentFile(url);
//		if (pDoc) pDoc->ResetTitle();
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


