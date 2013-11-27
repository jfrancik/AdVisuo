// AdVisuoDoc.cpp - a part of the AdVisuo Client Software

// AdVisuoDoc.cpp : implementation of the CAdVisuoDoc class
//

#include "stdafx.h"
#include "AdVisuoDoc.h"
#include "AdVisuoView.h"
#include "AdVisuo.h"
#include "DlgHtBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAdVisuoDoc

IMPLEMENT_DYNCREATE(CAdVisuoDoc, CDocument)

BEGIN_MESSAGE_MAP(CAdVisuoDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CAdVisuoDoc::OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CAdVisuoDoc::OnUpdateFileSave)
	ON_COMMAND(ID_OTHER_FAILURE, &CAdVisuoDoc::OnOtherFailure)
END_MESSAGE_MAP()


// CAdVisuoDoc construction/destruction

CAdVisuoDoc::CAdVisuoDoc() : m_prj()
{
	m_h = S_FALSE;
	m_timeLoaded = 0;
	m_http.create();
}

CAdVisuoDoc::~CAdVisuoDoc()
{
}

CString CAdVisuoDoc::GetDiagnosticMessage()
{
	POSITION pos = GetFirstViewPosition();
	CAdVisuoView *pView = (CAdVisuoView*)GetNextView(pos);
	if (pView)
		return pView->GetEngine()->GetDiagnosticMessage();
	else
		return L"";
}

CString CAdVisuoDoc::GetPathInfo()
{
	return m_strUrl.IsEmpty() ? m_strPathName : m_strUrl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Open/New/Download Operations

BOOL CAdVisuoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL CAdVisuoDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	const LPCTSTR prefixA = L"advisuo://";
	const LPCTSTR prefixH = L"http://";
	
	if (_wcsnicmp(lpszPathName, prefixA, wcslen(prefixA)) == 0 || _wcsnicmp(lpszPathName, prefixH, wcslen(prefixH)) == 0)
		return OnDownloadDocument(lpszPathName);
	
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	OutText(L"Loading simulation from file: %s", lpszPathName);

	CWaitCursor wait;
	DeleteContents();

	std::wstringstream err;
	try
	{
		GetProject()->LoadFromFile(lpszPathName);	// throws _prj_error and _com_error

		SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_NAME).c_str());
		m_timeLoaded = GetProject()->GetMaxSimulationTime();

		m_h = S_OK;
		OutText(L"File successfully loaded.");
		SetModifiedFlag(FALSE);
	}
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch(...)
	{
		CDlgHtFailure dlg(lpszPathName);
		dlg.DoModal();
		return false;
	}
	return true;
}

BOOL CAdVisuoDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	OutText(L"Storing simulation to file: %s", L"unknown");

	CWaitCursor wait;

	std::wstringstream err;
	try
	{
		GetProject()->StoreToFile(lpszPathName);

		OutText(L"File successfully stored.");
		SetModifiedFlag(FALSE);
	}
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, lpszPathName);
		dlg.DoModal();
		return false;
	}
	catch(...)
	{
		CDlgHtFailure dlg(lpszPathName);
		dlg.DoModal();
		return false;
	}
	return true;
}

