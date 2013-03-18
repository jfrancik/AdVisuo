// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "xmlrequest.h"

#include <sstream>

UINT __cdecl WorkerThread(void *p)
{
	TRACE(L"Entry");
	((CXMLRequest*)p)->ExecWorkerThread();
	TRACE(L"Exit");
	return 0;
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strRequest, bool bWait)
{
	if (m_strUrl.empty()) return (m_h = E_INVALIDARG);

	m_strFunction = strFunction;
	m_strRequest = strRequest;

	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);

	AfxBeginThread(WorkerThread, this);

	if (bWait)
	{
		wait(); 
		if (!ok())
			throw_exceptions();
	}

	return S_OK;
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam, AVLONG val, bool bWait)
{
	std::wstringstream s;
	s << strParam << L"=" << val;
	return call(strFunction, s.str(), bWait);
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, bool bWait)
{
	std::wstringstream s;
	s << strParam1 << L"=" << val1 << L"&" << strParam2 << L"=" << val2;
	return call(strFunction, s.str(), bWait);
}

HRESULT CXMLRequest::call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, std::wstring strParam3, AVLONG val3, bool bWait)
{
	std::wstringstream s;
	s << strParam1 << L"=" << val1 << L"&" << strParam2 << L"=" << val2 << L"&" << strParam3 << L"=" << val3;
	return call(strFunction, s.str(), bWait);
}

HRESULT CXMLRequest::wait(DWORD dwTimeout)
{
	return (WaitForSingleObject(m_hEvCompleted, dwTimeout) == WAIT_OBJECT_0) ? S_OK : E_FAIL;
}

void CXMLRequest::throw_exceptions()
{
	if (ok()) return;
	if FAILED(m_h) throw m_com_error;
	throw _xmlreq_error(m_nStatus, m_strStatus);
}

HRESULT CXMLRequest::ExecWorkerThread()
{
	reset();

	CoInitialize(NULL);
	MSXML2::IXMLHTTPRequestPtr ptrHttpRequest;
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
		m_nReadyState = ptrHttpRequest->readyState;
		m_nStatus = ptrHttpRequest->status;
		m_strStatus = ptrHttpRequest->statusText;
		m_strResponse = ptrHttpRequest->responseText;
	}
	catch(_com_error& e)
	{
		m_h = e.Error();
		m_com_error = _com_error(e);
		ASSERT(FAILED(m_h));

		//m_h = e.Error();
		//m_strComStatus = (wchar_t *)e.Description() ? e.Description() : e.ErrorMessage();

		//if (ptrHttpRequest)
		//{
		//	m_nReadyState = ptrHttpRequest->readyState;
		//	m_nStatus = ptrHttpRequest->status;
		//	m_strStatus = ptrHttpRequest->statusText;
		//	m_strResponse = ptrHttpRequest->responseText;
		//}
	}
	catch(...)
	{
		m_h = E_FAIL;
	}

	// End http session
	if (ptrHttpRequest) ptrHttpRequest->abort();

	SetEvent(m_hEvCompleted);
	CoUninitialize();
	return 0;
}
