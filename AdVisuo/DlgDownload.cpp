// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgDownload.h"
#include "VisProject.h"
#include "XMLRequest.h"


IMPLEMENT_DYNAMIC(CDlgDownload, CDialog)

CDlgDownload::CDlgDownload(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDownload::IDD, pParent)
{
	m_server = _T("");
	m_details = _T("");
}

CDlgDownload::~CDlgDownload()
{
	for each (CProjectVis *pPrj in m_prjs)
		delete pPrj;
	m_prjs.clear();
}

void CDlgDownload::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO1, m_server);
	DDX_Control(pDX, IDC_COMBO1, m_combo);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_DETAILS, m_details);
}


BEGIN_MESSAGE_MAP(CDlgDownload, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgDownload::OnBnClickedOk)
	ON_BN_CLICKED(IDC_REFRESH, &CDlgDownload::OnBnClickedRefresh)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgDownload::OnSelchangeCombo)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CDlgDownload::OnColumnclickList)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDlgDownload::OnClickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDlgDownload::OnDblclkList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CDlgDownload::OnItemchangedList)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST1, &CDlgDownload::OnGetinfotipList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		int nRes;
		CProjectVis *pPrj1 = (CProjectVis*)lParam1, *pPrj2 = (CProjectVis*)lParam2;
		switch (lParamSort % 100)
		{
			case 0: nRes = pPrj1->GetSimulationId() < pPrj2->GetSimulationId() ? -1 : (pPrj1->GetSimulationId() > pPrj2->GetSimulationId() ? 1 : 0); break;
			case 1: nRes = wcscmp(pPrj1->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str(), pPrj2->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str()); break;
			case 2: nRes = wcscmp(pPrj1->GetProjectInfo(CProjectVis::PRJ_BUILDING_NAME).c_str(), pPrj2->GetProjectInfo(CProjectVis::PRJ_BUILDING_NAME).c_str()); break;
		}
		return lParamSort < 100 ? nRes : -nRes;
	}

BOOL CDlgDownload::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style = cs.style | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_INFOTIP;
	return CDialog::PreCreateWindow(cs);
}

BOOL CDlgDownload::OnInitDialog()
{
	CDialog::OnInitDialog();

	// initial setup for the list control
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	m_list.InsertColumn(0, L"id");
	m_list.InsertColumn(1, L"Project Title");
	m_list.InsertColumn(2, L"Building");

	m_list.SetColumnWidth(0, 35);
	m_list.SetColumnWidth(1, 250);
	m_list.SetColumnWidth(2, 160);

	m_nSort = 0;
	m_bAscending[0] = false;
	m_bAscending[1] = m_bAscending[2] = true;

	// tokenize and initialise server names
	int curPos = 0;
	CString token = m_servers.Tokenize(_T(";"), curPos);
	m_server = token;
	while (token != _T(""))
	{
		m_combo.AddString(token);
		token = m_servers.Tokenize(_T(";"), curPos);
	};
	UpdateData(0);

	// post message for loading project titles
	::PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_REFRESH, BN_CLICKED), (LPARAM)0);

	return TRUE;
}

void CDlgDownload::OnBnClickedRefresh()
{
	if (m_server.IsEmpty())
		return;

	// show wait state...
	CWaitCursor wait;
	m_list.DeleteAllItems();

	for each (CProjectVis *pPrj in m_prjs)
		delete pPrj;
	m_prjs.clear();

	ShowWindow(SW_SHOW);
	UpdateWindow();

	UpdateData();
	m_details = L"";
	UpdateData(FALSE);

	CString url;
	url.Format(L"http://%s/advsrv.asmx", m_server);

	// Download available projects
	std::wstringstream str;
	CXMLRequest http;
	try
	{
		http.setURL((LPCTSTR)url);
		http.AVIndex();

		std::wstring response = http.response();
		CProjectVis::LoadIndexFromBuf(http.response().c_str(), m_prjs);
		
		m_list.DeleteAllItems();
		for (unsigned i = 0; i < m_prjs.size(); i++)
		{
			CProjectVis *pPrj = m_prjs[i];
			LV_ITEM lvItem;
			lvItem.mask = LVIF_TEXT;
			wchar_t buf[1025];
			lvItem.cchTextMax = 1024;
			_snwprintf_s(buf, 80, L"%d", pPrj->GetSimulationId());
			lvItem.iItem = m_list.InsertItem(0, buf);
			m_list.SetItemData(lvItem.iItem, (DWORD_PTR)pPrj);
			lvItem.iSubItem = 1;
			_snwprintf_s(buf, 1024, L"%ls", (LPWSTR)pPrj->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str());
			lvItem.pszText = buf;
			m_list.SetItem(&lvItem);
			lvItem.iSubItem = 2;
			_snwprintf_s(buf, 1024, L"%ls", (LPWSTR)pPrj->GetProjectInfo(CProjectVis::PRJ_BUILDING_NAME).c_str());
			lvItem.pszText = buf;
			m_list.SetItem(&lvItem);
		}
		m_list.SortItems(CompareFunc, m_bAscending[m_nSort] ? m_nSort : 100 + m_nSort);
		m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_list.SetSelectionMark(0);

		return;

	}
	catch (_prj_error pe)
	{
		str << "Error while analysing downloaded data: " << pe.ErrorMessage() << ".";
	}
	catch (_com_error ce)
	{
		str << "System error while downloading from " << http.URL() << ":" << endl;
		if ((wchar_t*)ce.Description())
			str << ce.Description();
		else
			str << ce.ErrorMessage();
	}
	catch (_xmlreq_error xe)
	{
		str << L"HTTP error " << xe.status() << L": " << xe.msg() << L" at " << http.URL() << L".";
	}
	catch(...)
	{
		str << L"Unidentified errors while downloading from " << http.URL();
	}
	http.reset();
	m_list.DeleteAllItems();
	Debug(str.str().c_str());
	AfxMessageBox(str.str().c_str(), MB_OK | MB_ICONHAND);
}


