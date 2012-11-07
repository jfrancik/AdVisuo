// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "BaseData.h"
#include <functional>

class CSim;
class CLift;
class CPassenger;

class CSim : public dbtools::CCollection
{
	// project information
	AVULONG m_nId;					// sim id
	AVULONG m_nProjectId;			// project id
	AVULONG m_nSIMVersionId;		// SIM version id
	AVULONG m_nIndex;				// index in multi-group structures
	AVLONG m_nSimulationTime;
	AVULONG m_nTimeSaved;
	
	CBuilding *m_pBuilding;
	AVVECTOR m_vecOffset;

	std::vector<CLift*> m_lifts;
	std::vector<CPassenger*> m_passengers;

public:
	CSim(CBuilding *pBuilding, AVULONG nIndex);
	virtual ~CSim();

	CBuilding *GetBuilding()					{ return m_pBuilding;}
	void SetBuilding(CBuilding *pBuilding)		{ m_pBuilding = pBuilding; }

	std::wstring GetSIMFileName()				{ return ME[L"SIMFileName"]; }
	std::wstring GetIFCFileName()				{ return ME[L"IFCFileName"]; }
	AVULONG GetBldgFloors()						{ return ME[L"Floors"]; }
	AVULONG GetBldgShafts()						{ return ME[L"Shafts"]; }
	DATE GetTimeStamp()							{ return ME[L"TimeStamp"]; }
	enum ALGORITHM { DESTINATION, COLLECTIVE } ;
	ALGORITHM GetAlgorithm()					{ return (ALGORITHM)(ULONG)ME[L"Algorithm"]; }

	AVULONG GetId()								{ return m_nId; }
	AVULONG GetProjectId()						{ return m_nProjectId; }
	AVULONG GetSIMVersionId()					{ return m_nSIMVersionId; }
	AVULONG GetIndex()							{ return m_nIndex; }

	void SetId(AVULONG n)						{ m_nId = n; }
	void SetProjectId(AVULONG n)				{ m_nProjectId = n; }
	void SetSIMVersionId(AVULONG n)				{ m_nSIMVersionId = n; }
	void SetIndex(AVULONG n)					{ m_nIndex = n; }

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

class CProject : public dbtools::CCollection
{
	AVULONG m_nSimulationId;		// original (console) simulation id

	AVULONG m_nId;					// project id
	AVULONG m_nAVVersionId;			// AV Version id
	AVULONG m_nLiftGroupsCount;		// number of lift groups

protected:
	std::vector<CSim*> m_sims;
//	int m_nDefault;

public:
	CProject();
	virtual ~CProject();

	AVULONG GetSimulationId()					{ return m_nSimulationId; }
	AVULONG GetId()								{ return m_nId; }
	AVULONG GetAVVersionId()					{ return m_nAVVersionId; }
	static AVULONG GetAVNativeVersionId()		{ return 10900; }
	AVULONG GetLiftGroupsCount()				{ return m_nLiftGroupsCount; }
	
	void SetSimulationId(AVULONG n)				{ m_nSimulationId = n; }
	void SetId(AVULONG n)						{ m_nId = n; }
	void SetAVVersionId(AVULONG n)				{ m_nAVVersionId = n; }
	void SetLiftGroupsCount(AVULONG n)			{ m_nLiftGroupsCount = n; }

	CSim *GetSim(int i)							{ return m_sims[i]; }
	CSim *GetSim()								{ return GetSim(GetDefault()); }
	CBuilding *GetBuilding(int i)				{ return GetSim(i)->GetBuilding(); }
	CBuilding *GetBuilding()					{ return GetSim()->GetBuilding(); }
	int GetDefault()							{ return m_nDefault; }
	void SetDefault(int n)						{ m_nDefault = n; }

	enum PRJ_INFO { PRJ_PROJECT_NAME, PRJ_BUILDING_NAME, PRJ_LANGUAGE, PRJ_UNITS, PRJ_COMPANY, PRJ_CITY, PRJ_LB_RGN, PRJ_COUNTY, PRJ_DESIGNER, PRJ_COUNTRY, PRJ_CHECKED_BY, PRJ_POST_CODE };
	std::wstring GetProjectInfo(PRJ_INFO what);

	void ResolveMe();
	void ResolveLiftGroups();

	void Scale(AVFLOAT fScale)						{ for each (CSim *pSim in m_sims) pSim->GetBuilding()->Scale(fScale); }
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)		{ for each (CSim *pSim in m_sims) pSim->GetBuilding()->Move(x, y, z); }

protected:
	virtual CBuilding *CreateBuilding(AVULONG iIndex) = 0;
	virtual CSim *CreateSim(CBuilding *pBuilding, AVULONG iIndex) = 0;
};

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

