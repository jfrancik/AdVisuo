#include "StdAfx.h"
#include "_base.h"
#include "AdVisuoLoader.h"
#include "xmlrequest.h"
#include "VisProject.h"
#include "VisSim.h"
#include "VisLiftGroup.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "FailureStr.h"


CAdVisuoLoader::CAdVisuoLoader(CProjectVis *pProject) : m_pProject(pProject)
{
	m_status = NOT_STARTED;
	m_reasonForTermination = NOT_TERMINATED;
	m_posInQueue = 0;
	m_requestToStop = false;
	m_requestToFail = false;
	m_timeReceived = 0;
	m_timeLoaded = 0;
	m_timeStep = 120000;
	m_pThread = NULL;

	m_hEvCompleted = NULL;
	InitializeCriticalSection(&m_csJourney);
	InitializeCriticalSection(&m_csPassenger);
}


CAdVisuoLoader::~CAdVisuoLoader(void)
{
	Stop();

	DeleteCriticalSection(&m_csJourney);
	DeleteCriticalSection(&m_csPassenger);
}

void CAdVisuoLoader::Start(std::wstring strUrl, CXMLRequest *pAuthAgent, AVULONG nProjectId)
{
	m_http.create();
	m_http.setURL(strUrl);
	m_http.take_authorisation_from(pAuthAgent);
	m_nProjectId = nProjectId;

	m_timeReceived = 0;			// m_pProject->GetMinSimulationTime();
	m_timeLoaded = 0;
	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_pThread = AfxBeginThread(::WorkerThread, this);
}

UINT CAdVisuoLoader::LoadSynchronous(std::wstring strUrl, CXMLRequest *pAuthAgent, AVULONG nProjectId)
{
	m_http.create();
	m_http.setURL(strUrl);
	m_http.take_authorisation_from(pAuthAgent);
	m_nProjectId = nProjectId;

	m_timeReceived = 0;			// m_pProject->GetMinSimulationTime();
	m_timeLoaded = 0;
	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);

	UINT res = WorkerThread();
	WaitForSingleObject(m_hEvCompleted, 5000);
	return res;
}


void CAdVisuoLoader::Stop()
{
	if (m_pThread == NULL) return;

	if (m_status != TERMINATED)
	{
		m_requestToStop = true;
		WaitForSingleObject(m_hEvCompleted, 5000);
		if (m_status != TERMINATED)
			::TerminateThread(m_pThread->m_hThread, 0);
	}
	m_pThread = NULL;

	for each (CPassengerVis *pPassenger in m_passengers)
		delete pPassenger;
	m_journeys.clear();
	m_journeySimId.clear();
	m_journeyLiftId.clear();
	m_passengers.clear();
	m_passengerSimId.clear();
}

void CAdVisuoLoader::Fail()
{
	if (m_status == TERMINATED)
		m_reasonForTermination = FAILED;

	if (m_pThread == NULL) return;

	if (m_status != TERMINATED)
	{
		m_requestToFail = true;
		WaitForSingleObject(m_hEvCompleted, 5000);
		if (m_status != TERMINATED)
			::TerminateThread(m_pThread->m_hThread, 0);
	}
	m_pThread = NULL;

	for each (CPassengerVis *pPassenger in m_passengers)
		delete pPassenger;
	m_journeys.clear();
	m_journeySimId.clear();
	m_journeyLiftId.clear();
	m_passengers.clear();
	m_passengerSimId.clear();
}

	class _prj_deleted
	{
	};

#define PROGRESS_MASK		0xC0000000
#define PROGRESS_STORING	0x00000000
#define PROGRESS_READY		0x40000000
#define PROGRESS_FAILED		0x80000000
#define PROGRESS_QUEUED		0xC0000000


