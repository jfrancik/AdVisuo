// Building.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CBuilding::SHAFT::Store(dbtools::CDataBase db, ULONG nBuildingID)
{
	if (!db) throw db;
	CDataBase::INSERT ins;
	ins = db.insert(L"AVShafts");
	ins << ME;
	ins[L"BuildingID"] = nBuildingID;
	ins.execute();
	return S_OK;
}

HRESULT CBuilding::STOREY::Store(dbtools::CDataBase db, ULONG nBuildingID)
{
	if (!db) throw db;
	CDataBase::INSERT ins;
	ins = db.insert(L"AVFloors");
	ins << ME;
	ins[L"BuildingID"] = nBuildingID;
	ins.execute();
	return S_OK;
}

HRESULT CBuilding::Store(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	CDataBase::INSERT ins;

	// check if already stored...
	sel = db.select(L"SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
	if (sel) return S_FALSE;		// already done!

	// store lobby layout data
	ins = db.insert(L"AVBuildings");
	ins << *this;
	ins[L"ProjectId"] = nProjectID;
	ins.erase(L"ID");
	ins.execute();

	// retrieve the Building ID
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetId(sel[(short)0]);

	// store lift data
	for (ULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Store(db, GetId());

	// store floor data
	for (ULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Store(db, GetId());

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CBuilding::SHAFT::LoadFromConsole(CDataBase::SELECT &sel)
{
	sel >> ME;

	dupaCorrect();
	dupaSetupVars();
	
	return S_OK;
}

HRESULT CBuilding::STOREY::LoadFromConsole(CDataBase::SELECT &sel)
{
	sel >> ME;
	
	dupaCorrect();
	dupaSetupVars();

	return S_OK;
}

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Building (for building id and name - the latter not very useful)
	sel = db.select(L"SELECT BuildingId AS ID, BuildingName FROM Buildings WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;
		
	// Query for Layout Details (for almost full lobby info)
	sel = db.select(L"SELECT LobbyArrangement, BookingPointPositionM AS PosLiftBookM, NoBookingPoints AS NoOfBook, LiftLobbyCeilingHeight AS LobbyCeilingSlabHeight, LiftShaftArrangement, LobbyDepth, -1 AS LobbyWidth, FrontWallThickness, FrontWallThickness AS ShaftWallThickness, FrontWallThickness AS SideWallThickness, StructureMaterial AS Structure, MachineRoomSlab AS MachRoomSlab, LiftingBeamHeight AS LiftBeamHeight, IntermediateDividingBeam AS IntDivBeamWidth, IntermediateDividingBeam * 250 / 150 AS IntDivBeamHeight FROM LayoutDetailsDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Lift Dataset (for NoOfShafts only)
	sel = db.select(L"SELECT NoOfLifts AS NoOfShafts FROM LiftDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Floor Dataset (for StoreysAboveGround and StoreysBelowGround)
	sel = db.select(L"SELECT NoFloorsAboveMain + 1 AS FloorsAboveGround, NoFloorsBelowMain AS FloorsBelowGround FROM FloorDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// prepare buffers for shafts and storeys
	CreateShafts(ME[L"NoOfShafts"]);
	CreateStoreys((ULONG)ME[L"FloorsAboveGround"] + (ULONG)ME[L"FloorsBelowGround"], (ULONG)ME[L"FloorsBelowGround"]);

	// Query for Shaft Data
	sel = db.select(L"SELECT Acceleration, Capacity, CarDepth, CarHeight, CarWidth, ClosingTime * 1000 AS ClosingTime, CounterWeightPosition, DoorType, FloorsServed, LiftHeadroom AS HeadRoom, 	Jerk, DoorHeight AS LiftDoorHeight, DoorWidth AS LiftDoorWidth, LoadingTime * 1000 AS LoadingTime, -1 AS MachRoomExt, MachineRoomHeight AS MachRoomHeight, 	MotorStartDelay * 1000 AS MotorStartDelay, NoOfCarEntrance AS NoOfCarEntrances, 1 AS NumberOfLifts, OpeningTime * 1000 AS OpeningTime, 	OverallCarHeight AS OverallHeight, LiftPitDepth AS PitDepth, PreOperTime * 1000 AS PreOperTime, ShaftDepth, Number AS ShaftID, ShaftWidth, Speed, LiftType AS TypeOfLift, UnloadingTime * 1000 AS UnloadingTime FROM Lifts WHERE BuildingId=%d ORDER BY Number", (ULONG)ME[L"ID"]);
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		GetShaft(i)->LoadFromConsole(sel);

	// Query for Storey Data
	sel = db.select(L"SELECT Absentee, Area, Escalator, GroundIndex AS FloorID, FloorHeight * 1000 AS HeightValue, Name, PopDensity, StairFactor FROM Floors WHERE BuildingId=%d ORDER BY GroundIndex", (ULONG)ME[L"ID"]);
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		GetStorey(i)->LoadFromConsole(sel);

	dupaCorrect();
	dupaSetupVars();

	// Resolve and test
	Resolve();
	if (!IsValid())
		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CBuilding::SHAFT::LoadFromVisualisation(dbtools::CDataBase::SELECT &sel)
{
	sel >> ME;
	dupaSetupVars();
	return S_OK;
}

HRESULT CBuilding::STOREY::LoadFromVisualisation(dbtools::CDataBase::SELECT &sel)
{
	sel >> ME;
	dupaSetupVars();
	return S_OK;
}

HRESULT CBuilding::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> *this;

	// prepare buffers for shafts and storeys
	CreateShafts(ME[L"NoOfShafts"]);
	CreateStoreys((ULONG)ME[L"FloorsAboveGround"] + (ULONG)ME[L"FloorsBelowGround"], (ULONG)ME[L"FloorsBelowGround"]);

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVShafts WHERE BuildingID=%d ORDER BY ShaftID", GetId());
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		GetShaft(i)->LoadFromVisualisation(sel);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM AVFloors WHERE BuildingID=%d ORDER BY FloorID", GetId());
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		GetStorey(i)->LoadFromVisualisation(sel);

	dupaSetupVars();

	// Resolve and test
	Resolve();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	return S_OK;
}

