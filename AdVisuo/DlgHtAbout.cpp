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

BOOL CDlgHtSplash::OnInitDialog()
{
	CDlgHtOutText::OnInitDialog();

	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(RGB(255, 0, 0), 128, LWA_COLORKEY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgHtSplash, CDlgHtOutText)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtSplash)
END_DHTML_EVENT_MAP()
