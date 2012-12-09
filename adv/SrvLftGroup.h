// SrvLftGroup.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrLftGroup.h"
#include "../CommonFiles/DBTools.h"

class CSimSrv;

class CLftGroupSrv : public CLftGroupConstr
{
public:
	CLftGroupSrv(CProject *pProject, AVULONG nIndex) : CLftGroupConstr(pProject, nIndex)	{ }

protected:
	virtual CSim *CreateSim();

public:
	CSimSrv *GetSim()							{ return (CSimSrv*)CLftGroupConstr::GetSim(); }

	SHAFT  *AddShaft()							{ return (SHAFT *)CLftGroupConstr::AddShaft(); }
	STOREY *AddStorey()							{ return (STOREY*)CLftGroupConstr::AddStorey(); }
	LIFT   *AddLift()							{ return (LIFT  *)CLftGroupConstr::AddLift(); }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nLftGroupId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLftGroupID);
};
