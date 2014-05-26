#include "StdAfx.h"
#include "_base.h"
#include "AdVisuoLoader.h"
#include "xmlrequest.h"
#include "AdVisuo.h"
#include "VisProject.h"
#include "VisSim.h"
#include "VisLiftGroup.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "DlgHtBase.h"


CAdVisuoLoader::CAdVisuoLoader(CProjectVis *pProject) : m_pProject(pProject)
{
	m_status = NOT_STARTED;
	m_timeLoaded = 0;
	m_timeStep = 120000;

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

void CAdVisuoLoader::Start(std::wstring strUrl, std::wstring strUsername, std::wstring strTicket, AVULONG nProjectId)
{
	m_http.create();
	m_http.setURL(strUrl);
	m_http.set_authorisation_data(strUsername, strTicket);
	m_nProjectId = nProjectId;

	m_timeLoaded = m_pProject->GetMinSimulationTime();
	m_hEvCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);

	AfxBeginThread(::WorkerThread, this);
}

void CAdVisuoLoader::Stop()
{
	if (m_status != COMPLETE)
	{
		m_status = REQUEST_TO_STOP;
		WaitForSingleObject(m_hEvCompleted, 5000);
	}

	for each (CPassengerVis *pPassenger in m_passengers)
		delete pPassenger;
	m_journeys.clear();
	m_journeySimId.clear();
	m_journeyLiftId.clear();
	m_passengers.clear();
	m_passengerSimId.clear();
}

UINT CAdVisuoLoader::WorkerThread()
{
	std::wstring response;
	try
	{
		// Phase 1 - Load Structure
		m_status = LOADING_STRUCTURE;

		// check authorisation
		if (m_http.AVIsAuthorised() <= 0)
			throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);

		// check version
		int verreq = m_http.AVGetRequiredVersion();
		if (VERSION < verreq)
			throw _version_error(verreq, m_http.AVGetRequiredVersionDate().c_str(), m_http.AVGetLatestVersionDownloadPath().c_str());

		if (GetKeyState(VK_RMENU) < 0)
			throw _version_error(verreq, m_http.AVGetRequiredVersionDate().c_str(), m_http.AVGetLatestVersionDownloadPath().c_str());

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

		//Sleep(10000);

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

		// Phase 2 - Load Data
		m_status = LOADING_DATA;

		std::wstring response;
		while (m_timeLoaded < m_pProject->GetTimeSaved() && m_status != REQUEST_TO_STOP)
		{
			if (m_http.AVIsAuthorised() <= 0)
				throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);
			m_http.AVPrjData(m_pProject->GetId(), m_timeLoaded, m_timeLoaded + m_timeStep);

			Load(response.c_str());
			m_http.get_response(response);

			m_timeLoaded += m_timeStep;
		}
		Load(response.c_str());

		// Completed!
		m_status = COMPLETE;
		SetEvent(m_hEvCompleted);
		return 0;
	}
	catch (_prj_error pe)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(pe);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(pe, m_http.getURL().c_str());
	}
	catch (_com_error ce)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ce);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ce, m_http.getURL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(xe);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(xe, m_http.getURL().c_str());
	}
	catch (_version_error ve)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch (dbtools::_value_error ve)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch(...)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle();
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(m_http.getURL().c_str());
	}
	m_status = EXCEPTION;
	SetEvent(m_hEvCompleted);
	return 0;
}

CAdVisuoLoader::STATUS CAdVisuoLoader::Update(CEngine *pEngine)
{
	STATUS status = m_status;
	if (status == COMPLETE || status == FAILED)
		return status;

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

		if (status == EXCEPTION)
		{
			m_status = status = FAILED;
			::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		}
		return status;
	}
	catch (_prj_error pe)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(pe);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(pe, m_http.getURL().c_str());
	}
	catch (_com_error ce)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ce);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ce, m_http.getURL().c_str());
	}
	catch (_xmlreq_error xe)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(xe);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(xe, m_http.getURL().c_str());
	}
	catch (_version_error ve)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch (dbtools::_value_error ve)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle(ve);
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(ve, m_http.getURL().c_str());
	}
	catch(...)
	{
		m_strFailureTitle = (LPCTSTR)CDlgHtFailure::GetFailureTitle();
		m_strFailureText  = (LPCTSTR)CDlgHtFailure::GetFailureString(m_http.getURL().c_str());
	}
	Stop();
	ASSERT(m_status == COMPLETE);
	m_status = FAILED;
	return FAILED;
}

CAdVisuoLoader::STATUS CAdVisuoLoader::Update()
{
	STATUS status = m_status;

	if (status == EXCEPTION)
	{
		m_status = FAILED;
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_OTHER_FAILURE, 0), (LPARAM)0);
		return FAILED;
	}
	
	return status;
}

void CAdVisuoLoader::Load(xmltools::CXmlReader reader)
{
	AVULONG iShaft = 0, iStorey = 0;

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
		}
		else
		if (reader.getName() == L"AVProject"
		|| reader.getName() == L"AVLiftGroup"
		|| reader.getName() == L"AVFloor"
		|| reader.getName() == L"AVShaft"
		|| reader.getName() == L"AVSim")
			throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
	}
}

