// Dialogs.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "DlgMat.h"
#include "Engine.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CDlgMaterials dialog
//

IMPLEMENT_DYNAMIC(CDlgMaterials, CDialogEx)

CDlgMaterials *CDlgMaterials::c_dlg = NULL;

CDlgMaterials::CDlgMaterials(CEngine *pEngine, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMaterials::IDD, pParent), m_pEngine(pEngine)
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
		m_list.AddString(m_pEngine->GetMatLabel(i));

	m_list.SetCurSel(1);
	UpdateControls(m_list.GetCurSel());

	c_dlg = this;

	return TRUE;
}

void CDlgMaterials::UpdateControls(int i)
{
	if (m_pEngine->GetMatSolid(i))
	{
		m_bSolid = false;
		m_strFName = "";
		m_ctrlColor.SetColor(AVCOLOR2COLORREF(m_pEngine->GetMatColor(i)));
		m_ctrlColor.EnableWindow(TRUE);
	}
	else
	{
		m_bSolid = true;
		m_strFName = m_pEngine->GetMatFName(i);
		m_ctrlColor.EnableWindow(FALSE);
	}
	m_nAlpha = 100 - m_pEngine->GetMatAlpha(i) * 100;
	m_nAlpha2 = 100 - m_pEngine->GetMatAlpha(i) * 100;
	UpdateData(FALSE);
}

void CDlgMaterials::SetupMaterial(int i)
{
	if (!UpdateData()) return;

	COLORREF c = m_ctrlColor.GetColor();
	
	if (m_bSolid == false)
		m_pEngine->SetSolidMat(i, GetRValue(c), GetGValue(c), GetBValue(c), 255 * (1.0f - (AVFLOAT)m_nAlpha2 / 100.0f));
	else
		m_pEngine->SetTexturedMat(i, (AVSTRING)(LPCOLESTR)m_strFName, m_pEngine->GetMatUTile(i), m_pEngine->GetMatVTile(i), 1.0f - (AVFLOAT)m_nAlpha2 / 100.0f);
	m_pEngine->CreateMat(i);
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
