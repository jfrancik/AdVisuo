// AdVisuoDoc.h - a part of the AdVisuo Client Software
// interface of the CAdVisuoDoc class
//

#include "Building.h"
#include "Project.h"
#include "Sim.h"
#include "Lift.h"
#include "xmlrequest.h"

interface IAction;
class CAdVisuoView;

#pragma once

class CAdVisuoDoc : public CDocument
{
// Attributes

	CProject m_prj;			// The Project
//	CSim m_sim;				// The Simulation Engine
//	CBuilding m_building;	// The Building
	HRESULT m_h;			// The result of m_sim.Load

	CXMLRequest m_http;		// XML HTTP Request object
	CString m_strStatus;	// status of internet connection
	CString m_strResponse;	// response from the internet connection

	AVULONG m_timeLoaded;	// streamed loading: time loaded
	AVULONG m_groupLoaded;	// streamed loading: group loaded

protected: // create from serialization only
	CAdVisuoDoc();
	DECLARE_DYNCREATE(CAdVisuoDoc)
public:
	virtual ~CAdVisuoDoc();

	CString GetDiagnosticMessage();

	// Attributes and Basic Operations
	CProject *GetProject()					{ return &m_prj; }
	CBuilding *GetBuilding()				{ return GetProject()->GetBuilding(); }
	CSim *GetSim()							{ return GetProject()->GetSim(); }
	CBuilding *_GetBuilding(int i)			{ return GetProject()->GetBuilding(i); }
	CSim *GetSim(int i)						{ return GetProject()->GetSim(i); }

	bool IsSimReady()						{ return m_h == S_OK; }
	bool IsSIMDataReady()					{ return m_http.ready(); }
	bool IsDownloadComplete()				{ return m_timeLoaded >= GetSim()->GetTimeSaved(); }

	AVULONG GetLoadedTime()					{ return m_timeLoaded; }
	AVULONG GetSimulationTime()				{ return GetSim()->GetSimulationTime(); }
	AVLONG GetTimeLowerBound()				{ return GetSim()->GetTimeLowerBound(); }

	void ResetTitle()						{ SetTitle(m_prj.GetProjectInfo(CProject::PRJ_PROJECT_NAME).c_str()); m_strPathName = GetTitle(); }

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


