// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "xmlrequest.h"
#include "../CommonFiles/XMLTools.h"
#include <sstream>

UINT __cdecl WorkerThread(void *p)
{
	((CXMLRequest*)p)->ExecWorkerThread();
	return 0;
}

CXMLRequest::CXMLRequest() : m_hEvRequestSet(NULL), m_hEvCompleted(NULL), m_com_error(0)	
{ 
	m_hEvRequestSet = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
	AfxBeginThread(WorkerThread, this);
}

CXMLRequest::CXMLRequest(std::wstring strUrl) : m_strUrl(strUrl), m_hEvRequestSet(NULL), m_hEvCompleted(NULL), m_com_error(0)	
{ 
	m_hEvRequestSet = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
	AfxBeginThread(WorkerThread, this);
}

CXMLRequest::~CXMLRequest()
{
	m_strFunction = L"";
	SetEvent(m_hEvRequestSet);
	WaitForSingleObject(m_hEvCompleted, 5000);
}

void CXMLRequest::setreq(std::wstring strRequest)
{
	m_strRequest = strRequest;
}

void CXMLRequest::addreq(std::wstring strRequest)
{
	if (m_strRequest != L"")
		m_strRequest += L"&";
	m_strRequest += strRequest;
}

void CXMLRequest::addparam(std::wstring strParam,  AVLONG val)
{
	std::wstringstream s;
	s << strParam << L"=" << val;
	addreq(s.str());
}

void CXMLRequest::addparam(std::wstring strParam,  std::wstring strVal)
{
	std::wstringstream s;
	s << strParam << L"=" << strVal;
	addreq(s.str());
}

void CXMLRequest::call(std::wstring strFunction)
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
	return (WaitForSingleObject(m_hEvCompleted, dwTimeout) == WAIT_OBJECT_0) ? S_OK : E_FAIL;
}

void CXMLRequest::get_response(std::wstring &strResponse, DWORD dwTimeout)
{
	if (dwTimeout > 0)
		wait(dwTimeout);

	if (!ok())
		throw_exceptions();
	
	strResponse = ready() ? m_strResponse : L"";

	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 

	SetEvent(m_hEvResponse);
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

	SetEvent(m_hEvResponse);
}

std::wstring CXMLRequest::unpack_as_string(std::wstring &strResponse, std::wstring defValue)
{
	xmltools::CXmlReader reader(strResponse.c_str());
	reader.read_simple_type(L"string");
	if (reader.find(L"string") == reader.end())
		return defValue;
	std::wstring str = reader[L"string"];
	return str;
}

int CXMLRequest::unpack_as_int(std::wstring &strResponse, int defValue)
{
	xmltools::CXmlReader reader(strResponse.c_str());
	reader.read_simple_type(L"int");
	if (reader.find(L"int") == reader.end())
		return defValue;
	int i = reader[L"int"];
	return i;
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

			// Read response and status
			m_strResponse = ptrHttpRequest->responseText;
			m_nReadyState = ptrHttpRequest->readyState;
			m_strStatus = ptrHttpRequest->statusText;
			m_nStatus = ptrHttpRequest->status;

			WaitForSingleObject(m_hEvResponse, INFINITE);
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

		SetEvent(m_hEvCompleted);
		WaitForSingleObject(m_hEvRequestSet, INFINITE);
	}

	SetEvent(m_hEvCompleted);
	CoUninitialize();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Functions

std::wstring CXMLRequest::AVGetAppName()
{ 
	std::wstring response;
	setreq(L"keyName=APP_NAME");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

int CXMLRequest::AVGetRequiredVersion()
{
	std::wstring response;
	setreq(L"keyName=VERSION_REQUIRED");
	call(L"AVGetIntStatus"); 
	get_response(response);
	return unpack_as_int(response);
}

std::wstring CXMLRequest::AVGetRequiredVersionDate()
{
	std::wstring response;
	setreq(L"keyName=VERSION_REQUIRED");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

int CXMLRequest::AVGetLatestVersion()
{ 
	std::wstring response;
	setreq(L"keyName=VERSION_LATEST");
	call(L"AVGetIntStatus"); 
	get_response(response);
	return unpack_as_int(response);
}

std::wstring CXMLRequest::AVGetLatestVersionDate()
{ 
	std::wstring response;
	setreq(L"keyName=VERSION_LATEST");
	call(L"AVGetStrStatus"); 
	get_response(response);
	return unpack_as_string(response);
}

std::wstring CXMLRequest::AVGetLatestVersionDownloadPath()
{
	std::wstring response;
	setreq(L"keyName=DOWNLOAD_PATH");
	call(L"AVGetStrStatus"); 
	get_response(response);
	CString path;
	path.Format(unpack_as_string(response).c_str(), AVGetLatestVersion());
	return (LPCTSTR)path;
}



bool CXMLRequest::AVLogin(std::wstring strUsername, std::wstring strPassword)
{
	m_strUsername = strUsername;
	std::wstring response;
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strPassword", strPassword);
	call(L"AVCreateTicket");
	get_response(response);
	m_strTicket = unpack_as_string(response);
	return m_strTicket.size() > 0;
}

bool CXMLRequest::AVIsAuthorised()
{
	return false;
}

bool CXMLRequest::AVExtendAuthorisation()
{
	return false;
}

void CXMLRequest::AVIndex()
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	call(L"AVIndex"); 
}

void CXMLRequest::AVProject(AVLONG nSimulationId)
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nSimulationId", nSimulationId); 
	call(L"AVProject"); 
}

void CXMLRequest::AVLiftGroups(AVLONG nProjectId)
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nProjectId", nProjectId); 
	call(L"AVLiftGroups"); 
}

void CXMLRequest::AVFloors(AVLONG nLiftGroupId)	
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nLiftGroupId", nLiftGroupId); 
	call(L"AVFloors"); 
}

void CXMLRequest::AVShafts(AVLONG nLiftGroupId)	
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nLiftGroupId", nLiftGroupId);
	call(L"AVShafts");
}

void CXMLRequest::AVSim(AVLONG nLiftGroupId)		
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nLiftGroupId", nLiftGroupId); 
	call(L"AVSim"); 
}

void CXMLRequest::AVSimData(AVLONG nSimId, AVLONG timeFrom, AVLONG timeTo)
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nSimId", nSimId);
	addparam(L"timeFrom", timeFrom);
	addparam(L"timeTo", timeTo);
	call(L"AVSimData"); 
}

void CXMLRequest::AVPrjData(AVLONG nProjectId, AVLONG timeFrom, AVLONG timeTo)
{ 
	setreq();
	addparam(L"strUsername", m_strUsername);
	addparam(L"strTicket", m_strTicket);
	addparam(L"nProjectId", nProjectId);
	addparam(L"timeFrom", timeFrom);
	addparam(L"timeTo", timeTo);
	call(L"AVPrjData");
}
