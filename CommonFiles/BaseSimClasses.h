// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "dbtools.h"
#include <functional>
#include "BaseLiftGroup.h"

class CProject;
class CSim;
class CLift;
class CPassenger;

/////////////////////////////////////////////////////////////
// Simulation Class - encapsulates all sim data

class CSim : public dbtools::CCollection
{
	// basic information
	AVULONG m_nId;					// sim id
	AVULONG m_nLiftGroupId;			// lift group id
	AVULONG m_nIndex;				// index in multi-group structures

	CLiftGroup *m_pLiftGroup;
	AVVECTOR m_vecOffset;

	std::vector<CLift*> m_lifts;
	std::vector<CPassenger*> m_passengers;

public:
	CSim();
	virtual ~CSim();

	CLiftGroup *GetLiftGroup()					{ return m_pLiftGroup;}
	void SetLiftGroup(CLiftGroup *pLiftGroup)	{ m_pLiftGroup = pLiftGroup; }

	CProject *GetProject()						{ return GetLiftGroup()->GetProject(); }

	AVULONG GetIndex()							{ return m_nIndex; }
	void SetIndex(AVULONG n)					{ m_nIndex = n; }

	bool IsCur()								{ return GetLiftGroup()->GetCurSim() == this; }
	void SetCur()								{ GetLiftGroup()->SetCurSim(this); }

	std::wstring GetSIMFileName()				{ return ME[L"SIMFileName"]; }
	std::wstring GetIFCFileName()				{ return ME[L"IFCFileName"]; }
	AVULONG GetBldgFloors()						{ return ME[L"Floors"]; }
	AVULONG GetBldgShafts()						{ return ME[L"Shafts"]; }
	DATE GetTimeStamp()							{ return ME[L"TimeStamp"]; }
	enum ALGORITHM { DESTINATION, COLLECTIVE } ;
	ALGORITHM GetAlgorithm()					{ return (ALGORITHM)(ULONG)ME[L"Algorithm"]; }

	AVULONG GetScenarioTypeId()					{ return ME[L"TrafficPatternTypeId"]; }
	std::wstring GetScenarioName()				{ return ME[L"TrafficPatternName"]; }
	std::wstring GetScenarioDesc();

	AVULONG GetId()								{ return m_nId; }
	AVULONG GetLiftGroupId()					{ return m_nLiftGroupId; }

	void SetId(AVULONG n)						{ m_nId = n; }
	void SetLiftGroupId(AVULONG n)				{ m_nLiftGroupId = n; }

	std::vector<CLift*> &GetLfts()				{ return m_lifts; }
	std::vector<CPassenger*> &GetPassengers()	{ return m_passengers; }

	AVVECTOR GetOffsetVector()					{ return m_vecOffset; }
	void SetOffsetVector(AVVECTOR v)			{ m_vecOffset = v; }

	// access & initialisation
	AVULONG GetLiftCount()						{ return m_lifts.size(); }
	CLift *GetLift(AVULONG i)					{ return i < GetLiftCount() ? m_lifts[i] : NULL; }
	void AddLift(CLift *p)						{ m_lifts.push_back(p); }
	void DeleteLifts();
	AVULONG GetJourneyTotalCount();

	AVULONG GetPassengerCount()					{ return m_passengers.size(); }
	CPassenger *GetPassenger(AVULONG i)			{ return i < GetPassengerCount() ? m_passengers[i] : NULL; }
	void AddPassenger(CPassenger *p)			{ m_passengers.push_back(p); }
	void DeletePassengers();

	void ResolveMe();

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId) = 0;
	virtual CLift *CreateLift(AVULONG nId) = 0;
};

/////////////////////////////////////////////////////////////
// Lift journey definition - directly used by CLift(Base) objects

// Unddefined Value
enum { UNDEF = 0xffffffff };

struct JOURNEY
{
	struct DOOR
	{
		AVULONG m_timeOpen, m_timeClose;
		DOOR()					{ reset(); }

		void reset()			{ m_timeOpen = m_timeClose = UNDEF; }

//		AVULONG timeOpened()	{ return m_timeOpen + m_durationOpen; }
//		AVULONG timeClosed()	{ return m_timeClose + m_durationClose; }

		friend std::wstringstream &operator << (std::wstringstream &s, DOOR &d);
		friend std::wstringstream &operator >> (std::wstringstream &s, DOOR &d);