void CDlgDownload::OnSelchangeCombo()
{
	::PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_REFRESH, BN_CLICKED), (LPARAM)0);
}

void CDlgDownload::OnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// ascending or descending?
	if (m_nSort == pNMLV->iSubItem)
		m_bAscending[m_nSort] = !m_bAscending[m_nSort];
	else
		m_nSort = pNMLV->iSubItem;

	m_list.SortItems(CompareFunc, m_bAscending[m_nSort] ? m_nSort : 100 + m_nSort);

	*pResult = 0;
}

static CString _hlpStr(std::wstring str, std::wstring def) { if (str.empty()) return def.c_str(); else return str.c_str(); }
static CString _hlpStr(std::wstring str) { if (str.empty()) return L""; CString s; s.Format(L", %s", str.c_str()); return s; }

void CDlgDownload::OnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	UpdateData();
	CProjectVis *pPrj = (CProjectVis*)m_list.GetItemData(pNMLV->iItem);
	m_details.Format(
		L"Project: %s\n\r"
		L"Building: %s (%d lift groups)\n\r"
		L"Client: %s%s%s%s%s"
		, 
		_hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME), L"<untitled project>"),
		_hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_BUILDING_NAME), L"lift group name"), pPrj->GetLiftGroupsCount(),
		_hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_COMPANY), L"<unknown>"), _hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_CITY)), _hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_POST_CODE)), _hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_COUNTY)), _hlpStr(pPrj->GetProjectInfo(CProjectVis::PRJ_COUNTRY))
		);

	CString str;
	if (!pPrj->GetProjectInfo(CProjectVis::PRJ_DESIGNER).empty())	 { str.Format(L"\n\rDesigned by: %s", pPrj->GetProjectInfo(CProjectVis::PRJ_DESIGNER).c_str()); m_details += str; }
	if (!pPrj->GetProjectInfo(CProjectVis::PRJ_CHECKED_BY).empty()) { str.Format(L"\n\rChecked by: %s", pPrj->GetProjectInfo(CProjectVis::PRJ_CHECKED_BY).c_str()); m_details += str; }

	UpdateData(0);

	*pResult = 0;
}

void CDlgDownload::OnClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

void CDlgDownload::OnDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBnClickedOk();
	*pResult = 0;
}

void CDlgDownload::OnBnClickedOk()
{
	UpdateData();

	// obtain item data
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (!pos) return;
	int nPos = m_list.GetNextSelectedItem(pos);
	if (nPos < 0) return;
	CProjectVis *pPrj = (CProjectVis*)m_list.GetItemData(nPos);
	if (!pPrj) return;

	m_servers = m_server;
	for (int i = 0; i < min(m_combo.GetCount(), 5); i++)
	{
		CString txt;
		m_combo.GetLBText(i, txt);

		if (txt == m_server)
			continue;

		m_servers += ";";
		m_servers += txt;
	}

	m_url.Format(L"http://%s/advsrv.asmx?request=%d", m_server, pPrj->GetSimulationId());

	CDialog::OnOK();
}

void CDlgDownload::OnGetinfotipList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	pGetInfoTip->pszText = L"Ala ma kota";
	*pResult = 0;
}

void CDlgDownload::OnDestroy()
{
	CDialog::OnDestroy();
}

