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

	static std::wstring WAIT_FOR_RESPONSE;

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
	HRESULT call(std::wstring strFunction, std::wstring strRequest, std::wstring &strResponse = WAIT_FOR_RESPONSE);
	HRESULT call(std::wstring strFunction, std::wstring strParam,  AVLONG val, std::wstring &strResponse = WAIT_FOR_RESPONSE);
	HRESULT call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, std::wstring &strResponse = WAIT_FOR_RESPONSE);
	HRESULT call(std::wstring strFunction, std::wstring strParam1, AVLONG val1, std::wstring strParam2, AVLONG val2, std::wstring strParam3, AVLONG val3, std::wstring &strResponse = WAIT_FOR_RESPONSE);

	// calls - helpers
	HRESULT AVVersion(std::wstring &strResponse = WAIT_FOR_RESPONSE)						{ return call(L"AVVersion", L"", strResponse); }
	HRESULT AVIndex(std::wstring &strResponse = WAIT_FOR_RESPONSE)							{ return call(L"AVIndex", L"", strResponse); }
	HRESULT AVProject(AVLONG nSimulationId, std::wstring &strResponse = WAIT_FOR_RESPONSE)	{ return call(L"AVProject", L"nSimulationId", nSimulationId, strResponse); }
	HRESULT AVLiftGroups(AVLONG nProjectId, std::wstring &strResponse = WAIT_FOR_RESPONSE)	{ return call(L"AVLiftGroups", L"nProjectId", nProjectId, strResponse); }
	HRESULT AVFloors(AVLONG nLiftGroupId, std::wstring &strResponse = WAIT_FOR_RESPONSE)	{ return call(L"AVFloors", L"nLiftGroupId", nLiftGroupId, strResponse); }
	HRESULT AVShafts(AVLONG nLiftGroupId, std::wstring &strResponse = WAIT_FOR_RESPONSE)	{ return call(L"AVShafts", L"nLiftGroupId", nLiftGroupId, strResponse); }
	HRESULT AVSim(AVLONG nLiftGroupId, std::wstring &strResponse = WAIT_FOR_RESPONSE)		{ return call(L"AVSim", L"nLiftGroupId", nLiftGroupId, strResponse); }
	HRESULT AVSimData(AVLONG nSimId, AVLONG timeFrom, AVLONG timeTo, std::wstring &strResponse = WAIT_FOR_RESPONSE)
											{ return call(L"AVSimData", L"nSimId", nSimId, L"timeFrom", timeFrom, L"timeTo", timeTo, strResponse); }
	HRESULT AVPrjData(AVLONG nProjectId, AVLONG timeFrom, AVLONG timeTo, std::wstring &strResponse = WAIT_FOR_RESPONSE)
											{ return call(L"AVPrjData", L"nProjectId", nProjectId, L"timeFrom", timeFrom, L"timeTo", timeTo, strResponse); }

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