BOOL CAdVisuoDoc::OnDownloadDocument(CString url)
{
	// Resolve the URL
	CString strUrl = url;
	m_strUrl = strUrl;
	AVULONG nId = 0;
	CString strUserid;
	CString strTicket;
	CString strPhase;

	if (url.Left(8).Compare(L"advisuo:") == 0)
		url = CString(L"http:") + url.Mid(8);

	int curpos = url.Find(L'?');
	if (curpos >= 0)
	{
		strUrl = url.Left(curpos);
		curpos++;

		CString name, val;
		while (1)
		{
			name = url.Tokenize(L"=", curpos);
			if (name.IsEmpty()) break;
			val = url.Tokenize(L"&", curpos);
			if (val.IsEmpty()) break;

			if (name.Compare(L"request") == 0)
				nId = _wtoi(val);
			if (name.Compare(L"userid") == 0)
				strUserid = val;
			if (name.Compare(L"ticket") == 0)
				strTicket = val;
			if (name.Compare(L"phase") == 0)
				strPhase = val;
		}
	}
	if (strUrl.Right(13) == "/GetAVProject") strUrl = strUrl.Left(strUrl.GetLength() - 13);
	OutText(L"Downloading project from:");
	OutText(L"%s (id=%d)", strUrl, nId);

	// Initiate the download
	std::wstringstream str;
	std::wstring response;
	try
	{
		// set-up the master autorisation/http object
		CXMLRequest *pMaster = AVGetApp()->GetAuthorisationAgent();
		pMaster->set_authorisation_data((LPCTSTR)strUserid, (LPCTSTR)strTicket);
		pMaster->setURL((LPCTSTR)strUrl);

		// prepare my own copy
		m_http.setURL(pMaster->getURL());
		m_http.take_authorisation_from(pMaster);
		
		if (m_http.AVIsAuthorised() <= 0)
			throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);

		std::wstring appname = m_http.AVGetAppName();
		int verreq = m_http.AVGetRequiredVersion();
		std::wstring date = m_http.AVGetRequiredVersionDate();
		std::wstring path = m_http.AVGetLatestVersionDownloadPath();
		if (VERSION < verreq)
			throw _version_error(verreq, date.c_str(), path.c_str());

		m_http.AVProject(nId);
		m_http.get_response(response);
		GetProject()->LoadFromBuf(response.c_str());

		SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_NAME).c_str());
		m_strPathName = GetTitle();

		m_http.AVLiftGroups(GetProject()->GetId());
		m_http.get_response(response);
		GetProject()->LoadFromBuf(response.c_str());

		// load lift groups
		for each (CLiftGroupVis *pGroup in GetProject()->GetLiftGroups())
		{
			m_http.AVFloors(pGroup->GetId());
			m_http.get_response(response);
			GetProject()->LoadFromBuf(response.c_str());

			m_http.AVShafts(pGroup->GetId());
			m_http.get_response(response);
			GetProject()->LoadFromBuf(response.c_str());

			m_http.AVSim(pGroup->GetId());
			m_http.get_response(response);
			GetProject()->LoadFromBuf(response.c_str());
		}

		// first SIM data chunk
		m_timeLoaded = GetProject()->GetMinSimulationTime();
		m_http.AVPrjData(GetProject()->GetId(), m_timeLoaded, m_timeLoaded + 60000);
		//m_http.wait();
		OnSIMDataLoaded();

		m_h = S_OK;
		OutText(L"Download initiated successfully, more data loading in background...");
		SetModifiedFlag(TRUE);
	}
	catch (_prj_error pe)
	{
		CDlgHtFailure dlg(pe, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (_com_error ce)
	{
		CDlgHtFailure dlg(ce, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (_xmlreq_error xe)
	{
		CDlgHtFailure dlg(xe, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (_version_error ve)
	{
		CDlgHtFailure dlg(ve, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch (dbtools::_value_error ve)
	{
		CDlgHtFailure dlg(ve, m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	catch(...)
	{
		CDlgHtFailure dlg(m_http.getURL().c_str());
		dlg.DoModal();
		return false;
	}
	return true;
}

BOOL CAdVisuoDoc::OnSIMDataLoaded()
{
	std::wstringstream str;
	try
	{
		// Process the most recently loaded data
		std::wstring response;
		m_http.get_response(response);
		GetProject()->LoadFromBuf(response.c_str());
		
		m_timeLoaded += 60000;

		if (!IsDownloadComplete())
		{
			m_http.take_authorisation_from(AVGetApp()->GetAuthorisationAgent());
			if (m_http.AVIsAuthorised() <= 0)
				throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);
			m_http.AVPrjData(GetProject()->GetId(), m_timeLoaded, m_timeLoaded + 60000);
		}

		return true;
	}
	catch (_prj_error pe)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle(pe);
		m_strFailureText  = CDlgHtFailure::GetFailureString(pe, m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	catch (_com_error ce)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle(ce);
		m_strFailureText  = CDlgHtFailure::GetFailureString(ce, m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	catch (_xmlreq_error xe)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle(xe);
		m_strFailureText  = CDlgHtFailure::GetFailureString(xe, m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	catch (_version_error ve)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	catch (dbtools::_value_error ve)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	catch(...)
	{
		m_strFailureTitle = CDlgHtFailure::GetFailureTitle();
		m_strFailureText  = CDlgHtFailure::GetFailureString(m_http.getURL().c_str());
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return false;
	}
	return true;
}

void CAdVisuoDoc::OnOtherFailure()
{
	CDlgHtFailure dlg(m_strFailureTitle, m_strFailureText);
	dlg.DoModal();
}

void CAdVisuoDoc::Serialize(CArchive& ar)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Non-Standard Command Handlers

void CAdVisuoDoc::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDownloadComplete() && !((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
}

void CAdVisuoDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// store the path fully qualified
	TCHAR szFullPath[_MAX_PATH];
	ENSURE(lpszPathName);
	if ( lstrlen(lpszPathName) >= _MAX_PATH )
	{
		ASSERT(FALSE);
		// MFC requires paths with length < _MAX_PATH
		// No other way to handle the error from a void function
		AfxThrowFileException(CFileException::badPath);
	}

	if (wcsncmp(lpszPathName, L"advisuo:", 8) == 0)
		m_strPathName = lpszPathName;
	else if (wcsncmp(lpszPathName, L"http:", 5) == 0)
	{
		m_strPathName = L"advisuo";
		m_strPathName += lpszPathName + 4;
	}
	else
	{
		if (GetFullPathName(lpszPathName, _MAX_PATH, szFullPath, NULL) == 0)
			AfxThrowFileException(CFileException::badPath);
		m_strPathName = szFullPath;
	}

	ASSERT(!m_strPathName.IsEmpty());       // must be set to something
	m_bEmbedded = FALSE;

	// add it to the file MRU list
	if (bAddToMRU)
		AVGetApp()->AddToRecentFileList(m_strPathName);

	ASSERT_VALID(this);
}


void CAdVisuoDoc::OnCloseDocument()
{
	//if (!IsDownloadComplete())
	//{
	//	// finish background download cycle to avoid a crash
	//	CWaitCursor cursor;
	//	m_http.wait(5000);
	//	m_http.reset();
	//	if (IsSIMDataReady())
	//		MessageBeep(-1);
	//}
	CDocument::OnCloseDocument();
}


BOOL CAdVisuoDoc::SaveModified()
{
	if (!IsModified())
		return TRUE;        // ok to continue

	// get name/title of document
	CString name;
	if (m_strPathName.IsEmpty())
	{
		name = m_strTitle;
		if (name.IsEmpty())
			ENSURE(name.LoadString(AFX_IDS_UNTITLED));
	}
	else
		name = m_strPathName;

	// we do not ask the user yet...
	// to be changed soon
	// but now we simply close the window without saving the file
	return TRUE;

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;

	case IDNO:
		// If not saving changes, revert the document
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}


