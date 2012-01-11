// Xml.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Building.h"
#include "Sim.h"
#include "Lift.h"
#include "Passenger.h"
#include <list>

#include "../CommonFiles/XMLTools.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// Load from XML

// in Ruby it would be a class extension
void Journey_Parse(JOURNEY &j, xmltools::CXmlReader reader, LPCWSTR pTagName, AVULONG &nLiftID);

void CSim::Load(xmltools::CXmlReader reader)
{
	CBuilding *pBuilding = GetBuilding();
	CBuilding::SHAFT *pShaft = NULL;
	CBuilding::STOREY *pStorey = NULL;
	if (!pBuilding) throw _sim_error(_sim_error::E_SIM_INTERNAL);

	AVULONG iLift = 0, iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			if (m_phase != PHASE_NONE) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_PRJ;

			reader >> ME;
			dupaSetupVars();
		}
		else
		if (reader.getName() == L"AVBuilding")
		{
			if (m_phase != PHASE_PRJ) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_BLD;

			pBuilding->CreateShafts(reader[L"NoOfShafts"]);
			pBuilding->CreateStoreys((ULONG)reader[L"FloorsAboveGround"] + (ULONG)reader[L"FloorsBelowGround"], (ULONG)reader[L"FloorsBelowGround"]);

			reader >> *pBuilding;
			pBuilding->dupaSetupVars();
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_STRUCT;

			if (iShaft >= GetBuilding()->GetShaftCount()) throw _sim_error(_sim_error::E_SIM_LIFTS);
			CBuilding::SHAFT *pPrevShaft = pShaft;
			pShaft = pBuilding->GetShaft(iShaft);

			reader >> *pShaft;
			pShaft->dupaSetupVars();
			
			iShaft++;

			for (AVULONG i = 0; i < pShaft->GetLiftCount(); i++)
			{
				CLiftBase *pLift = CreateLift(iLift);
				AddLift(pLift);
				iLift++;
			}
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_STRUCT;

			if (iStorey >= GetBuilding()->GetStoreyCount()) throw _sim_error(_sim_error::E_SIM_FLOORS);
			CBuilding::STOREY *pPrevStorey = pStorey;
			pStorey = pBuilding->GetStorey(iStorey);

			reader >> *pStorey;
			pStorey->dupaSetupVars();

			iStorey++;
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIM) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_SIM;

			JOURNEY journey;
			AVULONG nLiftID;
			Journey_Parse(journey, reader, L"AVJourney", nLiftID);
				  
			if (nLiftID >= pBuilding->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pBuilding->GetShaftCount() || journey.m_shaftTo >= pBuilding->GetShaftCount()) 
				throw _sim_error(_sim_error::E_SIM_LIFTS);

			GetLift(nLiftID)->AddJourney(journey);
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIM) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_SIM;

			CPassenger *pPassenger = (CPassenger*)CreatePassenger(0);

			reader >> *pPassenger;
			pPassenger->dupaSetupVars();

			AddPassenger(pPassenger);
		}
	}

	// Some tests
	if (m_nProjectID == 0)
		throw _sim_error(_sim_error::E_SIM_NOT_FOUND);
	if (m_phase == PHASE_STRUCT && iShaft != GetBuilding()->GetShaftCount())
		throw _sim_error(_sim_error::E_SIM_LIFTS);
	if (m_phase == PHASE_STRUCT && iStorey != GetBuilding()->GetStoreyCount())
		throw _sim_error(_sim_error::E_SIM_FLOORS);

	if (m_phase >= PHASE_STRUCT) 
		GetBuilding()->Resolve();

	if (m_phase >= PHASE_STRUCT && !GetBuilding()->IsValid())
		throw _sim_error(_sim_error::E_SIM_NO_BUILDING);
}

void CSim::LoadIndex(xmltools::CXmlReader reader, vector<CSim*> &sims)
{
	while (reader.read())
		if (reader.getName() == L"AVProject")
		{
			CSim *pSim = new CSim(NULL);
			reader >> *pSim;
			pSim->dupaSetupVars();
			sims.push_back(pSim);
		}
}

