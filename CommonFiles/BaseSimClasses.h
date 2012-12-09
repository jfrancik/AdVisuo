// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "dbtools.h"
#include <functional>

class CSim;
class CLift;
class CPassenger;

class CLftGroup;

/////////////////////////////////////////////////////////////
// Simulation Class - encapsulates all sim data

class CSim : public dbtools::CCollection
{
	// basic information
	AVULONG m_nId;					// sim id
	AVULONG m_nLftGroupId;			// lift group id
	AVULONG m_nSIMVersionId;		// SIM version id
	AVULONG m_nIndex;				// index in multi-group structures
	AVLONG m_nSimulationTime;
	AVULONG m_nTimeSaved;
	
	CLftGroup *m_pLftGroup;
	AVVECTOR m_vecOffset;

	std::vector<CLift*> m_lifts;
	std::vector<CPassenger*> m_passengers;

public:
	CSim();
	virtual ~CSim();

	CLftGroup *GetLftGroup()					{ return m_pLftGroup;}
	void SetLftGroup(CLftGroup *pLftGroup)		{ m_pLftGroup = pLftGroup; }

	AVULONG GetIndex()							{ return m_nIndex; }
	void SetIndex(AVULONG n)					{ m_nIndex = n; }

	std::wstring GetSIMFileName()				{ return ME[L"SIMFileName"]; }
	std::wstring GetIFCFileName()				{ return ME[L"IFCFileName"]; }
	AVULONG GetBldgFloors()						{ return ME[L"Floors"]; }
	AVULONG GetBldgShafts()						{ return ME[L"Shafts"]; }
	DATE GetTimeStamp()							{ return ME[L"TimeStamp"]; }
	enum ALGORITHM { DESTINATION, COLLECTIVE } ;
	ALGORITHM GetAlgorithm()					{ return (ALGORITHM)(ULONG)ME[L"Algorithm"]; }

	AVULONG GetId()								{ return m_nId; }
	AVULONG GetLftGroupId()						{ return m_nLftGroupId; }
	AVULONG GetSIMVersionId()					{ return m_nSIMVersionId; }

	void SetId(AVULONG n)						{ m_nId = n; }
	void SetLftGroupId(AVULONG n)				{ m_nLftGroupId = n; }
	void SetSIMVersionId(AVULONG n)				{ m_nSIMVersionId = n; }

	AVLONG GetSimulationTime()					{ return m_nSimulationTime; }
	void ReportSimulationTime(AVLONG n)			{ if (n > m_nSimulationTime) m_nSimulationTime = n; }

	AVULONG GetTimeSaved()						{ return m_nTimeSaved; }
	void ReportTimeSaved()						{ m_nTimeSaved = m_nSimulationTime; }

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

	void for_each_passenger(std::function<void (CPassenger*)> f)		{ for each (CPassenger *p in m_passengers) f(p); }
	void for_each_lift(std::function<void (CLift*)> f)					{ for each (CLift *p in m_lifts) f(p); }

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
		AVULONG m_timeOpen, m_durationOpen;
		AVULONG m_timeClose, m_durationClose;
		DOOR()					{ reset(); }

		void reset()			{ m_timeOpen = m_timeClose = UNDEF; m_durationOpen = m_durationClose = 1000; }

		AVULONG timeOpened()	{ return m_timeOpen + m_durationOpen; }
		AVULONG timeClosed()	{ return m_timeClose + m_durationClose; }

