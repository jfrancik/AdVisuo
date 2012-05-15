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

void CProject::Load(xmltools::CXmlReader reader)
{
	CSim *pSim = GetSim();
	CBuilding *pBuilding = GetBuilding();
	if (!pBuilding) throw _prj_error(_prj_error::E_PRJ_INTERNAL);

	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			if (m_phase != PHASE_NONE) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_PRJ;

			reader >> ME;
			ResolveMe();
		}
		else
		if (reader.getName() == L"AVSim")
		{
			if (m_phase != PHASE_PRJ) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_SIM;

			reader >> *pSim;
			pSim->ResolveMe();
		}
		else
		if (reader.getName() == L"AVBuilding")
		{
			if (m_phase != PHASE_SIM) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_BLD;

			reader >> *pBuilding;
			pBuilding->Init();
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_STRUCT;
			if (iShaft >= GetBuilding()->GetShaftCount()) throw _prj_error(_prj_error::E_PRJ_LIFTS);
			
			reader >> *pBuilding->GetShaft(iShaft++);
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_STRUCT;
			if (iStorey >= GetBuilding()->GetStoreyCount()) throw _prj_error(_prj_error::E_PRJ_FLOORS);
			
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

			if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_SIMDATA;

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
			if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phase = PHASE_SIMDATA;

			CPassenger *pPassenger = (CPassenger*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();
			pSim->AddPassenger(pPassenger);
		}
	}

	// Init lifts when known
	if (m_phase >= PHASE_STRUCT && pBuilding->GetLiftCount() == 0)
	{
		pBuilding->InitLifts();
		for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
			pSim->AddLift(pSim->CreateLift(i));
	}

	// Some tests
	if (GetId() == 0)
		throw _prj_error(_prj_error::E_PRJ_NOT_FOUND);
	if (m_phase == PHASE_STRUCT && iShaft != GetBuilding()->GetShaftCount())
		throw _prj_error(_prj_error::E_PRJ_LIFTS);
	if (m_phase == PHASE_STRUCT && iStorey != GetBuilding()->GetStoreyCount())
		throw _prj_error(_prj_error::E_PRJ_FLOORS);

	if (m_phase == PHASE_STRUCT) 
		m_phase = PHASE_SIMDATA;

	if (m_phase >= PHASE_STRUCT) 
	{
		GetBuilding()->Create();
		GetBuilding()->Scale(0.04f);
	}
//	if (m_phase == PHASE_SIMDATA)
//	{
//		for_each_passenger([](CPassengerBase *p) { p->ResolveMe(); });
//	}

	if (m_phase >= PHASE_STRUCT && !GetBuilding()->IsValid())
		throw _prj_error(_prj_error::E_PRJ_NO_BUILDING);
}

void CProject::LoadIndex(xmltools::CXmlReader reader, vector<CProject*> &prjs)
{
	while (reader.read())
		if (reader.getName() == L"AVProject")
		{
			CProject *pPrj = new CProject(NULL);
			reader >> *pPrj;
			pPrj->ResolveMe();
			prjs.push_back(pPrj);
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

void CProject::Store(xmltools::CXmlWriter writer)
{
	CSim *pSim = GetSim();
	CBuilding *pBuilding = GetBuilding();
	if (!pBuilding) throw _prj_error(_prj_error::E_PRJ_INTERNAL);

	writer.write(L"AVProject", *this);
	writer.write(L"AVBuilding", *pBuilding);
	
	for (ULONG i = 0; i < pBuilding->GetShaftCount(); i++)
		writer.write(L"AVShaft", *pBuilding->GetShaft(i));

	for (ULONG i = 0; i < pBuilding->GetStoreyCount(); i++)
		writer.write(L"AVFloor", *pBuilding->GetStorey(i));

	for (ULONG i = 0; i < pSim->GetLiftCount(); i++)
		for (ULONG j = 0; j < pSim->GetLift(i)->GetJourneyCount(); j++)
		{
			JOURNEY *pJ = pSim->GetLift(i)->GetJourney(j);
			writer[L"LiftID"] = pSim->GetLift(i)->GetId();
			writer[L"ShaftFrom"] = pJ->m_shaftFrom;
			writer[L"ShaftTo"] = pJ->m_shaftTo;
			writer[L"FloorFrom"] = pJ->m_floorFrom;
			writer[L"FloorTo"] = pJ->m_floorTo;
			writer[L"TimeGo"] = pJ->m_timeGo;
			writer[L"TimeDest"] = pJ->m_timeDest;
			writer[L"DC"] = pJ->StringifyDoorCycles();
			writer.write(L"AVJourney");
		}

	for (ULONG i = 0; i < pSim->GetPassengerCount(); i++)
		writer.write(L"AVPassenger", *pSim->GetPassenger(i));
}
