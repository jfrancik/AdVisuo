// Xml.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Building.h"
#include "Project.h"
#include "Sim.h"
#include "Lift.h"
#include "Passenger.h"
#include <list>

#include "../CommonFiles/XMLTools.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// Load from XML

void CProject::Load(xmltools::CXmlReader reader, AVULONG nLiftGroup)
{
	CSim *pSim = GetSim(nLiftGroup);
	CBuilding *pBuilding = GetBuilding(nLiftGroup);
	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			if (m_phases[nLiftGroup] != PHASE_NONE) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_PRJ;

			reader >> ME;
			ResolveMe();
		}
		else
		if (reader.getName() == L"AVSim")
		{
			if (m_phases[nLiftGroup] != PHASE_PRJ && m_phases[nLiftGroup] != PHASE_SIM) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] == PHASE_SIM)
			{
				nLiftGroup = Append();
				pSim = GetSim(nLiftGroup);
				pBuilding = GetBuilding(nLiftGroup);
			}
			
			m_phases[nLiftGroup] = PHASE_SIM;

			reader >> *pSim;
			pSim->ResolveMe();
		}
		else
		if (reader.getName() == L"AVBuilding")
		{
			if (m_phases[nLiftGroup] != PHASE_SIM) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_BLD;

			reader >> *pBuilding;
			pBuilding->Init();
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			if (m_phases[nLiftGroup] != PHASE_BLD && m_phases[nLiftGroup] != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_STRUCT;
			if (iShaft >= pBuilding->GetShaftCount()) throw _prj_error(_prj_error::E_PRJ_LIFTS);
			
			reader >> *pBuilding->GetShaft(iShaft++);
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			if (m_phases[nLiftGroup] != PHASE_BLD && m_phases[nLiftGroup] != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_STRUCT;
			if (iStorey >= pBuilding->GetStoreyCount()) throw _prj_error(_prj_error::E_PRJ_FLOORS);
			
			reader >> *pBuilding->GetStorey(iStorey++);
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			if (pBuilding->GetLiftCount() == 0)
			{
				pBuilding->InitLifts();
				for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
					pSim->AddLift(pSim->CreateLift(i));
			}

			if (m_phases[nLiftGroup] != PHASE_STRUCT && m_phases[nLiftGroup] != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_SIMDATA;

			JOURNEY journey;
			AVULONG nLiftID = reader[L"LiftID"];
			journey.m_shaftFrom = reader[L"ShaftFrom"];
			journey.m_shaftTo = reader[L"ShaftTo"];
			journey.m_floorFrom = reader[L"FloorFrom"];
			journey.m_floorTo = reader[L"FloorTo"];
			journey.m_timeGo = reader[L"TimeGo"];
			journey.m_timeDest = reader[L"TimeDest"];
			journey.ParseDoorCycles(reader[L"DC"]);
				  
			if (nLiftID >= pBuilding->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pBuilding->GetShaftCount() || journey.m_shaftTo >= pBuilding->GetShaftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);

			pSim->GetLift(nLiftID)->AddJourney(journey);
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			if (m_phases[nLiftGroup] != PHASE_STRUCT && m_phases[nLiftGroup] != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_SIMDATA;

			CPassenger *pPassenger = (CPassenger*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();
			pSim->AddPassenger(pPassenger);
		}
	}

	// Init lifts when known
	if (m_phases[nLiftGroup] >= PHASE_STRUCT && pBuilding->GetLiftCount() == 0)
	{
		pBuilding->InitLifts();
		for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
			pSim->AddLift(pSim->CreateLift(i));
	}

	// Some tests
	if (GetId() == 0)
		throw _prj_error(_prj_error::E_PRJ_NOT_FOUND);
	if (m_phases[nLiftGroup] == PHASE_STRUCT && iShaft != pBuilding->GetShaftCount())
		throw _prj_error(_prj_error::E_PRJ_LIFTS);
	if (m_phases[nLiftGroup] == PHASE_STRUCT && iStorey != pBuilding->GetStoreyCount())
		throw _prj_error(_prj_error::E_PRJ_FLOORS);

	if (m_phases[nLiftGroup] == PHASE_STRUCT) 
		m_phases[nLiftGroup] = PHASE_SIMDATA;

	if (m_phases[nLiftGroup] == PHASE_SIMDATA && !pBuilding->IsValid()) 
	{
		pBuilding->Create();
		pBuilding->Scale(0.04f);
	}
	if (m_phases[nLiftGroup] >= PHASE_STRUCT && !pBuilding->IsValid())
		throw _prj_error(_prj_error::E_PRJ_NO_BUILDING);
}

void CProject::LoadIndex(xmltools::CXmlReader reader, vector<CProject*> &prjs)
{
	while (reader.read())
		if (reader.getName() == L"AVProject")
		{
			CProject *pPrj = new CProject();
			reader >> *pPrj;
			pPrj->ResolveMe();
			prjs.push_back(pPrj);
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

void CProject::Store(xmltools::CXmlWriter writer)
{
	writer.write(L"AVProject", *this);
	writer.write(L"AVBuilding", *GetBuilding());
	
	for (ULONG i = 0; i < GetBuilding()->GetShaftCount(); i++)
		writer.write(L"AVShaft", *GetBuilding()->GetShaft(i));

	for (ULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
		writer.write(L"AVFloor", *GetBuilding()->GetStorey(i));

	for (ULONG i = 0; i < GetSim()->GetLiftCount(); i++)
		for (ULONG j = 0; j < GetSim()->GetLift(i)->GetJourneyCount(); j++)
		{
			JOURNEY *pJ = GetSim()->GetLift(i)->GetJourney(j);
			writer[L"LiftID"] = GetSim()->GetLift(i)->GetId();
			writer[L"ShaftFrom"] = pJ->m_shaftFrom;
			writer[L"ShaftTo"] = pJ->m_shaftTo;
			writer[L"FloorFrom"] = pJ->m_floorFrom;
			writer[L"FloorTo"] = pJ->m_floorTo;
			writer[L"TimeGo"] = pJ->m_timeGo;
			writer[L"TimeDest"] = pJ->m_timeDest;
			writer[L"DC"] = pJ->StringifyDoorCycles();
			writer.write(L"AVJourney");
		}

	for (ULONG i = 0; i < GetSim()->GetPassengerCount(); i++)
		writer.write(L"AVPassenger", *GetSim()->GetPassenger(i));
}
