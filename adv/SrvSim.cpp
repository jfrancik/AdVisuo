// Sim.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include <algorithm>
#include "../CommonFiles/BaseProject.h"
#include "SrvSim.h"
#include "SrvLift.h"
#include "SrvPassenger.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

using namespace dbtools;

CPassenger *CSimSrv::CreatePassenger(AVULONG nId)		{ return new CPassengerSrv(this, nId); }
CLift *CSimSrv::CreateLift(AVULONG nId)					{ return new CLiftSrv(this, nId); }

void CSimSrv::Play()
{
	GetProject()->ReportMinSimulationTime(-100);
	for each (CPassengerSrv *pPassenger in GetPassengers())
	{
		pPassenger->Play();
		GetProject()->ReportMaxSimulationTime(pPassenger->GetUnloadTime());
		GetProject()->ReportMinSimulationTime(pPassenger->GetSpawnTime());
	}
}

HRESULT CSimSrv::LoadFromVisualisation(CDataBase db, ULONG nSimId)
{
	if (!db) throw db;

	// Query for Project Data (for project id)
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT * FROM AVSims WHERE ID=%d", nSimId);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> ME;

	ResolveMe();

	return S_OK;
}

HRESULT CSimSrv::LoadFromReports(CDataBase db)
{
	// Checks
	if (!db) throw db;
	dbtools::CDataBase::SELECT sel;

	AVULONG nTrafficScenarioId = (*this)[L"TrafficScenarioId"];
	AVULONG nIteration = 0;

	bool bWarning = false;

	wcerr << (LPOLESTR)L"********************************" << endl;

	AVULONG iPassengerId = 0;
	for (AVULONG iLift = 0; iLift < GetLiftGroup()->GetLiftCount(); iLift++)
	{
		AVULONG nNativeId = GetLiftGroup()->GetLift(iLift)->GetShaft()->GetNativeId();

		std::vector<CPassenger*> collLoading, collUnloading;
		
		// Create and load passengers (Hall Calls)
		sel = db.select(L"SELECT * FROM HallCalls WHERE LiftId = %d AND TraffiicScenarioId=%d AND Iteration=%d ORDER BY StartLoading", nNativeId, nTrafficScenarioId, nIteration);
		for ( ; sel; sel++)
		{
			CPassengerSrv *pPassenger = (CPassengerSrv*)CreatePassenger(iPassengerId++);
			if (pPassenger->Load(sel) != S_OK) bWarning = true;
			pPassenger->SetLiftId(iLift);	// overwrite original values (which were native DB id's)
			pPassenger->SetShaftId(iLift);
			AddPassenger(pPassenger);
			collLoading.push_back(pPassenger);
			collUnloading.push_back(pPassenger);
		}
		std::sort(collUnloading.begin(), collUnloading.end(), [](CPassenger *p1, CPassenger *p2) -> bool { return p1->GetUnloadTime() < p2->GetUnloadTime(); });
	
		HRESULT h;

		// Create and load lifts & journeys
		CLiftSrv *pLift = (CLiftSrv*)CreateLift(iLift);
		h = pLift->Load(GetLiftGroup()->GetLift(iLift), db, nNativeId, nTrafficScenarioId, nIteration);
		//h = pLift->Load2(db, nNativeId, nTrafficScenarioId, nIteration, collUnloading, collLoading);
		//pLift->Adjust();

		CLiftSrv *pLift2 = (CLiftSrv*)CreateLift(iLift);
		h = pLift2->Load2(db, nNativeId, nTrafficScenarioId, nIteration, collUnloading, collLoading);
		pLift->Comp(pLift2);
		delete pLift2;

		if FAILED(h) return h;
		if (h != S_OK) bWarning = true;
		AddLift(pLift);
	}
	LogProgress();

	return bWarning ? WARNING_GENERIC : S_OK;
}

HRESULT CSimSrv::Store(CDataBase db)
{
	if (!db) throw db;

	CDataBase::INSERT ins = db.insert(L"AVSims");

	ins << ME;
	ins[L"LiftGroupId"] = GetLiftGroupId();
	ins[L"LiftGroupIndex"] = GetIndex();
	ins[L"Floors"] = GetLiftGroup()->GetStoreyCount();
	ins[L"Shafts"] = GetLiftGroup()->GetShaftCount();
	ins[L"Lifts"] = GetLiftGroup()->GetLiftCount();
	ins[L"Passengers"] = (ULONG)0;
	ins[L"JourneysSaved"] = (ULONG)0;
	ins[L"PassengersSaved"] = (ULONG)0;
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

HRESULT CSimSrv::Update(CDataBase db)
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
	upd[L"Floors"] = GetLiftGroup()->GetStoreyCount();
	upd[L"Shafts"] = GetLiftGroup()->GetShaftCount();
	upd[L"Lifts"] = GetLiftGroup()->GetLiftCount();
	upd[L"Passengers"] = GetPassengerCount();
	upd[L"JourneysSaved"] = GetJourneyTotalCount();
	upd[L"PassengersSaved"] = GetPassengerCount();
	upd[L"SavedAll"] = true;
	upd.execute();

	if (nLifts || nPassengers)
		LogProgress();

	return S_OK;
}

