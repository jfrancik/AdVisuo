// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseClasses.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProjectBase

CProjectBase::CProjectBase()
{
	m_nId = 0;
	m_nSimulationId = 0;
	m_nAVVersionId = 0;
}

void CProjectBase::ResolveMe()
{
	m_nId = ME[L"ID"];
	m_nSimulationId = ME[L"SimulationId"];
	m_nAVVersionId = ME[L"AVVersionId"];
	m_nLiftGroupsCount = ME[L"LiftGroupsCount"];
}

std::wstring CProjectBase::GetProjectInfo(PRJ_INFO what)
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
// CSimBase

CSimBase::CSimBase(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;
	m_nProjectId = 0;
	m_nSIMVersionId = 0;
	m_nSimulationTime = 0;
	m_nTimeSaved = 0;
}

CSimBase::~CSimBase()
{
	DeleteLifts();
	DeletePassengers();
}

void CSimBase::DeleteLifts()
{
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		if (GetLift(i)) delete GetLift(i);
	m_lifts.clear();
}

AVULONG CSimBase::GetJourneyTotalCount()
{
	AVULONG n = 0;
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		n += GetLift(i)->GetJourneyCount();
	return n;
}

void CSimBase::DeletePassengers()
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)) delete GetPassenger(i);
	m_passengers.clear();
}

void CSimBase::ResolveMe()
{
	m_nId = ME[L"ID"];
	SetProjectId(ME[L"ProjectId"]);
	SetSIMVersionId(ME[L"SIMVersionId"]);
	m_nSimulationTime = ME[L"SimulationTime"];
	m_nTimeSaved = ME[L"TimeSaved"];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLiftBase

CLiftBase::CLiftBase(CSimBase *pSim, AVULONG nLiftId, AVULONG nDecks) : m_pSim(pSim), m_nId(nLiftId), m_nDecks(nDecks)
{
}

CLiftBase::~CLiftBase()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPassengerBase

CPassengerBase::CPassengerBase(CSimBase *pSim, AVULONG nPassengerId) : m_pSim(pSim), m_nId(nPassengerId)
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

CPassengerBase::~CPassengerBase()
{
	if (m_nWaypoints) 
		delete [] m_pWaypoints;
}

void CPassengerBase::CreateWaypoints(AVULONG nCount)
{
	if (m_pWaypoints) delete [] m_pWaypoints;
	m_nWaypoints = nCount;
	m_pWaypoints = NULL;
	if (m_nWaypoints) m_pWaypoints = new WAYPOINT[m_nWaypoints];
}

void CPassengerBase::ResolveMe()
{
	SetId(ME[L"PassengerId"]);
	SetShaftId(ME[L"ShaftId"]);
	SetLiftId(ME[L"LiftId"]);
	SetDeck(ME[L"DeckId"]);
	SetArrivalFloor(ME[L"FloorArrival"]);
	SetDestFloor(ME[L"FloorDest"]);
	SetBornTime(ME[L"TimeBorn"]);
	SetArrivalTime(ME[L"TimeArrival"]);
	SetGoTime(ME[L"TimeGo"]);
	SetLoadTime(ME[L"TimeLoad"]);
	SetUnloadTime(ME[L"TimeUnload"]);
	SetWaitSpan(ME[L"SpanWait"]);

	ParseWayPoints(ME[L"WP"]);
}

std::wstring CPassengerBase::StringifyWayPoints()
{
	std::wstringstream s;
	s << GetWaypointCount() << " ";
	for (AVULONG i = 0; i < GetWaypointCount(); i++)
		s << *GetWaypoint(i);
	return s.str();
}

void CPassengerBase::ParseWayPoints(std::wstring wp)
{
	std::wstringstream s(wp);
	AVULONG nCount;
	s >> nCount;
	CreateWaypoints(nCount);
	for (AVULONG i = 0; i < GetWaypointCount(); i++)
		s >> *GetWaypoint(i);
}
