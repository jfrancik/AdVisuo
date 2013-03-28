// Dialogs.h - a part of the AdVisuo Client Software

#pragma once

#include "resource.h"
#include "afxcolorbutton.h"

class CEngine;

class CDlgMaterials : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMaterials)

	CEngine *m_pEngine;

public:
	CDlgMaterials(CEngine *pEngine, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMaterials();

// Dialog Data
	enum { IDD = IDD_ADV_MATERIALS };
	static CDlgMaterials *c_dlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void UpdateControls(int i);
	void SetupMaterial(int i);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_list;
	BOOL m_bSolid;
	CMFCColorButton m_ctrlColor;
	CString m_strFName;
	COLORREF m_color;
	int m_nAlpha;
	int m_nAlpha2;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedMfccolorbutton1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedCancel();
	CSpinButtonCtrl m_spinAlpha;
};

