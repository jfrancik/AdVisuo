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

#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "AdVisuo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()

	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	//m_wndObjectCombo.GetWindowRect(&rectCombo);

	//int cyCmb = rectCombo.Size().cy;
	//int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	int cyCmb = 0;
	int cyTlb = 0;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	//{
	//	TRACE0("Failed to create Properties Combo \n");
	//	return -1;      // fail to create
	//}

	//m_wndObjectCombo.AddString(_T("Application"));
	//m_wndObjectCombo.AddString(_T("Properties Window"));
	//m_wndObjectCombo.SetCurSel(0);

	m_wndPropList.SetListDelimiter(L':');
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	//m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	//m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	//m_wndToolBar.SetOwner(this);

	//// All commands will be routed via this control , not via the parent frame:
	//m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

enum { ASPECT_RATIO = 1, ASPECT_RATIO_LIST, ASPECT_RATIO_H, ASPECT_RATIO_V, COLOURING_MODE, THRESHOLD_GREEN, THRESHOLD_RED, NAV_MODE };

std::wstring TXT_ASPECT[] = { 
	L"Fit to window",
	L"Current screen ratio",
	L"16:9 Wide screen",
	L"16:10 Wide screen",
	L"4:3 Standard screen",
	L"Custom..."
	};
std::wstring TXT_COLOUR_MODE[] = { 
	L"No colour coding",
	L"Current waiting time",
	L"Expected waiting time"
};

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
	m_wndPropList.SetDescriptionRows(4);

	CMFCPropertyGridProperty *pGroup, *pList, *pProp;

	// Group #1
	m_wndPropList.AddProperty(pGroup = new CMFCPropertyGridProperty(L"View"));
	pGroup->SetDescription(L"Parameters of the view");

	pGroup->AddSubItem(pProp = new CMFCPropertyGridProperty(L"Aspect ratio", L"Fit to window", L"Choose from a range of commonly used image aspect ratio values", ASPECT_RATIO));
	for each (std::wstring str in TXT_ASPECT)
		pProp->AddOption(str.c_str());
	pProp->AllowEdit(FALSE);

	m_wndPropList.AddProperty(pList = new CMFCPropertyGridProperty(L"Custom ratio", ASPECT_RATIO_LIST, TRUE));
	pList->SetDescription(L"Any custom aspect ratio");

	pList->AddSubItem(pProp = new CMFCPropertyGridProperty(L"horizontally", (_variant_t) 16l, L"Horizontal factor of the ratio, like 16 in case of 16:9 ratio", ASPECT_RATIO_H));
	pProp->EnableSpinControl(TRUE, 1, 99);

	pList->AddSubItem(pProp = new CMFCPropertyGridProperty( L"vertically", (_variant_t) 10l, L"Vertical factor of the ratio, like 9 in case of 16:9 ratio", ASPECT_RATIO_V));
	pProp->EnableSpinControl(TRUE, 1, 99);

	
	// Group #2
	m_wndPropList.AddProperty(pGroup = new CMFCPropertyGridProperty(L"Passenger colour coding"));
	pGroup->SetDescription(L"Passenger skin are colour coded to reflect their waiting time: from green, through yellow and orange to red.");

	pGroup->AddSubItem(pProp = new CMFCPropertyGridProperty(L"Mode", TXT_COLOUR_MODE[SETTINGS::nColouringMode].c_str(), L"Specifies how the passenger colour coding is determined", COLOURING_MODE));
	for each (std::wstring str in TXT_COLOUR_MODE)
		pProp->AddOption(str.c_str());
	pProp->AllowEdit(FALSE);

	pGroup->AddSubItem(pProp = new CMFCPropertyGridProperty(L"Green threshold", (_variant_t)SETTINGS::nThresholdGreen, L"Passenger waiting time threshold - when the skin colour starts to gradually change from green towards red [seconds]", THRESHOLD_GREEN));
	pProp->EnableSpinControl(TRUE, 0, 300);
	pGroup->AddSubItem(pProp = new CMFCPropertyGridProperty(L"Red threshold", (_variant_t)SETTINGS::nThresholdRed, L"Passenger waiting time threshold - when the character skin becomes purely red [seconds]", THRESHOLD_RED));
	pProp->EnableSpinControl(TRUE, 0, 300);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	//m_wndObjectCombo.SetFont(&m_fntPropList);
}

LRESULT CPropertiesWnd::OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam)
{
	// Parameters:
	// [in] wparam: the control ID of the CMFCPropertyGridCtrl that changed.
	// [in] lparam: pointer to the CMFCPropertyGridProperty that changed.

	// Cast the lparam to a property.
	CMFCPropertyGridProperty * pProperty = (CMFCPropertyGridProperty*) lparam;

	COleVariant v = pProperty->GetValue();
	int i = 0;
	switch (pProperty->GetData())
	{
	case ASPECT_RATIO:
		break;
	case ASPECT_RATIO_LIST:
		break;
	case ASPECT_RATIO_H:
		break;
	case ASPECT_RATIO_V:
		break;
	case COLOURING_MODE:
		{
			CString val = v;
			for each (std::wstring str in TXT_COLOUR_MODE)
			{
				if (val == str.c_str())
				{
					SETTINGS::nColouringMode = i;
					break;
				}
				i++;
			}
		}
		break;
	case THRESHOLD_GREEN:
		SETTINGS::nThresholdGreen = v.intVal;
		break;
	case THRESHOLD_RED:
		SETTINGS::nThresholdRed = v.intVal;
		break;
	case NAV_MODE:
		break;
	}

	return 0; 
}