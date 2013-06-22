// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgLogin.h"


// CDlgLogin dialog

IMPLEMENT_DYNCREATE(CDlgLogin, CDHtmlDialog)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDlgLogin::IDD, CDlgLogin::IDH, pParent)
{

}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_ElementText(pDX, _T("idUserName"), DISPID_A_VALUE, m_strUsername); 
	DDX_DHtml_ElementText(pDX, _T("idPassword"), DISPID_A_VALUE, m_strPassword); 
}

BOOL CDlgLogin::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();


	
	CString str = _T("jarekf");
    BSTR bstr;
    bstr = str.AllocSysString();   
    
    SetElementText(L"ctrlLogin_UserName", bstr);       



	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgLogin, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgLogin)
	DHTML_EVENT_ONCLICK(_T("idLoginButton"), OnButtonLogin)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("idConnButton"), OnButtonConnection)
END_DHTML_EVENT_MAP()



// CDlgLogin message handlers

HRESULT CDlgLogin::OnButtonLogin(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgLogin::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

HRESULT CDlgLogin::OnButtonConnection(IHTMLElement* /*pElement*/)
{
	LoadFromResource(IDR_HTML_DLGCONNECTION);
	SetWindowText(L"Setup Server Connection");
	return S_OK;
}

void CDlgLogin::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

    //CString str = _T("jarekf");
    //BSTR bstr;
    //bstr = str.AllocSysString();   
    //
    //SetElementText(L"idUserName", bstr);       

    //str = _T("sv_penguin125");
    //bstr = str.AllocSysString();   
    //
    //SetElementText(L"idPassword", bstr);       
}
