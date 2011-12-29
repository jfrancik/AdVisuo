// Lift.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/DBTools.h"
#include "SimLoader.h"

class CSim;
class CBuilding;

class CLift : public CLiftBase
{
public:
	CLift(CSim *pSim, AVULONG nLiftId, AVULONG nDecks = 1);

	CSim *GetSim()					{ return (CSim*)CLiftBase::GetSim(); }

	// IO: load from SIM File, Store to DB
	DWORD Load(CSimLoader &loader, AVULONG nId, bool bCalcUnload = false, bool bCalcLoad = false);
	HRESULT Store(dbtools::CDataBase db, ULONG nProjectId);
};
