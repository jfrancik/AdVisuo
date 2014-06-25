// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "SrvLiftGroup.h"

class CSimSrv;
class CPassenger;

struct COMP_LOADING		{ bool operator()(CPassenger *p1, CPassenger *p2) const { return p1->GetLoadTime()   < p2->GetLoadTime();   }  };
struct COMP_UNLOADING	{ bool operator()(CPassenger *p1, CPassenger *p2) const { return p1->GetUnloadTime() < p2->GetUnloadTime(); }  };

class CLiftSrv : public CLift
{
	// Passenger collections - fed by AddPassenger while passengers are read, consumed by Feed
	std::multiset<CPassenger*, COMP_LOADING  > m_collLoading;
	std::multiset<CPassenger*, COMP_UNLOADING> m_collUnloading;

	std::deque<JOURNEY> m_collJourneyCache;

	JOURNEY *m_pJourney;
	AVULONG m_nPassengers[DECK_NUM];
	AVLONG m_nTimeLiftArrive[DECK_NUM];
	AVLONG m_nTimeOpen[DECK_NUM], m_nTimeClose[DECK_NUM];

	//AVLONG m_nTimeFed;

public:
	CLiftSrv(CSimSrv *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSimSrv *GetSim()							{ return (CSimSrv*)CLift::GetSim(); }

	void Feed(AVULONG nFloor, AVLONG nTime, AVLONG nDuration);
	//AVLONG GetTimeFed()							{ m_nTimeFed; }

	HRESULT Store(dbtools::CDataBase db, JOURNEY &j);
	HRESULT Store(dbtools::CDataBase db);	// <-- this one obsolete

	// helper storage to temporarily associate passengers and stops to this lift (not persistent)
	void AddPassenger(CPassenger *pPassenger)	{ m_collLoading.insert(pPassenger); m_collUnloading.insert(pPassenger);}

	// helper storage to temporarily keep JOURNEYs before storing
	size_t GetJourneyCacheSize()			{ return  m_collJourneyCache.size(); }
	JOURNEY PullCachedJourney()				{ JOURNEY J = m_collJourneyCache.front(); m_collJourneyCache.pop_front(); return J; }
};
