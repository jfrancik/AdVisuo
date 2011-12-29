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

	static AVULONG GetVersion()								{ return 10900; }

	HRESULT LoadSim();
	void Play();

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationID, ULONG &nProjectID);

	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationID);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);

	HRESULT Store(dbtools::CDataBase db, ULONG nSimulationID);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationID);
	static HRESULT CleanUpAll(dbtools::CDataBase db);

protected:
	virtual CPassengerBase *CreatePassenger(AVULONG nId)	{ return new CPassenger(this, nId); }
	virtual CLiftBase *CreateLift(AVULONG nId)				{ return new CLift(this, nId); }
};
