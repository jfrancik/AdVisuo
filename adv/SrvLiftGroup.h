// SrvLiftGroup.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrLiftGroup.h"
#include "../CommonFiles/DBTools.h"

class CSimSrv;
class CProjectSrv;

class CLiftGroupSrv : public CLiftGroupConstr
{
public:
	CLiftGroupSrv(CProject *pProject, AVULONG nIndex) : CLiftGroupConstr(pProject, nIndex)	{ }

protected:
	virtual CSim *CreateSim();

public:
	CSimSrv *AddSim()							{ return (CSimSrv*)CLiftGroupConstr::AddSim(); }
	CSimSrv *GetSim()							{ return (CSimSrv*)CLiftGroupConstr::GetSim(); }

	SHAFT  *AddShaft()							{ return (SHAFT *)CLiftGroupConstr::AddShaft(); }
	STOREY *AddStorey()							{ return (STOREY*)CLiftGroupConstr::AddStorey(); }
	LIFT   *AddLift()							{ return (LIFT  *)CLiftGroupConstr::AddLift(); }

	CProjectSrv *GetProject()					{ return (CProjectSrv*)CLiftGroupConstr::GetProject(); }
	STOREY *GetStorey(AVULONG i)				{ return (STOREY*)CLiftGroupConstr::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)		{ return (STOREY*)CLiftGroupConstr::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)					{ return (SHAFT*)CLiftGroupConstr::GetShaft(i); }
	LIFT *GetLift(AVULONG i)					{ return (LIFT*)CLiftGroupConstr::GetLift(i); }
	MACHINEROOM *GetMachineRoom()				{ return (MACHINEROOM*)CLiftGroupConstr::GetMachineRoom(); }
	PIT *GetPit()								{ return (PIT*)CLiftGroupConstr::GetPit(); }



public:
	// IO
	HRESULT Store(dbtools::CDataBase db);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nLiftGroupId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLiftGroupID);
};
