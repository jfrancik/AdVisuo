// DlgSplash.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgSplash.h"


// CDlgSplash dialog

IMPLEMENT_DYNCREATE(CDlgSplash, CDlgHtBase)

CDlgSplash::CDlgSplash(CWnd* pParent /*=NULL*/)
	: CDlgHtBase(CDlgSplash::IDD, CDlgSplash::IDH, pParent)
{

}

CDlgSplash::~CDlgSplash()
{
}

void CDlgSplash::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtBase::DoDataExchange(pDX);
}

BOOL CDlgSplash::OnInitDialog()
{
	CDlgHtBase::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgSplash, CDlgHtBase)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgSplash)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CDlgSplash message handlers

HRESULT CDlgSplash::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgSplash::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
