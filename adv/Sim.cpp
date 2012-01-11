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
	int nRes = loader.Load(m_strSIMFileName.c_str());


	// detect errors...
	if FAILED(nRes)
		return Logf(nRes, m_strSIMFileName.c_str());
	if ((ULONG)loader.nLifts != GetBuilding()->GetLiftCount())
		return Log(ERROR_FILE_INCONSISTENT_LIFTS);		// inconsistent number of floors
	if ((ULONG)loader.nFloors != GetBuilding()->GetStoreyCount())
		return Log(ERROR_FILE_INCONSISTENT_FLOORS);		// inconsistent number of lifts

	// check single/double decker consistency
	for (AVULONG i = 0; i < (ULONG)loader.nLifts;i++)
		if ((loader.pLifts[i].nDecks == 1 && GetBuilding()->GetShaft(i)->GetType() == CBuildingBase::LIFT_DOUBLE_DECK)
		|| (loader.pLifts[i].nDecks > 1 && GetBuilding()->GetShaft(i)->GetType() == CBuildingBase::LIFT_SINGLE_DECK))
			return Log(ERROR_FILE_INCONSISTENT_DECKS);

	m_nSIMVersionID = loader.nVersion;

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
		m_nSimulationTime = max(m_nSimulationTime, GetPassenger(i)->GetUnloadTime());
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
	sel = db.select(L"SELECT * FROM Projects WHERE ProjectId=%d", nProjectId);
	if (!sel) throw ERROR_PROJECT;
	m_strProjectName = sel[L"ProjectName"];
	m_strLanguage = sel[L"Languaje"];
	m_strMeasurementUnits = sel[L"MeasurementUnits"];
	m_strBuildingName = sel[L"BuildingName"];
	m_strClientCompanyName = sel[L"ClientCompanyName"];
	m_strCity = sel[L"City"];
	m_strLBRegionDistrict = sel[L"LBRegionDistrict"];
	m_strStateCounty = sel[L"StateCounty"];
	m_strLiftDesigner = sel[L"LiftDesigner"];
	m_strCountry = sel[L"Country"];
	m_strCheckedBy = sel[L"CheckedBy"];
	m_strPostalZipCode = sel[L"PostalZipCode"];


	// Query for Files (SIM file path)
	sel = db.select(L"SELECT * FROM FilePaths WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	m_strSIMFileName = sel[L"SimPath"];
	m_strIFCFileName = sel[L"VisPath"];

	// Query for AnalysisTypeDataSets (SIM file path)
	sel = db.select(L"SELECT * FROM AnalysisTypeDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	std::wstring strAlgorithm = sel[L"Algorithm"];

	if (strAlgorithm.compare(0, 19, L"Destination Control") == 0)
		m_nAlgorithm = DESTINATION;
	else if (strAlgorithm.compare(0, 16, L"Group Collective") == 0)
		m_nAlgorithm = COLLECTIVE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"dispatcher algorithm", strAlgorithm.c_str()), ERROR_GENERIC);

	return S_OK;
}

HRESULT CSim::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;
	m_nProjectID = nProjectID;
	CDataBase::SELECT sel;
		
	// Query for Project Data (for project id)
	sel = db.select(L"SELECT * FROM AVProjects WHERE ID=%d", m_nProjectID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	m_strSIMFileName = sel[L"SIMFileName"];
	m_strIFCFileName = sel[L"IFCFileName"];
	m_dateTimeStamp = sel[L"TimeStamp"];

	return S_OK;
}