		friend std::wstringstream &operator << (std::wstringstream &s, DOOR &d);
		friend std::wstringstream &operator >> (std::wstringstream &s, DOOR &d);
	};

	std::vector<DOOR> m_doorcycles[DECK_NUM];

	AVULONG		m_shaftFrom, m_shaftTo;		// shaft id
	AVULONG		m_floorFrom, m_floorTo;		// journey floors
	AVULONG		m_timeGo, m_timeDest;		// journey time

	AVULONG FirstOpenTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? UNDEF : m_doorcycles[iDeck][0].m_timeOpen; }
	AVULONG FirstOpenedTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? UNDEF : m_doorcycles[iDeck][0].timeOpened(); }
	AVULONG LastCloseTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? 0     : m_doorcycles[iDeck][N-1].m_timeClose; }
	AVULONG LastClosedTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? 0     : m_doorcycles[iDeck][N-1].timeClosed(); }

	AVULONG FirstOpenTime()					{ AVULONG n = UNDEF; for (AVULONG i = 0; i < DECK_NUM; i++) n = min(n, FirstOpenTime(i)); return n; }
	AVULONG FirstOpenedTime()				{ AVULONG n = UNDEF; for (AVULONG i = 0; i < DECK_NUM; i++) n = min(n, FirstOpenedTime(i)); return n; }
	AVULONG LastCloseTime()					{ AVULONG n = 0; for (AVULONG i = 0; i < DECK_NUM; i++) n = max(n, LastCloseTime(i)); return n; }
	AVULONG LastClosedTime()				{ AVULONG n = 0; for (AVULONG i = 0; i < DECK_NUM; i++) n = max(n, LastClosedTime(i)); return n; }

	std::wstring StringifyDoorCycles();
	void ParseDoorCycles(std::wstring);

	JOURNEY()								{ m_shaftFrom = m_shaftTo = 0; m_floorFrom = m_floorTo = m_timeGo = m_timeDest = UNDEF; }
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

	CSim *GetSim()				{ return m_pSim; }

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

enum ENUM_ACTION { MOVE, WAIT, WALK, TURN, ENTER_ARR_FLOOR, ENTER_LIFT, ENTER_DEST_FLOOR };
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
	AVULONG m_nSimId;			// Sim ID

	AVULONG m_nShaft;			// set later
	AVULONG m_nLift;			// read from file
	AVULONG m_nDeck;			// set later
	AVULONG m_nArrivalFloor;	// read from file
	AVULONG m_nDestFloor;		// read from file

	AVLONG m_timeBorn;			// calculated from data - appear at the starting pos
	AVLONG m_timeArrival;		// read from file		- arrival to the wait zone (lobby)
	AVLONG m_timeGo;			// calculated from data	- start towards the lift
	AVLONG m_timeLoad;			// read from file		- load time (in the lift door)
	AVLONG m_timeUnload;		// read from file		- unload time (in the lift door)

	AVULONG m_spanWait;			// read from file or derived from lift data

protected:
	// Way Points
	AVULONG m_nWaypoints;
	WAYPOINT *m_pWaypoints;

public:
	CPassenger(CSim *pSim, AVULONG nPassengerId);
	virtual ~CPassenger();

	CSim *GetSim()					{ return m_pSim; }

	AVULONG GetId()					{ return m_nId; }
	AVULONG GetSimId()				{ return m_nSimId; }

	AVULONG GetShaftId()			{ return m_nShaft; }
	AVULONG GetLiftId()				{ return m_nLift; }
	AVULONG GetDeck()				{ return m_nDeck; }
	AVULONG GetArrivalFloor()		{ return m_nArrivalFloor; }
	AVULONG GetDestFloor()			{ return m_nDestFloor; }
	AVLONG GetBornTime()			{ return m_timeBorn; }
	AVLONG GetArrivalTime()			{ return m_timeArrival; }
	AVLONG GetGoTime()				{ return m_timeGo; }
	AVLONG GetLoadTime()			{ return m_timeLoad; }
	AVLONG GetUnloadTime()			{ return m_timeUnload; }
	AVULONG GetWaitSpan()			{ return m_spanWait; }

	void SetId(AVULONG n)			{ m_nId = n; }
	void SetSimId(AVULONG nId)		{ m_nSimId = nId; }

	void SetShaftId(AVULONG n)		{ m_nShaft = n; }
	void SetLiftId(AVULONG n)		{ m_nLift = n; }
	void SetDeck(AVULONG n)			{ m_nDeck = n; }
	void SetArrivalFloor(AVULONG n)	{ m_nArrivalFloor = n; }
	void SetDestFloor(AVULONG n)	{ m_nDestFloor = n; }
	void SetBornTime(AVLONG n)		{ m_timeBorn = n; }
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

