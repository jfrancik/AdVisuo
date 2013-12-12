// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvProject.h"
#include "SrvLiftGroup.h"
#include "SrvPassenger.h"	// temporarily added - REMOVE
#include "SrvLift.h"	// temporarily added - REMOVE
#include "SrvSim.h"
#include <iostream>

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

AVULONG CProjectSrv::QueryVerInt(CDataBase db)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	try
	{
		sel = db.select(L"SELECT IntValue FROM AVStatus WHERE Name = 'VERSION_LATEST'");
	}
	catch (...)
	{
		return 0;
	}
	if (!sel) return 0;

	return sel[L"IntValue"];
}

std::wstring CProjectSrv::QueryVerStr(CDataBase db)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	try
	{
		sel = db.select(L"SELECT Value FROM AVStatus WHERE Name = 'VERSION_LATEST'");
	}
	catch (...)
	{
		return L"";
	}
	if (!sel) return 0;

	return sel[L"Value"];
}

AVULONG CProjectSrv::QuerySimCountFromConsole(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	try
	{
		sel = db.select(L"SELECT COUNT(s.TrafficScenarioId) AS ScenarioCount FROM LiftGroups g, Tenancies t, TrafficScenarios s WHERE t.SimulationId = %d AND t.TenancyId = g.TenancyId AND g.LiftGroupId = s.LiftGroupId", nSimulationId);
	}
	catch (...)
	{
		return S_FALSE;
	}
	if (!sel) return 0;

	return sel[L"ScenarioCount"];
}

AVULONG CProjectSrv::QuerySimCountFromVisualisation(CDataBase db, ULONG nProjectId)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	try
	{
		sel = db.select(L"SELECT COUNT(s.ID) AS ScenarioCount FROM AVSims s, AVLiftGroups g WHERE s.LiftGroupId = g.ID AND g.ProjectId = %d", nProjectId);
	}
	catch (...)
	{
		return S_FALSE;
	}
	if (!sel) return 0;

	return sel[L"ScenarioCount"];
}

void CProjectSrv::QueryAvailIds(dbtools::CDataBase db, std::vector<AVULONG> &ids)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
		
	try
	{
		sel = db.select(L"SELECT SimulationId FROM AVProjects ORDER BY SimulationId");
		for (; sel; sel++)
			ids.push_back(sel[L"SimulationId"]);
	}
	catch (...)
	{
	}
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
	SetAVVersionId(0);
	
	// Query for the Simulation (not Sim!)
	sel = db.select(L"SELECT p.*, f.Name AS ProjectFolderName, s.Name AS SimName, s.Comments AS SimComments, s.CreatedBy AS SimCreatedBy, s.CreatedDate AS SimCreatedDate, s.LastModifiedBy AS SimLastModifiedBy, s.LastModifiedDate AS SimLastModifiedDate FROM Projects p, ProjectFolders f, Simulations s WHERE p.ProjectId = s.ProjectId AND p.ProjectFolderId = f.ProjectFolderId AND s.SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

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
			LogProgress();
		}
	return S_OK;
}

