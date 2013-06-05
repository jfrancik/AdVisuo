// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"

class CLiftGroupSrv;
class CSimSrv;

class CProjectSrv : public CProjectConstr
{
public:
	CProjectSrv();

	virtual CElem *CreateElement(CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)
											{ return NULL; }

	CLiftGroupSrv *GetLiftGroup(int i)		{ return (CLiftGroupSrv*)CProjectConstr::GetLiftGroup(i); }
	CLiftGroupSrv *FindLiftGroup(int id)	{ return (CLiftGroupSrv*)CProjectConstr::FindLiftGroup(id); }
	CLiftGroupSrv *AddLiftGroup()			{ return (CLiftGroupSrv*)CProjectConstr::AddLiftGroup(); }

	CSimSrv *FindSim(int id)				{ return (CSimSrv*)CProjectConstr::FindSim(id); }

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromReports(dbtools::CDataBase db);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db);

	void Play();

	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);
	static HRESULT CleanUpAll(dbtools::CDataBase db);
	static HRESULT DropTables(dbtools::CDataBase db);

protected:
	virtual CLiftGroup *CreateLiftGroup(AVULONG nIndex);
	virtual CScenario *CreateScenario(AVULONG nIndex);
};
