// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuo.h"
#include "Dialogs.h"

#include "sim.h"
#include "XMLRequest.h"
#include "screen.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <afxtaskdialog.h>

#define RATIO_16_9_x1000	1777
#define RATIO_16_10_x1000	1600
#define RATIO_4_3_x1000		1333
#define half_of(x)			(int)((float)(x) / 2.0f + 0.5f)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgVideo dialog

void CMFCEditBrowseCtrlEx::OnBrowse()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	if (m_Mode != BrowseMode_File || !GetSaveMode())
		CMFCEditBrowseCtrl::OnBrowse();
	else
	{
		CString strFile;
		GetWindowText(strFile);

		if (!strFile.IsEmpty())
		{
			TCHAR fname [_MAX_FNAME];

			_tsplitpath_s(strFile, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0);

			CString strFileName = fname;
			strFileName.TrimLeft();
			strFileName.TrimRight();

			if (strFileName.IsEmpty())
			{
				strFile.Empty();
			}

			const CString strInvalidChars = _T("*?<>|");
			if (strFile.FindOneOf(strInvalidChars) >= 0)
			{
				if (!OnIllegalFileName(strFile))
				{
					SetFocus();
					return;
				}
			}
		}

		CFileDialog dlg(FALSE, !m_strDefFileExt.IsEmpty() ? (LPCTSTR)m_strDefFileExt : (LPCTSTR)NULL, strFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
			!m_strFileFilter.IsEmpty() ? (LPCTSTR)m_strFileFilter : (LPCTSTR)NULL, NULL);
		if (dlg.DoModal() == IDOK)
		{
			m_bConfirmed = true;
			if (strFile != dlg.GetPathName())
			{
				SetWindowText(dlg.GetPathName());
				SetModify(TRUE);
				OnAfterUpdate();
			}
		}

		if (GetParent() != NULL)
		{
			GetParent()->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}

	SetFocus();
}



IMPLEMENT_DYNAMIC(CDlgVideo, CDialog)

CDlgVideo::RESITEM CDlgVideo::c_resitems[] =
{
	{ 0, 1920, 1080, L"HD 1080" },
	{ 0, 1600, 900, L"HD+" },
	{ 0, 1280, 720, L"HD 720" },
	{ 0, 854, 480, L"480p" },
	{ 0, 640, 360, L"360p" },
	{ 0, 427, 240, L"240p" },

	{ 1, 1920, 1200, L"WUXGA" },
	{ 1, 1680, 1050, L"WSXGA+" },
	{ 1, 1440, 900, L"900p" },
	{ 1, 1280, 800, L"WXGA" },
	{ 1, 768, 480, L"WVGA" },
	{ 1, 320, 200, L"CGA" },

	{ 2, 1600, 1200, L"UXGA" },
	{ 2, 1400, 1050, L"SXGA+" },
	{ 2, 1024, 768, L"XGA" },
	{ 2, 800, 600, L"SVGA" },
	{ 2, 640, 480, L"VGA" },
	{ 2, 320, 240, L"QVGA" },
};

CDlgVideo::CDlgVideo(AVFLOAT fAspectRatio, AVULONG nFPS, CString path, 
					 AVULONG nTimeFrom, AVULONG nTimeTo, 
					 std::function<void(AVFLOAT fAspectRatio)> CBSwitchAspectRatio,
					 std::function<void()> CBRevertAspectRatio,
					 CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVideo::IDD, pParent)
{
	m_fAspectRatio = fAspectRatio;
	m_fViewAspectRatio = fAspectRatio;
	switch ((AVULONG)(m_fAspectRatio * 1000))
	{
	case RATIO_16_9_x1000:	break; 
	case RATIO_16_10_x1000:	break;
	case RATIO_4_3_x1000:	break;
	default:				m_fAspectRatio = 16.0f / 9.0f; break;
	}

	m_nResX = 1280;
	m_nResY = 720;
	m_radioFPS = 0;
	m_nFPS = nFPS;
	m_path = path; 
	m_nTimeFrom = nTimeFrom; 
	m_nTimeTo = nTimeTo;
	m_pLockOut = NULL;
	m_CBSwitchAspectRatio = CBSwitchAspectRatio;
	m_CBRevertAspectRatio = CBRevertAspectRatio;
	m_bInitialised = FALSE;
	m_bUseAspect = true;
	m_bShowCaptions = true;
	m_bShowClock = true;
}

CDlgVideo::~CDlgVideo()
{
}

void CDlgVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILEPATH, m_ctrlBrowse);
	DDX_Control(pDX, IDC_TIME_BEGIN, m_ctrlTimeFrom);
	DDX_Control(pDX, IDC_TIME_END, m_ctrlTimeTo);
	if (m_pLockOut != &m_nAspectRatio) DDX_Radio(pDX, IDC_RADIO_ASPECT_16_9, m_nAspectRatio);
	DDX_Control(pDX, IDC_COMBO_RES, m_comboRes);
	if (m_pLockOut != &m_nResX) DDX_Text(pDX, IDC_EDIT_RESX, m_nResX);
	if (m_pLockOut != &m_nResX) DDV_MinMaxInt(pDX, m_nResX, 1, 2048);
	if (m_pLockOut != &m_nResY) DDX_Text(pDX, IDC_EDIT_RESY, m_nResY);
	if (m_pLockOut != &m_nResY) DDV_MinMaxInt(pDX, m_nResY, 1, 2048);
	if (m_pLockOut != &m_radioFPS) DDX_Radio(pDX, IDC_RADIO_FPS_24, m_radioFPS);
	if (m_pLockOut != &m_nFPS) DDX_Text(pDX, IDC_EDIT_FPS, m_nFPS);
	if (m_pLockOut != &m_nFPS) DDV_MinMaxInt(pDX, m_nFPS, 1, 128);
	if (m_pLockOut != &m_fAspectRatio) 
		DDX_Text(pDX, IDC_EDIT_ASPECT, m_fAspectRatio);
	if (m_pLockOut != &m_fAspectRatio) 
		DDV_MinMaxFloat(pDX, m_fAspectRatio, 0.01f, 100);
	DDX_Control(pDX, IDC_SPIN1, m_spinFPS);
	DDX_Control(pDX, IDC_SPIN2, m_spinX);
	DDX_Control(pDX, IDC_SPIN3, m_spinY);
	DDX_Check(pDX, IDC_CHECK_CAPTIONS, m_bShowCaptions);
	DDX_Check(pDX, IDC_CHECK_CLOCK, m_bShowClock);
}


BEGIN_MESSAGE_MAP(CDlgVideo, CDialog)
	ON_BN_CLICKED(IDC_RADIO_ASPECT_16_9, &CDlgVideo::OnBnClickedRadioAspect_16_9)
	ON_BN_CLICKED(IDC_RADIO_ASPECT_16_10, &CDlgVideo::OnBnClickedRadioAspect_16_10)
	ON_BN_CLICKED(IDC_RADIO_ASPECT_4_3, &CDlgVideo::OnBnClickedRadioAspect_4_3)
	ON_BN_CLICKED(IDC_RADIO_ASPECT_CUSTOM, &CDlgVideo::OnBnClickedRadioAspectCustom)
	ON_BN_CLICKED(IDC_CUSTOM, &CDlgVideo::OnBnClickedCustom)
	ON_BN_CLICKED(IDC_SWITCH, &CDlgVideo::OnBnClickedSwitch)
	ON_CBN_SELCHANGE(IDC_COMBO_RES, &CDlgVideo::OnCbnSelchangeRes)
	ON_BN_CLICKED(IDC_RADIO_FPS_24, &CDlgVideo::OnBnClickedRadioFps24)
	ON_BN_CLICKED(IDC_RADIO_FPS_25P, &CDlgVideo::OnBnClickedRadioFps25)
	ON_BN_CLICKED(IDC_RADIO_FPS_30, &CDlgVideo::OnBnClickedRadioFps30)
	ON_BN_CLICKED(IDC_RADIO_FPS_50, &CDlgVideo::OnBnClickedRadioFps50)
	ON_BN_CLICKED(IDC_RADIO_FPS_60, &CDlgVideo::OnBnClickedRadioFps60)
	ON_EN_CHANGE(IDC_EDIT_RESX, &CDlgVideo::OnEnChangeEditResx)
	ON_EN_CHANGE(IDC_EDIT_RESY, &CDlgVideo::OnEnChangeEditResy)
	ON_EN_CHANGE(IDC_EDIT_FPS, &CDlgVideo::OnEnChangeEditFps)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_BEGIN, &CDlgVideo::OnDtnDatetimechangeTimeBegin)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_END, &CDlgVideo::OnDtnDatetimechangeTimeEnd)
	ON_EN_CHANGE(IDC_EDIT_ASPECT, &CDlgVideo::OnEnChangeEditAspect)
	ON_BN_CLICKED(IDC_REVERT, &CDlgVideo::OnBnClickedRevert)
	ON_BN_CLICKED(IDC_RADIO_FPS_CUSTOM, &CDlgVideo::OnBnClickedRadioFpsCustom)
	ON_BN_CLICKED(IDC_CHECK_RESTRICT, &CDlgVideo::OnBnClickedCheckRestrict)
END_MESSAGE_MAP()

// CDlgVideo message handlers
// Dialogs.cpp : implementation file
//

