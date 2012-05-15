// Passenger.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/DBTools.h"
#include "SimLoader.h"

class CSim;

class CPassenger : public CPassengerBase
{
public:
	CPassenger(CSim *pSim, AVULONG nPassengerId);

	CSim *GetSim()				{ return (CSim*)CPassengerBase::GetSim(); }

	void Play();

	// IO: load from SIM File, Store to DB
	DWORD Load(AVULONG nId, CSimLoader::Passenger &P);
	HRESULT Store(dbtools::CDataBase db);
};
