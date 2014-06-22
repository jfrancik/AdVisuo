#pragma once

#include "../CommonFiles/XMLTools.h"
#include "../CommonFiles/BaseSimClasses.h"
#include "xmlrequest.h"

#include <map>
#include <list>

class CProjectVis;
class CSimVis;
class CEngine;
class CPassengerVis;

template<class T>
class ProtectedVariable
{
	T value;
	CRITICAL_SECTION cs;
	T Value() 			{ EnterCriticalSection(&cs); T v = value; LeaveCriticalSection(&cs); return v; }
	T Value(T val)		{ EnterCriticalSection(&cs); value = val; LeaveCriticalSection(&cs); return val; }
public:
	ProtectedVariable<T>()			{ InitializeCriticalSection(&cs); }
	ProtectedVariable<T>(T value)	
	{ 
		InitializeCriticalSection(&cs); 
		Value(value); 
	}

	T operator=(T value)			
	{ 
		return Value(value); 
	}

	operator T()					{ return Value(); }
};

class CAdVisuoLoader
{
public:
	enum STATUS { NOT_STARTED, PREPROCESS, LOADING_STRUCTURE, LOADING_DATA, COMPLETE, REQUEST_TO_STOP, EXCEPTION, FAILED, SUCCEEDED };

private:
	// the project and its connection
	AVULONG m_nProjectId;
	CProjectVis *m_pProject;
	CXMLRequest m_http;

	// status
	ProtectedVariable<STATUS> m_status;
	ProtectedVariable<AVULONG> m_progress;
	ProtectedVariable<std::wstring> m_strFailureTitle;
	ProtectedVariable<std::wstring> m_strFailureText;

	// other params	
	AVLONG m_timeLoaded;
	AVLONG m_timeStep;

	// internal use collections
	std::map<AVULONG, CSimVis*> m_sims;
	std::list<JOURNEY> m_journeys;
	std::list<AVULONG> m_journeySimId;
	std::list<AVULONG> m_journeyLiftId;
	std::list<CPassengerVis*> m_passengers;
	std::list<AVULONG> m_passengerSimId;

	// synchronisation objects
	HANDLE m_hEvCompleted;
	CRITICAL_SECTION m_csJourney;
	CRITICAL_SECTION m_csPassenger;
public:
	CAdVisuoLoader(CProjectVis *pProject);
	~CAdVisuoLoader(void);

	// Accessors
	enum STATUS GetStatus()							{ return m_status; }
	AVULONG GetProgress()							{ return m_progress; };
	std::wstring GetFailureTitle()					{ return m_strFailureTitle; }
	std::wstring GetFailureText()					{ return m_strFailureText; }
	AVLONG GetTimeStep() const						{ return m_timeStep; }
	void SetTimeStep(AVLONG val)					{ m_timeStep = val; }
	AVLONG GetTimeLoaded()							{ return m_timeLoaded; }

	// Main Operations:
	// Start - starts the load process, then continues in a thread
	void Start(std::wstring strUrl, CXMLRequest *pAuthAgent, AVULONG nProjectId);
	// Updates the project with the latest loaded data - to be called from a timer proc
	CAdVisuoLoader::STATUS Update(CEngine *pEngine);
	CAdVisuoLoader::STATUS Update();
	// Safely stops the thread
	void Stop();

private:
	// main thread function
	UINT WorkerThread();

	// thread safe alternative for CProject::Load - used in LOADING_DATA phase only
	void Load(xmltools::CXmlReader reader);

	friend static UINT __cdecl WorkerThread(void *p)	{ return static_cast<CAdVisuoLoader*>(p)->WorkerThread(); }
};

