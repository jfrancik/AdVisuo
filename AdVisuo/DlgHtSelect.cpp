// DlgHtLogin.cpp : implementation file
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "DlgHtSelect.h"
#include "VisProject.h"
#include "XMLRequest.h"

#include "../CommonFiles/XMLTools.h"
#import "Msxml3.dll" named_guids raw_interfaces_only	//	XML parser

// CDlgHtSelect dialog

IMPLEMENT_DYNCREATE(CDlgHtSelect, CDlgHtBase)

CDlgHtSelect::CDlgHtSelect(std::wstring url, AVULONG nProjectId, AVULONG nSimulationId, bool bGotoSimulations, CWnd* pParent)
	: CDlgHtBase(IDD, IDH)
{
	m_url = url;
	m_nProjectId = 0;
	m_nSimulationId = 0;
	m_nSortModePrj = m_nSortModeSim = 0;
	m_bSortAscPrj  = m_bSortAscSim  = true;

	m_nSimulationToShow = nSimulationId;
	
	m_fnLoadComplHandle = [this, nProjectId, nSimulationId, bGotoSimulations] { if (!bGotoSimulations) GotoProjects(nProjectId); else GotoSimulations(nProjectId, nSimulationId); };
}

CDlgHtSelect::~CDlgHtSelect()
{
	for each (CProjectVis *pPrj in m_prjs)
		delete pPrj;
}

void CDlgHtSelect::Load(CXMLRequest &http)
{
	std::wstring response;

	http.AVFolders();
	http.get_response(response);
	CProjectVis::LoadFoldersFromBuf(response.c_str(), m_folders);

	http.AVIndex();
	http.get_response(response);
	CProjectVis::LoadIndexFromBuf(response.c_str(), m_prjs);
}

void CDlgHtSelect::DoDataExchange(CDataExchange* pDX)
{
	CDlgHtBase::DoDataExchange(pDX);
	DDX_DHtml_ElementText(pDX, _T("idProjectId"), DISPID_A_VALUE, m_nProjectId); 
	DDX_DHtml_ElementText(pDX, _T("idSimulationId"), DISPID_A_VALUE, m_nSimulationId); 
}

