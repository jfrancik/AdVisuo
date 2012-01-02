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
		if ((loader.pLifts[i].nDecks == 1 && GetBuilding()->GetShaft(i)->TypeOfLift == CBuildingBase::LIFT_DOUBLE_DECK)
		|| (loader.pLifts[i].nDecks > 1 && GetBuilding()->GetShaft(i)->TypeOfLift == CBuildingBase::LIFT_SINGLE_DECK))
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
	sel = db.select("SELECT MAX(ID) AS id FROM AVProjects WHERE SimulationID=%d", nSimulationID);
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
	sel = db.select("SELECT * FROM Simulations WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	AVULONG nProjectId = sel[L"ProjectId"];


	// Query for Projects (project general information)
	sel = db.select("SELECT * FROM Projects WHERE ProjectId=%d", nProjectId);
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
	sel = db.select("SELECT * FROM FilePaths WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_PROJECT;
	m_strSIMFileName = sel[L"SimPath"];
	m_strIFCFileName = sel[L"VisPath"];

	// Query for AnalysisTypeDataSets (SIM file path)
	sel = db.select("SELECT * FROM AnalysisTypeDataSets WHERE SimulationId=%d", nSimulationID);
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
	sel = db.select("SELECT * FROM AVProjects WHERE ID=%d", m_nProjectID);
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

	CDataBase::INSERT ins = db.insert("AVProjects");
	ins["SimulationID"] = nSimulationID;
	ins["SIMVersionID"] = m_nSIMVersionID;
	ins["AVVersionID"] = (float)(((AVFLOAT)CSim::GetVersion())/100.0);
	ins["Floors"] = m_nBldFloors;
	ins["Shafts"] = m_nBldShafts;
	ins["Lifts"] = m_nBldLifts;
	ins["Passengers"] = (ULONG)0;
	ins["SimulationTime"] = (ULONG)0;
	ins["JourneysSaved"] = (ULONG)0;
	ins["PassengersSaved"] = (ULONG)0;
	ins["TimeSaved"] = (ULONG)0;
	ins["SavedAll"] = (ULONG)0;
	ins["SIMFileName"] = m_strSIMFileName;
	ins["IFCFileName"] = m_strIFCFileName;
	ins["ProjectName"] = m_strProjectName;
	ins["Language"] = m_strLanguage;
	ins["MeasurementUnits"] = m_strMeasurementUnits;
	ins["BuildingName"] = m_strBuildingName;
	ins["ClientCompany"] = m_strClientCompanyName;
	ins["City"] = m_strCity;
	ins["LBRegionDistrict"] = m_strLBRegionDistrict;
	ins["StateCounty"] = m_strStateCounty;
	ins["LiftDesigner"] = m_strLiftDesigner;
	ins["Country"] = m_strCountry;
	ins["CheckedBy"] = m_strCheckedBy;
	ins["PostalZipCode"] = m_strPostalZipCode;
	ins["Algorithm"] = (ULONG)m_nAlgorithm;
	ins["TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins["TimeStamp"].act_as_symbol();
	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select("SELECT SCOPE_IDENTITY()");
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

	CDataBase::UPDATE upd = db.update("AVProjects", "WHERE ID=%d", m_nProjectID);
	upd["SIMVersionID"] = m_nSIMVersionID;
	upd["Floors"] = m_nBldFloors;
	upd["Shafts"] = m_nBldShafts;
	upd["Lifts"] = m_nBldLifts;
	upd["Passengers"] = GetPassengerCount();
	upd["SimulationTime"] = m_nSimulationTime;
	upd["JourneysSaved"] = m_nJourneysSaved;
	upd["PassengersSaved"] = m_nPassengersSaved;
	upd["TimeSaved"] = m_nTimeSaved;
	upd["SavedAll"] = m_bSavedAll;
	upd.execute();

	return S_OK;
}

HRESULT CSim::CleanUp(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	db.execute("DELETE FROM AVPassengers WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute("DELETE FROM AVJourneys WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute("DELETE FROM AVFloors WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationID=%d)", nSimulationID);
	db.execute("DELETE FROM AVShafts WHERE BuildingID IN (SELECT B.ID FROM AVBuildings B, AVProjects P WHERE B.ProjectID = P.ID AND P.SimulationID=%d)", nSimulationID);
	db.execute("DELETE FROM AVBuildings WHERE ProjectID IN (SELECT ID FROM AVProjects WHERE SimulationID=%d)", nSimulationID);
	db.execute("DELETE FROM AVProjects WHERE SimulationID=%d", nSimulationID);
	return S_OK;
}

HRESULT CSim::CleanUpAll(CDataBase db)
{
	if (!db) throw db;
	db.execute("DELETE FROM AVPassengers");
	db.execute("DELETE FROM AVJourneys");
	db.execute("DELETE FROM AVFloors");
	db.execute("DELETE FROM AVShafts");
	db.execute("DELETE FROM AVProjects");
	db.execute("DELETE FROM AVBuildings");
	return S_OK;
}
