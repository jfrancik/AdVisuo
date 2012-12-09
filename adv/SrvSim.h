// Sim.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"

class CPassengerSrv;
class CLiftSrv;
class CLftGroupSrv;

class CSimSrv : public CSim
{
public:
	CSimSrv()												{ }
	CLiftSrv *GetLift(int i)								{ return (CLiftSrv*)CSim::GetLift(i); }
	CPassengerSrv *GetPassenger(int i)						{ return (CPassengerSrv*)CSim::GetPassenger(i); }
	CLftGroupSrv *GetLftGroup()								{ return (CLftGroupSrv*)CSim::GetLftGroup(); }

	HRESULT LoadSim(dbtools::CDataBase db, AVULONG nSimulationId);
	void Play();

	// Database operations
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLftGroupId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId);
	virtual CLift *CreateLift(AVULONG nId);
};
