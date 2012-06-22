// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "../CommonFiles/DBTools.h"
#include "SrvProject.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CProjectSrv::CProjectSrv() : CProjectConstr()
{
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

	// Query for Simulations
	sel = db.select(L"SELECT * FROM Simulations s, Projects p WHERE p.ProjectId = s.ProjectId AND SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

	sel = db.select(L"SELECT COUNT(LiftGroupId) As LiftGroupsCount FROM LiftGroups WHERE SimulationId=%d", nSimulationId);
	sel >> ME;

	SetSimulationId(nSimulationId);
	SetAVVersionId(GetAVNativeVersionId());
	SetLiftGroupsCount(ME[L"LiftGroupsCount"]);

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

	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetId(sel[(short)0]);

	return S_OK;
}

HRESULT CProjectSrv::CleanUp(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVPassengers WHERE SimID IN (SELECT S.ID FROM AVSims S, AVProjects V WHERE S.ProjectId = V.ID AND V.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVJourneys     WHERE SimID IN (SELECT S.ID FROM AVSims S, AVProjects V WHERE S.ProjectId = V.ID AND V.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL AND OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DELETE FROM AVFloors WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVSims S, AVProjects P WHERE B.SimID = S.ID AND S.ProjectId = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL AND OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DELETE FROM AVShafts WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVSims S, AVProjects P WHERE B.SimID = S.ID AND S.ProjectId = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVBuildings WHERE SimID IN (SELECT S.ID FROM AVSims S, AVProjects V WHERE S.ProjectId = V.ID AND V.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVSims','U') IS NOT NULL DELETE FROM AVSims WHERE ProjectId IN (SELECT ID FROM AvProjects WHERE SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVProjects WHERE SimulationId=%d", nSimulationId);
	return S_OK;
}

HRESULT CProjectSrv::CleanUpAll(CDataBase db)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DELETE FROM AVPassengers");
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL DELETE FROM AVJourneys");
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL DELETE FROM AVFloors");
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL DELETE FROM AVShafts");
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVProjects");
	db.execute(L"IF OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DELETE FROM AVBuildings");
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
	db.execute(L"IF OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DROP TABLE AVBuildings");
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DROP TABLE AVProjects");
	return S_OK;
}
