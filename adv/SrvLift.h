// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/DBTools.h"
#include "SrvSimLoader.h"
#include "SrvBuilding.h"

class CSimSrv;

class CLiftSrv : public CLift
{
public:
	CLiftSrv(CSimSrv *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSimSrv *GetSim()					{ return (CSimSrv*)CLift::GetSim(); }

	// IO: load from SIM File, Store to DB
	DWORD Load(CBuildingSrv::LIFT *pLIFT, CSimLoader &loader, AVULONG nId, bool bCalcUnload = false, bool bCalcLoad = false);
	HRESULT Store(dbtools::CDataBase db);
};
