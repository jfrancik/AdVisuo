// SrvLiftGroup.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLiftGroup.h"
#include "SrvSim.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroupSrv implementation

CSim *CLiftGroupSrv::CreateSim()
{
	return new CSimSrv();
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CLiftGroupSrv::LoadFromConsole(CDataBase db, ULONG nLiftGroupId)
{
	if (!db) throw db;
	CDataBase::SELECT sel, sel1;

	ME[L"LiftGroupIndex"] = GetIndex();

//  original reason for this snippet is highly unclear
//	sel = db.select(L"SELECT f.* FROM FloorDataSets f, LiftGroups g WHERE f.SimulationId=g.SimulationId AND g.LiftGroupId=%d", nLiftGroupId);
//	if (!sel) throw ERROR_BUILDING;
//	sel >> *this;

	sel = db.select(L"SELECT * FROM LiftGroups WHERE LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(LiftId) AS NumberOfLifts FROM Lifts WHERE LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;


	sel = db.select(L"SELECT COUNT(FloorId) AS NumberOfStoreys FROM Floors WHERE SimulationId IN (SELECT t.SimulationId FROM LiftGroups g, Tenancies t WHERE t.TenancyId = g.TenancyId AND g.LiftGroupId=%d)", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Shaft Data and add /load shafts
	sel = db.select(L"SELECT * FROM Lifts l, Doors d WHERE LiftGroupId=%d  AND d.LiftId = l.LiftId AND d.DoorConfigurationId=1 ORDER BY LiftNumber", nLiftGroupId);
	while (sel)
	{
		SHAFT *pShaft = AddShaft();

		sel >> *pShaft;
		pShaft->erase(L"LiftGroupId");

		// Queries for Stories Served
		std::wstring ss((AVULONG)(*this)[L"NumberOfStoreys"], L'0');
		sel1 = db.select(L"SELECT lf.Priority AS Priority, f.GroundIndex AS GroundIndex FROM LiftFloors lf, Floors f WHERE lf.LiftId = %d AND lf.FloorId = f.FloorId ORDER BY f.GroundIndex ", (AVULONG)sel[L"LiftId"]); 
		while (sel1)
		{
			AVLONG nPriority = sel1[L"Priority"];
			AVLONG nGroundIndex = sel1[L"GroundIndex"];
			ss[nGroundIndex] = L'1';
			sel1++;
		}
		(*pShaft)[L"StoreysServed"] = ss;

		sel++;
	}

	// Query for the Main Floors (aka Lobbies)
	std::wstring ss((AVULONG)(*this)[L"NumberOfStoreys"], L'0');
	sel1 = db.select(L"SELECT f.GroundIndex AS GroundIndex FROM LiftGroupLobbies lgl, Floors f WHERE lgl.LiftGroupId = %d AND lgl.FloorId = f.FloorId ORDER BY f.GroundIndex ", nLiftGroupId); 
	while (sel1)
	{
		AVLONG nGroundIndex = sel1[L"GroundIndex"];
		ss[nGroundIndex] = '1';
		sel1++;
	}
	ME[L"MainFloors"] = ss;


	// Query for Storey Data and add/load storeys

	sel = db.select(L"SELECT * FROM Floors WHERE SimulationId IN (SELECT t.SimulationId FROM LiftGroups g, Tenancies t WHERE t.TenancyId = g.TenancyId AND g.LiftGroupId=%d) ORDER BY GroundIndex", nLiftGroupId);
	while (sel)
	{
		STOREY *pStorey = AddStorey();
		sel >> *pStorey;
		sel++;
	}

	// Query for Traffic Scenario Data
	sel = db.select(L"SELECT s.TrafficScenarioId, s.TrafficPatternTypeId, s.TrafficProfileId, s.TrafficScenarioIndex, s.Incoming, s.Outgoing, s.Interfloor, t.Name AS TrafficPatternName, p.Name AS TrafficProfileName FROM TrafficScenarios s, TrafficPatternTypes t, TrafficProfiles p WHERE s.TrafficPatternTypeId=t.TrafficPatternTypeId AND s.TrafficProfileId=p.TrafficProfileId AND s.LiftGroupId=%d ORDER BY TrafficScenarioIndex", nLiftGroupId);
	while (sel)	// target is to replace it with while and collect ALL traffic scenarios!
	{
		CSimSrv *pSim = AddSim();
		sel >> *pSim;
		sel++;
	}

	AddExtras();
	ResolveMe();

	// Resolve and test
	ConsoleCreate();
	if (!IsValid())
		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CLiftGroupSrv::LoadFromVisualisation(CDataBase db, ULONG nLiftGroupID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVLiftGroups WHERE ID=%d", nLiftGroupID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> *this;

	// Query for Shaft Data and add /load shafts
	sel = db.select(L"SELECT * FROM AVShafts WHERE LiftGroupId=%d ORDER BY ShaftID", nLiftGroupID);
	while (sel)
	{
		SHAFT *pShaft = AddShaft();
		sel >> *pShaft;
		sel++;
	}

	// Query for Storey Data and add/load storeys
	sel = db.select(L"SELECT * FROM AVFloors WHERE LiftGroupId=%d ORDER BY FloorId", nLiftGroupID);
	while (sel)
	{
		STOREY *pStorey = AddStorey();
		sel >> *pStorey;
		sel++;
	}

	AddExtras();
	ResolveMe();

	// Resolve and test
	Create();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	// Query for Sims
	// previously also had "AND LiftGroupIndex=%d" - no idea why
	sel = db.select(L"SELECT * FROM AVSims WHERE LiftGroupId=%d", nLiftGroupID);
	while (sel)
	{
		CSimSrv *pSim = AddSim();
		HRESULT h = pSim->LoadFromVisualisation(db, sel[L"ID"]);
		if FAILED(h) return h;
		sel++;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CLiftGroupSrv::Store(CDataBase db)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	CDataBase::INSERT ins;

	// store lobby layout data
	ins = db.insert(L"AVLiftGroups");
	ins << *this;
	ins.erase(L"ID");
	ins[L"ProjectId"] = GetProjectId();
	ins[L"LiftGroupIndex"] = GetIndex();
	ins.execute();

	std::wstring str = ins.query();

	// retrieve the Building ID
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetId(sel[(short)0]);

	// store shaft/lift data
	for (ULONG i = 0; i < GetShaftCount(); i++)
	{
		CDataBase::INSERT ins;
		ins = db.insert(L"AVShafts");
		ins << *GetShaft(i);
		ins[L"LiftGroupId"] = GetId();
		ins.execute();
	}

	// store floor data
	for (ULONG i = 0; i < GetStoreyCount(); i++)
	{
		CDataBase::INSERT ins;
		ins = db.insert(L"AVFloors");
		ins << *GetStorey(i);
		ins[L"LiftGroupId"] = GetId();
		ins.execute();
	}

	// store Sim
	HRESULT h;
	for each (CSimSrv *pSim in GetSims())
	{
		pSim->SetLiftGroupId(GetId());
		if FAILED(h = pSim->Store(db)) return h;
	}

	return S_OK;
}

