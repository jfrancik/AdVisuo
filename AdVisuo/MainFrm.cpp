// MainFrm.cpp - a part of the AdVisuo Client Software

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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "AdVisuoDoc.h"
#include "RibbonScenarioButton.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WINDOWS_7, ID_VIEW_APPLOOK_VS_2008, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WINDOWS_7, ID_VIEW_APPLOOK_VS_2008, &CMainFrame::OnUpdateApplicationLook)
	
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_OUTPUTWND, &CMainFrame::OnViewOutputwnd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &CMainFrame::OnUpdateViewOutputwnd)
	ON_COMMAND(ID_VIEW_PROPERTIESWND, &CMainFrame::OnViewPropertieswnd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIESWND, &CMainFrame::OnUpdateViewPropertieswnd)

//	ON_COMMAND(ID_TEST, &CMainFrame::OnTest)
//	ON_UPDATE_COMMAND_UI(ID_TEST, &CMainFrame::OnUpdateTest)
	ON_WM_GETMINMAXINFO()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_VIEW_HIDE_ALL_PANES, &CMainFrame::OnViewHideAllPanes)
	ON_COMMAND(ID_VIEW_VIEW, &CMainFrame::OnViewView)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_WINDOWS_7);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams	);

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// scenario button
	m_wndRibbonBar.GetCategory(1)->GetPanel(0)->Add(new CRibbonScenarioButton(ID_SCENARIO_MENU, L"Scenario", -1, 23, IDB_SCENARIOS, 32));


	SetTimer(100, 1000 / 50, NULL);
	SetTimer(101, 1000 * 60, NULL);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);


	// Create output window
	CString strOutputWnd;
	strOutputWnd.LoadString(IDS_OUTPUT_WND);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 200, 500), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return -1;
	}
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	// Create properties window
	CString strPropertiesWnd;
	strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 500), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return -1;
	}
	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	EnableFullScreenMode (55555);
	EnableFullScreenMainMenu(FALSE);

	if (theApp.GetInt(L"Version") < 30700)
	{
		EnableLoadDockState(FALSE);
		PostMessage(WM_COMMAND, MAKEWPARAM(ID_VIEW_HIDE_ALL_PANES, 0), (LPARAM)0);
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;

	return TRUE;
}


// CMainFrame diagnostics
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
//	CFrameWnd *pFrame = GetActiveFrame();
//	if (pFrame)
//	{
//		CAdVisuoDoc *pDoc = (CAdVisuoDoc*)pFrame->GetActiveDocument();
//		if (pDoc)
//			if (pDoc) pDoc->OnTimer();
//	}
//	CMDIFrameWndEx::OnTimer(nIDEvent);
	//CMFCRibbonPanelLayout::UpdateStatus();

	switch (nIDEvent)
	{
	case 100:
		SendMessageToDescendants(WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0, TRUE, TRUE);
		break;
	case 101:
		AVGetApp()->ExtendAuthorisation();
		break;
	}
}

void CMainFrame::OnViewOutputwnd()
{
	SetFocus();
	if (m_wndOutput.IsWindowVisible())
		ShowPane(&m_wndOutput, FALSE, FALSE, FALSE);
	else
		ShowPane(&m_wndOutput, TRUE, FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewOutputwnd(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndOutput.IsWindowVisible());
}

void CMainFrame::OnViewPropertieswnd()
{
	SetFocus();
	if (m_wndProperties.IsWindowVisible())
		ShowPane(&m_wndProperties, FALSE, FALSE, FALSE);
	else
		ShowPane(&m_wndProperties, TRUE, FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewPropertieswnd(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndProperties.IsWindowVisible());
}

// makes min max info more relaxed in full screen mode
void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (IsFullScreen())
	{
		CRect rect;
		GetDesktopWindow()->GetWindowRect(rect);
		lpMMI->ptMaxSize.y = rect.Height() + 54 + 13;
		lpMMI->ptMaxTrackSize.y = rect.Height() + 54 + 13;
		lpMMI->ptMaxSize.x = rect.Width() + 26;
		lpMMI->ptMaxTrackSize.x = rect.Width() + 26;
	}
	else
		CMDIFrameWndEx::OnGetMinMaxInfo(lpMMI);
}
 
// disables mfc-predefined ESC key behaviour in full screen mode
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && IsFullScreen())
		return 0;
	else
		return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}


void CMainFrame::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CMDIFrameWndEx::OnWindowPosChanging(lpwndpos);
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID)
	{
	case SC_RESTORE: 
		if (AVGetApp()->GetSplashWindow())
		{
			((CWnd*)AVGetApp()->GetSplashWindow())->ShowWindow(SW_RESTORE);
		}
		else
			CMDIFrameWndEx::OnSysCommand(nID, lParam); break;
		break;

	default: 
		CMDIFrameWndEx::OnSysCommand(nID, lParam); break;
	}

	
}

void CMainFrame::OnViewHideAllPanes()
{
	m_wndProperties.SetAutoHideMode(TRUE, m_wndProperties.GetDefaultPaneDivider()->GetCurrentAlignment(), 0, 0);
	m_wndOutput.SetAutoHideMode(TRUE, m_wndOutput.GetDefaultPaneDivider()->GetCurrentAlignment(), 0, 0);
	EnableLoadDockState(TRUE);
}

void CMainFrame::OnViewView()
{
}
