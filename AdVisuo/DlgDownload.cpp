// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgDownload.h"
#include "VisProject.h"
#include "XMLRequest.h"


IMPLEMENT_DYNAMIC(CDlgDownload, CDialog)

CDlgDownload::CDlgDownload(std::vector<CProjectVis*> &prjs, CWnd* pParent /*=NULL*/)
	: m_prjs(prjs), CDialog(CDlgDownload::IDD, pParent)
{
	m_details = L"";
	m_nProjectId = 0;
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
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_DETAILS, m_details);
}


BEGIN_MESSAGE_MAP(CDlgDownload, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgDownload::OnBnClickedOk)
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

	UpdateData(0);



	// show wait state...
	CWaitCursor wait;
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

	return true;
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

	m_nProjectId = pPrj->GetSimulationId();

	AfxGetApp()->m_pMainWnd = NULL;
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

