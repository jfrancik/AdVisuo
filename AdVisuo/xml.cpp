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

void CSim::Load(xmltools::CXmlReader reader)
{
	CBuilding *pBuilding = GetBuilding();
	if (!pBuilding) throw _sim_error(_sim_error::E_SIM_INTERNAL);

	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			if (m_phase != PHASE_NONE) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_PRJ;

			reader >> ME;
			ResolveMe();
		}
		else
		if (reader.getName() == L"AVBuilding")
		{
			if (m_phase != PHASE_PRJ) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_BLD;

			reader >> *pBuilding;
			pBuilding->Init();
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_STRUCT;
			if (iShaft >= GetBuilding()->GetShaftCount()) throw _sim_error(_sim_error::E_SIM_LIFTS);
			
			reader >> *pBuilding->GetShaft(iShaft++);
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_STRUCT;
			if (iStorey >= GetBuilding()->GetStoreyCount()) throw _sim_error(_sim_error::E_SIM_FLOORS);
			
			reader >> *pBuilding->GetStorey(iStorey++);
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			if (pBuilding->GetLiftCount() == 0)
			{
				pBuilding->InitLifts();
				for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
					AddLift(CreateLift(i));
			}

			if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIM) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
			m_phase = PHASE_SIM;

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
			pPassenger->ResolveMe();
			AddPassenger(pPassenger);
		}
	}

	// Init lifts when known
	if (m_phase >= PHASE_STRUCT && pBuilding->GetLiftCount() == 0)
	{
		pBuilding->InitLifts();
		for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
			AddLift(CreateLift(i));
	}

	// Some tests
	if (GetProjectId() == 0)
		throw _sim_error(_sim_error::E_SIM_NOT_FOUND);
	if (m_phase == PHASE_STRUCT && iShaft != GetBuilding()->GetShaftCount())
		throw _sim_error(_sim_error::E_SIM_LIFTS);
	if (m_phase == PHASE_STRUCT && iStorey != GetBuilding()->GetStoreyCount())
		throw _sim_error(_sim_error::E_SIM_FLOORS);

	if (m_phase == PHASE_STRUCT) 
		m_phase = PHASE_SIM;

	if (m_phase >= PHASE_STRUCT) 
	{
		GetBuilding()->Create();
		GetBuilding()->Scale(0.04f);
	}
//	if (m_phase == PHASE_SIM)
//	{
//		for_each_passenger([](CPassengerBase *p) { p->ResolveMe(); });
//	}

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
			pSim->ResolveMe();
			sims.push_back(pSim);
		}
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
