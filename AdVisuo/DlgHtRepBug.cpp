// DlgRepBug.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgHtRepBug.h"
#include "AdVisuoDoc.h"
#include "AdVisuo.h"
#include "Engine.h"

using namespace std;

IMPLEMENT_DYNAMIC(CDlgHtRepBug, CDlgHtBase)

CDlgHtRepBug::CDlgHtRepBug(CString msg, CWnd* pParent /*=NULL*/)
	: CDlgHtBase(CDlgHtRepBug::IDD, CDlgHtRepBug::IDH, pParent)
{
	m_desc = _T("");
	m_sys = _T("");
	m_errormsg = msg;
	m_cat = 0;
}

CDlgHtRepBug::~CDlgHtRepBug()
{
}

void CDlgHtRepBug::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtBase::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DESC, m_desc);
	DDX_Control(pDX, IDC_COMBO_CAT, m_combo);
	DDX_Text(pDX, IDC_EDIT_SYSTEM, m_sys);
	DDX_CBIndex(pDX, IDC_COMBO_CAT, m_cat);
}


BEGIN_MESSAGE_MAP(CDlgHtRepBug, CDlgHtBase)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtRepBug)
	DHTML_EVENT_ONCLICK(_T("idSendButton"), OnButtonSend)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
END_DHTML_EVENT_MAP()

// CDlgHtRepBug message handlers

BOOL CDlgHtRepBug::OnInitDialog()
{
	CDlgHtBase::OnInitDialog();

	m_combo.AddString(L"--- please choose a category ---");
	m_combo.AddString(L"Critical error - application crashes");
	m_combo.AddString(L"Major error - wrong behaviour of the program");
	m_combo.AddString(L"Minor problem");
	m_combo.AddString(L"Missing functionality");
	m_combo.AddString(L"Proposition of extension or change");
	m_combo.SetCurSel(0);

	// diagnostic string
	CAdVisuoDoc *pDoc = NULL;
	CFrameWnd *pFrame = ((CFrameWnd*)AfxGetMainWnd())->GetActiveFrame();
	if (pFrame) pDoc = (CAdVisuoDoc*)pFrame->GetActiveDocument();

	if (pDoc)
	{
		m_diagnostic = (pDoc && pDoc->GetEngine()) ? pDoc->GetEngine()->GetDiagnosticMessage() : L"";
		m_nId = pDoc->GetProject()->GetSimulationId();
		m_path = pDoc->GetPathInfo();
	}
	else
	{
		m_diagnostic = L"No document loaded.";
		m_nId = 0;
	}
	
	if (m_errormsg.IsEmpty())
		m_sys = m_diagnostic;
	else
		m_sys += m_diagnostic + L"\r\n" + m_errormsg;

	UpdateData(FALSE);

	return TRUE;
}

HRESULT CDlgHtRepBug::OnButtonSend(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgHtRepBug::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

wstring char2hex( wchar_t dec )
{
    wchar_t dig1 = (dec&0xF0)>>4;
    wchar_t dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    wstring r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

void CDlgHtRepBug::OnOK()
{
	if (!this->UpdateData()) return;

	m_desc.Trim();

	if (m_cat == 0)
	{
		AfxMessageBox(L"Please choose the category");
		return;
	}
	if (m_desc.IsEmpty())
	{
		AfxMessageBox(L"Please use the space provided to describe the case");
		return;
	}

	CWaitCursor crs;
	if (AVGetApp()->Report(m_nId, (LPOLESTR)(LPCTSTR)m_path, m_cat, (LPOLESTR)(LPCTSTR)m_desc, (LPOLESTR)(LPCTSTR)m_diagnostic, (LPOLESTR)(LPCTSTR)m_errormsg))
		AfxMessageBox(L"Message sent, thank you!", MB_ICONINFORMATION);

	CDlgHtBase::OnOK();
}