		bool operator == (DOOR &d)	
		{
			return abs((int)m_timeOpen - (int)d.m_timeOpen) <= 400 && abs((int)m_timeClose - (int)d.m_timeClose) <= 400;
		}
		bool operator != (DOOR &j)		{ return !operator ==(j); } 
	};

	std::vector<DOOR> m_doorcycles[DECK_NUM];

	AVULONG		m_id;						// for debug use only!
	AVULONG		m_shaftFrom, m_shaftTo;		// shaft id
	AVULONG		m_floorFrom, m_floorTo;		// journey floors
	AVLONG		m_timeGo, m_timeDest;		// journey time

	AVULONG FirstOpenTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? UNDEF : m_doorcycles[iDeck][0].m_timeOpen; }
	AVULONG LastCloseTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? 0     : m_doorcycles[iDeck][N-1].m_timeClose; }

	AVULONG FirstOpenTime()					{ AVULONG n = UNDEF; for (AVULONG i = 0; i < DECK_NUM; i++) n = min(n, FirstOpenTime(i)); return n; }
	AVULONG LastCloseTime()					{ AVULONG n = 0; for (AVULONG i = 0; i < DECK_NUM; i++) n = max(n, LastCloseTime(i)); return n; }

	std::wstring StringifyDoorCycles();
	void ParseDoorCycles(std::wstring);

	JOURNEY()								{ m_shaftFrom = m_shaftTo = 0; m_floorFrom = m_floorTo = m_timeGo = m_timeDest = UNDEF; }

	bool operator == (JOURNEY &j)
	{
		if (m_id != j.m_id || m_shaftFrom != j.m_shaftFrom || m_shaftTo != j.m_shaftTo || m_floorFrom != j.m_floorFrom || m_floorTo != j.m_floorTo || m_timeGo != j.m_timeGo || m_timeDest != j.m_timeDest)
			return false;
		for (int iDeck = 0; iDeck < 1; iDeck++)
		{
			if (m_doorcycles[iDeck].size() != j.m_doorcycles[iDeck].size()) 
				return false; 
			for (unsigned i = 0; i < m_doorcycles[iDeck].size(); i++)
				if (m_doorcycles[iDeck][i] != j.m_doorcycles[iDeck][i])
					return false;
		}
		return true;
	}
	bool operator != (JOURNEY &j)		{ return !operator ==(j); } 
};

/////////////////////////////////////////////////////////////
// Lift Representation in the Simulation Data

class CLift : public dbtools::CCollection
{
	CSim *m_pSim;			// main sim object

	AVULONG m_nId;				// lift id
	AVULONG m_nSimId;			// Sim ID
	AVULONG m_nDecks;			// number of decks (2 for double decker, 1 for single decker)

protected:
	// Lift Journeys
	std::vector<JOURNEY> m_journeys;

public:
	CLift(CSim *pSim, AVULONG nLiftId, AVULONG nDecks = 1);
	virtual ~CLift();

	CSim *GetSim()					{ return m_pSim; }
	CProject *GetProject()			{ return m_pSim->GetProject(); }

	CLiftGroup::LIFT *GetLIFT()		{ return GetSim()->GetLiftGroup()->GetLift(m_nId); }
	CLiftGroup::SHAFT *GetSHAFT()	{ return GetSim()->GetLiftGroup()->GetShaft(m_nId); }

	AVULONG GetId()					{ return m_nId; }
	void SetId(AVULONG n)			{ m_nId = n; }

	AVULONG GetSimId()				{ return m_nSimId; }
	void SetSimId(AVULONG nId)		{ m_nSimId = nId; }

	bool IsDoubleDecker()			{ return m_nDecks >= 2; }
	AVULONG GetDecks()				{ return m_nDecks; }
	void SetDecks(AVULONG n)		{ m_nDecks = n; }

	AVULONG GetJourneyCount()		{ return m_journeys.size(); }
	JOURNEY *GetJourney(AVULONG i)	{ return i < GetJourneyCount() ? &m_journeys[i] : NULL; }
	void AddJourney(JOURNEY &j)		{ m_journeys.push_back(j); }
};

/////////////////////////////////////////////////////////////
// Passenger Waypoint definition - directly used by CPassenger(Base) objects

