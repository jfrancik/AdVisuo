// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "xmlrequest.h"
#include "../CommonFiles/XMLTools.h"
#include <sstream>
#include <iomanip>

using namespace std;

	static wstring _url_encoded(wstring &str)
	{
		wstringstream s;
		s << setbase(16);
		for each (wchar_t ch in str)
		{
			if (isalnum(ch) || ch == '_')
				s << ch;
			else
				s << L"%" << setw(2) << setfill(L'0') << ((unsigned int)(unsigned char)ch);
		}
		return s.str();
	}

static UINT __cdecl WorkerThread(void *p)
{
	((CXMLRequest*)p)->ExecWorkerThread();
	return 0;
}

CXMLRequest::CXMLRequest() : m_hEvRequestSet(NULL), m_hEvResponseRdy(NULL), m_com_error(0)	
{
	InitializeCriticalSection(&cs);
	m_pAuthAgent = NULL;
}

CXMLRequest::~CXMLRequest()
{
	m_strFunction = L"";
	SetEvent(m_hEvRequestSet);
	WaitForSingleObject(m_hEvResponseRdy, 5000);
}

void CXMLRequest::create()
{
	m_hEvRequestSet = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEvResponseRdy = CreateEvent(NULL, FALSE, FALSE, NULL);
	AfxBeginThread(WorkerThread, this);
}

void CXMLRequest::take_authorisation_from(CXMLRequest *pAuthAgent)
{ 
	m_pAuthAgent = pAuthAgent; 
}

void CXMLRequest::set_authorisation_data(std::wstring strUsername, std::wstring strTicket)
{
	if (m_pAuthAgent)
		m_pAuthAgent->set_authorisation_data(strUsername, strTicket);
	else
	{
		EnterCriticalSection(&cs);
		m_strUsername = strUsername;
		m_strTicket = strTicket;
		LeaveCriticalSection(&cs);
	}
}

void CXMLRequest::get_authorisation_data(std::wstring &strUsername, std::wstring &strTicket)
{
	if (m_pAuthAgent)
		m_pAuthAgent->get_authorisation_data(strUsername, strTicket);
	else
	{
		EnterCriticalSection(&cs);
		strUsername = m_strUsername;
		strTicket = m_strTicket;
		LeaveCriticalSection(&cs);
	}
}

void CXMLRequest::setreq(wstring strRequest)
{
	m_strRequest = strRequest;
}

void CXMLRequest::addreq(wstring strRequest)
{
	if (m_strRequest != L"")
		m_strRequest += L"&";
	m_strRequest += strRequest;
}

void CXMLRequest::addparam(wstring strParam,  AVLONG val)
{
	wstringstream s;
	s << strParam << L"=" << val;
	addreq(s.str());
}

void CXMLRequest::addparam(wstring strParam,  wstring strVal)
{
	wstringstream s;
	s << strParam << L"=" << _url_encoded(strVal);
	wstring sss = s.str();
	addreq(s.str());
}

void CXMLRequest::addparam_authorisation()
{
	wstring strUsername;
	wstring strTicket;
	get_authorisation_data(strUsername, strTicket);
	addparam(L"strUsername", strUsername);
	addparam(L"strTicket", strTicket);
}


void CXMLRequest::call(wstring strFunction)
{
	if (m_strUrl.empty())
	{
		m_h = E_INVALIDARG;
		throw_exceptions();
	}

	m_strFunction = strFunction;
	SetEvent(m_hEvRequestSet);
}

HRESULT CXMLRequest::wait(DWORD dwTimeout)
{
	return (WaitForSingleObject(m_hEvResponseRdy, dwTimeout) == WAIT_OBJECT_0) ? S_OK : E_FAIL;
}

void CXMLRequest::get_response(wstring &strResponse)
{
	wait(INFINITE);

	if (!ok())
		throw_exceptions();
	
	strResponse = ready() ? m_strResponse : L"";

	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 
}

void CXMLRequest::ignore_response()
{
	if (!ok())
		throw_exceptions();

	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 
}

