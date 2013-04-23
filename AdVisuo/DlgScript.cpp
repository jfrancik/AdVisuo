// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgScript.h"
#include "script.h"

// CDlgScript dialog

CDlgScript *CDlgScript::c_dlg = NULL;

IMPLEMENT_DYNAMIC(CDlgScript, CDialogEx)

CDlgScript::CDlgScript(CScript *pScript, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgScript::IDD, pParent), m_pScript(pScript)
{
}

CDlgScript::~CDlgScript()
{
}

void CDlgScript::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_list);
}


BEGIN_MESSAGE_MAP(CDlgScript, CDialogEx)
	ON_BN_CLICKED(IDC_ADD_EVENT, &CDlgScript::OnBnClickedAddEvent)
	ON_LBN_SELCHANGE(IDC_LIST2, &CDlgScript::OnLbnSelchangeList2)
	ON_BN_CLICKED(IDC_PLAY_EVENT, &CDlgScript::OnBnClickedPlayEvent)
	ON_BN_CLICKED(IDC_REMOVE_EVENT, &CDlgScript::OnBnClickedRemoveEvent)
	ON_BN_CLICKED(IDCANCEL, &CDlgScript::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_PROPERTIES, &CDlgScript::OnBnClickedProperties)
	ON_BN_CLICKED(IDC_LOAD, &CDlgScript::OnBnClickedLoad)
	ON_BN_CLICKED(IDC_SAVE, &CDlgScript::OnBnClickedSave)
END_MESSAGE_MAP()


// CDlgScript message handlers


BOOL CDlgScript::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	PopulateList();
	c_dlg = this;

	return TRUE;
}


void CDlgScript::PostNcDestroy()
{
	c_dlg = NULL;
	CDialogEx::PostNcDestroy();
	delete this;
}

void CDlgScript::PopulateList()
{
	m_list.ResetContent();
	for (AVULONG i = 0; i < m_pScript->size(); i++)
	{
		std::wstringstream s;
		s << i+1 << L". " << std::wstring((*m_pScript)[i]->GetDesc());
		m_list.AddString(s.str().c_str());
	}
	GetDlgItem(IDC_PLAY_EVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOVE_EVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROPERTIES)->EnableWindow(FALSE);
}

void CDlgScript::OnLbnSelchangeList2()
{
	GetDlgItem(IDC_PLAY_EVENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_REMOVE_EVENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_PROPERTIES)->EnableWindow(TRUE);
}

void CDlgScript::OnBnClickedAddEvent()
{
	m_pScript->Record();
	PopulateList();
}


void CDlgScript::OnBnClickedPlayEvent()
{
	if (m_list.GetCurSel() >= 0)
		m_pScript->Play(m_list.GetCurSel());
}


void CDlgScript::OnBnClickedRemoveEvent()
{
	if (m_list.GetCurSel() >= 0)
		m_pScript->Delete(m_list.GetCurSel());
	PopulateList();
}


void CDlgScript::OnBnClickedCancel()
{
	ShowWindow(SW_HIDE);
}


void CDlgScript::OnBnClickedProperties()
{
	if (m_list.GetCurSel() >= 0)
	{
		CDlgScriptProperties dlg((*m_pScript)[m_list.GetCurSel()]);
		if (dlg.DoModal() == IDOK)
		{
			m_pScript->Sort();
			PopulateList();
		}
	}
}

void CDlgScript::OnBnClickedLoad()
{
	CFileDialog dlg(TRUE, L"scr");
	if (dlg.DoModal() == IDOK)
	{
		CFile f(dlg.GetPathName(), CFile::modeRead);
		CArchive ar(&f, CArchive::load);
		m_pScript->Serialize(ar);
		PopulateList();
	}
}


void CDlgScript::OnBnClickedSave()
{
	CFileDialog dlg(FALSE, L"scr");
	if (dlg.DoModal() == IDOK)
	{
		CFile f(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite);
		CArchive ar(&f, CArchive::store);
		m_pScript->Serialize(ar);
	}
}

// CDlgScriptProperties dialog

IMPLEMENT_DYNAMIC(CDlgScriptProperties, CDialogEx)

CDlgScriptProperties::CDlgScriptProperties(CScriptEvent *pEvent, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgScriptProperties::IDD, pParent)
	, m_time(0)
	, m_nAnim(0)
	, m_bFF(FALSE)
	, m_timeFF(0)
	, m_pEvent(pEvent)
	, m_desc(_T(""))
	, m_fAccel(0)
{
	m_desc = m_pEvent->GetDesc();
	m_time = CTime(2011, 7, 30, (m_pEvent->GetTime() / 3600000) % 24, (m_pEvent->GetTime() / 60000) % 60, (m_pEvent->GetTime() / 1000) % 60);
	m_nAnim = m_pEvent->GetAnimTime();
	m_bFF = m_pEvent->GetFFTime() > 0;
	m_timeFF = CTime(2011, 7, 30, (m_pEvent->GetFFTime() / 3600000) % 24, (m_pEvent->GetFFTime() / 60000) % 60, (m_pEvent->GetFFTime() / 1000) % 60);
	m_fAccel = m_pEvent->GetAccel();
}

CDlgScriptProperties::~CDlgScriptProperties()
{
}

void CDlgScriptProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctrlTime);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_time);
	DDX_Text(pDX, IDC_EDIT3, m_nAnim);
	DDV_MinMaxInt(pDX, m_nAnim, 0, 60000);
	DDX_Check(pDX, IDC_CHECK1, m_bFF);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_ctrlTimeFF);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_timeFF);
	DDX_Text(pDX, IDC_EDIT1, m_desc);
	DDX_Text(pDX, IDC_EDIT2, m_fAccel);
	DDV_MinMaxFloat(pDX, m_fAccel, 0.1f, 10);
}


BEGIN_MESSAGE_MAP(CDlgScriptProperties, CDialogEx)
END_MESSAGE_MAP()


// CDlgScriptProperties message handlers


BOOL CDlgScriptProperties::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}


void CDlgScriptProperties::OnOK()
{
	UpdateData();

	m_pEvent->SetTime(m_time.GetSecond() * 1000 + m_time.GetMinute() * 60000 + m_time.GetHour() * 3600000);
	m_pEvent->SetAnimTime(m_nAnim);
	m_pEvent->SetAccel(m_fAccel);
	if (m_bFF) 
		m_pEvent->SetFFTime(m_timeFF.GetSecond() * 1000 + m_timeFF.GetMinute() * 60000 + m_timeFF.GetHour() * 3600000);

	CDialogEx::OnOK();
}




