// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "SrvLiftGroup.h"

class CSimSrv;
class CPassenger;

class CLiftSrv : public CLift
{
public:
	struct STOP
	{
		AVULONG nFloor;
		AVLONG nTime, nDuration;
	};
	std::deque<CPassenger*> m_collPassengers;
	std::deque<STOP> m_collStops;
public:
	CLiftSrv(CSimSrv *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSimSrv *GetSim()					{ return (CSimSrv*)CLift::GetSim(); }

	DWORD Load(dbtools::CDataBase db);
	DWORD Load();
	HRESULT Store(dbtools::CDataBase db);

	// helper storage to temporarily associate passengers and stops to this lift (not persistent)
	void AddPassenger(CPassenger *pPassenger)						{ m_collPassengers.push_back(pPassenger); }
	void AddStop(AVULONG nFloor, AVLONG nTime, AVLONG nDuration)	{ STOP st = { nFloor, nTime, nDuration }; m_collStops.push_back(st); }

	// IO: load from SIM File, Store to DB
	DWORD legacy_Load(CLiftGroupSrv::LIFT *pLIFT, dbtools::CDataBase db, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration);
	DWORD legacy_Adjust();
	bool legacy_ReportDifferences(CLiftSrv *p);
};
