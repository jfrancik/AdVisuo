// DlgSplash.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtAbout.h"

// CDlgHtAbout dialog

IMPLEMENT_DYNCREATE(CDlgHtAbout, CDlgHtBase)

void CDlgHtAbout::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtBase::DoDataExchange(pDX);
}

BOOL CDlgHtAbout::OnInitDialog()
{
	CDlgHtBase::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgHtAbout, CDlgHtBase)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtAbout)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
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

IMPLEMENT_DYNCREATE(CDlgHtSplash, CDlgHtAbout)

BOOL CDlgHtSplash::OnInitDialog()
{
	CDlgHtAbout::OnInitDialog();

	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(RGB(255, 0, 0), 128, LWA_COLORKEY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgHtSplash, CDlgHtAbout)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtSplash)
END_DHTML_EVENT_MAP()
