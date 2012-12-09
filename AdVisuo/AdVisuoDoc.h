// AdVisuoDoc.h - a part of the AdVisuo Client Software
// interface of the CAdVisuoDoc class
//

#include "VisLiftGroup.h"
#include "VisProject.h"
#include "VisSim.h"
#include "VisLift.h"
#include "xmlrequest.h"

interface IAction;
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

	AVULONG m_timeLoaded;	// streamed loading: time loaded

protected: // create from serialization only
	CAdVisuoDoc();
	DECLARE_DYNCREATE(CAdVisuoDoc)
public:
	virtual ~CAdVisuoDoc();

	CString GetDiagnosticMessage();

	// Attributes and Basic Operations
	CProjectVis *GetProject()				{ return &m_prj; }

	bool IsSimReady()						{ return m_h == S_OK; }
	bool IsSIMDataReady()					{ return m_http.ready(); }
	bool IsDownloadComplete(int i = 0)		{ return m_timeLoaded >= GetProject()->GetSim(i)->GetTimeSaved(); }

	AVULONG GetLoadedTime()					{ return m_timeLoaded; }
	AVULONG GetSimulationTime(int i)		{ return GetProject()->GetSim(i)->GetSimulationTime(); }
	AVLONG GetTimeLowerBound(int i)			{ return GetProject()->GetSim(i)->GetTimeLowerBound(); }

	void ResetTitle()						{ SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str()); m_strPathName = GetTitle(); }

	// Open/New/Download Operations
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnDownloadDocument(CString url);

	BOOL OnSIMDataLoaded(IAction *pActionTick);

// Generated message map functions
protected:
	CString m_strUrl;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void OnCloseDocument();
	virtual BOOL SaveModified();
};