BOOL CDlgVideo::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetAspectRatio(m_fAspectRatio);
	SetFPS(m_nFPS);

	UpdateData(FALSE);

	GetDlgItem(IDC_REVERT)->ShowWindow(SW_HIDE);
	m_spinFPS.SetRange(1, 128);
	m_spinX.SetRange(1, 2047);
	m_spinY.SetRange(1, 2047);
	CheckDlgButton(IDC_CHECK_RESTRICT, 1);

	m_ctrlBrowse.EnableFileBrowseButton(L"avi", L"AVI Files (*.avi)|*.avi|All Files|*.*||");
	m_ctrlBrowse.SetSaveMode();
	m_ctrlBrowse.SetWindowText(m_path);

	CTime t1(2011, 7, 30, (m_nTimeFrom / 3600000) % 24, (m_nTimeFrom / 60000) % 60, (m_nTimeFrom / 1000) % 60);
	CTime t2(2011, 7, 30, (m_nTimeTo / 3600000) % 24, (m_nTimeTo / 60000) % 60, (m_nTimeTo / 1000) % 60);
	m_ctrlTimeFrom.SetTime(&t1);
	m_ctrlTimeTo.SetTime(&t2);
	m_ctrlTimeFrom.SetRange(&t1, &t2);
	m_ctrlTimeTo.SetRange(&t1, &t2);

	m_bInitialised = TRUE;

	return TRUE;
}


void CDlgVideo::OnOK()
{
	if (!UpdateData()) return;

	m_ctrlBrowse.GetWindowText(m_path);
	
	// if file exists
	ifstream ifile(m_path);
	if (ifile && !m_ctrlBrowse.IsPathConfirmed())
	{
		AVULONG nRes = IDCANCEL;
		CString strMessage((LPCSTR)IDS_DLGVIDEO_MESSAGE);
		CString strDialogTitle((LPCSTR)IDS_DLGVIDEO_TITLE);
		CString strMainInstruction((LPCSTR)IDS_DLGVIDEO_INSTR);
		CString strFooter((LPCSTR)IDS_DLGVIDEO_FOOTER);

		if (CTaskDialog::IsSupported())
		{
			CTaskDialog taskDialog(strMessage, strMainInstruction, strDialogTitle, IDS_DLGVIDEO_OPTION1, IDS_DLGVIDEO_OPTION2, TDCBF_CANCEL_BUTTON, 17, strFooter);
			taskDialog.SetMainIcon(TD_INFORMATION_ICON);
			switch (taskDialog.DoModal())
			{
				case IDS_DLGVIDEO_OPTION1: nRes = IDYES; break;
				case IDS_DLGVIDEO_OPTION2: nRes = IDNO; break;
				default: nRes = IDCANCEL; break;
			}
		}
		else
		{
			strMessage.LoadString(IDS_DLGVIDEO_MESSAGE2);
			nRes = AfxMessageBox(strMessage, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		}

		switch (nRes)
		{
		case IDYES: 
				m_ctrlBrowse.OnBrowse();
				m_ctrlBrowse.GetWindowText(m_path);
				break;
		case IDNO: 
				break;
		default: 
			return;
		}
	}

	CTime t;
	m_ctrlTimeFrom.GetTime(t);
	m_nTimeFrom = ((t.GetHour() * 60 + t.GetMinute()) * 60 + t.GetSecond()) * 1000;
	m_ctrlTimeTo.GetTime(t);
	m_nTimeTo = ((t.GetHour() * 60 + t.GetMinute()) * 60 + t.GetSecond()) * 1000;

	CDialog::OnOK();
}

void CDlgVideo::SetAspectRatio(float fAspectRatio)
{
	m_fAspectRatio = fAspectRatio;
	switch ((AVULONG)(m_fAspectRatio * 1000))
	{
	case RATIO_16_9_x1000:	m_nAspectRatio = 0; FillCombo(0); break; 
	case RATIO_16_10_x1000:	m_nAspectRatio = 1; FillCombo(1); break;
	case RATIO_4_3_x1000:	m_nAspectRatio = 2; FillCombo(2); break;
	default:				m_nAspectRatio = 3; FillCombo(3); break;
	}
}

void CDlgVideo::SetFPS(int nFPS)
{
	m_nFPS = nFPS;
	switch (m_nFPS)
	{
	case 24: m_radioFPS = 0; break;
	case 25: m_radioFPS = 1; break;
	case 30: m_radioFPS = 2; break;
	case 50: m_radioFPS = 3; break;
	case 60: m_radioFPS = 4; break;
	default: m_radioFPS = 5; break;
	}
}

void CDlgVideo::FillCombo(int nOption)
{
	m_comboRes.ResetContent();
	m_comboRes.AddString(L"choose standard resolution");
	int nNewSel = -1;
	for (int i = 0; i < sizeof(c_resitems) / sizeof(RESITEM); i++)
		if (nOption < 0 || c_resitems[i].nOption == nOption)
		{
			wchar_t buf[80]; _snwprintf_s(buf, 80, L"%s (%d x %d)", c_resitems[i].pText, c_resitems[i].x, c_resitems[i].y);
			int pos = m_comboRes.AddString(buf);
			m_comboRes.SetItemDataPtr(pos, &c_resitems[i]);

			if (nOption >= 0)
			{
				if (nNewSel == -1 || abs(((RESITEM*)m_comboRes.GetItemDataPtr(nNewSel))->y - m_nResY) > abs(c_resitems[i].y - m_nResY))
					nNewSel = pos;
			}
			else
			{
				if (nNewSel == -1 || abs(((RESITEM*)m_comboRes.GetItemDataPtr(nNewSel))->x - m_nResX) + abs(((RESITEM*)m_comboRes.GetItemDataPtr(nNewSel))->y - m_nResY) > abs(c_resitems[i].x - m_nResX) + abs(c_resitems[i].y - m_nResY))
					nNewSel = pos;
			}
		}

	if (nNewSel > 0)
	{
		m_comboRes.SetCurSel(nNewSel);
		RESITEM *pItem = (RESITEM*)m_comboRes.GetItemDataPtr(nNewSel);
		if (!pItem) return;
		m_nResX = pItem->x;
		m_nResY = pItem->y;
	}
	else
	{
		m_comboRes.ResetContent();
		m_comboRes.AddString(L"no standard resolution information");
		m_comboRes.SetCurSel(0);
		m_nResX = (int)((AVFLOAT)m_nResY * m_fAspectRatio + 0.5);
	}
}

void CDlgVideo::OnBnClickedRadioAspect_16_9()	{ SetAspectRatio(16.0f / 9.0f); UpdateData(&m_nAspectRatio, FALSE); }
void CDlgVideo::OnBnClickedRadioAspect_16_10()	{ SetAspectRatio(16.0f / 10.0f); UpdateData(&m_nAspectRatio, FALSE); }
void CDlgVideo::OnBnClickedRadioAspect_4_3()	{ SetAspectRatio(4.0f / 3.0f); UpdateData(&m_nAspectRatio, FALSE); }
void CDlgVideo::OnBnClickedRadioAspectCustom()	{ GetDlgItem(IDC_EDIT_ASPECT)->SetFocus(); }

void CDlgVideo::OnEnChangeEditAspect()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	SetAspectRatio(m_fAspectRatio);
	UpdateData(&m_fAspectRatio, FALSE);
}

