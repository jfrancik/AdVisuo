// AdVisuoDoc.h - a part of the AdVisuo Client Software
// interface of the CAdVisuoDoc class
//

#include "Building.h"
#include "Sim.h"
#include "Lift.h"
#include "xmlrequest.h"

interface IAction;
class CAdVisuoView;

#pragma once

class CAdVisuoDoc : public CDocument
{
// Attributes

	CBuilding m_building;	// The Building
	CSim m_sim;				// The Simulation Engine
	HRESULT m_h;			// The result of m_sim.Load

	CXMLRequest m_http;		// XML HTTP Request object
	CString m_strStatus;	// status of internet connection
	CString m_strResponse;	// response from the internet connection

	AVULONG m_timeLoaded;

protected: // create from serialization only
	CAdVisuoDoc();
	DECLARE_DYNCREATE(CAdVisuoDoc)
public:
	virtual ~CAdVisuoDoc();

	CString GetDiagnosticMessage();

	// Attributes and Basic Operations
	CBuilding *GetBuilding()				{ return &m_building; }
	CSim *GetSim()							{ return &m_sim; }

	bool IsSimReady()						{ return m_h == S_OK; }
	bool IsSIMDataReady()					{ return m_http.ready(); }
	bool IsDownloadComplete()				{ return m_timeLoaded >= m_sim.GetTimeSaved(); }

	AVULONG GetLoadedTime()					{ return m_timeLoaded; }
	AVULONG GetSimulationTime()				{ return m_sim.GetSimulationTime(); }
	AVLONG GetTimeLowerBound()				{ return m_sim.GetTimeLowerBound(); }

	void ResetTitle()						{ SetTitle(m_sim.GetProjectInfo(CSim::PRJ_PROJECT_NAME).c_str()); m_strPathName = GetTitle(); }

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


