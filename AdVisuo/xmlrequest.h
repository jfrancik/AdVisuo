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
	HANDLE m_hEvRequestSet;		// set by the client when a request is set
	HANDLE m_hEvResponseRdy;	// request completed - response ready to be read
	//HANDLE m_hEvResponse;		// 
	
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
	CXMLRequest *m_pAuthSource;

public:
	CXMLRequest();
	virtual ~CXMLRequest();

	void create();

	void setURL(std::wstring strUrl)			{ m_strUrl = strUrl; }
	std::wstring getURL()						{ return m_strUrl; }

	// Authorisation

	void take_authorisation_from(CXMLRequest *pSource);
	void set_authorisation_data(std::wstring strUsername, std::wstring strTicket);
	void get_authorisation_data(std::wstring &strUsername, std::wstring &strTicket);

	// status
	bool ready()			{ return m_nReadyState == 4; }
	bool ok()				{ return ready() && SUCCEEDED(m_h) && m_nStatus <= 299; }

	// calls - generic
	void setreq(std::wstring strRequest = L"");
	void addreq(std::wstring strRequest);
	void addparam(std::wstring strParam,  AVLONG val);
	void addparam(std::wstring strParam,  std::wstring strVal);
	void addparam_authorisation();
	void call(std::wstring strFunction);

	// wait & get response
	HRESULT wait(DWORD dwTimeout = INFINITE);
	void get_response(std::wstring &strResponse);
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
	int AVIsAuthorised();
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