void CDlgVideo::OnBnClickedCustom()
{
	CDlgAspectRatio dlg(m_fViewAspectRatio, (AVFLOAT)GetSystemMetrics(SM_CXSCREEN) / (AVFLOAT)GetSystemMetrics(SM_CYSCREEN));
	dlg.SetAspectRatio(m_fAspectRatio);
	if (dlg.DoModal() == IDOK)
	{
		SetAspectRatio(dlg.m_f3);
		UpdateData(FALSE);
	}
}

void CDlgVideo::OnBnClickedSwitch()
{
	m_CBSwitchAspectRatio(m_fAspectRatio);
	GetDlgItem(IDC_REVERT)->ShowWindow(SW_SHOW);
}

void CDlgVideo::OnBnClickedRevert()
{
	m_CBRevertAspectRatio();
	GetDlgItem(IDC_REVERT)->ShowWindow(SW_HIDE);
}

void CDlgVideo::OnCbnSelchangeRes()
{
	RESITEM *pItem = (RESITEM*)m_comboRes.GetItemDataPtr(m_comboRes.GetCurSel());
	if (!pItem) return;
	m_nResX = pItem->x;
	m_nResY = pItem->y;
	UpdateData(FALSE);
}

void CDlgVideo::OnEnChangeEditResx()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	if (m_bUseAspect)
	{
		m_nResY = (int)((AVFLOAT)m_nResX / m_fAspectRatio + 0.5);
		UpdateData(&m_nResX, FALSE);
	}

	m_comboRes.SetCurSel(0);
	for (int i = 0; i < m_comboRes.GetCount(); i++)
		if (m_comboRes.GetItemDataPtr(i) && ((RESITEM*)m_comboRes.GetItemDataPtr(i))->x == m_nResX && ((RESITEM*)m_comboRes.GetItemDataPtr(i))->y == m_nResY)
			m_comboRes.SetCurSel(i);
}

void CDlgVideo::OnEnChangeEditResy()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	if (m_bUseAspect)
	{
		m_nResX = (int)((AVFLOAT)m_nResY * m_fAspectRatio + 0.5);
		UpdateData(&m_nResY, FALSE);
	}

	m_comboRes.SetCurSel(0);
	for (int i = 0; i < m_comboRes.GetCount(); i++)
		if (m_comboRes.GetItemDataPtr(i) && ((RESITEM*)m_comboRes.GetItemDataPtr(i))->x == m_nResX && ((RESITEM*)m_comboRes.GetItemDataPtr(i))->y == m_nResY)
			m_comboRes.SetCurSel(i);
}

