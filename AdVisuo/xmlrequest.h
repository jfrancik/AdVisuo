// Sim.h - a part of the AdVisuo Client Software

#pragma once

#import "msxml3.dll"

class _xmlreq_error
{
public:
	_xmlreq_error(long status, std::wstring msg) : _status(status), _msg(msg)  { }
	long status()			{ return _status; }
	std::wstring msg()	{ return _msg; }
private:
	long _status;
	std::wstring _msg;
};

class CXMLRequest
{
	std::wstring m_strUrl;
	std::wstring m_strFunction;
	std::wstring m_strRequest;

	HANDLE m_hEvCompleted;
	
	long m_nReadyState;
	long m_nStatus;
	std::wstring m_strStatus;
	std::wstring m_strResponse;

	HRESULT m_h;
	_com_error m_com_error;

public:
	CXMLRequest() : m_hEvCompleted(NULL), m_com_error(0)	{ reset(); }
	CXMLRequest(std::wstring strUrl) : m_strUrl(strUrl), m_hEvCompleted(NULL), m_com_error(0)	{ reset(); }
	virtual ~CXMLRequest()	{ }

	void setURL(std::wstring strUrl) { m_strUrl = strUrl; }

	// status
	std::wstring URL()		{ return m_strUrl; }
	bool ready()			{ return m_nReadyState == 4; }
	bool ok()				{ return ready() && SUCCEEDED(m_h) && m_nStatus <= 299; }
	HRESULT h()				{ return m_h; }
	long status()			{ return m_nStatus; }

	// response
	std::wstring response()	{ return ok() ? m_strResponse : L""; }

	// call
	HRESULT call(std::wstring strFunction, std::wstring strRequest, bool bWait = true);
	HRESULT call(std::wstring strFunction, std::wstring strParam, AVULONG val, bool bWait = true);
	HRESULT call(std::wstring strFunction, std::wstring strParam1, AVULONG val1, std::wstring strParam2, AVULONG val2, bool bWait = true);
	HRESULT call(std::wstring strFunction, std::wstring strParam1, AVULONG val1, std::wstring strParam2, AVULONG val2, std::wstring strParam3, AVULONG val3, bool bWait = true);

	HRESULT AVVersion(bool bWait = true)						{ return call(L"AVVersion", L"", bWait); }
	HRESULT AVIndex(bool bWait = true)							{ return call(L"AVIndex", L"", bWait); }
	HRESULT AVProject(AVULONG nSimulationId, bool bWait = true)	{ return call(L"AVProject", L"nSimulationId", nSimulationId, bWait); }
	HRESULT AVLiftGroups(AVULONG nProjectId, bool bWait = true)	{ return call(L"AVLiftGroups", L"nProjectId", nProjectId, bWait); }
	HRESULT AVFloors(AVULONG nLiftGroupId, bool bWait = true)	{ return call(L"AVFloors", L"nLiftGroupId", nLiftGroupId, bWait); }
	HRESULT AVShafts(AVULONG nLiftGroupId, bool bWait = true)	{ return call(L"AVShafts", L"nLiftGroupId", nLiftGroupId, bWait); }
	HRESULT AVSim(AVULONG nLiftGroupId, bool bWait = true)		{ return call(L"AVSim", L"nLiftGroupId", nLiftGroupId, bWait); }
	HRESULT AVSimData(AVULONG nSimId, AVULONG timeFrom, AVULONG timeTo, bool bWait = true)
												{ return call(L"AVSimData", L"nSimId", nSimId, L"timeFrom", timeFrom, L"timeTo", timeTo, bWait); }
	HRESULT AVPrjData(AVULONG nProjectId, AVULONG timeFrom, AVULONG timeTo, bool bWait = true)
												{ return call(L"AVPrjData", L"nProjectId", nProjectId, L"timeFrom", timeFrom, L"timeTo", timeTo, bWait); }

	// wait
	HRESULT wait(DWORD dwTimeout = INFINITE);

	// throw
	void throw_exceptions();

	// reset
	void reset()								{ m_nReadyState = 0; m_nStatus = 0; m_h = S_OK; m_strStatus = L""; m_strResponse = L""; }

private:
	HRESULT ExecWorkerThread();
	friend UINT WorkerThread(void *p);
};
