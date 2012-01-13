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
	int nRes = loader.Load(GetSIMFileName().c_str());


	// detect errors...
	if FAILED(nRes)
		return Logf(nRes, GetSIMFileName().c_str());
	if ((ULONG)loader.nLifts != GetBuilding()->GetLiftCount())
		return Log(ERROR_FILE_INCONSISTENT_LIFTS);		// inconsistent number of floors
	if ((ULONG)loader.nFloors != GetBuilding()->GetStoreyCount())
		return Log(ERROR_FILE_INCONSISTENT_FLOORS);		// inconsistent number of lifts

	// check single/double decker consistency
	for (AVULONG i = 0; i < (ULONG)loader.nLifts;i++)
		if ((loader.pLifts[i].nDecks == 1 && GetBuilding()->GetShaft(i)->GetType() == CBuildingBase::LIFT_DOUBLE_DECK)
		|| (loader.pLifts[i].nDecks > 1 && GetBuilding()->GetShaft(i)->GetType() == CBuildingBase::LIFT_SINGLE_DECK))
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

HRESULT CSim::FindProjectID(CDataBase db, ULONG nSimulationID, ULONG &nProjectID)
{
	if (!db) throw db;
	nProjectID = 0;
	CDataBase::SELECT sel;
		
	// Query for Project Data (test for any existing)
	sel = db.select(L"SELECT MAX(ID) AS id FROM AVProjects WHERE SimulationID=%d", nSimulationID);
	if (!sel) return S_FALSE;

	if (sel[L"id"].isNull()) return S_FALSE;
	nProjectID = sel[L"id"];
	return S_OK;
}

HRESULT CSim::LoadFromConsole(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Simulations (for project id)
	sel = db.select(L"SELECT * FROM Simulations WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	AVULONG nProjectId = sel[L"ProjectId"];

	// Query for Projects (project general information)
	sel = db.select(L"SELECT ProjectName, Languaje AS Language, MeasurementUnits, BuildingName, ClientCompanyName AS ClientCompany, City, LBRegionDistrict, StateCounty, LiftDesigner, Country, CheckedBy, PostalZipCode FROM Projects WHERE ProjectId=%d", nProjectId);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

	// Query for Files (SIM file path)
	sel = db.select(L"SELECT SimPath AS SIMFileName, VisPath AS IFCFileName FROM FilePaths WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

	// Query for AnalysisTypeDataSets (SIM file path)
	sel = db.select(L"SELECT Algorithm FROM AnalysisTypeDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	sel >> ME;

	if      (ME[L"Algorithm"].as_wstring().substr(0, 19) == L"Destination Control")	ME[L"Algorithm"] = (ULONG)DESTINATION;
	else if (ME[L"Algorithm"].as_wstring().substr(0, 16) == L"Group Collective")		ME[L"Algorithm"] = (ULONG)COLLECTIVE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"dispatcher algorithm", (ME[L"Algorithm"]).as_wstring().c_str()), ERROR_GENERIC);

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

HRESULT CSim::Store(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	if (!GetBuilding())
		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);

	SetSimulationId(nSimulationID);
	SetAVVersionId(GetAVNativeVersionId());
	
	CDataBase::INSERT ins = db.insert(L"AVProjects");

	ins << ME;
	ins[L"SimulationID"] = GetSimulationId();
	ins[L"SIMVersionID"] = GetSIMVersionId();
	ins[L"AVVersionID"] = (float)(((AVFLOAT)GetAVVersionId())/100.0);
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
	upd[L"SIMVersionID"] = GetSIMVersionId();
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

HRESULT CSim::CleanUp(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	db.execute(L"DELETE FROM AVPassengers WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute(L"DELETE FROM AVJourneys WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute(L"DELETE FROM AVFloors WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationID=%d)", nSimulationID);
	db.execute(L"DELETE FROM AVShafts WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationID=%d)", nSimulationID);
	db.execute(L"DELETE FROM AVBuildings WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute(L"DELETE FROM AVProjects WHERE SimulationID=%d", nSimulationID);
	return S_OK;
}

HRESULT CSim::CleanUpAll(CDataBase db)
{
	if (!db) throw db;
	db.execute(L"DELETE FROM AVPassengers");
	db.execute(L"DELETE FROM AVJourneys");
	db.execute(L"DELETE FROM AVFloors");
	db.execute(L"DELETE FROM AVShafts");
	db.execute(L"DELETE FROM AVProjects");
	db.execute(L"DELETE FROM AVBuildings");
	return S_OK;
}