void CDlgVideo::OnBnClickedCheckRestrict()
{
	m_bUseAspect = (IsDlgButtonChecked(IDC_CHECK_RESTRICT) != 0);
	if (m_bUseAspect)
	{
		GetDlgItem(IDC_RADIO_ASPECT_16_9)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ASPECT_16_10)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ASPECT_4_3)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ASPECT_CUSTOM)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_ASPECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_SWITCH)->EnableWindow(TRUE);

		SetAspectRatio(m_fAspectRatio);
		UpdateData(FALSE);
	}
	else
	{
		GetDlgItem(IDC_RADIO_ASPECT_16_9)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ASPECT_16_10)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ASPECT_4_3)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ASPECT_CUSTOM)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_ASPECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_SWITCH)->EnableWindow(FALSE);

		FillCombo(-1);
	}
}

void CDlgVideo::OnBnClickedRadioFps24()		{ SetFPS(24); UpdateData(&m_radioFPS, FALSE); }
void CDlgVideo::OnBnClickedRadioFps25()		{ SetFPS(25); UpdateData(&m_radioFPS, FALSE); }
void CDlgVideo::OnBnClickedRadioFps30()		{ SetFPS(30); UpdateData(&m_radioFPS, FALSE); }
void CDlgVideo::OnBnClickedRadioFps50()		{ SetFPS(50); UpdateData(&m_radioFPS, FALSE); }
void CDlgVideo::OnBnClickedRadioFps60()		{ SetFPS(60); UpdateData(&m_radioFPS, FALSE); }
void CDlgVideo::OnBnClickedRadioFpsCustom()	{ GetDlgItem(IDC_EDIT_FPS)->SetFocus(); }

void CDlgVideo::OnEnChangeEditFps()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	SetFPS(m_nFPS);
	UpdateData(&m_nFPS, FALSE);
}


void CDlgVideo::OnDtnDatetimechangeTimeBegin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	CTime t1(2011, 7, 30, (m_nTimeFrom / 3600000) % 24, (m_nTimeFrom / 60000) % 60, (m_nTimeFrom / 1000) % 60);
	CTime t2(2011, 7, 30, (m_nTimeTo / 3600000) % 24, (m_nTimeTo / 60000) % 60, (m_nTimeTo / 1000) % 60);
	CTime t;
	m_ctrlTimeFrom.GetTime(t);
	m_ctrlTimeTo.SetRange(&t, &t2);

	*pResult = 0;
}

void CDlgVideo::OnDtnDatetimechangeTimeEnd(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	CTime t1(2011, 7, 30, (m_nTimeFrom / 3600000) % 24, (m_nTimeFrom / 60000) % 60, (m_nTimeFrom / 1000) % 60);
	CTime t2(2011, 7, 30, (m_nTimeTo / 3600000) % 24, (m_nTimeTo / 60000) % 60, (m_nTimeTo / 1000) % 60);
	CTime t;
	m_ctrlTimeTo.GetTime(t);
	m_ctrlTimeFrom.SetRange(&t1, &t);

	*pResult = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgVideoCtrl dialog

IMPLEMENT_DYNAMIC(CDlgVideoCtrl, CDialog)

CDlgVideoCtrl::CDlgVideoCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVideoCtrl::IDD, pParent)
{
	m_bStopping = false;
}

CDlgVideoCtrl::~CDlgVideoCtrl()
{
}

void CDlgVideoCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_wndStatus);
	DDX_Control(pDX, IDC_STATIC_TIME, m_wndTime);
	DDX_Control(pDX, IDC_STOP, m_wndStop);
	DDX_Control(pDX, IDC_CHECK_PREVIEW, m_wndPreview);
}


