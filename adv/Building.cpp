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
	try
	{
		sel = db.select(L"SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
		if (sel) return S_FALSE;		// already done!
	}
	catch (...)
	{
	}

	// store lobby layout data
	ins = db.insert(L"AVBuildings");
	ins << *this;
	ins.erase(L"ID");
	ins[L"ProjectId"] = nProjectID;
	ins.execute();

	std::wstring str = ins.query();

	// retrieve the Building ID
	sel = db.select(L"SELECT SCOPE_IDENTITY()");
	SetId(sel[(short)0]);

	// store lift data
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

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

// NEW DATABASE:
	//sel = db.select(L"SELECT * FROM FloorDataSets WHERE SimulationId=%d", nSimulationID);
	//if (!sel) throw ERROR_BUILDING;
	//sel >> *this;

	//sel = db.select(L"SELECT * FROM LiftGroups WHERE SimulationId=%d", nSimulationID);
	//if (!sel) throw ERROR_BUILDING;
	//sel >> *this;

	//AVULONG nLiftGroupId = ME[L"LiftGroupId"];

	//sel = db.select(L"SELECT COUNT(LiftId) AS NumberOfLifts FROM Lifts WHERE LiftGroupId=%d", nLiftGroupId);
	//if (!sel) throw ERROR_BUILDING;
	//sel >> *this;

	//ME[L"ID"] = nSimulationID;


	// Query for Building (for building id and name - the latter not very useful)
	sel = db.select(L"SELECT BuildingId AS ID FROM Buildings WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;
		
	// Query for Layout Details (for almost full lobby info)
	sel = db.select(L"SELECT * FROM LayoutDetailsDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Lift Dataset (for NoOfShafts only)
	sel = db.select(L"SELECT * FROM LiftDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Floor Dataset (for StoreysAboveGround and StoreysBelowGround)
	sel = db.select(L"SELECT * FROM FloorDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// prepare buffers for shafts and storeys
	PreXxxx();

// NEW DATABASE
	//// Query for Shaft Data
	//sel = db.select(L"SELECT * FROM Lifts WHERE LiftGroupId=%d ORDER BY LiftNumber", nLiftGroupId);
	//for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
	//	sel >> *GetShaft(i);

	//// Query for Storey Data
	//sel = db.select(L"SELECT * FROM Floors WHERE SimulationId=%d ORDER BY GroundIndex", nSimulationID);
	//for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
	//	sel >> *GetStorey(i);

	// Query for Shaft Data
	sel = db.select(L"SELECT * FROM Lifts WHERE BuildingId=%d ORDER BY Number", GetId());
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM Floors WHERE BuildingId=%d ORDER BY GroundIndex", GetId());
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
	if      (ME[L"StructureMaterial"].as_wstring() == L"Concrete") ME[L"StructureMaterial"] = (ULONG)STRUCT_CONCRETE;
	else if (ME[L"StructureMaterial"].as_wstring() == L"Steel") ME[L"StructureMaterial"] = (ULONG)STRUCT_STEEL;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"structure material", (ME[L"StructureMaterial"]).as_wstring().c_str()), ERROR_GENERIC);
	if ((ULONG)ME[L"LobbyArrangement"] == (ULONG)LOBBY_OPENPLAN && (ULONG)ME[L"LiftShaftArrangement"] == (ULONG)SHAFT_OPPOSITE)
		ME[L"LobbyArrangement"] = (ULONG)LOBBY_THROUGH;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
#define IT (*GetShaft(i))
		if      (IT[L"DoorType"].as_wstring() == L"Centre")	IT[L"DoorType"] = (ULONG)DOOR_CENTRE;
		else if (IT[L"DoorType"].as_wstring() == L"Side")	IT[L"DoorType"] = (ULONG)DOOR_SIDE;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"door type", (IT[L"DoorType"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"LiftType"].as_wstring() == L"Single Deck") IT[L"LiftType"] = (ULONG)LIFT_SINGLE_DECK;
		else if (IT[L"LiftType"].as_wstring() == L"Double Deck") IT[L"LiftType"] = (ULONG)LIFT_DOUBLE_DECK;
		else if (IT[L"LiftType"].as_wstring() == L"Multi Car")   IT[L"LiftType"] = (ULONG)LIFT_MULTI_CAR;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"type of lift", (IT[L"LiftType"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"NoOfCarEntrance"].as_wstring() == L"Front") IT[L"NoOfCarEntrance"] = (ULONG)CAR_FRONT;
		else if (IT[L"NoOfCarEntrance"].as_wstring() == L"Rear")  IT[L"NoOfCarEntrance"] = (ULONG)CAR_REAR;
		else if (IT[L"NoOfCarEntrance"].as_wstring() == L"Both")  IT[L"NoOfCarEntrance"] = (ULONG)CAR_BOTH;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"configuration of car entrances", (IT[L"NoOfCarEntrance"]).as_wstring().c_str()), ERROR_GENERIC);
		if      (IT[L"CounterWeightPosition"].as_wstring() == L"Rear") IT[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_REAR;
		else if (IT[L"CounterWeightPosition"].as_wstring() == L"Side") IT[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_SIDE;
		else throw (Log(ERROR_UNRECOGNISED_STRING, L"counterweight position", (IT[L"CounterWeightPosition"]).as_wstring().c_str()), ERROR_GENERIC);
	}

	// Resolve and test
	Xxxx();
	if (!IsValid())
		throw ERROR_BUILDING;

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
	PreXxxx();

	// Query for Lobby Data
	sel = db.select(L"SELECT * FROM AVShafts WHERE BuildingId=%d ORDER BY ShaftID", GetId());
	for (AVULONG i = 0; i < GetShaftCount() && sel; i++, sel++)
		sel >> *GetShaft(i);

	// Query for Storey Data
	sel = db.select(L"SELECT * FROM AVFloors WHERE BuildingId=%d ORDER BY FloorId", GetId());
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		sel >> *GetStorey(i);

	// Resolve and test
	Yyyy();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	return S_OK;
}

