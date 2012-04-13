// Sim.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Sim.h"
#include "../CommonFiles/DBTools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CSim::CSim(CBuildingBase *pBuilding) : CSimBase(pBuilding)
{
}

HRESULT CSim::LoadSim()
{
	if (!GetBuilding())
		return Log(ERROR_INTERNAL, L"SIM file loading without the building set.");

	// load!
	CSimLoader loader;
//	int nRes = loader.Load(GetSIMFileName().c_str());
	int nRes = loader.Load(L"c:\\Users\\Jarek\\Desktop\\testCirc18lift_251Floors_ver109.sim");


	// detect errors...
	AVULONG nnnn = GetBuilding()->GetLiftCount();
	if FAILED(nRes)
		return Logf(nRes, GetSIMFileName().c_str());
	if ((ULONG)loader.nLifts != GetBuilding()->GetLiftCount())
		return Log(ERROR_FILE_INCONSISTENT_LIFTS);		// inconsistent number of floors
	if ((ULONG)loader.nFloors != GetBuilding()->GetStoreyCount())
		return Log(ERROR_FILE_INCONSISTENT_FLOORS);		// inconsistent number of lifts

	// check single/double decker consistency
	for (AVULONG i = 0; i < (ULONG)loader.nLifts;i++)
		if ((loader.pLifts[i].nDecks == 1 && GetBuilding()->GetLift(i)->GetShaft()->GetDeck() == CBuildingBase::DECK_DOUBLE)
		|| (loader.pLifts[i].nDecks > 1 && GetBuilding()->GetLift(i)->GetShaft()->GetDeck() == CBuildingBase::DECK_SINGLE))
			return Log(ERROR_FILE_INCONSISTENT_DECKS);

	SetSIMVersionId(loader.nVersion);

	bool bWarning = false;

	// load passenger (hall calls) data
	for (AVULONG i = 0; i < (ULONG)loader.nPassengers; i++)
	{
		CPassenger *pPassenger = (CPassenger*)CreatePassenger(i);
		if (pPassenger->Load(i, loader.pPassengers[i]) != S_OK)
			bWarning = true;
		AddPassenger(pPassenger);
	}

	// load, analyse and consolidate simulation data
	for (AVULONG i = 0; i < (ULONG)loader.nLifts; i++)
	{
		CLift *pLift = (CLift*)CreateLift(i);
		HRESULT h = pLift->Load(loader, i, true, true);
		if FAILED(h) return h;
		if (h != S_OK) bWarning = true;
		AddLift(pLift);
	}

	return bWarning ? WARNING_GENERIC : S_OK;
}

void CSim::Play()
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
	{
		GetPassenger(i)->Play();
		ReportSimulationTime(GetPassenger(i)->GetUnloadTime());
	}
}

HRESULT CSim::FindProjectID(CDataBase db, ULONG nSimulationId, ULONG &nProjectID)
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

HRESULT CSim::LoadFromConsole(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Simulations (for project id)
	sel = db.select(L"SELECT * FROM Simulations s, Projects p WHERE p.ProjectId = s.ProjectId AND SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_PROJECT;
	AVULONG nProjectId = sel[L"ProjectId"];
	sel >> ME;

	//// Query for Projects (project general information)
	//sel = db.select(L"SELECT ProjectName, Languaje AS Language, MeasurementUnits, BuildingName, ClientCompanyName AS ClientCompany, City, LBRegionDistrict, StateCounty, LiftDesigner, Country, CheckedBy, PostalZipCode FROM Projects WHERE ProjectId=%d", nProjectId);
	//if (!sel) throw ERROR_PROJECT;
	//sel >> ME;

	//// Query for Files (SIM file path)
	//sel = db.select(L"SELECT SimPath AS SIMFileName, VisPath AS IFCFileName FROM FilePaths WHERE SimulationId=%d", nSimulationId);
	//if (!sel) throw ERROR_PROJECT;
	//sel >> ME;

	//// Query for AnalysisTypeDataSets (SIM file path)
	//sel = db.select(L"SELECT Algorithm FROM AnalysisTypeDataSets WHERE SimulationId=%d", nSimulationId);
	//if (!sel) throw ERROR_PROJECT;
	//sel >> ME;

	//if      (ME[L"Algorithm"].as_wstring().substr(0, 19) == L"Destination Control")	ME[L"Algorithm"] = (ULONG)DESTINATION;
	//else if (ME[L"Algorithm"].as_wstring().substr(0, 16) == L"Group Collective")		ME[L"Algorithm"] = (ULONG)COLLECTIVE;
	//else throw (Log(ERROR_UNRECOGNISED_STRING, L"dispatcher algorithm", (ME[L"Algorithm"]).as_wstring().c_str()), ERROR_GENERIC);

	return S_OK;
}

HRESULT CSim::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;

	SetProjectId(nProjectID);
		
	// Query for Project Data (for project id)
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT * FROM AVProjects WHERE ID=%d", GetProjectId());
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> ME;

	ResolveMe();

	return S_OK;
}

