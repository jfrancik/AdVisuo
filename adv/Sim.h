// Sim.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "Lift.h"
#include "Passenger.h"
#include "Building.h"

class CSim : public CSimBase
{
public:
	CSim(CBuildingBase *pBuilding);
	CLift *GetLift(int i)									{ return (CLift*)CSimBase::GetLift(i); }
	CPassenger *GetPassenger(int i)							{ return (CPassenger*)CSimBase::GetPassenger(i); }
	CBuilding *GetBuilding()								{ return (CBuilding*)CSimBase::GetBuilding(); }

	HRESULT LoadSim();
	void Play();

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);
	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);
	static HRESULT CleanUpAll(dbtools::CDataBase db);
	static HRESULT DropTables(dbtools::CDataBase db);

protected:
	virtual CPassengerBase *CreatePassenger(AVULONG nId)	{ return new CPassenger(this, nId); }
	virtual CLiftBase *CreateLift(AVULONG nId)				{ return new CLift(this, nId); }
};
