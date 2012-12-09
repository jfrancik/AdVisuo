// Sim.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"

class CPassengerSrv;
class CLiftSrv;
class CLiftGroupSrv;

class CSimSrv : public CSim
{
public:
	CSimSrv()												{ }
	CLiftSrv *GetLift(int i)								{ return (CLiftSrv*)CSim::GetLift(i); }
	CPassengerSrv *GetPassenger(int i)						{ return (CPassengerSrv*)CSim::GetPassenger(i); }
	CLiftGroupSrv *GetLiftGroup()							{ return (CLiftGroupSrv*)CSim::GetLiftGroup(); }

	HRESULT LoadSim(dbtools::CDataBase db, AVULONG nSimulationId);
	void Play();

	// Database operations
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLiftGroupId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId);
	virtual CLift *CreateLift(AVULONG nId);
};
