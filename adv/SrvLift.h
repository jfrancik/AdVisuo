// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "SrvLiftGroup.h"

class CSimSrv;

class CLiftSrv : public CLift
{
public:
	CLiftSrv(CSimSrv *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSimSrv *GetSim()					{ return (CSimSrv*)CLift::GetSim(); }

	// IO: load from SIM File, Store to DB
	DWORD Load(CLiftGroupSrv::LIFT *pLIFT, dbtools::CDataBase db, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration);
	HRESULT Store(dbtools::CDataBase db);

	DWORD Adjust(bool bCalcUnload = true, bool bCalcLoad = true);
};
