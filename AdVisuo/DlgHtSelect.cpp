// DlgHtLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtSelect.h"
#include "VisProject.h"

// CDlgHtSelect dialog

IMPLEMENT_DYNCREATE(CDlgHtSelect, CDlgHtBase)

CDlgHtSelect::CDlgHtSelect(std::vector<CProjectVis*> *pPrjs, CWnd* pParent)
	: CDlgHtBase(IDD, IDH)
{
	m_pPrjs = pPrjs;
	m_nProjectId = 0;
	m_fnLoadComplHandle = [this] { };
}

CDlgHtSelect::~CDlgHtSelect()
{
}

void CDlgHtSelect::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtBase::DoDataExchange(pDX);
	DDX_DHtml_ElementText(pDX, _T("idProjectId"), DISPID_A_VALUE, m_nProjectId); 
}

BEGIN_MESSAGE_MAP(CDlgHtSelect, CDlgHtBase)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtSelect)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("idOpenButton"), OnButtonOpen)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtSelect, CDlgHtBase)
END_DISPATCH_MAP()

////////////////////////////////////////////////////////////////////////
// Javascript Bounding

////////////////////////////////////////////////////////////////////////
// Button Handlers

void CDlgHtSelect::OnOK()
{
	CDlgHtBase::OnOK();
}

void CDlgHtSelect::OnCancel()
{
	CDlgHtBase::OnCancel();
}

HRESULT CDlgHtSelect::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonOpen(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

