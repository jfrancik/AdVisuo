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
	// connection params
	std::wstring m_strUrl;
	std::wstring m_strFunction;
	std::wstring m_strRequest;

	// event handlers
	HANDLE m_hEvRequestSet;
	HANDLE m_hEvResponse;
	HANDLE m_hEvCompleted;
	
	// status information
	long m_nReadyState;
	long m_nStatus;
	std::wstring m_strStatus;
	std::wstring m_strResponse;
	HRESULT m_h;
	_com_error m_com_error;

	// authorisation details
	std::wstring m_strUsername;
	std::wstring m_strTicket;

public:
	CXMLRequest();
	CXMLRequest(std::wstring strUrl);
	virtual ~CXMLRequest();

	void setURL(std::wstring strUrl)			{ m_strUrl = strUrl; }
	void authorise_from(CXMLRequest &req)		{ m_strUsername = req.m_strUsername; m_strTicket = req.m_strTicket; }

	// status
	std::wstring URL()		{ return m_strUrl; }
	bool ready()			{ return m_nReadyState == 4; }
	bool ok()				{ return ready() && SUCCEEDED(m_h) && m_nStatus <= 299; }

	// calls - generic
	void setreq(std::wstring strRequest = L"");
	void addreq(std::wstring strRequest);
	void addparam(std::wstring strParam,  AVLONG val);
	void addparam(std::wstring strParam,  std::wstring strVal);
	void call(std::wstring strFunction);

	// wait & get response
	HRESULT wait(DWORD dwTimeout = INFINITE);
	void get_response(std::wstring &strResponse, DWORD dwTimeout = INFINITE);
	void ignore_response();

	// unpack value
	std::wstring unpack_as_string(std::wstring &strResponse, std::wstring defValue = L"");
	int unpack_as_int(std::wstring &strResponse, int defValue = 0);

	// throw
	void throw_exceptions();

	// Functions
	int AVGetRequiredVersion();
	std::wstring AVGetRequiredVersionDate();
	int AVGetLatestVersion();
	std::wstring AVGetLatestVersionDate();
	std::wstring AVGetLatestVersionDownloadPath();
	std::wstring AVGetAppName();

	bool AVLogin(std::wstring strUsername, std::wstring strPassword);
	bool AVIsAuthorised();
	bool AVExtendAuthorisation();

	void AVIndex();
	void AVProject(AVLONG nSimulationId);
	void AVLiftGroups(AVLONG nProjectId);
	void AVFloors(AVLONG nLiftGroupId);
	void AVShafts(AVLONG nLiftGroupId);
	void AVSim(AVLONG nLiftGroupId);
	void AVSimData(AVLONG nSimId, AVLONG timeFrom, AVLONG timeTo);
	void AVPrjData(AVLONG nProjectId, AVLONG timeFrom, AVLONG timeTo);


private:
	HRESULT ExecWorkerThread();
	friend UINT WorkerThread(void *p);
};

