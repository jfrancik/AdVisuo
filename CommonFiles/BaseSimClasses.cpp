// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseSimClasses.h"
#include "dbtools.h"
#include <string>
//#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSim

CSim::CSim()
{
	m_pLiftGroup = NULL;
	m_nIndex = 0;
	m_nLiftGroupId = 0;
	m_vecOffset = Vector(0);
}

CSim::~CSim()
{
	DeleteLifts();
	DeletePassengers();
}

std::wstring CSim::GetScenarioDesc()
{
	std::wstringstream str;
	str << (std::wstring)ME[L"TrafficPatternName"] << L"\n"
		<< L" incoming: " << (ULONG)ME[L"Incoming"]
		<< L" outgoing: " << (ULONG)ME[L"Outgoing"]
		<< L" interfloor: " << (ULONG)ME[L"Interfloor"]
		<< L"; " << (std::wstring)ME[L"TrafficProfileName"] << L" profile";
	return str.str();
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
	SetLiftGroupId(ME[L"LiftGroupId"]);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// JOURNEY

std::wstringstream &operator << (std::wstringstream &s, JOURNEY::DOOR &d)
{
	s << d.m_timeOpen << L" " << d.m_durationOpen << L" " << d.m_timeClose << L" " << d.m_durationClose << L" ";
	return s;
}

std::wstringstream &operator >> (std::wstringstream &s, JOURNEY::DOOR &d)
{
	s >> d.m_timeOpen >> d.m_durationOpen >> d.m_timeClose >> d.m_durationClose;
	return s;
}

std::wstring JOURNEY::StringifyDoorCycles()
{
	std::wstringstream s;
	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
	{
		s << m_doorcycles[iDeck].size() << L" ";
		for (AVULONG iCycle = 0; iCycle < m_doorcycles[iDeck].size(); iCycle++)
			s << m_doorcycles[iDeck][iCycle];
	}
	return s.str();
}

void JOURNEY::ParseDoorCycles(std::wstring dc)
{
	std::wstringstream s(dc);
	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
	{
		AVULONG n;
		s >> n;
		m_doorcycles[iDeck].resize(n);
		for (AVULONG iCycle = 0; iCycle < n; iCycle++)
			s >> m_doorcycles[iDeck][iCycle];
	}
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
// WAYPOINT

std::wstringstream &operator << (std::wstringstream &s, WAYPOINT &w)
{
	s << w.nAction << L" ";
	switch (w.nAction)
	{
	case MOVE:
		s << w.vector.x << L" " << w.vector.y << L" ";
		break;
	case WAIT:
		s << w.nTime << L" ";
		break;
	case WALK:
		s << w.vector.x << L" " << w.vector.y << L" ";
		break;
	case TURN:
		break;
	case ENTER_ARR_FLOOR:
	case ENTER_LIFT:
	case ENTER_DEST_FLOOR:
		break;
	}

	if (!w.wstrStyle.empty())
		s << w.wstrStyle << L" ";
	else
		s << L"(null)" << L" ";
	
	return s;
}

std::wstringstream &operator >> (std::wstringstream &s, WAYPOINT &w)
{
	s >> (ULONG&)w.nAction;
	switch (w.nAction)
	{
	case MOVE:
		s >> w.vector.x >> w.vector.y;
		break;
	case WAIT:
		s >> w.nTime;
		break;
	case WALK:
		s >> w.vector.x >> w.vector.y;
		break;
	case TURN:
		break;
	case ENTER_ARR_FLOOR:
	case ENTER_LIFT:
	case ENTER_DEST_FLOOR:
		break;
	}

	s >> w.wstrStyle;

	return s;
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
	m_timeSpawn = 0;
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
	SetSpawnTime(ME[L"TimeBorn"]);
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
