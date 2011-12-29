// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "BaseData.h"
#include "BaseBuilding.h"

class CLiftBase;
class CPassengerBase;

class CSimBase
{
protected:
	CBuildingBase *m_pBuilding;

	std::vector<CLiftBase*> m_lifts;
	std::vector<CPassengerBase*> m_passengers;

public:
	// project information
	std::wstring m_strSIMFileName;			// SIM filename
	std::wstring m_strIFCFileName;			// IFC filename
	DATE m_dateTimeStamp;					// time stamp

	std::wstring m_strProjectName;
	std::wstring m_strLanguage;
	std::wstring m_strMeasurementUnits;
	std::wstring m_strBuildingName;
	std::wstring m_strClientCompanyName;
	std::wstring m_strCity;
	std::wstring m_strLBRegionDistrict;
	std::wstring m_strStateCounty;
	std::wstring m_strLiftDesigner;
	std::wstring m_strCountry;
	std::wstring m_strCheckedBy;
	std::wstring m_strPostalZipCode;

	AVULONG m_nProjectID;
	AVULONG m_nSimulationID;
	AVULONG m_nSIMVersionID;
	AVULONG m_nAVVersionID;
	AVULONG m_nBldFloors;
	AVULONG m_nBldShafts;
	AVULONG m_nBldLifts;
	AVULONG m_nPassengers;
	AVLONG m_nSimulationTime;
	bool m_bSavedAll;
	AVULONG m_nJourneysSaved, m_nPassengersSaved, m_nTimeSaved;
	
	enum ALGORITHM { DESTINATION, COLLECTIVE } m_nAlgorithm;

public:
	CSimBase(CBuildingBase *pBuilding);
	virtual ~CSimBase();

	CBuildingBase *GetBuilding()				{ return m_pBuilding;}
	void SetBuilding(CBuildingBase *pBuilding)	{ m_pBuilding = pBuilding; }

	// access & initialisation
	AVULONG GetLiftCount()					{ return m_lifts.size(); }
	CLiftBase *GetLift(AVULONG i)			{ return i < GetLiftCount() ? m_lifts[i] : NULL; }
	void AddLift(CLiftBase *p)				{ m_lifts.push_back(p); }
	void DeleteLifts();
	AVULONG GetJourneyTotalCount();

	AVULONG GetPassengerCount()				{ return m_passengers.size(); }
	CPassengerBase *GetPassenger(AVULONG i)	{ return i < GetPassengerCount() ? m_passengers[i] : NULL; }
	void AddPassenger(CPassengerBase *p)	{ m_passengers.push_back(p); }
	void DeletePassengers();

protected:
	virtual CPassengerBase *CreatePassenger(AVULONG nId) = 0;
	virtual CLiftBase *CreateLift(AVULONG nId) = 0;
};

class CLiftBase
{
protected:
	CSimBase *m_pSim;			// main sim object

	AVULONG m_nId;				// lift id
	AVULONG m_nDecks;			// number of decks (2 for double decker, 1 for single decker)

	// Lift Journeys
	std::vector<JOURNEY> m_journeys;

public:
	CLiftBase(CSimBase *pSim, AVULONG nLiftId, AVULONG nDecks = 1);
	virtual ~CLiftBase();

	CSimBase *GetSim()				{ return m_pSim; }

	AVULONG GetId()					{ return m_nId; }
	void SetId(AVULONG n)			{ m_nId = n; }

	bool IsDoubleDecker()			{ return m_nDecks >= 2; }
	AVULONG GetDecks()				{ return m_nDecks; }
	void SetDecks(AVULONG n)		{ m_nDecks = n; }

	AVULONG GetJourneyCount()		{ return m_journeys.size(); }
	JOURNEY *GetJourney(AVULONG i)	{ return i < GetJourneyCount() ? &m_journeys[i] : NULL; }
	void AddJourney(JOURNEY &j)		{ m_journeys.push_back(j); }
};

class CPassengerBase
{
protected:
	CSimBase *m_pSim;

	// Simulation & Derived Data
	AVULONG m_nId;
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

	// Way Points
	AVULONG m_nWaypoints;
	WAYPOINT *m_pWaypoints;

public:
	CPassengerBase(CSimBase *pSim, AVULONG nPassengerId);
	virtual ~CPassengerBase();

	CSimBase *GetSim()				{ return m_pSim; }

	AVULONG GetId()					{ return m_nId; }
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

	std::wstring StringifyWayPoints();
	void ParseWayPoints(std::wstring);
};

class CSimBaseEx : public CSimBase
{
protected:
	virtual CPassengerBase *CreatePassenger(AVULONG nId)	{ return new CPassengerBase(this, nId); }
	virtual CLiftBase *CreateLift(AVULONG nId)				{ return new CLiftBase(this, nId); }
};