//#include <iostream>
//using namespace std;
UINT CAdVisuoLoader::WorkerThread()
{
	std::wstring response;
	try
	{
		//cout << "Worker Thread Started" << endl;
		// Phase 0 - Authorisation and Pre-Load Checks
		m_status = PREPROCESS;

		// check authorisation
		if (m_http.AVIsAuthorised() <= 0)
			throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);

		// check version
		int verreq = m_http.AVGetRequiredVersion();
		if (VERSION < verreq)
			throw _version_error(verreq, m_http.AVGetRequiredVersionDate().c_str(), m_http.AVGetLatestVersionDownloadPath().c_str());

		if (GetKeyState(VK_RMENU) < 0)
			throw _version_error(verreq, m_http.AVGetRequiredVersionDate().c_str(), m_http.AVGetLatestVersionDownloadPath().c_str());

		// Synchronise - wait for the queue to finish
		AVULONG progress = m_http.AVPrjProgress(m_nProjectId);
		while ((progress & PROGRESS_MASK) == PROGRESS_QUEUED)
		{
			m_posInQueue = progress & 0xffff;		// position in queue
			Sleep(250);
			progress = m_http.AVPrjProgress(m_nProjectId);
		}
		if ((progress & PROGRESS_MASK) == PROGRESS_FAILED)
			throw _prj_deleted();				// error condition
		m_posInQueue = 0;
		ASSERT((progress & PROGRESS_MASK) == PROGRESS_STORING || (progress & PROGRESS_MASK) == PROGRESS_READY);


		// Phase 1 - Load Structure
		m_status = LOADING_STRUCTURE;
		//cout << "THREAD: Loading structure." << endl;

		// load project
		m_http.AVProject(m_nProjectId);
		m_http.get_response(response);
		m_pProject->LoadFromBuf(response.c_str());

		//	SetTitle(m_pProject->GetProjectInfo(CProjectVis::PRJ_NAME).c_str());
		//	m_strPathName = GetTitle();

		// load lift groups
		m_http.AVLiftGroups(m_pProject->GetId());
		m_http.get_response(response);
		m_pProject->LoadFromBuf(response.c_str());

		// load floors, shafts and sims for each lift group
		for each (CLiftGroupVis *pGroup in m_pProject->GetLiftGroups())
		{
			m_http.AVFloors(pGroup->GetId());
			m_http.get_response(response);
			m_pProject->LoadFromBuf(response.c_str());

			m_http.AVShafts(pGroup->GetId());
			m_http.get_response(response);
			m_pProject->LoadFromBuf(response.c_str());

			m_http.AVSim(pGroup->GetId());
			m_http.get_response(response);
			m_pProject->LoadFromBuf(response.c_str());
		}

		// Preparatory Stage
		for each (CLiftGroup *pGroup in m_pProject->GetLiftGroups())
			for each (CSim *pSim in pGroup->GetSims())
				m_sims[pSim->GetId()] = static_cast<CSimVis*>(pSim);


		// Phase Extra - Delay
//		int seconds = 10;
//		for (int i = 0; i < seconds * 2 && !m_requestToStop && !m_requestToFail; i++)
//			Sleep(500);

		// Phase 2 - Load Data
		//cout << "THREAD: Loading data." << endl;
		m_status = LOADING_DATA;

		std::wstring response;
		while (m_timeReceived < m_pProject->GetMaxTime() && !m_requestToStop && !m_requestToFail)
		{
			// ensure the authorisation
			if (m_http.AVIsAuthorised() <= 0)
				throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);

			// synchronise - wait for the current chunk to be entirely saved
			progress = m_http.AVPrjProgress(m_nProjectId);
			while ((progress & PROGRESS_MASK) == PROGRESS_STORING && (AVLONG)(progress & 0x3fffffff) < m_timeReceived + m_timeStep)
			{
				Sleep(250);
				progress = m_http.AVPrjProgress(m_nProjectId);
			}
			if ((progress & PROGRESS_MASK) == PROGRESS_FAILED || (progress & PROGRESS_MASK) == PROGRESS_QUEUED)
				throw _prj_deleted();				// error condition
			ASSERT((progress & PROGRESS_MASK) == PROGRESS_STORING || (progress & PROGRESS_MASK) == PROGRESS_READY);

			// Load data
			m_http.AVPrjData(m_pProject->GetId(), m_timeReceived, m_timeReceived + m_timeStep);

			Load(response.c_str());
			m_http.get_response(response);

			m_timeLoaded = m_timeReceived;
			m_timeReceived += m_timeStep;
			//cout << "THREAD: Loaded until " << m_timeReceived << endl;
		}
		Load(response.c_str());
		m_timeLoaded = m_timeReceived;

		// Completed!
		if (m_requestToFail)
			m_reasonForTermination = FAILED;
		else if (m_requestToStop)
			m_reasonForTermination = STOPPED;
		else
			m_reasonForTermination = COMPLETE;
		m_status = TERMINATED;
		SetEvent(m_hEvCompleted);
		//cout << "THREAD: TERMINATING" << endl;
		return 0;
	}
	catch (_prj_deleted)
	{
		std::wstringstream str;
		str << L"Project has been deleted from " << (LPCTSTR)m_http.getURL().c_str();
		m_strFailureTitle = L"";
		m_strFailureText  = str.str().c_str();
	}
	catch (_prj_error pe)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(pe);
		m_strFailureText  = (LPCTSTR)::GetFailureString(pe, m_http.getURL().c_str());
	}
	catch (_com_error ce)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ce);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ce, m_http.getURL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(xe);
		m_strFailureText  = (LPCTSTR)::GetFailureString(xe, m_http.getURL().c_str());
	}
	catch (_version_error ve)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch (dbtools::_value_error ve)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch(...)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle();
		m_strFailureText  = (LPCTSTR)::GetFailureString(m_http.getURL().c_str());
	}
	m_reasonForTermination = FAILED;
	m_status = TERMINATED;
	SetEvent(m_hEvCompleted);
	return 0;
}

