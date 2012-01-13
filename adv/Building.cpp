// Building.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// Database Store

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
	{
		CDataBase::INSERT ins;
		ins = db.insert(L"AVShafts");
		ins << *GetShaft(i);
		ins[L"BuildingID"] = GetId();
		ins.execute();
	}

	// store floor data
	for (ULONG i = 0; i < GetStoreyCount(); i++)
	{
		CDataBase::INSERT ins;
		ins = db.insert(L"AVFloors");
		ins << *GetStorey(i);
		ins[L"BuildingID"] = GetId();
		ins.execute();
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationID, AVFLOAT fScale)
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
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT Absentee, Area, Escalator, GroundIndex AS FloorID, FloorHeight * 1000 AS HeightValue, Name, PopDensity, StairFactor FROM Floors WHERE BuildingId=%d ORDER BY GroundIndex", (ULONG)ME[L"ID"]);
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		sel >> *GetStorey(i);

	// Post-Load Corrections
	if      (ME[L"LobbyArrangement"].as_wstring() == L"Through")				  ME[L"LobbyArrangement"] = (ULONG)LOBBY_THROUGH;
	else if (ME[L"LobbyArrangement"].as_wstring() == L"Open Plan")			  ME[L"LobbyArrangement"] = (ULONG)LOBBY_OPENPLAN;
	else if (ME[L"LobbyArrangement"].as_wstring() == L"Dead End on the Left")  ME[L"LobbyArrangement"] = (ULONG)LOBBY_DEADEND_LEFT;
	else if (ME[L"LobbyArrangement"].as_wstring() == L"Dead End on the Right") ME[L"LobbyArrangement"] = (ULONG)LOBBY_DEADEND_RIGHT;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"lobby arrangement", (ME[L"LobbyArrangement"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME[L"LiftShaftArrangement"].as_wstring() == L"Inline") ME[L"LiftShaftArrangement"] = (ULONG)SHAFT_INLINE;
	else if (ME[L"LiftShaftArrangement"].as_wstring() == L"Opposite") ME[L"LiftShaftArrangement"] = (ULONG)SHAFT_OPPOSITE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"lift shaft arrangement", (ME[L"LiftShaftArrangement"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME[L"Structure"].as_wstring() == L"Concrete") ME[L"Structure"] = (ULONG)STRUCT_CONCRETE;
	else if (ME[L"Structure"].as_wstring() == L"Steel") ME[L"Structure"] = (ULONG)STRUCT_STEEL;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"structure material", (ME[L"Structure"]).as_wstring().c_str()), ERROR_GENERIC);
	if ((ULONG)ME[L"LobbyArrangement"] == (ULONG)LOBBY_OPENPLAN && (ULONG)ME[L"LiftShaftArrangement"] == (ULONG)SHAFT_OPPOSITE)
		ME[L"LobbyArrangement"] = (ULONG)LOBBY_THROUGH;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
#define IT (*GetShaft(i))
		if      (IT[L"DoorType"].as_wstring() == L"Centre")	IT[L"DoorType"] = (ULONG)DOOR_CENTRE;
		else if (IT[L"DoorType"].as_wstring() == L"Side")	IT[L"DoorType"] = (ULONG)DOOR_SIDE;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"door type", (IT[L"DoorType"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"TypeOfLift"].as_wstring() == L"Single Deck") IT[L"TypeOfLift"] = (ULONG)LIFT_SINGLE_DECK;
		else if (IT[L"TypeOfLift"].as_wstring() == L"Double Deck") IT[L"TypeOfLift"] = (ULONG)LIFT_DOUBLE_DECK;
		else if (IT[L"TypeOfLift"].as_wstring() == L"Multi Car")   IT[L"TypeOfLift"] = (ULONG)LIFT_MULTI_CAR;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"type of lift", (IT[L"TypeOfLift"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"NoOfCarEntrances"].as_wstring() == L"Front") IT[L"NoOfCarEntrances"] = (ULONG)CAR_FRONT;
		else if (IT[L"NoOfCarEntrances"].as_wstring() == L"Rear")  IT[L"NoOfCarEntrances"] = (ULONG)CAR_REAR;
		else if (IT[L"NoOfCarEntrances"].as_wstring() == L"Both")  IT[L"NoOfCarEntrances"] = (ULONG)CAR_BOTH;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"configuration of car entrances", (IT[L"NoOfCarEntrances"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"CounterWeightPosition"].as_wstring() == L"Rear") IT[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_REAR;
		else if (IT[L"CounterWeightPosition"].as_wstring() == L"Side") IT[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_SIDE;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"counterweight position", (IT[L"CounterWeightPosition"]).as_wstring().c_str()), ERROR_GENERIC);
	}

	// Resolve and test
	ResolveMe(fScale);
	if (!IsValid())
		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CBuilding::LoadFromVisualisation(CDataBase db, ULONG nProjectID, AVFLOAT fScale)
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
	SetId(ME[L"ID"]);

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVShafts WHERE BuildingID=%d ORDER BY ShaftID", GetId());
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM AVFloors WHERE BuildingID=%d ORDER BY FloorID", GetId());
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		sel >> *GetStorey(i);

	// Resolve and test
	ResolveMe(fScale);
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	return S_OK;
}

