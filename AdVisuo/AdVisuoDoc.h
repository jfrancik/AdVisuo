// AdVisuoDoc.h - a part of the AdVisuo Client Software
// interface of the CAdVisuoDoc class
//

//#include "VisLiftGroup.h"
#include "VisProject.h"
//#include "VisSim.h"
//#include "VisLift.h"
#include "xmlrequest.h"

class CAdVisuoView;

#pragma once

class CAdVisuoDoc : public CDocument
{
// Attributes

	CProjectVis m_prj;		// The Project
	HRESULT m_h;			// The result of m_sim.Load

	CXMLRequest m_http;		// XML HTTP Request object
	CString m_strStatus;	// status of internet connection
	CString m_strResponse;	// response from the internet connection

	AVLONG m_timeLoaded;	// streamed loading: time loaded

	// data used by delayed Failure Dialog Box
	CString m_strFailureTitle;
	CString m_strFailureText;

protected: // create from serialization only
	CAdVisuoDoc();
	DECLARE_DYNCREATE(CAdVisuoDoc)
public:
	virtual ~CAdVisuoDoc();

	CString GetDiagnosticMessage();
	CString GetPathInfo();

	// Attributes and Basic Operations
	CProjectVis *GetProject()				{ return &m_prj; }

	bool IsSimReady()						{ return m_h == S_OK; }
	bool IsSIMDataReady()					{ return m_http.ready(); }
	bool IsDownloadComplete()				{ return m_timeLoaded >= GetProject()->GetTimeSaved(); }

	AVLONG GetLoadedTime()					{ return m_timeLoaded; }

	void ResetTitle()						{ SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_NAME).c_str()); m_strPathName = GetTitle(); }

	// Open/New/Download Operations
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnDownloadDocument(CString url);

	BOOL OnSIMDataLoaded();

// Generated message map functions
protected:
	CString m_strUrl;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void OnCloseDocument();
	virtual BOOL SaveModified();
	afx_msg void OnOtherFailure();
};