HRESULT CSim::Store(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	if (!GetBuilding())
		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);
	
	// Update building data
	if (GetBuilding())
	{
		m_nBldFloors = GetBuilding()->GetStoreyCount();
		m_nBldShafts = GetBuilding()->GetShaftCount();
		m_nBldLifts = GetBuilding()->GetLiftCount();
	}

	CDataBase::INSERT ins = db.insert(L"AVProjects");
	ins[L"SimulationID"] = nSimulationID;
	ins[L"SIMVersionID"] = m_nSIMVersionID;
	ins[L"AVVersionID"] = (float)(((AVFLOAT)CSim::GetVersion())/100.0);
	ins[L"Floors"] = m_nBldFloors;
	ins[L"Shafts"] = m_nBldShafts;
	ins[L"Lifts"] = m_nBldLifts;
	ins[L"Passengers"] = (ULONG)0;
	ins[L"SimulationTime"] = (ULONG)0;
	ins[L"JourneysSaved"] = (ULONG)0;
	ins[L"PassengersSaved"] = (ULONG)0;
	ins[L"TimeSaved"] = (ULONG)0;
	ins[L"SavedAll"] = (ULONG)0;
	ins[L"SIMFileName"] = m_strSIMFileName;
	ins[L"IFCFileName"] = m_strIFCFileName;
	ins[L"ProjectName"] = m_strProjectName;
	ins[L"Language"] = m_strLanguage;
	ins[L"MeasurementUnits"] = m_strMeasurementUnits;
	ins[L"BuildingName"] = m_strBuildingName;
	ins[L"ClientCompany"] = m_strClientCompanyName;
	ins[L"City"] = m_strCity;
	ins[L"LBRegionDistrict"] = m_strLBRegionDistrict;
	ins[L"StateCounty"] = m_strStateCounty;
	ins[L"LiftDesigner"] = m_strLiftDesigner;
	ins[L"Country"] = m_strCountry;
	ins[L"CheckedBy"] = m_strCheckedBy;
	ins[L"PostalZipCode"] = m_strPostalZipCode;
	ins[L"Algorithm"] = (ULONG)m_nAlgorithm;
	ins[L"TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins[L"TimeStamp"].act_as_symbol();
	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	m_nProjectID = sel[(short)0];

	// Store the Building
	if (GetBuilding())
		GetBuilding()->Store(db, m_nProjectID);

	Update(db, 0);

	return S_OK;
}

HRESULT CSim::Update(CDataBase db, AVLONG nTime)
{
	if (!db) throw db;
	if (nTime < 0) nTime = m_nSimulationTime;
	
	if (m_nProjectID == 0)
		throw (Log(ERROR_INTERNAL, L"Project update run with ID=0"), ERROR_GENERIC);

	// Store the Journeys
	AVULONG nLifts = GetLiftCount();
	for (AVULONG i = 0; i < nLifts; i++)
		GetLift(i)->Store(db, m_nProjectID);

	// Store the Passengers
	AVULONG nPassengers = GetPassengerCount();
	for (AVULONG i = 0; i < nPassengers; i++)
		GetPassenger(i)->Store(db, m_nProjectID);

	m_bSavedAll = true;
	m_nJourneysSaved = GetJourneyTotalCount();
	m_nPassengersSaved = GetPassengerCount();
	m_nTimeSaved = m_nSimulationTime;

	// Update building data
	if (GetBuilding())
	{
		m_nBldFloors = GetBuilding()->GetStoreyCount();
		m_nBldShafts = GetBuilding()->GetShaftCount();
		m_nBldLifts = GetBuilding()->GetLiftCount();
	}

	CDataBase::UPDATE upd = db.update(L"AVProjects", L"WHERE ID=%d", m_nProjectID);
	upd[L"SIMVersionID"] = m_nSIMVersionID;
	upd[L"Floors"] = m_nBldFloors;
	upd[L"Shafts"] = m_nBldShafts;
	upd[L"Lifts"] = m_nBldLifts;
	upd[L"Passengers"] = GetPassengerCount();
	upd[L"SimulationTime"] = m_nSimulationTime;
	upd[L"JourneysSaved"] = m_nJourneysSaved;
	upd[L"PassengersSaved"] = m_nPassengersSaved;
	upd[L"TimeSaved"] = m_nTimeSaved;
	upd[L"SavedAll"] = m_bSavedAll;
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
