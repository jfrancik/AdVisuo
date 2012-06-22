// Passenger.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/DBTools.h"
#include "SrvSimLoader.h"

class CSimSrv;

class CPassengerSrv : public CPassenger
{
public:
	CPassengerSrv(CSimSrv *pSim, AVULONG nPassengerId);

	CSimSrv *GetSim()				{ return (CSimSrv*)CPassenger::GetSim(); }

	void Play();

	// IO: load from SIM File, Store to DB
	DWORD Load(AVULONG nId, CSimLoader::Passenger &P);
	HRESULT Store(dbtools::CDataBase db);
};
