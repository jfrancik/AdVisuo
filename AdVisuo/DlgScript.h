// Dialogs.h - a part of the AdVisuo Client Software

#pragma once

#include "resource.h"

class CScript;
class CScriptEvent;

class CDlgScript : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgScript)

	CScript *m_pScript;

public:
	CDlgScript(CScript *pScript, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgScript();

// Dialog Data
	enum { IDD = IDD_ADV_SCRIPT };
	static CDlgScript *c_dlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void PopulateList();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedAddEvent();
	CListBox m_list;
	afx_msg void OnLbnSelchangeList2();
	afx_msg void OnBnClickedPlayEvent();
	afx_msg void OnBnClickedRemoveEvent();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedProperties();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSave();
};
#pragma once


// CDlgScriptProperties dialog

class CDlgScriptProperties : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgScriptProperties)

	CScriptEvent *m_pEvent;

public:
	CDlgScriptProperties(CScriptEvent *pEvent, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgScriptProperties();

// Dialog Data
	enum { IDD = IDD_ADV_SCRIPT_PROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CDateTimeCtrl m_ctrlTime;
	CTime m_time;
	int m_nAnim;
	BOOL m_bFF;
	CDateTimeCtrl m_ctrlTimeFF;
	CTime m_timeFF;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CString m_desc;
	float m_fAccel;
};
