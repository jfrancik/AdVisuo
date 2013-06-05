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

	void Play();

	// Database operations
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nSimId);
	HRESULT LoadFromReports(dbtools::CDataBase db);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db);

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId);
	virtual CLift *CreateLift(AVULONG nId);
};