BEGIN_MESSAGE_MAP(CDlgVideoCtrl, CDialog)
	ON_BN_CLICKED(IDC_STOP, &CDlgVideoCtrl::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDlgVideoCtrl message handlers

void CDlgVideoCtrl::OnBnClickedButton1()
{
	m_bStopping = true;
}

void CDlgVideoCtrl::SetStatus(LPCTSTR pText, BOOL bEnableStop)
{
	m_wndStatus.SetWindowTextW(pText);
	m_wndStop.EnableWindow(bEnableStop);
}

void CDlgVideoCtrl::SetTime(ULONG nTime)
{
	CString str;
	str.Format(L"%d.%02d", nTime/1000, (nTime/10)%100);
	m_wndTime.SetWindowTextW(str);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgDownload dialog

IMPLEMENT_DYNAMIC(CDlgDownload, CDialog)

CDlgDownload::CDlgDownload(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDownload::IDD, pParent)
{
	m_server = _T("");
	m_details = _T("");
}

CDlgDownload::~CDlgDownload()
{
	for each (CSim *pSim in m_sims)
		delete pSim;
	m_sims.clear();
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
END_MESSAGE_MAP()


// CDlgDownload message handlers
// Dialogs.cpp : implementation file
//

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		int nRes;
		CSim *pSim1 = (CSim*)lParam1, *pSim2 = (CSim*)lParam2;
		switch (lParamSort % 100)
		{
			case 0: nRes = pSim1->GetSimulationId() < pSim2->GetSimulationId() ? -1 : (pSim1->GetSimulationId() > pSim2->GetSimulationId() ? 1 : 0); break;
			case 1: nRes = wcscmp(pSim1->GetProjectInfo(CSim::PRJ_PROJECT_NAME).c_str(), pSim2->GetProjectInfo(CSim::PRJ_PROJECT_NAME).c_str()); break;
			case 2: nRes = wcscmp(pSim1->GetProjectInfo(CSim::PRJ_BUILDING_NAME).c_str(), pSim2->GetProjectInfo(CSim::PRJ_BUILDING_NAME).c_str()); break;
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

	m_list.SetColumnWidth(0, 30);
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

	for each (CSim *pSim in m_sims)
		delete pSim;
	m_sims.clear();

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
		http.wait(); 
		if (!http.ok()) 
			http.throw_exceptions();

		std::wstring response = http.response();
		CSim::LoadIndexFromBuf(http.response().c_str(), m_sims);
		
		m_list.DeleteAllItems();
		for (unsigned i = 0; i < m_sims.size(); i++)
		{
			CSim *pSim = m_sims[i];
			LV_ITEM lvItem;
			lvItem.mask = LVIF_TEXT;
			wchar_t buf[80]; _snwprintf_s(buf, 80, L"%d", pSim->GetSimulationId());
			lvItem.iItem = m_list.InsertItem(0, buf);
			m_list.SetItemData(lvItem.iItem, (DWORD_PTR)pSim);
			lvItem.iSubItem = 1;
			lvItem.pszText = _wcsdup((LPWSTR)pSim->GetProjectInfo(CSim::PRJ_PROJECT_NAME).c_str());
			m_list.SetItem(&lvItem);
			lvItem.iSubItem = 2;
			lvItem.pszText = _wcsdup((LPWSTR)pSim->GetProjectInfo(CSim::PRJ_BUILDING_NAME).c_str());
			m_list.SetItem(&lvItem);
		}
		m_list.SortItems(CompareFunc, m_bAscending[m_nSort] ? m_nSort : 100 + m_nSort);
		m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_list.SetSelectionMark(0);

		return;

	}
	catch (_sim_error se)
	{
		str << "Error while analysing downloaded data: " << se.ErrorMessage() << ".";
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
	CSim *pSim = (CSim*)m_list.GetItemData(pNMLV->iItem);
	m_details.Format(
		L"Project: %s\n\r"
		L"Building: %s (%d floors, %d lift shafts)\n\r"
		L"Client: %s%s%s%s%s"
		, 
		_hlpStr(pSim->GetProjectInfo(CSim::PRJ_PROJECT_NAME), L"<untitled project>"),
		_hlpStr(pSim->GetProjectInfo(CSim::PRJ_BUILDING_NAME), L"main building"), pSim->GetBldgFloors(), pSim->GetBldgShafts(),
		_hlpStr(pSim->GetProjectInfo(CSim::PRJ_COMPANY), L"<unknown>"), _hlpStr(pSim->GetProjectInfo(CSim::PRJ_CITY)), _hlpStr(pSim->GetProjectInfo(CSim::PRJ_POST_CODE)), _hlpStr(pSim->GetProjectInfo(CSim::PRJ_COUNTY)), _hlpStr(pSim->GetProjectInfo(CSim::PRJ_COUNTRY))
		);

	CString str;
	if (!pSim->GetProjectInfo(CSim::PRJ_DESIGNER).empty())	 { str.Format(L"\n\rDesigned by: %s", pSim->GetProjectInfo(CSim::PRJ_DESIGNER).c_str()); m_details += str; }
	if (!pSim->GetProjectInfo(CSim::PRJ_CHECKED_BY).empty()) { str.Format(L"\n\rChecked by: %s", pSim->GetProjectInfo(CSim::PRJ_CHECKED_BY).c_str()); m_details += str; }

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
	CSim *pSim = (CSim*)m_list.GetItemData(nPos);
	if (!pSim) return;

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

	m_url.Format(L"http://%s/advsrv.asmx?request=%d", m_server, pSim->GetSimulationId());

	CDialog::OnOK();
}

void CDlgDownload::OnGetinfotipList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	pGetInfoTip->pszText = L"Ala ma kota";
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgAspectRatio dialog

IMPLEMENT_DYNAMIC(CDlgAspectRatio, CDialogEx)

CDlgAspectRatio::CDlgAspectRatio(AVFLOAT fWinRatio, AVFLOAT fScreenRatio, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAspectRatio::IDD, pParent)
	, m_nOption(0)
	, m_f1(0)
	, m_f2(0)
	, m_f3(0)
{
	m_fWinRatio = fWinRatio;
	m_fScreenRatio = fScreenRatio;
	m_bInitialised = FALSE;
	m_pLockOut = NULL;
}

CDlgAspectRatio::~CDlgAspectRatio()
{
}

void CDlgAspectRatio::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	if (m_pLockOut != &m_nOption) DDX_Radio(pDX, IDC_RADIO1, m_nOption);
	if (m_pLockOut != &m_f1 && m_pLockOut != &m_f2) 
	{
		DDX_Text(pDX, IDC_EDIT1, m_f1);
		DDV_MinMaxFloat(pDX, m_f1, 1, 1000);
		DDX_Text(pDX, IDC_EDIT2, m_f2);
		DDV_MinMaxFloat(pDX, m_f2, 1, 1000);
	}
	if (m_pLockOut != &m_f3)
	{
		DDX_Text(pDX, IDC_EDIT3, m_f3);
		DDV_MinMaxFloat(pDX, m_f3, 0.01f, 100);
	}
	DDX_Control(pDX, IDC_SPIN3, m_spin1);
	DDX_Control(pDX, IDC_SPIN1, m_spin2);
}


void CDlgAspectRatio::SetAspectRatio(AVFLOAT f)
{
	SetOption(f);
	SetRatio(f);
	SetRatio2(f);
}

void CDlgAspectRatio::SetOption(AVFLOAT f)
{
	switch ((AVULONG)(f * 1000))
	{
	case 0:					m_nOption = 0; break;
	case RATIO_16_9_x1000:	m_nOption = 2; break; 
	case RATIO_16_10_x1000:	m_nOption = 3; break;
	case RATIO_4_3_x1000:	m_nOption = 4; break;
	default:				if (floorf(f * 1000.0f + 0.5f) == floorf(m_fWinRatio * 1000.0f + 0.5f))
								m_nOption = 0;
							else
								m_nOption = 5; break;
	}
}

void CDlgAspectRatio::SetRatio(AVFLOAT f)
{
	if (f == 0) f = m_fWinRatio;
	m_f3 = f;
}

void CDlgAspectRatio::SetRatio2(AVFLOAT f)
{
	if (f == 0) f = m_fWinRatio;
	for (AVULONG i = 1; i < 10000; i++)
		if ((int)(f * i * 100.0f + 0.5f) % 100 == 0)
		{
			m_f1 = floorf(f * (float)i * 100.0f + 0.5f) / 100.0f;
			m_f2 = (float)i;
			return;
		}
	m_f1 = m_f2 = 0;
}

void CDlgAspectRatio::SetShape(AVULONG nId, AVULONG nIdRef, AVFLOAT f)
{
	CWnd *pWnd = GetDlgItem(nId);
	CWnd *pRef = GetDlgItem(nIdRef);
	if (!pWnd || !pRef) return;

	CRect rect, rectRef;
	pWnd->GetWindowRect(rect);
	pRef->GetWindowRect(rectRef);

	int x = 395, y = 140;
	int w, h;
	
	int W = rectRef.Width(), H = rectRef.Height();
	float F = (float)W / (float)H;

	if (f > F)
	{
		w = W;
		h = (int)((float)W / f);
	}
	else
	{
		w = (int)((float)H * f);
		h = H;
	}

	pWnd->SetWindowPos(NULL, x - half_of(w), y - half_of(h), w, h, SWP_NOZORDER);
}

BEGIN_MESSAGE_MAP(CDlgAspectRatio, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO3, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO4, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO5, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO6, &CDlgAspectRatio::OnBnClickedRadio1)
	ON_EN_CHANGE(IDC_EDIT1, &CDlgAspectRatio::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT2, &CDlgAspectRatio::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CDlgAspectRatio::OnEnChangeEdit)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgAspectRatio message handlers

BOOL CDlgAspectRatio::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_spin1.SetRange(1, 1000);
	m_spin2.SetRange(1, 1000);

	SetShape(IDC_STATIC_1, IDC_STATIC_1, m_fWinRatio);
	
	SetTimer(0, 100, NULL);

	m_bInitialised = TRUE;

	return TRUE;
}


void CDlgAspectRatio::OnBnClickedRadio1()
{
	if (!UpdateData()) return;
	switch (m_nOption)
	{
	case 0: SetAspectRatio(0); break;
	case 1: SetAspectRatio(m_fScreenRatio); m_nOption = 1; break;
	case 2: SetAspectRatio(RATIO_16_9); break;
	case 3: SetAspectRatio(RATIO_16_10); break;
	case 4: SetAspectRatio(RATIO_4_3); break;
	}
	UpdateData(FALSE);
	
}

void CDlgAspectRatio::OnEnChangeEdit2()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	AVFLOAT f = (AVFLOAT)m_f1 / (AVFLOAT)m_f2;
	SetOption(f);
	SetRatio(f);
	UpdateData(&m_f1, FALSE);
}

