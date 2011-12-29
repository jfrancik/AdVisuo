// AdVisuo.h - a part of the AdVisuo Client Software

// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// AdVisuo.h : main header file for the AdVisuo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

class CAdVisuoApp;

inline CAdVisuoApp *GetAdVisuoApp()		{ return (CAdVisuoApp*)AfxGetApp(); }

// CAdVisuoApp:
// See AdVisuo.cpp for the implementation of this class
//

class CAdVisuoApp : public CWinAppEx
{
public:
	CAdVisuoApp();

private:
	// global application modes
	int m_nWalkMode;						// Walk or CCTV mode
	int m_nColouringMode;					// Character Colouring Mode
	CString m_servers;						// list of servers for the CDlgDownload

	ULONG m_nSessionId;

	CMultiDocTemplate* m_pAVDocTemplate;	// doc template

public:
	ULONG GetWalkMode()			{ return (ULONG)m_nWalkMode; }
	ULONG GetColouringMode()	{ return (ULONG)m_nColouringMode; }

	ULONG GetSessionId()		{ return m_nSessionId; }

	// SIM file paths - obsolete
	//CString m_strSimPathName;	// simulation file pathname - if provided as a 2nd cmd line param
	//LPCOLESTR GetSimPathName()	{ return m_strSimPathName.IsEmpty() ? NULL : m_strSimPathName; }
	//void ResetSimPathName()		{ m_strSimPathName.Empty(); }

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnNavigationCctv();
	afx_msg void OnUpdateNavigationCctv(CCmdUI *pCmdUI);
	afx_msg void OnNavigationWalk();
	afx_msg void OnUpdateNavigationWalk(CCmdUI *pCmdUI);
	afx_msg void OnNavigationGhost();
	afx_msg void OnUpdateNavigationGhost(CCmdUI *pCmdUI);
	afx_msg void OnCharacterNocolourcoding();
	afx_msg void OnUpdateCharacterNocolourcoding(CCmdUI *pCmdUI);
	afx_msg void OnCharacterCurrentwaitingtime();
	afx_msg void OnUpdateCharacterCurrentwaitingtime(CCmdUI *pCmdUI);
	afx_msg void OnCharacterExpectedwaitingtime();
	afx_msg void OnUpdateCharacterExpectedwaitingtime(CCmdUI *pCmdUI);
	virtual BOOL OnIdle(LONG lCount);
	afx_msg void OnFileDownload();
	afx_msg void OnUpdateFileDownload(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);
	afx_msg void OnReportBug();
	virtual int ExitInstance();
};

extern CAdVisuoApp theApp;
