// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "SrvLiftGroup.h"

class CSimSrv;
class CPassenger;

class CLiftSrv : public CLift
{
	// Passenger collections - fed by AddPassenger while passengers are read, consumed by Feed
	std::deque<CPassenger*> m_collPassengers;
	std::deque<CPassenger*> m_collUnloading;

	std::deque<JOURNEY> m_collJourneyCache;

	JOURNEY *m_pJourney;
	AVULONG m_nPassengers[DECK_NUM];
	AVLONG m_nTimeLiftArrive[DECK_NUM];
	AVLONG m_nTimeOpen[DECK_NUM], m_nTimeClose[DECK_NUM];

public:
	CLiftSrv(CSimSrv *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSimSrv *GetSim()							{ return (CSimSrv*)CLift::GetSim(); }

	void Feed(AVULONG nFloor, AVLONG nTime, AVLONG nDuration);

	HRESULT Store(dbtools::CDataBase db, JOURNEY &j);
	HRESULT Store(dbtools::CDataBase db);	// <-- this one obsolete

	// helper storage to temporarily associate passengers and stops to this lift (not persistent)
	void AddPassenger(CPassenger *pPassenger)	{ m_collPassengers.push_back(pPassenger); m_collUnloading.push_back(pPassenger);}

	// helper storage to temporarily keep JOURNEYs before storing
	size_t GetJourneyCacheSize()			{ return  m_collJourneyCache.size(); }
	JOURNEY PullCachedJourney()				{ JOURNEY J = m_collJourneyCache.front(); m_collJourneyCache.pop_front(); return J; }
};