void CDlgAspectRatio::OnEnChangeEdit()
{
	if (!m_bInitialised) return;
	if (!UpdateData()) return;
	SetOption(m_f3);
	SetRatio2(m_f3);
	UpdateData(&m_f3, FALSE);
}


void CDlgAspectRatio::OnTimer(UINT_PTR nIDEvent)
{
	SetShape(IDC_STATIC_2, IDC_STATIC_1, m_f3);

	CDialogEx::OnTimer(nIDEvent);
}



//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// Dialogs.cpp : implementation file
//



// CDlgReportBug dialog

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

#include "AdVisuoDoc.h"
#include "AdVisuo.h"

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
		http.wait(); 
		if (!http.ok()) 
			http.throw_exceptions();
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
		http.wait(); 
		if (!http.ok()) 
			http.throw_exceptions();
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
// Dialogs.cpp : implementation file
//


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgMaterials dialog
//

IMPLEMENT_DYNAMIC(CDlgMaterials, CDialogEx)

CDlgMaterials *CDlgMaterials::c_dlg = NULL;

CDlgMaterials::CDlgMaterials(CMaterialManager *pMat, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMaterials::IDD, pParent), m_pMat(pMat)
	, m_bSolid(FALSE)
	, m_strFName(_T(""))
	, m_nAlpha(0)
	, m_nAlpha2(0)
{

}