HRESULT CProjectSrv::FastLoadFromReports(dbtools::CDataBase dbReports, dbtools::CDataBase dbVis)
{
	if (!dbReports) throw dbReports;
	if (!dbVis) throw dbVis;

	// preparatory step: create lifts and map structures for quickly accessing them
	map<AVULONG, CSimSrv*> mapSims;
	map<std::pair<AVULONG, AVULONG>, CLiftSrv*> mapLifts;
	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
		for each (CSimSrv *pSim in pGroup->GetSims())
		{
			AVULONG nTrafficScenario = (*pSim)[L"TrafficScenarioId"];
			mapSims[nTrafficScenario] = pSim;
			for (AVULONG iLift = 0; iLift < pGroup->GetLiftCount(); iLift++)
			{
				AVULONG nLiftId = pGroup->GetLift(iLift)->GetShaft()->GetNativeId();
				CLiftSrv *pLift = (CLiftSrv*)pSim->CreateLift(iLift);
				pSim->AddLift((CLift*)pLift);
				mapLifts[std::make_pair(nTrafficScenario, nLiftId)] = pLift;
			}
		}

	bool bWarning = false;

	// Load Passengers - from HallCalls
	AVULONG iPassengerId = 0;
	dbtools::CDataBase::SELECT sel;
	sel = dbReports.select(L"SELECT * FROM HallCalls WHERE Iteration=0 AND SimulationId = %d ORDER BY StartLoading", GetSimulationId());
	for ( ; sel; sel++)
	{
		// identify the sim & lift from the maps
		AVULONG nTrafficScenario = sel[L"TraffiicScenarioId"];
		AVULONG nLiftId = sel[L"LiftId"];
		CSimSrv *pSim = mapSims[nTrafficScenario];
		CLiftSrv *pLift = mapLifts[std::make_pair(nTrafficScenario, nLiftId)];
		ASSERT(pSim != NULL && pLift != NULL);

		CPassengerSrv *pPassenger = (CPassengerSrv*)pSim->CreatePassenger(iPassengerId++);
		if (pPassenger->Load(sel) != S_OK) bWarning = true;
		pPassenger->SetLiftId(pLift->GetId());		// overwrite original values (which were native DB id's)
		pPassenger->SetShaftId(pLift->GetId());
		pSim->AddPassenger(pPassenger);
		pLift->AddPassenger(pPassenger);	// passengers are added to the lifts only to speed-up the next process below
	}

	// feed lifts with LiftStops
	sel = dbReports.select(L"SELECT * FROM LiftStops WHERE Iteration=0 AND SimulationId = %d ORDER BY Time", this->GetSimulationId());
	for( ; sel; sel++)
	{
		AVULONG nTrafficScenario = sel[L"TrafficScenarioId"];
		AVULONG nLiftId = sel[L"LiftId"];
		CLiftSrv *pLift = mapLifts[std::make_pair(nTrafficScenario, nLiftId)];
		ASSERT(pLift != NULL);
		pLift->AddStop(sel[L"Floor"], sel[L"Time"].msec(), sel[L"Duration"].msec());
	}

	// Load Lifts (Generate Journeys)
	// Each lift received a list of passengers and stops in the previous steps
	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
		for each (CSimSrv *pSim in pGroup->GetSims())
			for each (CLiftSrv *pLift in pSim->GetLifts())
				pLift->Load();

#ifdef _DEBUG
	// optional - compare with the legacy version - FOR DEBUG ONLY!
	//for each (CLiftGroupSrv *pGroup in GetLiftGroups())
	//	for each (CSimSrv *pSim in pGroup->GetSims())
	//		for each (CLiftSrv *pLift in pSim->GetLifts())
	//		{
	//			CLiftSrv *pLiftFromLogs = (CLiftSrv*)pSim->CreateLift(pLift->GetId());
	//			auto pLIFT = pGroup->GetLift(pLift->GetId());
	//			pLiftFromLogs->legacy_Load(pLIFT, dbReports, pLIFT->GetShaft()->GetNativeId(), (*pSim)[L"TrafficScenarioId"], 0);
	//			pLiftFromLogs->legacy_ReportDifferences(pLift);
	//			delete pLiftFromLogs;
	//		}
#endif
	
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

void CProjectSrv::Play()
{ 
	for each (CLiftGroupSrv *pGroup in GetLiftGroups()) 
		for each (CSimSrv *pSim in pGroup->GetSims())
			pSim->Play();
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
			LogProgress();
		}

	CDataBase::UPDATE upd = db.update(L"AVProjects", L"WHERE ID=%d", GetId());
	upd[L"MinSimulationTime"] = GetMinSimulationTime();
	upd[L"MaxSimulationTime"] = GetMaxSimulationTime();
	upd[L"TimeSaved"] = GetMaxSimulationTime();
	upd[L"SavedAll"] = true;
	upd.execute();

	return S_OK;
}

HRESULT CProjectSrv::PlayAndUpdate(dbtools::CDataBase dbVis)
{
	for each (CLiftGroupSrv *pGroup in GetLiftGroups())
		for each (CSimSrv *pSim in pGroup->GetSims())
		{
			HRESULT h;

			pSim->Play();

			h = pSim->Update(dbVis);
			if FAILED(h) return h;
			LogProgress();
		}

	CDataBase::UPDATE upd = dbVis.update(L"AVProjects", L"WHERE ID=%d", GetId());
	upd[L"MinSimulationTime"] = GetMinSimulationTime();
	upd[L"MaxSimulationTime"] = GetMaxSimulationTime();
	upd[L"TimeSaved"] = GetMaxSimulationTime();
	upd[L"SavedAll"] = true;
	upd.execute();
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

HRESULT CProjectSrv::CleanUpSim(CDataBase db, ULONG nProjectId)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DELETE FROM AVPassengers WHERE SimID IN (SELECT S.ID FROM AVSims S, AVLiftGroups G WHERE S.LiftGroupId = G.ID AND G.ProjectId = %d)", nProjectId);
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U')   IS NOT NULL DELETE FROM AVJourneys   WHERE SimID IN (SELECT S.ID FROM AVSims S, AVLiftGroups G WHERE S.LiftGroupId = G.ID AND G.ProjectId = %d)", nProjectId);
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

HRESULT CProjectSrv::CreateTicket(dbtools::CDataBase db, AVSTRING strUserId, AVSTRING strBuf)
{
	static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	srand(time(NULL));
	for (int i = 0; i < 27; i++)
		strBuf[i] = base64_chars[rand() % 64];
	strBuf[27] = '=';
	strBuf[28] = '\0';
	
	CDataBase::INSERT ins = db.insert(L"AVTickets");

	ins[L"UserId"] = strUserId;
	ins[L"Ticket"] = strBuf;
	ins[L"TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins[L"TimeStamp"].act_as_symbol();

	ins.execute();

	return S_OK;
}