wstring CXMLRequest::unpack_as_string(wstring &strResponse, wstring defValue)
{
	xmltools::CXmlReader reader(strResponse.c_str());
	reader.read_simple_type(L"string");
	if (reader.find(L"string") == reader.end())
		return defValue;
	wstring str = reader[L"string"];
	return str;
}

int CXMLRequest::unpack_as_int(wstring &strResponse, int defValue)
{
	xmltools::CXmlReader reader(strResponse.c_str());
	reader.read_simple_type(L"int");
	if (reader.find(L"int") == reader.end())
		return defValue;
	int i = reader[L"int"];
	return i;
}

unsigned CXMLRequest::unpack_as_unsigned(std::wstring &strResponse, unsigned defValue)
{
	xmltools::CXmlReader reader(strResponse.c_str());
	reader.read_simple_type(L"unsignedInt");
	if (reader.find(L"unsignedInt") == reader.end())
		return defValue;
	unsigned u = (unsigned)(int)reader[L"unsignedInt"];
	return u;
}

void CXMLRequest::throw_exceptions()
{
	if (ok()) return;
	if FAILED(m_h) throw m_com_error;
	throw _xmlreq_error(m_nStatus, m_strStatus);
}

HRESULT CXMLRequest::ExecWorkerThread()
{
	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 

	CoInitialize(NULL);
	MSXML2::IXMLHTTPRequestPtr ptrHttpRequest;
	WaitForSingleObject(m_hEvRequestSet, INFINITE);
	while (!m_strFunction.empty())
	{
		try
		{
			m_h = ptrHttpRequest.CreateInstance("Microsoft.XMLHTTP");
			if (FAILED(m_h)) throw _com_error(m_h);
			// Send Http Request
			m_h = ptrHttpRequest->open(L"POST", (m_strUrl + L"\\" + m_strFunction).c_str(), false);
			if (FAILED(m_h)) throw _com_error(m_h);
			m_h = ptrHttpRequest->setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
			if (FAILED(m_h)) throw _com_error(m_h);
			m_h = ptrHttpRequest->send(m_strRequest.c_str());
			if (FAILED(m_h)) throw _com_error(m_h);

			m_strRequest.clear();

			// Read response and status
			m_strResponse = ptrHttpRequest->responseText;
			m_nReadyState = ptrHttpRequest->readyState;
			m_strStatus = ptrHttpRequest->statusText;
			m_nStatus = ptrHttpRequest->status;
		}
		catch(_com_error& e)
		{
			m_h = e.Error();
			m_com_error = _com_error(e);
			ASSERT(FAILED(m_h));
		}
		catch(...)
		{
			m_h = E_FAIL;
		}

		// End http session
		if (ptrHttpRequest) ptrHttpRequest->abort();

		SetEvent(m_hEvResponseRdy);
		WaitForSingleObject(m_hEvRequestSet, INFINITE);
	}

	SetEvent(m_hEvResponseRdy);
	CoUninitialize();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Functions

wstring CXMLRequest::AVGetAppName()
{ 
	wstring response;
	setreq(L"keyName=APP_NAME");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

int CXMLRequest::AVGetRequiredVersion()
{
	wstring response;
	setreq(L"keyName=VERSION_REQUIRED");
	call(L"AVGetIntStatus"); 
	get_response(response);
	return unpack_as_int(response);
}

wstring CXMLRequest::AVGetRequiredVersionDate()
{
	wstring response;
	setreq(L"keyName=VERSION_REQUIRED");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

int CXMLRequest::AVGetLatestVersion()
{ 
	wstring response;
	setreq(L"keyName=VERSION_LATEST");
	call(L"AVGetIntStatus"); 
	get_response(response);
	return unpack_as_int(response);
}

wstring CXMLRequest::AVGetLatestVersionDate()
{ 
	wstring response;
	setreq(L"keyName=VERSION_LATEST");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

wstring CXMLRequest::AVGetLatestVersionDownloadPath()
{
	wstring response;
	setreq(L"keyName=DOWNLOAD_PATH");
	call(L"AVGetStrStatus"); 
	get_response(response);
	CString path;
	path.Format(unpack_as_string(response).c_str(), AVGetLatestVersion());
	return (LPCTSTR)path;
}

bool CXMLRequest::AVLogin(wstring strUsername, wstring strPassword)
{
	wstring response;
	setreq();
	addparam(L"strUsername", strUsername);
	addparam(L"strPassword", strPassword);
	call(L"AVCreateTicket");
	get_response(response);
	wstring strTicket = unpack_as_string(response);
	if (strTicket.size() == 0)
		return false;
	set_authorisation_data(strUsername, strTicket);
	return true;
}

int CXMLRequest::AVIsAuthorised()
{
	wstring response;
	setreq();
	addparam_authorisation();
	call(L"AVValidateTicket");
	get_response(response);
	return unpack_as_int(response);
}

bool CXMLRequest::AVExtendAuthorisation()
{
	wstring response;
	setreq();
	addparam_authorisation();
	call(L"AVRevalidateTicket");
	get_response(response);

	wstring strUsername;
	wstring strTicket;
	get_authorisation_data(strUsername, strTicket);
	strTicket = unpack_as_string(response);
	if (strTicket.size() == 0)
		return false;
	set_authorisation_data(strUsername, strTicket);
	return true;
}

unsigned CXMLRequest::AVPrjProgress(AVLONG nSimulationId)
{
	wstring response;
	setreq();
	addparam_authorisation();
	addparam(L"nSimulationId", nSimulationId);
	call(L"AVPrjProgress");
	get_response(response);
	return unpack_as_unsigned(response);
}

void CXMLRequest::AVReportIssue(std::wstring url, std::wstring strUsername, std::wstring strTicket, AVULONG nVersion, AVULONG nId, std::wstring strPath, AVULONG nCat, std::wstring strUserDesc, std::wstring strDiagnostic, std::wstring strErrorMsg)
{
	setreq();
	addparam(L"url", url);
	addparam(L"strUsername", strUsername);
	addparam(L"strTicket", strTicket);
	addparam(L"nVersion", nVersion);
	addparam(L"nId", nId);
	addparam(L"strPath", strPath);
	addparam(L"nCat", nCat);
	addparam(L"strUserDesc", strUserDesc);
	addparam(L"strDiagnostic", strDiagnostic);
	addparam(L"strErrorMsg", strErrorMsg);
	call(L"AVReportIssue");
}

void CXMLRequest::AVFolders()
{
	setreq();
	addparam_authorisation();
	call(L"AVFolders"); 
}

void CXMLRequest::AVIndex()
{
	setreq();
	addparam_authorisation();
	call(L"AVIndex"); 
}

void CXMLRequest::AVProject(AVLONG nSimulationId)
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nSimulationId", nSimulationId); 
	call(L"AVProject"); 
}

void CXMLRequest::AVLiftGroups(AVLONG nProjectId)
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nProjectId", nProjectId); 
	call(L"AVLiftGroups"); 
}

void CXMLRequest::AVFloors(AVLONG nLiftGroupId)	
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nLiftGroupId", nLiftGroupId); 
	call(L"AVFloors"); 
}

void CXMLRequest::AVShafts(AVLONG nLiftGroupId)	
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nLiftGroupId", nLiftGroupId);
	call(L"AVShafts");
}

void CXMLRequest::AVSim(AVLONG nLiftGroupId)		
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nLiftGroupId", nLiftGroupId); 
	call(L"AVSim"); 
}

void CXMLRequest::AVSimData(AVLONG nSimId, AVLONG timeFrom, AVLONG timeTo)
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nSimId", nSimId);
	addparam(L"timeFrom", timeFrom);
	addparam(L"timeTo", timeTo);
	call(L"AVSimData"); 
}

void CXMLRequest::AVPrjData(AVLONG nProjectId, AVLONG timeFrom, AVLONG timeTo)
{ 
	setreq();
	addparam_authorisation();
	addparam(L"nProjectId", nProjectId);
	addparam(L"timeFrom", timeFrom);
	addparam(L"timeTo", timeTo);
	call(L"AVPrjData");
}
