// Sim.h - a part of the AdVisuo Client Software

#pragma once

#import "msxml3.dll"

class _xmlreq_error
{
public:
	_xmlreq_error(long status, std::wstring msg) : _status(status), _msg(msg)  { }
	long status()			{ return _status; }
	std::wstring msg()		{ return _msg; }
private:
	long _status;
	std::wstring _msg;
};

class CXMLRequest
{
	std::wstring m_strUrl;
	std::wstring m_strFunction;
	std::wstring m_strRequest;

	HANDLE m_hEvRequestSet;
	HANDLE m_hEvResponse;
	HANDLE m_hEvCompleted;
	
	long m_nReadyState;
	long m_nStatus;
	std::wstring m_strStatus;
	std::wstring m_strResponse;

	HRESULT m_h;
	_com_error m_com_error;

public:
	static std::wstring ASYNC_RESPONSE;

public:
	CXMLRequest();
	CXMLRequest(std::wstring strUrl);
	virtual ~CXMLRequest();

	void setURL(std::wstring strUrl) { m_strUrl = strUrl; }

	// status
	std::wstring URL()		{ return m_strUrl; }
	bool ready()			{ return m_nReadyState == 4; }
	bool ok()				{ return ready() && SUCCEEDED(m_h) && m_nStatus <= 299; }

	// calls - generic
	void setreq(std::wstring strRequest = L"");
	void addreq(std::wstring strRequest);
	void addparam(std::wstring strParam,  AVLONG val);
	void addparam(std::wstring strParam,  std::wstring strVal);
	HRESULT call(std::wstring strFunction, std::wstring &strResponse = ASYNC_RESPONSE);

	// wait & get response
	HRESULT wait(DWORD dwTimeout = INFINITE);
	HRESULT get_response(std::wstring &strResponse);
	HRESULT ignore_response();

	// throw
	void throw_exceptions();

private:
	HRESULT ExecWorkerThread();
	friend UINT WorkerThread(void *p);
};

class CAVRequest : public CXMLRequest
{
	static std::wstring m_strUsername;
	static std::wstring m_strTicket;
public:
	CAVRequest() : CXMLRequest()							{ }
	CAVRequest(std::wstring strUrl) : CXMLRequest(strUrl)	{ }
	virtual ~CAVRequest()									{ }

	// calls - helpers
	HRESULT AVVersion(std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVIndex(std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVProject(AVLONG nSimulationId, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVLiftGroups(AVLONG nProjectId, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVFloors(AVLONG nLiftGroupId, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVShafts(AVLONG nLiftGroupId, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVSim(AVLONG nLiftGroupId, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVSimData(AVLONG nSimId, AVLONG timeFrom, AVLONG timeTo, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
	HRESULT AVPrjData(AVLONG nProjectId, AVLONG timeFrom, AVLONG timeTo, std::wstring &strResponse = CXMLRequest::ASYNC_RESPONSE);
};
