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
#include "XMLRequest.h"

class CAdVisuoApp;

// CAdVisuoApp:
// See AdVisuo.cpp for the implementation of this class
//

class CAdVisuoApp : public CWinAppEx
{
	// global HTTP connection - used for authorisation
	CXMLRequest m_http;

	// global application modes
	int m_nWalkMode;						// Walk or CCTV mode
	int m_nColouringMode;					// Character Colouring Mode
	CString m_servers;						// list of servers for the CDlgDownload
	CString m_url;							// server where the sim is loaded from

	CMultiDocTemplate* m_pAVDocTemplate;	// doc template

public:
	UINT  m_nAppLook;
	BOOL  m_bOutputWindow;
	BOOL  m_bHiColorIcons;

public:
	CAdVisuoApp();

	AVULONG GetWalkMode()			{ return (ULONG)m_nWalkMode; }
	void SetWalkMode(AVULONG n)		{ m_nWalkMode = n; }
	AVULONG GetColouringMode()		{ return (ULONG)m_nColouringMode; }
	void SetColouringMode(AVULONG n){ m_nColouringMode = n; }

	CXMLRequest *GetAuthorisationAgent()	{ return &m_http; }
	bool IsLogged()					{ return m_http.logged(); }

// Overrides
public:
	virtual BOOL InitInstance();

	// tools:
	bool AskLogin();								// Displays Login dialog box
	AVULONG SelectSimulation(CString url, AVULONG nProjectId = 0, AVULONG nSimulationId = 0, bool bGotoSimulations = false);	// Displays Project dialog
	CString URLFromSimulationId(AVULONG nSimulationId);
	bool InitSimulation(CString name);					// Loads the project with all decorations (splash windows, debuf info etc...)
	bool LoadSimulation(AVULONG nProjectId = 0, AVULONG nSimulationId = 0, bool bGotoSimulations = false);

	void ExtendAuthorisation();

	bool Report(AVULONG nSimulationId, AVSTRING strPath, AVULONG nCat, AVSTRING strUserDesc, AVSTRING strDiagnostic, AVSTRING strErrorMsg);
	
	void LoadRemoteServerConfig(CString strDefault);
	void AddRemoteServer(CString url);
	void SaveRemoteServerConfig();

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnIdle(LONG lCount);
	afx_msg void OnFileDownload();
	afx_msg void OnUpdateFileDownload(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);
	afx_msg void OnReportBug();
	virtual int ExitInstance();
	virtual BOOL LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd);
};

extern CAdVisuoApp theApp;
