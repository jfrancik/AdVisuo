// AdVisuoDoc.cpp - a part of the AdVisuo Client Software

// AdVisuoDoc.cpp : implementation of the CAdVisuoDoc class
//

#include "stdafx.h"
#include "AdVisuoDoc.h"
#include "AdVisuoView.h"

#include "freewill.c"			// #FreeWill: Obligatory!

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAdVisuoDoc

IMPLEMENT_DYNCREATE(CAdVisuoDoc, CDocument)

BEGIN_MESSAGE_MAP(CAdVisuoDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CAdVisuoDoc::OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CAdVisuoDoc::OnUpdateFileSave)
END_MESSAGE_MAP()


// CAdVisuoDoc construction/destruction

CAdVisuoDoc::CAdVisuoDoc() : m_prj()
{
	m_h = S_FALSE;
	m_timeLoaded = 0;
}

CAdVisuoDoc::~CAdVisuoDoc()
{
}

CString CAdVisuoDoc::GetDiagnosticMessage()
{
	CString str;
	str.Format(L"Project id: [%d] %s\r\nPath: %s", GetProject()->GetId(), GetTitle(), m_strUrl.IsEmpty() ? m_strPathName : m_strUrl);

	POSITION pos = GetFirstViewPosition();
	CAdVisuoView *pView = (CAdVisuoView*)GetNextView(pos);
	if (pView)
	{
		str += "\r\n";
		str += pView->GetDiagnosticMessage();
	}
	return str;
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
	{
		return OnDownloadDocument(lpszPathName);
	}
	else
	{
		if (!CDocument::OnOpenDocument(lpszPathName))
			return FALSE;

		Debug(L"Loading simulation from file: %s", lpszPathName);

		CWaitCursor wait;
		DeleteContents();

		std::wstringstream err;
		try
		{
			GetProject()->LoadFromFile(lpszPathName);	// throws _prj_error and _com_error

			SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str());
			m_timeLoaded = GetProject()->GetMaxSimulationTime();

			m_h = S_OK;
			Debug(L"File successfully loaded.");
			SetModifiedFlag(FALSE);
			return true;
		}
		catch (_prj_error pe)
		{
			err << "Error while analysing loaded data: " << pe.ErrorMessage() << ".";
		}
		catch (_com_error ce)
		{
			err << "System error while loading from " << lpszPathName << ":" << endl;
			if ((wchar_t*)ce.Description())
				err << ce.Description();
			else
				err << ce.ErrorMessage();
		}
		catch (dbtools::_value_error ve)
		{
			err << "Error while analysing downloaded data: " << ve.ErrorMessage() << ".";
		}
		catch(...)
		{
			err << L"Unidentified errors while loading from " << lpszPathName;
		}
		Debug(err.str().c_str());
		AfxMessageBox(err.str().c_str(), MB_OK | MB_ICONHAND);
		return false;
	}
}

BOOL CAdVisuoDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	Debug(L"Storing simulation to file: %s", L"unknown");

	CWaitCursor wait;

	std::wstringstream err;
	try
	{
		GetProject()->StoreToFile(lpszPathName);

		Debug(L"File successfully stored.");
		SetModifiedFlag(FALSE);
		return true;
	}
	catch (_prj_error pe)
	{
		err << "Error while preparing data to store: " << pe.ErrorMessage() << ".";
	}
	catch (_com_error ce)
	{
		err << "System error while storing to " << lpszPathName << ":" << endl;
		if ((wchar_t*)ce.Description())
			err << ce.Description();
		else
			err << ce.ErrorMessage();
	}
	catch (dbtools::_value_error ve)
	{
		err << "Error while analysing downloaded data: " << ve.ErrorMessage() << ".";
	}
	catch(...)
	{
		err << L"Unidentified errors while loading from " << lpszPathName;
	}
	Debug(err.str().c_str());
	AfxMessageBox(err.str().c_str(), MB_OK | MB_ICONHAND);
	return false;
}

