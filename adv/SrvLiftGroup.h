// SrvLiftGroup.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrLiftGroup.h"
#include "../CommonFiles/DBTools.h"

class CSimSrv;

class CLiftGroupSrv : public CLiftGroupConstr
{
public:
	CLiftGroupSrv(CProject *pProject, AVULONG nIndex) : CLiftGroupConstr(pProject, nIndex)	{ }

protected:
	virtual CSim *CreateSim();

public:
	CSimSrv *GetSim()							{ return (CSimSrv*)CLiftGroupConstr::GetSim(); }

	SHAFT  *AddShaft()							{ return (SHAFT *)CLiftGroupConstr::AddShaft(); }
	STOREY *AddStorey()							{ return (STOREY*)CLiftGroupConstr::AddStorey(); }
	LIFT   *AddLift()							{ return (LIFT  *)CLiftGroupConstr::AddLift(); }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nLiftGroupId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLiftGroupID);
};
