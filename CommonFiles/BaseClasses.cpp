// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseClasses.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProject

CProject::CProject()
{
	m_nId = 0;
	m_nSimulationId = 0;
	m_nAVVersionId = 0;
	m_nLiftGroupsCount = 0;
	m_nDefault = -1;
}

CProject::~CProject()
{
	for each (CSim *pSim in m_sims)
		delete pSim;
}

void CProject::ResolveMe()
{
	m_nId = ME[L"ID"];
	m_nSimulationId = ME[L"SimulationId"];
	m_nAVVersionId = ME[L"AVVersionId"];
	m_nLiftGroupsCount = ME[L"LiftGroupsCount"];
}

void CProject::ResolveLiftGroups()
{
	for (AVULONG i = 0; i < GetLiftGroupsCount(); i++)
		m_sims.push_back(CreateSim(CreateBuilding(i), i));
	m_nDefault = 0;
}

std::wstring CProject::GetProjectInfo(PRJ_INFO what)
{
	switch (what)
	{
		case PRJ_PROJECT_NAME: return ME[L"ProjectName"];
		case PRJ_BUILDING_NAME: return ME[L"BuildingName"];
		case PRJ_LANGUAGE: return ME[L"Language"];
		case PRJ_UNITS: return ME[L"MeasurementUnits"];
		case PRJ_COMPANY: return ME[L"ClientCompany"];
		case PRJ_CITY: return ME[L"City"];
		case PRJ_LB_RGN: return ME[L"LBRegionDistrict"];
		case PRJ_COUNTY: return ME[L"County"];
		case PRJ_DESIGNER: return ME[L"LiftDesigner"];
		case PRJ_COUNTRY: return ME[L"Country"];
		case PRJ_CHECKED_BY: return ME[L"CheckedBy"];
		case PRJ_POST_CODE: return ME[L"PostalZipCode"];
		default: return L"(unknown)";
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSim

CSim::CSim(CBuilding *pBuilding, AVULONG nIndex) : m_pBuilding(pBuilding), m_nIndex(nIndex)
{
	m_nProjectId = 0;
	m_nSIMVersionId = 0;
	m_nSimulationTime = 0;
	m_nTimeSaved = 0;
	m_vecOffset = Vector(0);
}

CSim::~CSim()
{
	if (m_pBuilding) delete m_pBuilding;
	DeleteLifts();
	DeletePassengers();
}

void CSim::DeleteLifts()
{
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		if (GetLift(i)) delete GetLift(i);
	m_lifts.clear();
}

AVULONG CSim::GetJourneyTotalCount()
{
	AVULONG n = 0;
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		n += GetLift(i)->GetJourneyCount();
	return n;
}

void CSim::DeletePassengers()
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)) delete GetPassenger(i);
	m_passengers.clear();
}

void CSim::ResolveMe()
{
	m_nId = ME[L"ID"];
	SetIndex(ME[L"LiftGroupIndex"]);
	SetProjectId(ME[L"ProjectId"]);
	SetSIMVersionId(ME[L"SIMVersionId"]);
	m_nSimulationTime = ME[L"SimulationTime"];
	m_nTimeSaved = ME[L"TimeSaved"];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLift

CLift::CLift(CSim *pSim, AVULONG nLiftId, AVULONG nDecks) : m_pSim(pSim), m_nId(nLiftId), m_nDecks(nDecks)
{
}

CLift::~CLift()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPassenger

CPassenger::CPassenger(CSim *pSim, AVULONG nPassengerId) : m_pSim(pSim), m_nId(nPassengerId)
{
	m_nShaft = 0;
	m_nLift = 0;
	m_nDeck = 0;
	m_nArrivalFloor = 0;
	m_nDestFloor = 0;
	m_timeBorn = 0;
	m_timeArrival = 0;
	m_timeGo = 0;
	m_timeLoad = 0;
	m_timeUnload = 0;
	m_spanWait = 0;
	m_pWaypoints = NULL;
	m_nWaypoints = 0;
}

CPassenger::~CPassenger()
{
	if (m_nWaypoints) 
		delete [] m_pWaypoints;
}

void CPassenger::CreateWaypoints(AVULONG nCount)
{
	if (m_pWaypoints) delete [] m_pWaypoints;
	m_nWaypoints = nCount;
	m_pWaypoints = NULL;
	if (m_nWaypoints) m_pWaypoints = new WAYPOINT[m_nWaypoints];
}

void CPassenger::ResolveMe()
{
	SetId(ME[L"PassengerId"]);
	SetShaftId(ME[L"ShaftId"]);
	SetLiftId(ME[L"LiftId"]);
	SetDeck(ME[L"DeckId"]);
	SetArrivalFloor((AVULONG)ME[L"FloorArrival"]);
	SetDestFloor((AVULONG)ME[L"FloorDest"]);
	SetBornTime(ME[L"TimeBorn"]);
	SetArrivalTime(ME[L"TimeArrival"]);
	SetGoTime(ME[L"TimeGo"]);
	SetLoadTime(ME[L"TimeLoad"]);
	SetUnloadTime(ME[L"TimeUnload"]);
	SetWaitSpan(ME[L"SpanWait"]);

	ParseWayPoints(ME[L"WP"]);
}

std::wstring CPassenger::StringifyWayPoints()
{
	std::wstringstream s;
	s << GetWaypointCount() << " ";
	for (AVULONG i = 0; i < GetWaypointCount(); i++)
		s << *GetWaypoint(i);
	return s.str();
}

void CPassenger::ParseWayPoints(std::wstring wp)
{
	std::wstringstream s(wp);
	AVULONG nCount;
	s >> nCount;
	CreateWaypoints(nCount);
	for (AVULONG i = 0; i < GetWaypointCount(); i++)
		s >> *GetWaypoint(i);
}