CDlgMaterials::~CDlgMaterials()
{
}

void CDlgMaterials::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Radio(pDX, IDC_RADIO1, m_bSolid);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON1, m_ctrlColor);
	DDX_Text(pDX, IDC_EDIT1, m_strFName);
	DDX_Text(pDX, IDC_MFCCOLORBUTTON1, m_color);
	DDX_Text(pDX, IDC_EDIT2, m_nAlpha);
	DDV_MinMaxInt(pDX, m_nAlpha, 0, 100);
	DDX_Slider(pDX, IDC_SLIDER1, m_nAlpha2);
	DDV_MinMaxInt(pDX, m_nAlpha2, 0, 100);
	DDX_Control(pDX, IDC_SPIN2, m_spinAlpha);
}


BEGIN_MESSAGE_MAP(CDlgMaterials, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlgMaterials::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_MFCCOLORBUTTON1, &CDlgMaterials::OnBnClickedMfccolorbutton1)
	ON_EN_CHANGE(IDC_EDIT2, &CDlgMaterials::OnEnChangeEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CDlgMaterials::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDCANCEL, &CDlgMaterials::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgMaterials message handlers


BOOL CDlgMaterials::OnInitDialog()
{

	CDialogEx::OnInitDialog();

	m_spinAlpha.SetRange(0, 100);

	for (int i = 0; i < MAT_NUM; i++)
		m_list.AddString(m_pMat->GetLabel(i));

	m_list.SetCurSel(1);
	UpdateControls(m_list.GetCurSel());

	c_dlg = this;

	return TRUE;
}

void CDlgMaterials::UpdateControls(int i)
{
	MATERIAL &mat = (*m_pMat)[i];
	if (mat.m_bSolid)
	{
		m_bSolid = false;
		m_strFName = "";
		m_ctrlColor.SetColor(mat.m_color);
		m_ctrlColor.EnableWindow(TRUE);
	}
	else
	{
		m_bSolid = true;
		m_strFName = mat.m_fname.c_str();
		m_ctrlColor.EnableWindow(FALSE);
	}
	m_nAlpha = 100 - mat.m_alpha * 100;
	m_nAlpha2 = 100 - mat.m_alpha * 100;
	UpdateData(FALSE);
}

void CDlgMaterials::SetupMaterial(int i)
{
	if (!UpdateData()) return;

	if (m_bSolid == false)
		m_pMat->Set((MATERIALS)i, m_ctrlColor.GetColor(), 1.0f - (AVFLOAT)m_nAlpha2 / 100.0f);
	else
		m_pMat->Set((MATERIALS)i, m_strFName, (*m_pMat)[i].m_fUTile, (*m_pMat)[i].m_fVTile, 1.0f - (AVFLOAT)m_nAlpha2 / 100.0f);
}

void CDlgMaterials::OnLbnSelchangeList1()
{
	UpdateControls(m_list.GetCurSel());
}

void CDlgMaterials::OnBnClickedMfccolorbutton1()
{
	SetupMaterial(m_list.GetCurSel());
}

void CDlgMaterials::OnEnChangeEdit2()
{
	if (c_dlg == NULL) return;
	if (!UpdateData()) return;
	m_nAlpha2 = m_nAlpha;
	UpdateData(FALSE);
	SetupMaterial(m_list.GetCurSel());
}

void CDlgMaterials::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!UpdateData()) return;
	m_nAlpha = m_nAlpha2;
	UpdateData(FALSE);
	SetupMaterial(m_list.GetCurSel());
}

void CDlgMaterials::PostNcDestroy()
{
	c_dlg = NULL;
	CDialogEx::PostNcDestroy();
	delete this;
}

void CDlgMaterials::OnBnClickedCancel()
{
	DestroyWindow();
}
