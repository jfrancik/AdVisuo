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
	CSimSrv *GetSim(AVULONG i)					{ return (CSimSrv*)CLiftGroupConstr::GetSim(i); }
	CProjectSrv *GetProject()					{ return (CProjectSrv*)CLiftGroupConstr::GetProject(); }
	
public:
	// IO
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nLiftGroupId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nLiftGroupID);
	HRESULT Store(dbtools::CDataBase db);
};
