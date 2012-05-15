// Building.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CBuilding::Store(CDataBase db)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	CDataBase::INSERT ins;

	// store lobby layout data
	ins = db.insert(L"AVBuildings");
	ins << *this;
	ins.erase(L"ID");
	ins[L"SimId"] = GetSimId();
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
		ins[L"BuildingId"] = GetId();
		ins.execute();
	}

	// store floor data
	for (ULONG i = 0; i < GetStoreyCount(); i++)
	{
		CDataBase::INSERT ins;
		ins = db.insert(L"AVFloors");
		ins << *GetStorey(i);
		ins[L"BuildingId"] = GetId();
		ins.execute();
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	sel = db.select(L"SELECT * FROM FloorDataSets WHERE SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT * FROM LiftGroups WHERE SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	AVULONG nLiftGroupId = ME[L"LiftGroupId"];

	sel = db.select(L"SELECT COUNT(LiftId) AS NumberOfLifts FROM Lifts WHERE LiftGroupId=%d", nLiftGroupId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(FloorId) AS NumberOfStoreys FROM Floors WHERE SimulationId=%d", nSimulationId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	sel = db.select(L"SELECT COUNT(FloorId) AS NumberOfBasementStoreys FROM Floors WHERE SimulationId=%d AND GroundIndex < 0", nSimulationId);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// prepare buffers for shafts and storeys
	Init(nSimulationId);

	// Query for Shaft Data
	sel = db.select(L"SELECT * FROM Lifts l, Doors d WHERE LiftGroupId=%d  AND d.LiftId = l.LiftId AND d.DoorConfigurationId=1 ORDER BY LiftNumber", nLiftGroupId);
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM Floors WHERE SimulationId=%d ORDER BY GroundIndex", nSimulationId);
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		sel >> *GetStorey(i);

	InitLifts();

	// Resolve and test
	ConsoleCreate();
	if (!IsValid())
		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CBuilding::LoadFromVisualisation(CDataBase db, ULONG nSimID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVBuildings WHERE SimID=%d", nSimID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> *this;

	// prepare buffers for shafts and storeys
	Init();

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVShafts WHERE BuildingId=%d ORDER BY ShaftID", GetId());
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM AVFloors WHERE BuildingId=%d ORDER BY FloorId", GetId());
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		sel >> *GetStorey(i);

	InitLifts();

	// Resolve and test
	Create();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	return S_OK;
}

