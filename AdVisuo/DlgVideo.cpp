// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgVideo.h"
#include <afxtaskdialog.h>

#define RATIO_16_9			(16.0f / 9.0f)
#define RATIO_16_10			(16.0f / 10.0f)
#define RATIO_4_3			(4.0f / 3.0f)
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
	std::ifstream ifile(m_path);
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