BEGIN_MESSAGE_MAP(CDlgHtSelect, CDlgHtBase)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgHtSelect)
	DHTML_EVENT_ONCLICK(_T("idCancelButton"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("idOpenProject"), OnButtonOpenProject)
	DHTML_EVENT_ONCLICK(_T("idOpenSimulation"), OnButtonOpenSimulation)
	DHTML_EVENT_ONCLICK(_T("idBack"), OnButtonBack)

	DHTML_EVENT_ONCLICK(_T("idSortByPrjDate"), OnButtonSortPrjDate)
	DHTML_EVENT_ONCLICK(_T("idSortByPrjName"), OnButtonSortPrjName)
	DHTML_EVENT_ONCLICK(_T("idSortByPrjNo"), OnButtonSortPrjNo)
	DHTML_EVENT_ONCLICK(_T("idSortBySimDate"), OnButtonSortSimDate)
	DHTML_EVENT_ONCLICK(_T("idSortBySimName"), OnButtonSortSimName)
	
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(CDlgHtSelect, CDlgHtBase)
END_DISPATCH_MAP()

////////////////////////////////////////////////////////////////////////
// Javascript Bounding

////////////////////////////////////////////////////////////////////////
// Button Handlers

void CDlgHtSelect::OnOK()
{
	CDlgHtBase::OnOK();
}

void CDlgHtSelect::OnCancel()
{
	CDlgHtBase::OnCancel();
}

HRESULT CDlgHtSelect::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonOpenProject(IHTMLElement* /*pElement*/)
{
	UpdateData(TRUE);
	GotoSimulations(m_nProjectId, m_nSimulationToShow);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonOpenSimulation(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonBack(IHTMLElement* /*pElement*/)
{
	GotoProjects(m_nProjectId);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonSortPrjDate(IHTMLElement *pElement)
{
	if (m_nSortModePrj == 0) m_bSortAscPrj = !m_bSortAscPrj; else { m_nSortModePrj = 0; m_bSortAscPrj = true; }
	UpdateData(TRUE);
	GotoProjects(m_nProjectId);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonSortPrjName(IHTMLElement *pElement)
{
	if (m_nSortModePrj == 1) m_bSortAscPrj = !m_bSortAscPrj; else { m_nSortModePrj = 1; m_bSortAscPrj = true; }
	UpdateData(TRUE);
	GotoProjects(m_nProjectId);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonSortPrjNo(IHTMLElement *pElement)
{
	if (m_nSortModePrj == 2) m_bSortAscPrj = !m_bSortAscPrj; else { m_nSortModePrj = 2; m_bSortAscPrj = true; }
	UpdateData(TRUE);
	GotoProjects(m_nProjectId);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonSortSimDate(IHTMLElement *pElement)
{
	if (m_nSortModeSim == 0) m_bSortAscSim = !m_bSortAscSim; else { m_nSortModeSim = 0; m_bSortAscSim = true; }
	UpdateData(TRUE);
	GotoSimulations(m_nProjectId, m_nSimulationId);
	return S_OK;
}

HRESULT CDlgHtSelect::OnButtonSortSimName(IHTMLElement *pElement)
{
	if (m_nSortModeSim == 1) m_bSortAscSim = !m_bSortAscSim; else { m_nSortModeSim = 1; m_bSortAscSim = true; }
	UpdateData(TRUE);
	GotoSimulations(m_nProjectId, m_nSimulationId);
	return S_OK;
}

	void __debug_save(LPCTSTR pFilename, _bstr_t &bstr)
	{
		CComPtr<IStream> pFileStream;
		HRESULT h = SHCreateStreamOnFile(pFilename, STGM_CREATE | STGM_WRITE, &pFileStream); if (FAILED(h)) throw _com_error(h);
		CString str = bstr;
		pFileStream->Write((LPCTSTR)str, 2 * str.GetLength(), NULL);
	}

void CDlgHtSelect::GotoProjects(AVULONG nProjectId)
{
	CString str = GenContent();
	WriteDoc(str);
	if (nProjectId) SelectProject(nProjectId);
}

void CDlgHtSelect::GotoSimulations(AVULONG nProjectId, AVULONG nSimulationId)
{
	CString str = GenContent(nProjectId);
	WriteDoc(str);
	if (nSimulationId) SelectSimulation(nSimulationId);
}

void CDlgHtSelect::SelectProject(AVULONG nProjectId)
{
	std::wstringstream s;
	s << L"selectProjectById(" << nProjectId << L")";
	ExecJS(s.str().c_str());
}

void CDlgHtSelect::SelectSimulation(AVULONG nSimulationId)
{
	std::wstringstream s;
	s << L"selectSimulationById(" << nSimulationId << L")";
	ExecJS(s.str().c_str());
}

CString CDlgHtSelect::GenContent()
{
	// sort projects
	AVULONG nSortMode = m_nSortModePrj;
	bool bSortAsc = m_bSortAscPrj;
	std::sort(m_prjs.begin(), m_prjs.end(),
		[nSortMode, bSortAsc](CProjectVis *p1, CProjectVis *p2) -> bool
		{
			switch (nSortMode)
			{
			case 0:
				{
					DATE date1 = (*p1)[L"CreatedDate"];
					DATE date2 = (*p2)[L"CreatedDate"];
					if (date1 == date2) break;
					if (bSortAsc) return date1 < date2; else return date1 > date2; 
				}
			case 1:
				{
					std::wstring name1 = (*p1)[L"ProjectName"];
					std::wstring name2 = (*p2)[L"ProjectName"];
					std::transform(name1.begin(), name1.end(), name1.begin(), toupper);
					std::transform(name2.begin(), name2.end(), name2.begin(), toupper);
					if (name1 == name2) break;
					if (bSortAsc) return name1 < name2; else return name1 > name2; 
				}
			case 2:
				{
					std::wstring prno1 = (*p1)[L"ProjectNo"];
					std::wstring prno2 = (*p2)[L"ProjectNo"];
					std::transform(prno1.begin(), prno1.end(), prno1.begin(), toupper);
					std::transform(prno2.begin(), prno2.end(), prno2.begin(), toupper);
					if (prno1 == prno2) break;
					if (bSortAsc) return prno1 < prno2; else return prno1 > prno2; 
				}
			}
			// if equal - sort by id (to keep projects together) - with no regard to asc/desc
			AVULONG id1 = (*p1)[L"ProjectId"];
			AVULONG id2 = (*p2)[L"ProjectId"];
			return id1 > id2;
		});

	// find the module name - for the CSS link
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);
	wchar_t lpszModule[_MAX_PATH];
	DWORD dm = GetModuleFileName(hInstance, lpszModule, _MAX_PATH);
	ASSERT(dm);

	// Create XML structure
	xmltools::CXmlWriter writer(L"utf-8", 4 * 1024 * 1024);
	writer.write(L"AdVisuo-Path", lpszModule);
	writer.write(L"Server-Path", m_url.c_str());
	writer.write(L"Software-Version-Major", VERSION_MAJOR);
	writer.write(L"Software-Version-Minor", VERSION_MINOR);
	writer.write(L"Software-Version-Rel", VERSION_REV);
	writer.write(L"Software-Version-Date", VERSION_DATE);
	writer.write(L"SortMode", m_nSortModePrj);
	writer.write(L"SortAsc", (int)m_bSortAscPrj);

	for each (std::wstring str in m_folders)
		writer.write(L"AVFolder", str.c_str());
	AVLONG id = -1;
	for each (CProjectVis *pProject in m_prjs)
	{
		AVLONG i = (*pProject)[L"ProjectId"];
		if (i != id)
			writer.write(L"AVProject", *pProject);
		id = i;
	}

	return GenContent(writer, IDR_XSLT_SELECT_PRJ);
}

CString CDlgHtSelect::GenContent(AVULONG nProjectId)
{
	// sort projects
	AVULONG nSortMode = m_nSortModeSim;
	bool bSortAsc = m_bSortAscSim;
	std::sort(m_prjs.begin(), m_prjs.end(),
		[nSortMode, bSortAsc](CProjectVis *p1, CProjectVis *p2) -> bool
		{
			switch (nSortMode)
			{
			case 0:
				{
					DATE date1 = (*p1)[L"SimCreatedDate"];
					DATE date2 = (*p2)[L"SimCreatedDate"];
					if (date1 == date2) break;
					if (bSortAsc) return date1 < date2; else return date1 > date2; 
				}
			case 1:
				{
					std::wstring name1 = (*p1)[L"SimName"];
					std::wstring name2 = (*p2)[L"SimName"];
					std::transform(name1.begin(), name1.end(), name1.begin(), toupper);
					std::transform(name2.begin(), name2.end(), name2.begin(), toupper);
					if (name1 == name2) break;
					if (bSortAsc) return name1 < name2; else return name1 > name2; 
				}
			}
			// if equal - sort by id (to keep projects together) - with no regard to asc/desc
			AVULONG id1 = (*p1)[L"ProjectId"];
			AVULONG id2 = (*p2)[L"ProjectId"];
			return id1 > id2;
		});

	// find the module name - for the CSS link
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);
	wchar_t lpszModule[_MAX_PATH];
	DWORD dm = GetModuleFileName(hInstance, lpszModule, _MAX_PATH);
	ASSERT(dm);

	// Create XML structure
	xmltools::CXmlWriter writer(L"utf-8", 4 * 1024 * 1024);
	writer.write(L"AdVisuo-Path", lpszModule);
	writer.write(L"Server-Path", m_url.c_str());
	writer.write(L"Software-Version-Major", VERSION_MAJOR);
	writer.write(L"Software-Version-Minor", VERSION_MINOR);
	writer.write(L"Software-Version-Rel", VERSION_REV);
	writer.write(L"Software-Version-Date", VERSION_DATE);
	writer.write(L"SortMode", m_nSortModeSim);
	writer.write(L"SortAsc", (int)m_bSortAscSim);
	for each (std::wstring str in m_folders)
		writer.write(L"AVFolder", str.c_str());
	for each (CProjectVis *pProject in m_prjs)
	{
		AVLONG i = (*pProject)[L"ProjectId"];
		if (i == nProjectId)
			writer.write(L"AVProject", *pProject);
	}

	return GenContent(writer, IDR_XSLT_SELECT_SIM);
}

CString CDlgHtSelect::GenContent(xmltools::CXmlWriter &writer, AVULONG nXslRes)
{
	HRESULT h;

	// XML Text
	LPCSTR pXml8 = (LPSTR)writer.LockBuffer();
	int nSize = MultiByteToWideChar(CP_UTF8 /*28591*/, 0, pXml8, -1, NULL, 0);
	LPTSTR pXml = new wchar_t[nSize];
	MultiByteToWideChar(CP_UTF8 /*28591*/, 0, pXml8, -1, pXml, nSize);
	_bstr_t bstrXml = ::SysAllocString(pXml);
	writer.UnlockBuffer();

	MSXML2::IXMLDOMDocument *pXmlDoc = NULL;
  	if FAILED(h = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pXmlDoc)) throw(_com_error(h));
	h = pXmlDoc->loadXML(bstrXml);

	// XSL Text - 8 bit
	HRSRC hRes = ::FindResource(NULL, MAKEINTRESOURCE(nXslRes), RT_HTML);
	HGLOBAL hGlob = ::LoadResource(NULL, hRes);
	LPCSTR pRes = (LPCSTR )::LockResource(hGlob);
	nSize = SizeofResource(NULL, hRes);
	char *pXsl8 = new char[nSize+1];
	strncpy_s(pXsl8, nSize+1, pRes, nSize);
	pXsl8[nSize] = '\0';
	nSize = MultiByteToWideChar(CP_UTF8 /*28591*/, 0, pXsl8, -1, NULL, 0);
	LPTSTR pXsl = new wchar_t[nSize];
	MultiByteToWideChar(CP_UTF8 /*28591*/, 0, pXsl8, -1, pXsl, nSize);
	delete [] pXsl8;
	_bstr_t bstrXsl = ::SysAllocString(pXsl);

	MSXML2::IXMLDOMDocument *pXslDoc = NULL;
  	if FAILED(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pXslDoc)) throw(_com_error(h));
	h = pXslDoc->loadXML(bstrXsl);

	// Debug
	//__debug_save(L"log.xml", bstrXml);
	//__debug_save(L"log.xsl", bstrXsl);

	// transform 
	CString str = pXmlDoc->transformNode(pXslDoc);

	// Debug
	//_bstr_t bstrHtml = str.AllocSysString();
	//__debug_save(L"log.html", bstrHtml);

	return str;
}

BOOL CDlgHtSelect::OnInitDialog()
{
	SetTimer(100, 60000, NULL);
	return CDlgHtBase::OnInitDialog();
}

void CDlgHtSelect::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDlgHtBase::OnDocumentComplete(pDisp, szUrl);
}


void CDlgHtSelect::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
		AVGetApp()->ExtendAuthorisation();

	CDlgHtBase::OnTimer(nIDEvent);
}
