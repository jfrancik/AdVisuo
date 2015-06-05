// Sim.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
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
//	GetProject()->ReportMinSimulationTime(-100);
//	for each (CPassengerSrv *pPassenger in GetPassengers())
//	{
//		pPassenger->Play();
//		GetProject()->ReportMaxSimulationTime(pPassenger->GetUnloadTime());
//		GetProject()->ReportMinSimulationTime(pPassenger->GetSpawnTime());
//	}
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

	ins[L"TimeStamp"] = L"CURRENT_TIMESTAMP";
	ins[L"TimeStamp"].act_as_symbol();

	ins.execute();
		
	// retrieve the Project ID
	CDataBase::SELECT sel;
	sel = db.select(L"SELECT @@identity");		// used to have SCOPE_IDENTITY() here but it doesn't work with ODBC
	SetId(sel[(short)0]);

	return S_OK;
}

HRESULT CSimSrv::Update(CDataBase db)
{
	if (!db) throw db;

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

	return S_OK;
}

