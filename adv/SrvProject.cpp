// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvProject.h"
#include "SrvLiftGroup.h"
#include "SrvSim.h"
#include "../CommonFiles/BaseScenario.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CProjectSrv::CProjectSrv() : CProjectConstr()
{
}

CLiftGroup *CProjectSrv::CreateLiftGroup(AVULONG nIndex)
{ 
	return new CLiftGroupSrv(this, nIndex); 
}

CScenario *CProjectSrv::CreateScenario(AVULONG iIndex)
{
	return new CScenario(this);
}

HRESULT CProjectSrv::FindProjectID(CDataBase db, ULONG nSimulationId, ULONG &nProjectID)
{
	if (!db) throw db;
	nProjectID = 0;
	CDataBase::SELECT sel;
		
	// Query for Project Data (test for any existing)
	try
	{
		sel = db.select(L"SELECT MAX(ID) AS id FROM AVProjects WHERE SimulationId=%d", nSimulationId);
	}
	catch (...)
	{
		return S_FALSE;
	}
	if (!sel) return S_FALSE;

	if (sel[L"id"].isNull()) return S_FALSE;
	nProjectID = sel[L"id"];
	return S_OK;
}

HRESULT CProjectSrv::LoadFromConsole(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	SetSimulationId(nSimulationId);
	SetAVVersionId(GetAVNativeVersionId());
	
	// Query for the Simulation (not Sim!)
	sel = db.select(L"SELECT * FROM Simulations s, Projects p WHERE p.ProjectId = s.ProjectId AND s.SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

	//// Query for Traffic Scenarios
	//sel = db.select(L"SELECT s.*, t.Name AS TrafficPatternName FROM TrafficScenarios s, TrafficPatternTypes t WHERE s.TrafficPatternTypeId=t.TrafficPatternTypeId AND s.LiftGroupId=%d ORDER BY TrafficScenarioIndex", nLiftGroupId);
	//while (sel)	// target is to replace it with while and collect ALL traffic scenarios!
	//{
	//	CSimSrv *pSim = AddSim();
	//	sel >> *pSim;
	//	sel++;
	//}


	// Query for Lift Groups
	sel = db.select(L"SELECT LiftGroupId FROM LiftGroups WHERE TenancyId IN (SELECT TenancyId FROM Tenancies WHERE SimulationId=%d)", nSimulationId);
	while (sel)
	{
		CLiftGroupSrv *pGroup = AddLiftGroup();
		pGroup->LoadFromConsole(db, sel[L"LiftGroupId"]);
		sel++;
	}

	return S_OK;
}

HRESULT CProjectSrv::LoadFromReports(dbtools::CDataBase db)
{
	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
		for each (CSimSrv *pSim in pGroup->GetSims())
		{
			HRESULT h = pSim->LoadFromReports(db);
			if FAILED(h) return h;
		}
	return S_OK;
}

HRESULT CProjectSrv::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;

	// Query for Project Data (for project id)
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT * FROM AVProjects WHERE ID=%d", nProjectID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> ME;

	// Query for Lift Groups
	sel = db.select(L"SELECT * FROM AVLiftGroups WHERE ProjectId=%d", nProjectID);
	while (sel)
	{
		CLiftGroupSrv *pGroup = AddLiftGroup();
		pGroup->LoadFromVisualisation(db, sel[L"ID"]);
		sel++;
	}

	ResolveMe();

	return S_OK;
}

