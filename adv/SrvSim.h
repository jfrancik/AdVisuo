// Sim.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "SrvBuilding.h"

class CSimSrv : public CSim
{
public:
	CSimSrv(CBuilding *pBuilding, AVULONG nIndex);
	CLiftSrv *GetLift(int i)								{ return (CLiftSrv*)CSim::GetLift(i); }
	CPassengerSrv *GetPassenger(int i)						{ return (CPassengerSrv*)CSim::GetPassenger(i); }
	CBuildingSrv *GetBuilding()								{ return (CBuildingSrv*)CSim::GetBuilding(); }

	HRESULT LoadSim(dbtools::CDataBase db, AVULONG nSimulationId);
	void Play();

	// Database operations
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId)		{ return new CPassengerSrv(this, nId); }
	virtual CLift *CreateLift(AVULONG nId)					{ return new CLiftSrv(this, nId); }
};
