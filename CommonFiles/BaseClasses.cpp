// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseClasses.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSimBase

CSimBase::CSimBase(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;
	m_nProjectID = 0;
	m_nSIMVersionID = 0;
	m_nAVVersionID = 0;
	m_nBldFloors = 0;
	m_nBldShafts = 0;
	m_nBldLifts = 0;
	m_nSimulationTime = 0;
	m_bSavedAll = false;
	m_nJourneysSaved = m_nPassengersSaved = m_nTimeSaved = 0;
	m_nAlgorithm = COLLECTIVE;
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
