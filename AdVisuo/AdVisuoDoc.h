// AdVisuoDoc.h - a part of the AdVisuo Client Software
// interface of the CAdVisuoDoc class
//

#include "VisProject.h"
#include "AdVisuoLoader.h"

class CAdVisuoView;
class CEngine;

#pragma once

class CAdVisuoDoc : public CDocument
{
// Attributes

	CProjectVis m_prj;			// The Project
	CAdVisuoLoader m_loader;	// The Project Loader

protected: // create from serialization only
	CAdVisuoDoc();
	DECLARE_DYNCREATE(CAdVisuoDoc)
public:
	virtual ~CAdVisuoDoc();

	CEngine *GetEngine();

	CString GetPathInfo()					{ return m_strUrl.IsEmpty() ? m_strPathName : m_strUrl; }

	// Attributes and Basic Operations
	CProjectVis *GetProject()				{ return &m_prj; }
	void ResetTitle()						{ SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_NAME).c_str()); m_strPathName = GetTitle(); }

	// Open/New/Download Operations
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnDownloadDocument(CString url);

	// Loader Status
	AVULONG GetLoaderStatus()				{ return m_loader.GetStatus(); }
	bool IsDownloadTerminated()				{ return m_loader.GetStatus() == CAdVisuoLoader::TERMINATED; }
	bool IsBuildingDownloaded()				{ return m_loader.GetStatus() >= CAdVisuoLoader::LOADING_DATA; }
	AVULONG GetLoaderReasonForTermination()	{ return m_loader.GetReasonForTermination(); }
	AVULONG GetLoaderPosInQueue()			{ return m_loader.GetPosInQueue(); }

	AVLONG GetLoadedTime()					{ return m_loader.GetTimeLoaded(); }

	void StopDownload()						{ m_loader.Stop(); }
	AVULONG UpdateDownload(CEngine *pEngine){ return m_loader.Update(pEngine); }	// returns status


// Generated message map functions
protected:
	CString m_strUrl;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual BOOL SaveModified();
	virtual void OnCloseDocument();
};