BOOL CAdVisuoDoc::OnDownloadDocument(CString url)
{
	// Resolve the URL
	CString strUrl = url;
	m_strUrl = strUrl;
	AVULONG nId = 0;

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
		}
	}
	if (strUrl.Right(13) == "/GetAVProject") strUrl = strUrl.Left(strUrl.GetLength() - 13);
	Debug(L"Downloading project from %s with id=%d", strUrl, nId);

	// Initiate the download
	std::wstringstream str;
	try
	{
		m_http.setURL((LPCTSTR)strUrl);

		m_http.AVProject(nId);
		GetProject()->LoadFromBuf(m_http.response().c_str());

		SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_PROJECT_NAME).c_str());
		m_strPathName = GetTitle();

		m_http.AVLiftGroups(GetProject()->GetId());
		GetProject()->LoadFromBuf(m_http.response().c_str());

		// load lift groups
		for each (CLiftGroupVis *pGroup in GetProject()->GetLiftGroups())
		{
			m_http.AVFloors(pGroup->GetId());
			GetProject()->LoadFromBuf(m_http.response().c_str());

			m_http.AVShafts(pGroup->GetId());
			GetProject()->LoadFromBuf(m_http.response().c_str());

			m_http.AVSim(pGroup->GetId());
			GetProject()->LoadFromBuf(m_http.response().c_str());
		}

		// first SIM data chunk
		m_timeLoaded = 0;
		m_http.AVPrjData(GetProject()->GetId(), m_timeLoaded, m_timeLoaded + 60000);
		OnSIMDataLoaded(NULL);

		m_h = S_OK;
		Debug(L"Downloading initiated successfully, more data transfered in background...");
		SetModifiedFlag(TRUE);
		return true;
	}
	catch (_prj_error pe)
	{
		str << "Error while analysing downloaded data: " << pe.ErrorMessage() << ".";
	}
	catch (_com_error ce)
	{
		str << "System error while downloading from " << m_http.URL() << ":" << endl;
		if ((wchar_t*)ce.Description())
			str << ce.Description();
		else
			str << ce.ErrorMessage();
	}
	catch (_xmlreq_error xe)
	{
		str << L"HTTP error " << xe.status() << L": " << xe.msg() << L" at " << m_http.URL() << L".";
	}
	catch (dbtools::_value_error ve)
	{
		str << "Error while analysing downloaded data: " << ve.ErrorMessage() << ".";
	}
	catch(...)
	{
		str << L"Unidentified errors while downloading from " << m_http.URL();
	}
	Debug(str.str().c_str());
	AfxMessageBox(str.str().c_str(), MB_OK | MB_ICONHAND);
	m_http.reset();
	return false;
}

BOOL CAdVisuoDoc::OnSIMDataLoaded(IAction *pActionTick)
{
	std::wstringstream str;
	try
	{
		// Process the most recently loaded data
		if (!m_http.ok()) m_http.throw_exceptions();
		GetProject()->LoadFromBuf(m_http.response().c_str());

		if (pActionTick)
			for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
				GetProject()->GetSim(i)->Play(pActionTick, m_timeLoaded);
		
		m_timeLoaded += 60000;

		if (!IsDownloadComplete())
		{
			str << L"Simulation data download continued in background (" << m_timeLoaded << L").";
			m_http.AVPrjData(GetProject()->GetId(), m_timeLoaded, m_timeLoaded + 60000, false);
		}
		else
			m_http.reset();

		return true;

	}
	catch (_prj_error pe)
	{
		str << "Error while analysing downloaded data: " << pe.ErrorMessage() << ".";
	}
	catch (_com_error ce)
	{
		str << "System error while downloading from " << m_http.URL() << ":" << endl;
		if ((wchar_t*)ce.Description())
			str << ce.Description();
		else
			str << ce.ErrorMessage();
	}
	catch (_xmlreq_error xe)
	{
		str << L"HTTP error " << xe.status() << L": " << xe.msg() << L" at " << m_http.URL() << L".";
	}
	catch (dbtools::_value_error ve)
	{
		str << "Error while analysing downloaded data: " << ve.ErrorMessage() << ".";
	}
	catch(...)
	{
		str << L"Unidentified errors while downloading from " << m_http.URL();
	}
	m_http.reset();
	Debug(str.str().c_str());
	AfxMessageBox(str.str().c_str(), MB_OK | MB_ICONHAND);
	return false;
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
		AfxGetApp()->AddToRecentFileList(m_strPathName);

	ASSERT_VALID(this);
}


void CAdVisuoDoc::OnCloseDocument()
{
	if (!IsDownloadComplete())
	{
		// finish background download cycle to avoid a crash
		CWaitCursor cursor;
		m_http.wait(5000);
		m_http.reset();
		if (IsSIMDataReady())
			MessageBeep(-1);
	}
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
