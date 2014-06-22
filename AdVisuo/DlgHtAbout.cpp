// DlgSplash.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtAbout.h"

// CDlgHtOutText dialog

BOOL CDlgHtOutText::OnInitDialog()
{
	CDlgHtBase::OnInitDialog();
	RegisterOutTextSink(this);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgHtOutText, CDlgHtBase)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtOutText)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("CancelButton"), OnButtonCancel)
END_DHTML_EVENT_MAP()

void CDlgHtOutText::OnOK()
{
	UnRegisterOutTextSink(this);
	CDlgHtBase::OnOK();
}

void CDlgHtOutText::OnCancel()
{
	UnRegisterOutTextSink(this);
	CDlgHtBase::OnCancel();
}

// CDlgHtAbout dialog

IMPLEMENT_DYNCREATE(CDlgHtAbout, CDlgHtOutText)

CDlgHtAbout::CDlgHtAbout(UINT nIDTemplate, UINT nHtmlResID, CWnd* pParent) : CDlgHtOutText(nIDTemplate, nHtmlResID, pParent)
{
	m_fnLoadComplHandle = [this]() { ShowVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_DATE); };
}

void CDlgHtAbout::OutText(LPCTSTR lpszItem)
{
}

void CDlgHtAbout::ShowVersion(AVULONG nMajor, AVULONG nMinor, AVULONG nRel, CString date)
{
	std::wstringstream str;
	str << L"showVer(" << nMajor << L", " << nMinor << L", " << nRel << L", \"" << (LPCTSTR)date << L"\")";
	ExecJS(str.str().c_str());
}


BEGIN_MESSAGE_MAP(CDlgHtAbout, CDlgHtOutText)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtAbout)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("CancelButton"), OnButtonCancel)
END_DHTML_EVENT_MAP()

// CDlgHtAbout message handlers

HRESULT CDlgHtAbout::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgHtAbout::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////
// CDlgHtSplash dialog

IMPLEMENT_DYNCREATE(CDlgHtSplash, CDlgHtOutText)

void CDlgHtSplash::OutText(LPCTSTR lpszItem)
{
	if (!IsWindow(m_hWnd)) return;

	CListBox *pList = (CListBox*)(GetDlgItem(IDC_DEBUG));
	
	pList->AddString(lpszItem);
	CRect rect; pList->GetClientRect(rect);
	int n = rect.Height() / pList->GetItemHeight(0) - 2;
	n = pList->GetCount() - n;
	pList->SetTopIndex(n);

	pList->UpdateWindow();
}

bool CDlgHtSplash::OutWaitMessage(AVLONG nWaitStage, AVULONG &nMsecs)
{
	// call JS to display the message
	std::wstringstream s;
	if (nWaitStage > 0)
		s << L"WAIT - TASK QUEUED #" << nWaitStage;
	else if (nWaitStage == 0)
		s << L"PROCESSING...";

	SetWindowText(CString(L"AdVisuo: ") + s.str().c_str());
	ExecJS(CString(L"displayWaitMsg('") + s.str().c_str() + L"')");

	// do tricky things with the window style

	if (nWaitStage > 0 && !IsDecorated())
		Decorate();

	if (nWaitStage < 0 && IsDecorated())
		Undecorate();

	// sleep and allow the user to play around
	if (nMsecs > 0)
		Sleep(nMsecs);
	nMsecs = 0;

	return !m_bQuitRequested;
}

bool CDlgHtSplash::IsDecorated()
{
	return (GetWindowLong(m_hWnd, GWL_STYLE) & WS_MINIMIZEBOX) == WS_MINIMIZEBOX;
}

void CDlgHtSplash::Decorate()
{
	if (IsDecorated()) return;

	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG nExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);

	LockWindowUpdate();
	CRect rect;
	GetWindowRect(&rect);
	SetWindowLong(m_hWnd, GWL_STYLE, nStyle | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | DS_FIXEDSYS | DS_MODALFRAME);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, nExStyle | WS_EX_DLGMODALFRAME);
	SetWindowPos(NULL, 
		rect.left - GetSystemMetrics(SM_CYFIXEDFRAME), 
		rect.top - GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CXFIXEDFRAME), 
		rect.Width() + 2 * GetSystemMetrics(SM_CYFIXEDFRAME)+1, 
		rect.Height() + GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CXFIXEDFRAME), 
		SWP_NOZORDER | SWP_NOREDRAW);

	ExecJS(L"decorate()");
	UnlockWindowUpdate();
	UpdateWindow();


	if (m_bProgramStarting)
		AfxGetMainWnd()->ShowWindow(SW_SHOW);
}

void CDlgHtSplash::Undecorate()
{
	if (!IsDecorated()) return;

	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG nExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);

	LockWindowUpdate();
	CRect rect;
	GetWindowRect(&rect);
	SetWindowLong(m_hWnd, GWL_STYLE, nStyle & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | DS_FIXEDSYS | DS_MODALFRAME));
	SetWindowLong(m_hWnd, GWL_EXSTYLE, nExStyle & ~WS_EX_DLGMODALFRAME);
	SetWindowPos(NULL, 
		rect.left + GetSystemMetrics(SM_CYFIXEDFRAME), 
		rect.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFIXEDFRAME), 
		rect.Width() - 2 * GetSystemMetrics(SM_CYFIXEDFRAME)-1, 
		rect.Height() - GetSystemMetrics(SM_CYCAPTION) - 2 * GetSystemMetrics(SM_CXFIXEDFRAME), 
		SWP_NOZORDER | SWP_NOREDRAW);

	ExecJS(L"undecorate()");
	UnlockWindowUpdate();
	UpdateWindow();
}

BOOL CDlgHtSplash::OnInitDialog()
{
	CDlgHtOutText::OnInitDialog();

	m_bProgramStarting = !AfxGetMainWnd()->IsWindowVisible();
	Undecorate();

	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(RGB(255, 0, 0), 128, LWA_COLORKEY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgHtSplash, CDlgHtOutText)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtSplash)
END_DHTML_EVENT_MAP()

void CDlgHtSplash::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID)
	{
	case SC_CLOSE: 
		ExecJS(L"displayWaitMsg('STOPPING...')");
		Undecorate(); 
		m_bQuitRequested = true; 
		Sleep(250); 
		break;
	case SC_MINIMIZE: 
		CDlgHtOutText::OnSysCommand(nID, lParam);
		ShowWindow(SW_HIDE); 
		break;
	default: 
		CDlgHtOutText::OnSysCommand(nID, lParam);
	}
}
