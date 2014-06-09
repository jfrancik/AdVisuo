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
	static AVULONG QueryVerInt(dbtools::CDataBase db);
	static std::wstring QueryVerStr(dbtools::CDataBase db);
	static AVULONG QuerySimCountFromConsole(dbtools::CDataBase db, ULONG nSimulationId);		// finds Sim count from the Console database
	static AVULONG QuerySimCountFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	static void QueryAvailIds(dbtools::CDataBase db, std::vector<AVULONG> &ids);

	static HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);	// searches for prj id in Visualisation database
	HRESULT LoadFromConsole(dbtools::CDataBase dbConsole, ULONG nSimulationId);	// special IFC version (no access to the reports)
	HRESULT LoadFromConsole(dbtools::CDataBase dbConsole, dbtools::CDataBase dbReports, ULONG nSimulationId);
	HRESULT FastLoadPlayUpdate(dbtools::CDataBase dbVis, dbtools::CDataBase dbReports);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);

	void Play();

	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db);

	HRESULT PlayAndUpdate(dbtools::CDataBase dbVis);

	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);		// deletes all data for a particular simulation
	static HRESULT CleanUpSim(dbtools::CDataBase db, ULONG nProjectId);		// deletes sim data (passengers and journeys) for a particular simulation
	static HRESULT CleanUpAll(dbtools::CDataBase db);						// deletes all simulations
	static HRESULT DropTables(dbtools::CDataBase db);						// drops all tables

	static HRESULT CreateTicket(dbtools::CDataBase db, AVSTRING strUserId, AVSTRING strBuf);

protected:
	virtual CLiftGroup *CreateLiftGroup(AVULONG nIndex);
};
