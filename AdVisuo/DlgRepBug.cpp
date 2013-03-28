// DlgRepBug.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgRepBug.h"
#include "AdVisuoDoc.h"
#include "AdVisuo.h"

IMPLEMENT_DYNAMIC(CDlgReportBug, CDialogEx)

CDlgReportBug::CDlgReportBug(CString msg, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgReportBug::IDD, pParent)
{
	m_name = _T("");
	m_email = _T("");
	m_cat = 0;
	m_desc = _T("");
	m_system = _T("");
	m_msg = msg;
}

CDlgReportBug::~CDlgReportBug()
{
}

void CDlgReportBug::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_EMAIL, m_email);
	DDX_CBIndex(pDX, IDC_COMBO_CAT, m_cat);
	DDX_Text(pDX, IDC_EDIT_DESC, m_desc);
	DDX_Control(pDX, IDC_COMBO_CAT, m_combo);
	DDX_Text(pDX, IDC_EDIT_SYSTEM, m_system);
}


BEGIN_MESSAGE_MAP(CDlgReportBug, CDialogEx)
END_MESSAGE_MAP()


// CDlgReportBug message handlers

BOOL CDlgReportBug::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	CString str;
	if (pDoc)
		str.Format(L"%s\r\nSession id: %d", pDoc->GetDiagnosticMessage(), ((CAdVisuoApp*)AfxGetApp())->GetSessionId());
	else
		str.Format(L"No document loaded.\r\nSession id: %d", ((CAdVisuoApp*)AfxGetApp())->GetSessionId());
	
	if (m_msg.IsEmpty())
		m_system = str;
	else
		m_system.Format(L"Error Message: %s\r\n%s", m_msg, str);

	UpdateData(FALSE);

	return TRUE;
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

wstring urlencode(wstring &c)
{
    wstring escaped=L"";
    int max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]==L'~' || c[i]==L'!' || c[i]==L'*' || c[i]==L'(' || c[i]==L')' || c[i]==L'\'')
        )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append(L"%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

void CDlgReportBug::Report(int nReason, CString message)
{
	CString req;
	req.Format(L"reason=%d&id=%d&msg=%s", nReason, ((CAdVisuoApp*)AfxGetApp())->GetSessionId(), urlencode((wstring)message));
	wstring url, function, request;
	url = L"http://francik.name/advisuo";
	function = L"report.php";
	request = (wstring)req;
	CXMLRequest http;
	try
	{
		http.setURL(url);
		http.call(function, request);
		std::wstring response = http.response();
	}
	catch(...)
	{
	}
	http.reset();
}


void CDlgReportBug::OnOK()
{
	if (!this->UpdateData()) return;

	m_name.Trim();
	m_email.Trim();
	m_desc.Trim();

	if (m_name.IsEmpty() && m_email.IsEmpty())
		if (AfxMessageBox(L"Without any contact details we will be unable to contact you in case more information is required. Do you want to send this message anyway?", MB_YESNO | MB_DEFBUTTON2) != IDYES)
			return;
	if (m_desc.IsEmpty())
	{
		AfxMessageBox(L"Please provide description of the case");
		return;
	}

	CWaitCursor crs;


	wstring name = urlencode((wstring)m_name);
	wstring email = urlencode((wstring)m_email);
	wstring desc = urlencode((wstring)m_desc);
	wstring sys = urlencode((wstring)m_system);

	CString cat, req;
	m_combo.GetWindowText(cat);
	req.Format(L"name=%s&email=%s&subject=%s&comments=%s%%0d%%0a%%0d%%0aSystem message follows:%%0d%%0a%%0d%%0a%s", name.c_str(), email.c_str(), cat, desc.c_str(), sys.c_str()); 

	wstring url, function, request;
	url = L"http://francik.name/advisuo";
	function = L"mailer.php";
	request = (wstring)req;

	std::wstringstream str;
	CXMLRequest http;
	try
	{
		http.setURL(url);
		http.call(function, request);
		std::wstring response = http.response();
	}
	catch (_com_error ce)
	{
		str << "System error while trying to initiate the Internet connection: ";
		if ((wchar_t*)ce.Description())
			str << ce.Description();
		else
			str << ce.ErrorMessage();
		AfxMessageBox(str.str().c_str(), MB_ICONEXCLAMATION);
		return;
	}
	catch (_xmlreq_error xe)
	{
		str << L"HTTP error " << xe.status() << L": " << xe.msg() << L" at " << http.URL() << L".";
		AfxMessageBox(str.str().c_str(), MB_ICONEXCLAMATION);
		return;
	}
	catch(...)
	{
		str << L"Unidentified errors while downloading from " << http.URL();
		AfxMessageBox(str.str().c_str(), MB_ICONEXCLAMATION);
		return;
	}
	http.reset();
	CDialogEx::OnOK();
	AfxMessageBox(L"Message sent, thank you!", MB_ICONINFORMATION);
}
