// Sim.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "../CommonFiles/DBTools.h"
#include "SrvSim.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CSimSrv::CSimSrv(CBuilding *pBuilding) : CSim(pBuilding)
{
}

HRESULT CSimSrv::LoadSim(CDataBase db, AVULONG nSimulationId)
{
	if (!GetBuilding())
		return Log(ERROR_INTERNAL, L"SIM file loading without the building set.");

	// load!
	CSimLoader loader;

	int nRes = loader.Load(GetBuilding(), db, nSimulationId);
	// int nRes = loader.Load(GetSIMFileName().c_str());
	//int nRes = loader.Load(GetBuilding(), L"c:\\Users\\Jarek\\Desktop\\testCirc18lift_251Floors_ver109.sim");

	// detect errors...
	if FAILED(nRes)
		return Logf(nRes, GetSIMFileName().c_str());
	
	// if ((ULONG)loader.nLifts != GetBuilding()->GetLiftCount())
	//	return Log(ERROR_FILE_INCONSISTENT_LIFTS);		// inconsistent number of floors
	// if ((ULONG)loader.nFloors != GetBuilding()->GetStoreyCount())
	//	return Log(ERROR_FILE_INCONSISTENT_FLOORS);		// inconsistent number of lifts
	// check single/double decker consistency
	// for (AVULONG i = 0; i < (ULONG)loader.nLifts;i++)
	//	if ((loader.pLifts[i].nDecks == 1 && GetBuilding()->GetLift(i)->GetShaft()->GetDeck() == CBuilding::DECK_DOUBLE)
	//	|| (loader.pLifts[i].nDecks > 1 && GetBuilding()->GetLift(i)->GetShaft()->GetDeck() == CBuilding::DECK_SINGLE))
	//		return Log(ERROR_FILE_INCONSISTENT_DECKS);

	SetSIMVersionId(loader.nVersion);

	bool bWarning = false;

	// load passenger (hall calls) data
	for (AVULONG i = 0; i < (ULONG)loader.nPassengers; i++)
	{
		CPassengerSrv *pPassenger = (CPassengerSrv*)CreatePassenger(i);
		if (pPassenger->Load(i, loader.pPassengers[i]) != S_OK)
			bWarning = true;
		AddPassenger(pPassenger);
	}

	// load, analyse and consolidate simulation data
	for (AVULONG i = 0; i < GetBuilding()->GetLiftCount(); i++)
	{
		CLiftSrv *pLift = (CLiftSrv*)CreateLift(i);
		HRESULT h = pLift->Load(GetBuilding()->GetLift(i), loader, i, true, true);
		if FAILED(h) return h;
		if (h != S_OK) bWarning = true;
		AddLift(pLift);
	}

	return bWarning ? WARNING_GENERIC : S_OK;
}

void CSimSrv::Play()
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
	{
		GetPassenger(i)->Play();
		ReportSimulationTime(GetPassenger(i)->GetUnloadTime());
	}
}

HRESULT CSimSrv::LoadFromConsole(CDataBase db, ULONG nSimulationId, ULONG iGroup)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Simulations
//	sel = db.select(L"SELECT * FROM Simulations s, Projects p WHERE p.ProjectId = s.ProjectId AND SimulationId=%d", nSimulationId);
//	if (!sel) throw ERROR_PROJECT;
//	sel >> ME;

	return S_OK;
}

HRESULT CSimSrv::LoadFromVisualisation(CDataBase db, ULONG nProjectID, ULONG iGroup)
{
	if (!db) throw db;

	// Query for Project Data (for project id)
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT * FROM AVSims WHERE ProjectId=%d AND LiftGroupIndex=%d", nProjectID, iGroup);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> ME;

	ResolveMe();

	return S_OK;
}

HRESULT CSimSrv::Store(CDataBase db)
{
	if (!db) throw db;
//	if (!GetBuilding())
//		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);

	CDataBase::INSERT ins = db.insert(L"AVSims");

	ins << ME;
	ins[L"ProjectId"] = GetProjectId();
	ins[L"SIMVersionId"] = GetSIMVersionId();
	ins[L"LiftGroupIndex"] = GetIndex();
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
	SetId(sel[(short)0]);

	// Store the Building
//	if (GetBuilding())
//		GetBuilding()->Store(db, GetProjectId());
//
//	Update(db, 0);

	return S_OK;
}

HRESULT CSimSrv::Update(CDataBase db, AVLONG nTime)
{
	if (!db) throw db;
	if (!GetBuilding())
		throw (Log(ERROR_INTERNAL, L"Project stored without the building set."), ERROR_GENERIC);
	if (GetProjectId() == 0)
		throw (Log(ERROR_INTERNAL, L"Project update run with ID=0"), ERROR_GENERIC);

	// Store the Journeys
	AVULONG nLifts = GetLiftCount();
	for (AVULONG i = 0; i < nLifts; i++)
	{
		GetLift(i)->SetSimId(GetId());
		GetLift(i)->Store(db);
	}

	// Store the Passengers
	AVULONG nPassengers = GetPassengerCount();
	for (AVULONG i = 0; i < nPassengers; i++)
	{
		GetPassenger(i)->SetSimId(GetId());
		GetPassenger(i)->Store(db);
	}

	CDataBase::UPDATE upd = db.update(L"AVSims", L"WHERE ID=%d", GetId());
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

