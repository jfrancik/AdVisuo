// SrvLftGroup.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLftGroup.h"
#include "SrvSim.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// CLftGroupSrv implementation

CSim *CLftGroupSrv::CreateSim()
{
	return new CSimSrv();
}

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CLftGroupSrv::Store(CDataBase db)
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
	GetSim()->SetLftGroupId(GetId());
	HRESULT h = GetSim()->Store(db);
	if FAILED(h) return h;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CLftGroupSrv::LoadFromConsole(CDataBase db, ULONG nLiftGroupId)
{
	if (!db) throw db;
	CDataBase::SELECT sel, sel1;

	ME[L"LiftGroupIndex"] = GetIndex();

	sel = db.select(L"SELECT f.* FROM FloorDataSets f, LiftGroups g WHERE f.SimulationId=g.SimulationId AND g.LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT * FROM LiftGroups WHERE LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(LiftId) AS NumberOfLifts FROM Lifts WHERE LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(f.FloorId) AS NumberOfStoreys FROM Floors f, LiftGroups g WHERE f.SimulationId=g.SimulationId AND g.LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(f.FloorId) AS NumberOfBasementStoreys FROM Floors f, LiftGroups g WHERE f.SimulationId=g.SimulationId AND g.LiftGroupId=%d AND f.GroundIndex < 0", nLiftGroupId);
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
		std::wstring ss = L"";
		sel1 = db.select(L"SELECT sf.IsServed AS IsServed, f.GroundIndex FROM ServedFloors sf, Floors f WHERE sf.FloorId = f.FloorId and sf.LiftId = %d ORDER BY f.GroundIndex ", (AVULONG)sel[L"LiftId"]); 
		while (sel1)
		{
			ss += sel1[L"IsServed"];
			sel1++;
		}
		(*pShaft)[L"StoreysServed"] = ss;

		sel++;
	}

	// Query for Storey Data and add/load storeys
	sel = db.select(L"SELECT f.* FROM Floors f, LiftGroups g WHERE f.SimulationId=g.SimulationId AND g.LiftGroupId=%d ORDER BY f.GroundIndex", nLiftGroupId);
	while (sel)
	{
		STOREY *pStorey = AddStorey();
		sel >> *pStorey;
		sel++;
	}

	AddExtras();
	AddSim();
	ResolveMe();

	// Resolve and test
	ConsoleCreate();
	if (!IsValid())
		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CLftGroupSrv::LoadFromVisualisation(CDataBase db, ULONG nLftGroupID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVLiftGroups WHERE ID=%d", nLftGroupID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> *this;

	// Query for Shaft Data and add /load shafts
	sel = db.select(L"SELECT * FROM AVShafts WHERE LiftGroupId=%d ORDER BY ShaftID", nLftGroupID);
	while (sel)
	{
		SHAFT *pShaft = AddShaft();
		sel >> *pShaft;
		sel++;
	}

	// Query for Storey Data and add/load storeys
	sel = db.select(L"SELECT * FROM AVFloors WHERE LiftGroupId=%d ORDER BY FloorId", nLftGroupID);
	while (sel)
	{
		STOREY *pStorey = AddStorey();
		sel >> *pStorey;
		sel++;
	}

	AddExtras();
	AddSim();
	ResolveMe();

	// Resolve and test
	Create();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	HRESULT h = GetSim()->LoadFromVisualisation(db, nLftGroupID);
	if FAILED(h) return h;

	return S_OK;
}

