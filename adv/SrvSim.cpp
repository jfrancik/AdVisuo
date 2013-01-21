// Sim.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvSim.h"
#include "SrvLift.h"
#include "SrvPassenger.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CPassenger *CSimSrv::CreatePassenger(AVULONG nId)		{ return new CPassengerSrv(this, nId); }
CLift *CSimSrv::CreateLift(AVULONG nId)					{ return new CLiftSrv(this, nId); }

HRESULT CSimSrv::LoadSim(CDataBase db, AVULONG nSimulationId)
{
	if (!db) throw db;
	dbtools::CDataBase::SELECT sel;

	if (!GetLiftGroup())
		return Log(ERROR_INTERNAL, L"SIM file loading without the building set.");

	// load!
	CSimLoader loader;

	// Specify nIteration - a priori zero!
	AVULONG nIteration = 0;

	// specify scenario id (query for minimal available)
#ifdef VER200                                                                                                   /*** REPEATING SPELLING ERROR HERE ***/
	sel = db.select(L"SELECT MIN(TraffiicScenarioId) As MinTrafficScenarioId FROM HallCalls WHERE LiftId=%d AND SimulationId=%d AND Iteration=%d", GetLiftGroup()->GetLift(0)->GetShaft()->GetNativeId(), nSimulationId, nIteration);
	if (!sel) return ERROR_SIM_MISSING;
	AVULONG nTrafficScenarioId = sel[L"MinTrafficScenarioId"];
#else
	AVULONG nTrafficScenarioId = 0;
#endif

	int nRes = loader.Load(GetLiftGroup(), db, nSimulationId, nTrafficScenarioId, nIteration);
	// int nRes = loader.Load(GetSIMFileName().c_str());
	//int nRes = loader.Load(GetLiftGroup(), L"c:\\Users\\Jarek\\Desktop\\testCirc18lift_251Floors_ver109.sim");

	// detect errors...
	if FAILED(nRes)
		return Logf(nRes, GetSIMFileName().c_str());
	
	// if ((ULONG)loader.nLifts != GetLiftGroup()->GetLiftCount())
	//	return Log(ERROR_FILE_INCONSISTENT_LIFTS);		// inconsistent number of floors
	// if ((ULONG)loader.nFloors != GetLiftGroup()->GetStoreyCount())
	//	return Log(ERROR_FILE_INCONSISTENT_FLOORS);		// inconsistent number of lifts
	// check single/double decker consistency
	// for (AVULONG i = 0; i < (ULONG)loader.nLifts;i++)
	//	if ((loader.pLifts[i].nDecks == 1 && GetLiftGroup()->GetLift(i)->GetShaft()->GetDeck() == CLiftGroup::DECK_DOUBLE)
	//	|| (loader.pLifts[i].nDecks > 1 && GetLiftGroup()->GetLift(i)->GetShaft()->GetDeck() == CLiftGroup::DECK_SINGLE))
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
	for (AVULONG i = 0; i < GetLiftGroup()->GetLiftCount(); i++)
	{
		CLiftSrv *pLift = (CLiftSrv*)CreateLift(i);
		HRESULT h = pLift->Load(GetLiftGroup()->GetLift(i), loader, i, true, true);
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

HRESULT CSimSrv::LoadFromVisualisation(CDataBase db, ULONG nLiftGroupId)
{
	if (!db) throw db;

	// Query for Project Data (for project id)
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT * FROM AVSims WHERE LiftGroupId=%d AND LiftGroupIndex=%d", nLiftGroupId, GetIndex());
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> ME;

	ResolveMe();

	return S_OK;
}

HRESULT CSimSrv::Store(CDataBase db)
{
	if (!db) throw db;

	CDataBase::INSERT ins = db.insert(L"AVSims");

	ins << ME;
	ins[L"LiftGroupId"] = GetLiftGroupId();
	ins[L"SIMVersionId"] = GetSIMVersionId();
	ins[L"LiftGroupIndex"] = GetIndex();
	ins[L"Floors"] = GetLiftGroup()->GetStoreyCount();
	ins[L"Shafts"] = GetLiftGroup()->GetShaftCount();
	ins[L"Lifts"] = GetLiftGroup()->GetLiftCount();
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

	return S_OK;
}

HRESULT CSimSrv::Update(CDataBase db, AVLONG nTime)
{
	if (!db) throw db;
	if (GetLiftGroupId() == 0)
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
	upd[L"Floors"] = GetLiftGroup()->GetStoreyCount();
	upd[L"Shafts"] = GetLiftGroup()->GetShaftCount();
	upd[L"Lifts"] = GetLiftGroup()->GetLiftCount();
	upd[L"Passengers"] = GetPassengerCount();
	upd[L"SimulationTime"] = GetSimulationTime();
	upd[L"JourneysSaved"] = GetJourneyTotalCount();
	upd[L"PassengersSaved"] = GetPassengerCount();
	upd[L"TimeSaved"] = GetSimulationTime();
	upd[L"SavedAll"] = true;
	upd.execute();

	return S_OK;
}

