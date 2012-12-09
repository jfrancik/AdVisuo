// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"

class CLftGroupSrv;
class CSimSrv;

class CProjectSrv : public CProjectConstr
{
public:
	CProjectSrv();

	virtual CElem *CreateElement(CLftGroup *pLftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)
											{ return NULL; }

	CLftGroupSrv *GetLftGroup(int i)		{ return (CLftGroupSrv*)CProjectConstr::GetLftGroup(i); }
	CLftGroupSrv *FindLftGroup(int id)		{ return (CLftGroupSrv*)CProjectConstr::FindLftGroup(id); }
	CLftGroupSrv *AddLftGroup()				{ return (CLftGroupSrv*)CProjectConstr::AddLftGroup(); }

	CSimSrv *GetSim(int i)					{ return (CSimSrv*)CProjectConstr::GetSim(i); }
	CSimSrv *FindSim(int id)				{ return (CSimSrv*)CProjectConstr::FindSim(id); }

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);
	static HRESULT CleanUpAll(dbtools::CDataBase db);
	static HRESULT DropTables(dbtools::CDataBase db);

	HRESULT LoadSim(dbtools::CDataBase db, AVULONG nSimulationId);
	
	void Play();

protected:
	virtual CLftGroup *CreateLftGroup(AVULONG nIndex);
};
