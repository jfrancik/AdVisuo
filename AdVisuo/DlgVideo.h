// Dialogs.h - a part of the AdVisuo Client Software

#pragma once
#include <functional>
#include "resource.h"

// CDlgVideo dialog

class CMFCEditBrowseCtrlEx : public CMFCEditBrowseCtrl
{
	bool m_bSaveMode;
	bool m_bConfirmed;
public:
	CMFCEditBrowseCtrlEx() : CMFCEditBrowseCtrl()	{ m_bSaveMode = false; m_bConfirmed = false; }
	void SetSaveMode(bool bSaveMode = true)			{ m_bSaveMode = bSaveMode; }
	bool GetSaveMode()								{ return m_bSaveMode; }
	bool IsPathConfirmed()							{ return m_bConfirmed; }
	virtual void OnBrowse();
};

class CDlgVideo : public CDialog
{
	DECLARE_DYNAMIC(CDlgVideo)

public:
	CDlgVideo(AVFLOAT fAspectRatio, AVULONG nFPS, 
				CString path, 
				AVULONG nTimeFrom, AVULONG nTimeTo, 
				std::function<void(AVFLOAT fAspectRatio)> CBSwitchAspectRatio,
				std::function<void()> CBRevertAspectRatio,
				CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVideo();

	CString GetPath()			{ return m_path; }
	AVULONG GetFPS()			{ return m_nFPS; }
	AVULONG GetX()				{ return m_nResX; }
	AVULONG GetY()				{ return m_nResY; }
	AVULONG GetTFrom()			{ return m_nTimeFrom; }
	AVULONG GetTTo()			{ return m_nTimeTo; }
	bool ShowCaptions()			{ return m_bShowCaptions == TRUE; }
	bool ShowClock()			{ return m_bShowClock == TRUE; }

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE)	{ return CDialog::UpdateData(bSaveAndValidate); }
	BOOL UpdateData(void *pLockOut, BOOL bSaveAndValidate = TRUE)
	{ m_pLockOut = pLockOut; BOOL b = CDialog::UpdateData(bSaveAndValidate); m_pLockOut = NULL; return b; }

	struct RESITEM
	{
		AVULONG nOption;		// option depending on the aspect ratio
		AVLONG x, y;			// resolution
		wchar_t *pText;
	};
	static RESITEM c_resitems[];

// Dialog Data
	enum { IDD = IDD_ADV_VIDEO };

	void SetAspectRatio(float fAspectRatio);
	void SetFPS(int nFPS);
	void FillCombo(int nOption);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bInitialised;
	bool m_bUseAspect;
	CString m_path;
	AVULONG m_nTimeFrom;
	AVULONG m_nTimeTo;
	CMFCEditBrowseCtrlEx m_ctrlBrowse;
	CDateTimeCtrl m_ctrlTimeFrom;
	CDateTimeCtrl m_ctrlTimeTo;
	afx_msg void OnBnClickedRadioAspect_16_9();
	afx_msg void OnBnClickedRadioAspect_16_10();
	afx_msg void OnBnClickedRadioAspect_4_3();
	afx_msg void OnBnClickedRadioAspectCustom();
	afx_msg void OnBnClickedCustom();
	afx_msg void OnBnClickedSwitch();
	afx_msg void OnCbnSelchangeRes();
	afx_msg void OnBnClickedRadioFps24();
	afx_msg void OnBnClickedRadioFps25();
	afx_msg void OnBnClickedRadioFps30();
	afx_msg void OnBnClickedRadioFps50();
	afx_msg void OnBnClickedRadioFps60();
	afx_msg void OnEnChangeEditResx();
	afx_msg void OnEnChangeEditResy();
	afx_msg void OnEnChangeEditFps();
	afx_msg void OnDtnDatetimechangeTimeBegin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeTimeEnd(NMHDR *pNMHDR, LRESULT *pResult);
	int m_nAspectRatio;
	CComboBox m_comboRes;
	int m_nResX;
	int m_nResY;
	int m_radioFPS;
	int m_nFPS;
	float m_fAspectRatio;
	float m_fViewAspectRatio;
	std::function<void(AVFLOAT fAspectRatio)> m_CBSwitchAspectRatio;
	std::function<void()> m_CBRevertAspectRatio;

	void *m_pLockOut;	// modifier for UpdateData

	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditAspect();
	afx_msg void OnBnClickedRevert();
	CSpinButtonCtrl m_spinFPS;
	afx_msg void OnBnClickedRadioFpsCustom();
	CSpinButtonCtrl m_spinX;
	CSpinButtonCtrl m_spinY;
	afx_msg void OnBnClickedCheckRestrict();
	virtual void OnOK();
	BOOL m_bShowCaptions;
	BOOL m_bShowClock;
};

// CDlgVideoCtrl dialog

class CDlgVideoCtrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgVideoCtrl)

public:
	CDlgVideoCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVideoCtrl();

// Dialog Data
	enum { IDD = IDD_ADV_VIDEOCTRL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();

	bool m_bStopping;
	CStatic m_wndStatus;
	CStatic m_wndTime;
	CButton m_wndStop;

	void SetStatus(LPCTSTR pText, BOOL bEnableStop);
	void SetTime(ULONG nTime);
	CButton m_wndPreview;
};


// CDlgAspectRatio dialog

class CDlgAspectRatio : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAspectRatio)

public:
	CDlgAspectRatio(AVFLOAT fWinRatio, AVFLOAT fScreenRatio, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAspectRatio();

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE)	{ return CDialog::UpdateData(bSaveAndValidate); }
	BOOL UpdateData(void *pLockOut, BOOL bSaveAndValidate = TRUE)
	{ m_pLockOut = pLockOut; BOOL b = CDialog::UpdateData(bSaveAndValidate); m_pLockOut = NULL; return b; }

	void SetAspectRatio(AVFLOAT fRatio);
	AVFLOAT GetAspectRatio()		{ return (m_nOption == 0) ? 0 : (AVFLOAT)m_f3; }
	AVFLOAT GetActualAspectRatio()	{ return m_f3; }

// Dialog Data
	enum { IDD = IDD_ADV_ASPECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void SetOption(AVFLOAT f);
	void SetRatio(AVFLOAT f);
	void SetRatio2(AVFLOAT f);
	void SetShape(AVULONG nId, AVULONG nIdRef, AVFLOAT f);

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bInitialised;
	void *m_pLockOut;
	float m_fWinRatio;
	float m_fScreenRatio;
	int m_nOption;
	float m_f1;
	float m_f2;
	float m_f3;
	afx_msg void OnBnClickedRadio1();
	virtual BOOL OnInitDialog();
	CSpinButtonCtrl m_spin1;
	CSpinButtonCtrl m_spin2;
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#pragma once
