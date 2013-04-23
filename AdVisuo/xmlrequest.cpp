// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "xmlrequest.h"
#include <sstream>

std::wstring CXMLRequest::WAIT_FOR_RESPONSE;

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

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strRequest, std::wstring &strResponse)
{
	if (m_strUrl.empty()) return (m_h = E_INVALIDARG);

	m_strFunction = strFunction;
	m_strRequest = strRequest;

	SetEvent(m_hEvRequestSet);

	if (&strResponse == &WAIT_FOR_RESPONSE)
		return S_OK;
	else
	{
		wait(); 
		if (!ok())
			throw_exceptions();
		return get_response(strResponse);
	}
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam, AVLONG val, std::wstring &strResponse)
{
	std::wstringstream s;
	s << strParam << L"=" << val;
	return call(strFunction, s.str(), strResponse);
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, std::wstring &strResponse)
{
	std::wstringstream s;
	s << strParam1 << L"=" << val1 << L"&" << strParam2 << L"=" << val2;
	return call(strFunction, s.str(), strResponse);
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, std::wstring strParam3, AVLONG val3, std::wstring &strResponse)
{
	std::wstringstream s;
	s << strParam1 << L"=" << val1 << L"&" << strParam2 << L"=" << val2 << L"&" << strParam3 << L"=" << val3;
	return call(strFunction, s.str(), strResponse);
}

HRESULT CXMLRequest::wait(DWORD dwTimeout)
{
	return (WaitForSingleObject(m_hEvCompleted, dwTimeout) == WAIT_OBJECT_0) ? S_OK : E_FAIL;
}

HRESULT CXMLRequest::get_response(std::wstring &strResponse)
{
	if (!ok())
		throw_exceptions();
	strResponse = ready() ? m_strResponse : L"";

	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 

	SetEvent(m_hEvResponse);

	return S_OK;
}

HRESULT CXMLRequest::ignore_response()
{
	if (!ok())
		throw_exceptions();

	m_strResponse = L"";
	m_strStatus = L""; 
	m_nStatus = 0;
	m_h = S_OK; 
	m_nReadyState = 0; 

	SetEvent(m_hEvResponse);

	return S_OK;
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