HRESULT CProjectSrv::Store(CDataBase db)
{
	if (!db) throw db;

	CDataBase::INSERT ins = db.insert(L"AVProjects");

	ins << ME;
	ins[L"SimulationId"] = GetSimulationId();
	ins[L"AVVersionId"] = (float)(((AVFLOAT)GetAVVersionId())/100.0);

	ins[L"TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins[L"TimeStamp"].act_as_symbol();

	ins[L"MinSimulationTime"] = GetMinSimulationTime();
	ins[L"MaxSimulationTime"] = GetMaxSimulationTime();
	ins[L"TimeSaved"] = GetMaxSimulationTime();
	ins[L"SavedAll"] = (ULONG)0;

	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetId(sel[(short)0]);

	// Save LiftGroups & Sims
	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
	{
		pGroup->SetProjectId(GetId());
		HRESULT h = pGroup->Store(db);
		if FAILED(h) return h;
	}
	return S_OK;
}

HRESULT CProjectSrv::Update(dbtools::CDataBase db)
{
	if (!db) throw db;

	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
		for each (CSimSrv *pSim in pGroup->GetSims())
		{
			HRESULT h = pSim->Update(db);
			if FAILED(h) return h;
		}

	CDataBase::UPDATE upd = db.update(L"AVProjects", L"WHERE ID=%d", GetId());
	upd[L"MinSimulationTime"] = GetMinSimulationTime();
	upd[L"MaxSimulationTime"] = GetMaxSimulationTime();
	upd[L"TimeSaved"] = GetMaxSimulationTime();
	upd[L"SavedAll"] = true;
	upd.execute();

	return S_OK;
}

void CProjectSrv::Play()
{ 
	for each (CLiftGroupSrv *pGroup in GetLiftGroups()) 
		for each (CSimSrv *pSim in pGroup->GetSims())
			pSim->Play();
}

HRESULT CProjectSrv::CleanUp(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DELETE FROM AVPassengers WHERE SimID IN (SELECT S.ID FROM AVSims S, AVLiftGroups G, AVProjects V WHERE S.LiftGroupId = G.ID AND G.ProjectId = V.ID AND V.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U')   IS NOT NULL DELETE FROM AVJourneys   WHERE SimID IN (SELECT S.ID FROM AVSims S, AVLiftGroups G, AVProjects V WHERE S.LiftGroupId = G.ID AND G.ProjectId = V.ID AND V.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U')     IS NOT NULL DELETE FROM AVFloors     WHERE LiftGroupId IN (SELECT G.ID FROM AVLiftGroups G, AVProjects P WHERE G.ProjectId = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U')     IS NOT NULL DELETE FROM AVShafts     WHERE LiftGroupId IN (SELECT G.ID FROM AVLiftGroups G, AVProjects P WHERE G.ProjectId = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVSims','U')       IS NOT NULL DELETE FROM AVSims       WHERE LiftGroupId  IN (SELECT G.ID FROM AVLiftGroups G, AVProjects P WHERE G.ProjectId = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVLiftGroups','U') IS NOT NULL DELETE FROM AVLiftGroups WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U')   IS NOT NULL DELETE FROM AVProjects WHERE SimulationId=%d", nSimulationId);
	return S_OK;
}

HRESULT CProjectSrv::CleanUpAll(CDataBase db)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DELETE FROM AVPassengers");
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL DELETE FROM AVJourneys");
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL DELETE FROM AVFloors");
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL DELETE FROM AVShafts");
	db.execute(L"IF OBJECT_ID('dbo.AVSims','U') IS NOT NULL DELETE FROM AVSims");
	db.execute(L"IF OBJECT_ID('dbo.AVLiftGroups','U') IS NOT NULL DELETE FROM AVLiftGroups");
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVProjects");
	return S_OK;
}

HRESULT CProjectSrv::DropTables(CDataBase db)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DROP TABLE AVPassengers");
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL DROP TABLE AVJourneys");
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL DROP TABLE AVFloors");
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL DROP TABLE AVShafts");
	db.execute(L"IF OBJECT_ID('dbo.AVSims','U') IS NOT NULL DROP TABLE AVSims");
	db.execute(L"IF OBJECT_ID('dbo.AVLiftGroups','U') IS NOT NULL DROP TABLE AVLiftGroups");
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DROP TABLE AVProjects");
	return S_OK;
}