CAdVisuoLoader::STATUS CAdVisuoLoader::Update(CEngine *pEngine)
{
	STATUS status = m_status;
	
	if (status == TERMINATED && m_journeySimId.size() + m_passengerSimId.size() == 0)
		return TERMINATED;

	try
	{
		while (m_journeySimId.size() + m_passengerSimId.size())
		{
			if (m_journeySimId.size())
			{
				EnterCriticalSection(&m_csJourney);
				AVULONG nSimId = m_journeySimId.front(); m_journeySimId.pop_front();
				AVULONG nLiftID = m_journeyLiftId.front(); m_journeyLiftId.pop_front();
				JOURNEY journey = m_journeys.front(); m_journeys.pop_front();
				LeaveCriticalSection(&m_csJourney);


				CSimVis *pSim = m_sims[nSimId];
				if (!pSim || !pSim->GetLiftGroup()) 
					throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
				CLiftGroupVis *pGroup = pSim->GetLiftGroup();

				if (nLiftID >= pGroup->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pGroup->GetShaftCount() || journey.m_shaftTo >= pGroup->GetShaftCount()) 
					throw _prj_error(_prj_error::E_PRJ_LIFTS);
				if (nLiftID >= pSim->GetLiftCount()) 
					throw _prj_error(_prj_error::E_PRJ_LIFTS);

				pSim->GetLift(nLiftID)->AddJourney(journey);

				if (pSim->IsCur())
					pSim->GetLift(nLiftID)->Play(pSim->GetLift(nLiftID)->GetJourneyCount() - 1, journey, pEngine);
			}
			if (m_passengerSimId.size())
			{
				EnterCriticalSection(&m_csPassenger);
				AVULONG nSimId = m_passengerSimId.front(); m_passengerSimId.pop_front();
				CPassengerVis *pPassenger = m_passengers.front(); m_passengers.pop_front();
				LeaveCriticalSection(&m_csPassenger);

				CSimVis *pSim = m_sims[nSimId];
				if (!pSim || !pSim->GetLiftGroup()) 
					throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);

				pSim->AddPassenger(pPassenger);

				if (pSim->IsCur()) 
					pPassenger->Play(pEngine);
			}
		}

		return status;
	}
	catch (_prj_error pe)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(pe);
		m_strFailureText  = (LPCTSTR)::GetFailureString(pe, m_http.getURL().c_str());
	}
	catch (_com_error ce)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ce);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ce, m_http.getURL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(xe);
		m_strFailureText  = (LPCTSTR)::GetFailureString(xe, m_http.getURL().c_str());
	}
	catch (_version_error ve)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch (dbtools::_value_error ve)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch(...)
	{
		m_strFailureTitle = (LPCTSTR)::GetFailureTitle();
		m_strFailureText  = (LPCTSTR)::GetFailureString(m_http.getURL().c_str());
	}
	Fail();
	return TERMINATED;
}

void CAdVisuoLoader::Load(xmltools::CXmlReader reader)
{
	AVULONG iShaft = 0, iStorey = 0;

	//cout << "Consuming data:";
	while (reader.read())
	{
		if (reader.getName() == L"AVJourney")
		{
			AVULONG nSimId = reader[L"SimID"];

			JOURNEY journey;
			AVULONG nLiftID = reader[L"LiftID"];
			journey.m_shaftFrom = reader[L"ShaftFrom"];
			journey.m_shaftTo = reader[L"ShaftTo"];
			journey.m_floorFrom = reader[L"FloorFrom"];
			journey.m_floorTo = reader[L"FloorTo"];
			journey.m_timeGo = reader[L"TimeGo"];
			journey.m_timeDest = reader[L"TimeDest"];
			journey.ParseDoorCycles(reader[L"DC"]);

			EnterCriticalSection(&m_csJourney);
			m_journeySimId.push_back(nSimId);
			m_journeyLiftId.push_back(nLiftID);
			m_journeys.push_back(journey);
			LeaveCriticalSection(&m_csJourney);
			//cout << ".";
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			AVULONG nSimId = reader[L"SimID"];
			CSimVis *pSim = m_sims[nSimId];
			if (!pSim || !pSim->GetLiftGroup()) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);

			CPassengerVis *pPassenger = (CPassengerVis*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();

			EnterCriticalSection(&m_csPassenger);
			m_passengerSimId.push_back(nSimId);
			m_passengers.push_back(pPassenger);
			LeaveCriticalSection(&m_csPassenger);
			//cout << ".";
		}
		else
		if (reader.getName() == L"AVProject"
		|| reader.getName() == L"AVLiftGroup"
		|| reader.getName() == L"AVFloor"
		|| reader.getName() == L"AVShaft"
		|| reader.getName() == L"AVSim")
			throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
	}
	//cout << endl;
}