HRESULT CSim::Store(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	if (!GetBuilding())
		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);

	SetSimulationId(nSimulationId);
	SetAVVersionId(GetAVNativeVersionId());
	
	CDataBase::INSERT ins = db.insert(L"AVProjects");

	ins << ME;
	ins[L"SimulationId"] = GetSimulationId();
	ins[L"SIMVersionId"] = GetSIMVersionId();
	ins[L"AVVersionId"] = (float)(((AVFLOAT)GetAVVersionId())/100.0);
	ins[L"Floors"] = GetBuilding()->GetStoreyCount();
	ins[L"Shafts"] = GetBuilding()->GetShaftCount();
	ins[L"Lifts"] = GetBuilding()->GetLiftCount();
	ins[L"Passengers"] = (ULONG)0;
	ins[L"SimulationTime"] = (ULONG)0;
	ins[L"JourneysSaved"] = (ULONG)0;
	ins[L"PassengersSaved"] = (ULONG)0;
	ins[L"TimeSaved"] = (ULONG)0;
	ins[L"SavedAll"] = (ULONG)0;

	ins[L"TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins[L"TimeStamp"].act_as_symbol();

	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetProjectId(sel[(short)0]);

	// Store the Building
	if (GetBuilding())
		GetBuilding()->Store(db, GetProjectId());

	Update(db, 0);

	return S_OK;
}

HRESULT CSim::Update(CDataBase db, AVLONG nTime)
{
	if (!db) throw db;
	if (!GetBuilding())
		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);
	if (GetProjectId() == 0)
		throw (Log(ERROR_INTERNAL, L"Project update run with ID=0"), ERROR_GENERIC);

	// Store the Journeys
	AVULONG nLifts = GetLiftCount();
	for (AVULONG i = 0; i < nLifts; i++)
		GetLift(i)->Store(db, GetProjectId());

	// Store the Passengers
	AVULONG nPassengers = GetPassengerCount();
	for (AVULONG i = 0; i < nPassengers; i++)
		GetPassenger(i)->Store(db, GetProjectId());

	CDataBase::UPDATE upd = db.update(L"AVProjects", L"WHERE ID=%d", GetProjectId());
	upd[L"SIMVersionId"] = GetSIMVersionId();
	upd[L"Floors"] = GetBuilding()->GetStoreyCount();
	upd[L"Shafts"] = GetBuilding()->GetShaftCount();
	upd[L"Lifts"] = GetBuilding()->GetLiftCount();
	upd[L"Passengers"] = GetPassengerCount();
	upd[L"SimulationTime"] = GetSimulationTime();
	upd[L"JourneysSaved"] = GetJourneyTotalCount();
	upd[L"PassengersSaved"] = GetPassengerCount();
	upd[L"TimeSaved"] = GetSimulationTime();
	upd[L"SavedAll"] = true;
	upd.execute();

	return S_OK;
}

HRESULT CSim::CleanUp(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVPassengers WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVJourneys WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL AND OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DELETE FROM AVFloors WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL AND OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DELETE FROM AVShafts WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL AND OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVBuildings WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationId=%d)", nSimulationId);
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DELETE FROM AVProjects WHERE SimulationId=%d", nSimulationId);
	return S_OK;
}

HRESULT CSim::CleanUpAll(CDataBase db)
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

HRESULT CSim::DropTables(CDataBase db)
{
	if (!db) throw db;
	db.execute(L"IF OBJECT_ID('dbo.AVPassengers','U') IS NOT NULL DROP TABLE AVPassengers");
	db.execute(L"IF OBJECT_ID('dbo.AVJourneys','U') IS NOT NULL DROP TABLE AVJourneys");
	db.execute(L"IF OBJECT_ID('dbo.AVFloors','U') IS NOT NULL DROP TABLE AVFloors");
	db.execute(L"IF OBJECT_ID('dbo.AVShafts','U') IS NOT NULL DROP TABLE AVShafts");
	db.execute(L"IF OBJECT_ID('dbo.AVProjects','U') IS NOT NULL DROP TABLE AVProjects");
	db.execute(L"IF OBJECT_ID('dbo.AVBuildings','U') IS NOT NULL DROP TABLE AVBuildings");
	return S_OK;
}
