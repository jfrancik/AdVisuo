// Passenger.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"

class CSimSrv;

class CPassengerSrv : public CPassenger
{
public:
	CPassengerSrv(CSimSrv *pSim, AVULONG nPassengerId);

	CSimSrv *GetSim()				{ return (CSimSrv*)CPassenger::GetSim(); }

	void Play();

	DWORD Load(dbtools::CDataBase::SELECT &sel);
	HRESULT Store(dbtools::CDataBase db);
};
