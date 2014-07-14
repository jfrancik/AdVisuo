// AdVisuoDoc.cpp - a part of the AdVisuo Client Software

// AdVisuoDoc.cpp : implementation of the CAdVisuoDoc class
//

#include "stdafx.h"
#include "AdVisuoDoc.h"
#include "AdVisuoView.h"
#include "AdVisuo.h"
#include "VisSim.h"
#include "DlgHtBase.h"

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

CAdVisuoDoc::CAdVisuoDoc() : m_loader(&m_prj)
{
}

CAdVisuoDoc::~CAdVisuoDoc()
{
	CWaitCursor cursor;
	if (AVGetApp() && AfxGetMainWnd() && AVGetApp()->CountDocuments() == 1)
		AfxGetMainWnd()->ShowWindow(SW_HIDE);
	m_loader.Stop();
}

CEngine *CAdVisuoDoc::GetEngine()
{
	POSITION pos = GetFirstViewPosition();
	CAdVisuoView *pView = (CAdVisuoView*)GetNextView(pos);
	return pView ? pView->GetEngine() : NULL;
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

	OutText(L"File: %s", lpszPathName);

	CWaitCursor wait;
	DeleteContents();

	std::wstringstream err;
	try
	{
		GetProject()->LoadFromFile(lpszPathName);	// throws _prj_error and _com_error

		SetTitle(GetProject()->GetProjectInfo(CProjectVis::PRJ_NAME).c_str());

		//ATTENTION: This line was to signal the project was READY
		//m_h = S_OK;
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
	OutText(L"URL is %s (id=%d)", strUrl, nId);

	// set-up the master autorisation/http object
	CXMLRequest *pMaster = AVGetApp()->GetAuthorisationAgent();
	pMaster->set_authorisation_data((LPCTSTR)strUserid, (LPCTSTR)strTicket);
	pMaster->setURL((LPCTSTR)strUrl);
	
	// initialise project loading...
	m_loader.Start((LPCTSTR)strUrl, pMaster, nId);

	OutText(L"Download initiated...");
	SetModifiedFlag(TRUE);
	
	return true;
}

/*bool CAdVisuoDoc::GetDownloadStatus(AVULONG &nStatus)
{
	CAdVisuoLoader::STATUS S = m_loader.Update();

	switch (S)
	{
		case CAdVisuoLoader::LOADING_DATA:
		case CAdVisuoLoader::COMPLETE:
			nStatus = 0;
			return true;
		case CAdVisuoLoader::FAILED:
			nStatus = 0x80000010;
			return false;
		default:
			AVULONG nProgress = m_loader.GetProgress();
			switch (nProgress >> 30)
			{
				case 2: nStatus = nProgress; break;				// this is error condition
				case 3: nStatus = (nProgress & 0xffff); break;	// waiting in the queue
				default:nStatus = 0; break;						// shouldn't go there... status will be the very front of the queue
			}
			return false;
	}
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////
// Non-Standard Command Handlers

void CAdVisuoDoc::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDownloadTerminated() && !((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
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

void CAdVisuoDoc::OnCloseDocument()
{
	if (GetLoaderStatus() == CAdVisuoLoader::TERMINATED && GetLoaderReasonForTermination() == CAdVisuoLoader::FAILED)
	{
		CDlgHtFailure dlg(m_loader.GetFailureTitle().c_str(), m_loader.GetFailureText().c_str(), m_loader.GetURL().c_str());
		dlg.DoModal();
	}

	CDocument::OnCloseDocument();
	if (AVGetApp()->CountDocuments() == 0)
		AfxGetMainWnd()->PostMessage(WM_CLOSE);

	OutText(L"Current session has been closed.");
}