enum ENUM_ACTION { MOVE, WAIT, WALK, TURN, ENTER_ARR_FLOOR, ENTER_LIFT, ENTER_DEST_FLOOR, SPAWN = 1000, DIE };
struct WAYPOINT
{
	ENUM_ACTION nAction;
	AVVECTOR vector;
	AVLONG nTime;
	std::wstring wstrStyle;

	friend std::wstringstream &operator << (std::wstringstream &s, WAYPOINT &w);
	friend std::wstringstream &operator >> (std::wstringstream &s, WAYPOINT &w);
};

/////////////////////////////////////////////////////////////
// Passenger representation in the Simulation Data

class CPassenger : public dbtools::CCollection
{
	CSim *m_pSim;

	// Simulation & Derived Data
	AVULONG m_nId;
	AVULONG m_nSimId;

	// all data below read from a DB except for m_timeSpawn and m_timeGo which are calculated from the data
	AVULONG m_nShaft;
	AVULONG m_nLift;
	AVULONG m_nDeck;
	AVULONG m_nArrivalFloor;
	AVULONG m_nDestFloor;

	AVLONG m_timeSpawn;			// when appears outside the lobby to step into the action --- calculated from the data - walking backwards from the arrival time
	AVLONG m_timeArrival;		// arrival time to the lobby - the main timestamp of the hallcall
	AVLONG m_timeGo;			// time to start walking towards the lift - calculated from data - walking backwards from the loading time
	AVLONG m_timeLoad;			// loading time - when passing the door into the lift
	AVLONG m_timeUnload;		// unloading time - when passing the door out of the lift

	AVLONG m_spanWait;			// waiting time (used to determine the colours)

protected:
	// Way Points
	AVULONG m_nWaypoints;
	WAYPOINT *m_pWaypoints;

public:
	CPassenger(CSim *pSim, AVULONG nPassengerId);
	virtual ~CPassenger();

	CSim *GetSim()					{ return m_pSim; }
	CProject *GetProject()			{ return m_pSim->GetProject(); }

	AVULONG GetId()					{ return m_nId; }
	AVULONG GetSimId()				{ return m_nSimId; }

	AVULONG GetShaftId()			{ return m_nShaft; }
	AVULONG GetLiftId()				{ return m_nLift; }
	AVULONG GetDeck()				{ return m_nDeck; }
	AVULONG GetArrivalFloor()		{ return m_nArrivalFloor; }
	AVULONG GetDestFloor()			{ return m_nDestFloor; }
	AVLONG GetSpawnTime()			{ return m_timeSpawn; }
	AVLONG GetArrivalTime()			{ return m_timeArrival; }
	AVLONG GetGoTime()				{ return m_timeGo; }
	AVLONG GetLoadTime()			{ return m_timeLoad; }
	AVLONG GetUnloadTime()			{ return m_timeUnload; }
	AVLONG GetWaitSpan()			{ return m_spanWait; }

	void SetId(AVULONG n)			{ m_nId = n; }
	void SetSimId(AVULONG nId)		{ m_nSimId = nId; }

	void SetShaftId(AVULONG n)		{ m_nShaft = n; }
	void SetLiftId(AVULONG n)		{ m_nLift = n; }
	void SetDeck(AVULONG n)			{ m_nDeck = n; }
	void SetArrivalFloor(AVULONG n)	{ m_nArrivalFloor = n; }
	void SetDestFloor(AVULONG n)	{ m_nDestFloor = n; }
	void SetSpawnTime(AVLONG n)		{ m_timeSpawn = n; }
	void SetArrivalTime(AVLONG n)	{ m_timeArrival = n; }
	void SetGoTime(AVLONG n)		{ m_timeGo = n; }
	void SetLoadTime(AVLONG n)		{ m_timeLoad = n; }
	void SetUnloadTime(AVLONG n)	{ m_timeUnload = n; }
	void SetWaitSpan(AVULONG n)		{ m_spanWait = n; }

	void CreateWaypoints(AVULONG nCount);
	AVULONG GetWaypointCount()					{ return m_nWaypoints; }
	WAYPOINT *GetWaypoint(AVULONG i)			{ return i < GetWaypointCount() ? &m_pWaypoints[i] : NULL; }
	void SetWaypoint(AVULONG i, WAYPOINT &wp)	{ m_pWaypoints[i] = wp; }

	void ResolveMe();

	std::wstring StringifyWayPoints();
	void ParseWayPoints(std::wstring);
};