void CSim::dupaSetupVars()
{
	m_nProjectID = ME[L"ID"];
	m_nSimulationID = ME[L"SimulationID"];
	m_nSIMVersionID = ME[L"SIMVersionID"];
	m_nAVVersionID = ME[L"AVVersionId"];
	m_nBldFloors = ME[L"Floors"];
	m_nBldShafts = ME[L"Shafts"];
	m_nBldLifts = ME[L"Lifts"];
	m_nPassengers = ME[L"Passengers"];
	m_nSimulationTime = ME[L"SimulationTime"];
	m_nJourneysSaved = ME[L"JourneysSaved"];
	m_nPassengersSaved = ME[L"PassengersSaved"];
	m_nTimeSaved = ME[L"TimeSaved"];
	m_bSavedAll = ME[L"SavedAll"];
	m_strSIMFileName = ME[L"SIMFileName"];
	m_strIFCFileName = ME[L"IFCFileName"];
	m_strProjectName = ME[L"ProjectName"];
	m_strLanguage = ME[L"Language"];
	m_strMeasurementUnits = ME[L"MeasurementUnits"];
	m_strBuildingName = ME[L"BuildingName"];
	m_strClientCompanyName = ME[L"ClientCompany"];
	m_strCity = ME[L"City"];
	m_strLBRegionDistrict = ME[L"LBRegionDistrict"];
	m_strStateCounty = ME[L"StateCounty"];
	m_strLiftDesigner = ME[L"LiftDesigner"];
	m_strCountry = ME[L"Country"];
	m_strCheckedBy = ME[L"CheckedBy"];
	m_strPostalZipCode = ME[L"PostalZipCode"];
	m_nAlgorithm = (CSimBase::ALGORITHM)(ULONG)ME[L"Algorithm"];
}

void Journey_Parse(JOURNEY &j, xmltools::CXmlReader reader, LPCWSTR pTagName, AVULONG &nLiftID)
{
	std::wstring DC;
	nLiftID = reader[L"LiftID"];
	j.m_shaftFrom = reader[L"ShaftFrom"];
	j.m_shaftTo = reader[L"ShaftTo"];
	j.m_floorFrom = reader[L"FloorFrom"];
	j.m_floorTo = reader[L"FloorTo"];
	j.m_timeGo = reader[L"TimeGo"];
	j.m_timeDest = reader[L"TimeDest"];
	j.ParseDoorCycles(reader[L"DC"]);
}

void CPassenger::dupaSetupVars()
{
	SetId(ME[L"PassengerId"]);
	SetShaftId(ME[L"ShaftId"]);
	SetLiftId(ME[L"LiftId"]);
	SetDeck(ME[L"DeckId"]);
	SetArrivalFloor(ME[L"FloorArrival"]);
	SetDestFloor(ME[L"FloorDest"]);
	SetBornTime(ME[L"TimeBorn"]);
	SetArrivalTime(ME[L"TimeArrival"]);
	SetGoTime(ME[L"TimeGo"]);
	SetLoadTime(ME[L"TimeLoad"]);
	SetUnloadTime(ME[L"TimeUnload"]);
	SetWaitSpan(ME[L"SpanWait"]);

	ParseWayPoints(ME[L"WP"]);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

void CSim::Store(xmltools::CXmlWriter writer)
{
	CBuilding *pBuilding = GetBuilding();
	if (!pBuilding) throw _sim_error(_sim_error::E_SIM_INTERNAL);

	writer.write(L"AVProject", *this);
	writer.write(L"AVBuilding", *pBuilding);
	
	for (ULONG i = 0; i < pBuilding->GetShaftCount(); i++)
		writer.write(L"AVShaft", *pBuilding->GetShaft(i));

	for (ULONG i = 0; i < pBuilding->GetStoreyCount(); i++)
		writer.write(L"AVFloor", *pBuilding->GetStorey(i));

	for (ULONG i = 0; i < GetLiftCount(); i++)
		for (ULONG j = 0; j < GetLift(i)->GetJourneyCount(); j++)
		{
			JOURNEY *pJ = GetLift(i)->GetJourney(j);
			writer[L"LiftID"] = GetLift(i)->GetId();
			writer[L"ShaftFrom"] = pJ->m_shaftFrom;
			writer[L"ShaftTo"] = pJ->m_shaftTo;
			writer[L"FloorFrom"] = pJ->m_floorFrom;
			writer[L"FloorTo"] = pJ->m_floorTo;
			writer[L"TimeGo"] = pJ->m_timeGo;
			writer[L"TimeDest"] = pJ->m_timeDest;
			writer[L"DC"] = pJ->StringifyDoorCycles();
			writer.write(L"AVJourney");
		}

	for (ULONG i = 0; i < GetPassengerCount(); i++)
		writer.write(L"AVPassenger", *GetPassenger(i));
}
