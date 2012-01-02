// Building.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CBuilding::SHAFT::Store(dbtools::CDataBase db, ULONG nBuildingID, AVFLOAT MachRoomSlab, AVFLOAT LiftBeamHeight, LIFT_STRUCTURE Structure, AVFLOAT IntDivBeamWidth)
{
	if (!db) throw db;
	
	CDataBase::INSERT ins;

	ins = db.insert("AVShafts");
	ins["BuildingID"] = nBuildingID;
	ins["ShaftID"] = ShaftID;
	ins["Capacity"] = Capacity;
	ins["Speed"] = Speed;
	ins["Acceleration"] = Acceleration;
	ins["Jerk"] = Jerk;
	ins["DoorType"] = (ULONG)DoorType;
	ins["LiftDoorHeight"] = LiftDoorHeight;
	ins["LiftDoorWidth"] = LiftDoorWidth;
	ins["CarHeight"] = CarHeight;
	ins["TypeOfLift"] = (ULONG)TypeOfLift;
	ins["NumberOfLifts"] = NumberOfLifts;
	ins["MachRoomSlab"] = MachRoomSlab;		// kept for backward compatibility - 15/09/11
	ins["LiftBeamHeight"] = LiftBeamHeight;	// kept for backward compatibility - 15/09/11
	ins["NoOfCarEntrances"] = (ULONG)NoOfCarEntrances;
	ins["CounterWeightPosition"] = (ULONG)CounterWeightPosition;
	ins["Structure"] = (ULONG)Structure;		// kept for backward compatibility - 15/09/11
	ins["CarWidth"] = CarWidth;
	ins["CarDepth"] = CarDepth;
	ins["ShaftWidth"] = ShaftWidth;
	ins["ShaftDepth"] = ShaftDepth;
	ins["IntDivBeam"] = IntDivBeamWidth;		// kept for backward compatibility - 15/09/11
	ins["PitDepth"] = PitDepth;
	ins["OverallHeight"] = OverallHeight;
	ins["HeadRoom"] = HeadRoom;
	ins["MachRoomHeight"] = MachRoomHeight;
	ins["MachRoomExt"] = MachRoomExt;
	ins["PreOperTime"] = PreOperTime;
	ins["OpeningTime"] = OpeningTime;
	ins["ClosingTime"] = ClosingTime;
	ins["MotorStartDelay"] = MotorStartDelay;
	ins["LoadingTime"] = LoadingTime;
	ins["UnloadingTime"] = UnloadingTime;
	ins["FloorsServed"] = FloorsServed;
	ins.execute();

	return S_OK;
}

HRESULT CBuilding::STOREY::Store(dbtools::CDataBase db, ULONG nBuildingID)
{
	if (!db) throw db;
	
	CDataBase::INSERT ins;

	ins = db.insert("AVFloors");
	ins << ME;
	ins["BuildingID"] = nBuildingID;
	ins.execute();

	return S_OK;
}

HRESULT CBuilding::Store(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;
	CDataBase::INSERT ins;

	// check if already stored...
	sel = db.select("SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
	if (sel) return S_FALSE;		// already done!

	// store lobby layout data
	ins = db.insert("AVBuildings");
	ins << *this;
	ins.erase("ID");
	ins["ProjectId"] = nProjectID;
	ins.execute();

	// retrieve the Building ID
	sel = db.select("SELECT SCOPE_IDENTITY()");
	nBuildingID = sel[(short)0];

	// store lift data
	for (ULONG i = 0; i < NoOfShafts; i++)
		GetShaft(i)->Store(db, nBuildingID, MachRoomSlab, LiftBeamHeight, Structure, IntDivBeamWidth);

	// store floor data
	for (ULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Store(db, nBuildingID);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CBuilding::SHAFT::LoadFromConsole(CDataBase::SELECT &sel)
{
	ShaftID = sel[L"Number"];
	Capacity = sel[L"Capacity"];
	Speed = sel[L"Speed"];
	Acceleration = sel[L"Acceleration"];
	Jerk = sel[L"Jerk"];
	std::wstring strDoorType = sel[L"DoorType"];
	LiftDoorHeight = sel[L"DoorHeight"];
	LiftDoorWidth = sel[L"DoorWidth"];
	CarHeight = sel[L"CarHeight"];
	std::wstring strTypeOfLift = sel[L"LiftType"];
	std::wstring strNoOfCarEntrances = sel[L"NoOfCarEntrance"];
	std::wstring strCounterWeightPosition = sel[L"CounterWeightPosition"];
	CarWidth = sel[L"CarWidth"];
	CarDepth = sel[L"CarDepth"];
	ShaftWidth = sel[L"ShaftWidth"];
	ShaftDepth = sel[L"ShaftDepth"];
	PitDepth = sel[L"LiftPitDepth"];
	OverallHeight = sel[L"OverallCarHeight"];
	HeadRoom = sel[L"LiftHeadroom"];
	MachRoomHeight = sel[L"MachineRoomHeight"];

	// new values (+ Acceleration and Jerk)
	PreOperTime = (AVULONG)((AVFLOAT)sel[L"PreOperTime"] * 1000);
	OpeningTime = (AVULONG)((AVFLOAT)sel[L"OpeningTime"] * 1000);
	ClosingTime = (AVULONG)((AVFLOAT)sel[L"ClosingTime"] * 1000);
	MotorStartDelay = (AVULONG)((AVFLOAT)sel[L"MotorStartDelay"] * 1000);
	LoadingTime = (AVULONG)((AVFLOAT)sel[L"LoadingTime"] * 1000);
	UnloadingTime = (AVULONG)((AVFLOAT)sel[L"UnloadingTime"] * 1000);
	FloorsServed = sel[L"FloorsServed"];

	if (strDoorType == L"Centre") DoorType = DOOR_CENTRE;
	else if (strDoorType == L"Side") DoorType = DOOR_SIDE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"door type", strDoorType.c_str()), ERROR_GENERIC);
			
	if (strTypeOfLift == L"Single Deck") TypeOfLift = LIFT_SINGLE_DECK;
	else if (strTypeOfLift == L"Double Deck") TypeOfLift = LIFT_DOUBLE_DECK;
	else if (strTypeOfLift == L"Multi Car") TypeOfLift = LIFT_MULTI_CAR;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"type of lift", strTypeOfLift.c_str()), ERROR_GENERIC);

	if (strNoOfCarEntrances == L"Front") NoOfCarEntrances = CAR_FRONT;
	else if (strNoOfCarEntrances == L"Rear") NoOfCarEntrances = CAR_REAR;
	else if (strNoOfCarEntrances == L"Both") NoOfCarEntrances = CAR_BOTH;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"configuration of car entrances", strNoOfCarEntrances.c_str()), ERROR_GENERIC);

	if (strCounterWeightPosition == L"Rear") CounterWeightPosition = CNTRWEIGHT_REAR;
	else if (strCounterWeightPosition == L"Side") CounterWeightPosition = CNTRWEIGHT_SIDE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"counterweight position", strCounterWeightPosition.c_str()), ERROR_GENERIC);
			
	// not currently supported by the Console database:
	NumberOfLifts = 1;
	MachRoomExt = -1;

	return S_OK;
}

HRESULT CBuilding::STOREY::LoadFromConsole(CDataBase::SELECT &sel)
{
	sel >> ME;
	
	// set-up variables
	StoreyID = ME["FloorID"];
	HeightValue = ME["HeightValue"];
	Area = ME["Area"];
	PopDensity = ME["PopDensity"];
	Absentee = ME["Absentee"];
	StairFactor = ME["StairFactor"];
	Escalator = ME["Escalator"];
	Name = ME["Name"];

	return S_OK;
}

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Building (for building id and name - the latter not very useful)
	sel = db.select("SELECT BuildingId AS ID, BuildingName FROM Buildings WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;
		
	// Query for Layout Details (for almost full lobby info)
	sel = db.select("SELECT LobbyArrangement, BookingPointPositionM AS PosLiftBookM, NoBookingPoints AS NoOfBook, LiftLobbyCeilingHeight AS LobbyCeilingSlabHeight, LiftShaftArrangement, LobbyDepth, -1 AS LobbyWidth, FrontWallThickness, FrontWallThickness AS ShaftWallThickness, FrontWallThickness AS SideWallThickness, StructureMaterial AS Structure, MachineRoomSlab AS MachRoomSlab, LiftingBeamHeight AS LiftBeamHeight, IntermediateDividingBeam AS IntDivBeamWidth, IntermediateDividingBeam * 250 / 150 AS IntDivBeamHeight FROM LayoutDetailsDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Lift Dataset (for NoOfShafts only)
	sel = db.select("SELECT NoOfLifts AS NoOfShafts FROM LiftDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// Query for Floor Dataset (for StoreysAboveGround and StoreysBelowGround)
	sel = db.select("SELECT NoFloorsAboveMain + 1 AS FloorsAboveGround, NoFloorsBelowMain AS FloorsBelowGround FROM FloorDataSets WHERE SimulationId=%d", nSimulationID);
	if (!sel) throw ERROR_BUILDING;
	sel >> *this;

	// corrections
	if      (ME["LobbyArrangement"].as_wstring() == L"Through")				  ME["LobbyArrangement"] = (ULONG)LOBBY_THROUGH;
	else if (ME["LobbyArrangement"].as_wstring() == L"Open Plan")			  ME["LobbyArrangement"] = (ULONG)LOBBY_OPENPLAN;
	else if (ME["LobbyArrangement"].as_wstring() == L"Dead End on the Left")  ME["LobbyArrangement"] = (ULONG)LOBBY_DEADEND_LEFT;
	else if (ME["LobbyArrangement"].as_wstring() == L"Dead End on the Right") ME["LobbyArrangement"] = (ULONG)LOBBY_DEADEND_RIGHT;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"lobby arrangement", (ME["LobbyArrangement"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME["LiftShaftArrangement"].as_wstring() == L"Inline") ME["LiftShaftArrangement"] = (ULONG)SHAFT_INLINE;
	else if (ME["LiftShaftArrangement"].as_wstring() == L"Opposite") ME["LiftShaftArrangement"] = (ULONG)SHAFT_OPPOSITE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"lift shaft arrangement", (ME["LiftShaftArrangement"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME["Structure"].as_wstring() == L"Concrete") ME["Structure"] = (ULONG)STRUCT_CONCRETE;
	else if (ME["Structure"].as_wstring() == L"Steel") ME["Structure"] = (ULONG)STRUCT_STEEL;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"structure material", (ME["Structure"]).as_wstring().c_str()), ERROR_GENERIC);
	if ((ULONG)ME["LobbyArrangement"] == (ULONG)LOBBY_OPENPLAN && (ULONG)ME["LiftShaftArrangement"] == (ULONG)SHAFT_OPPOSITE)
		ME["LobbyArrangement"] = (ULONG)LOBBY_THROUGH;

	// setup variables!
	BuildingName = ME["BuildingName"];
	LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME["LobbyArrangement"];
	PosLiftBookM = ME["PosLiftBookM"];
	NoOfBook = ME["NoOfBook"];
	LobbyCeilingSlabHeight = ME["LobbyCeilingSlabHeight"];
	LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME["LiftShaftArrangement"];
	LobbyDepth = ME["LobbyDepth"];
	FrontWallThickness = ME["FrontWallThickness"];
	Structure = (LIFT_STRUCTURE)(ULONG)ME["Structure"];
	MachRoomSlab = ME["MachRoomSlab"];
	LiftBeamHeight = ME["LiftBeamHeight"];
	IntDivBeamWidth = ME["IntDivBeamWidth"];
	IntDivBeamHeight = ME["IntDivBeamHeight"];
	NoOfShafts = ME["NoOfShafts"];
	StoreysAboveGround = ME["FloorsAboveGround"];
	StoreysBelowGround = ME["FloorsBelowGround"];

	// prepare buffers for shafts and storeys
	CreateShafts();
	CreateStoreys();

	// Query for Shaft Data
	sel = db.select("SELECT * FROM Lifts WHERE BuildingId=%d ORDER BY Number", (ULONG)ME["ID"]);
	for (AVULONG i = 0; i < NoOfShafts && sel; i++, sel++)
		GetShaft(i)->LoadFromConsole(sel);

	// Query for Storey Data
	sel = db.select("SELECT Absentee, Area, Escalator, GroundIndex AS FloorID, FloorHeight * 1000 AS HeightValue, Name, PopDensity, StairFactor FROM Floors WHERE BuildingId=%d ORDER BY GroundIndex", (ULONG)ME["ID"]);
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		GetStorey(i)->LoadFromConsole(sel);

	// Resolve and test
	Resolve();
//	if (!IsValid())
//		throw ERROR_BUILDING;

	return S_OK;
}

HRESULT CBuilding::SHAFT::LoadFromVisualisation(dbtools::CDataBase::SELECT &sel, AVFLOAT &MachRoomSlab, AVFLOAT &LiftBeamHeight, LIFT_STRUCTURE &Structure, AVFLOAT &IntDivBeamWidth, AVFLOAT &IntDivBeamHeight)
{
	ShaftID = sel[L"ShaftID"];
	Capacity = sel[L"Capacity"];
	Speed = sel[L"Speed"];
	DoorType = (DOOR_TYPE)(ULONG)sel[L"DoorType"];
	LiftDoorHeight = sel[L"LiftDoorHeight"];
	LiftDoorWidth = sel[L"LiftDoorWidth"];
	CarHeight = sel[L"CarHeight"];
	TypeOfLift = (TYPE_OF_LIFT)(ULONG)sel[L"TypeOfLift"];
	NumberOfLifts = sel[L"NumberOfLifts"];
	NoOfCarEntrances = (CAR_ENTRANCES)(ULONG)sel[L"NoOfCarEntrances"];
	CounterWeightPosition = (CNTRWEIGHT_POS)(ULONG)sel[L"CounterWeightPosition"];
	CarWidth = sel[L"CarWidth"];
	CarDepth = sel[L"CarDepth"];
	ShaftWidth = sel[L"ShaftWidth"];
	ShaftDepth = sel[L"ShaftDepth"];
	PitDepth = sel[L"PitDepth"];
	OverallHeight = sel[L"OverallHeight"];
	HeadRoom = sel[L"HeadRoom"];
	MachRoomHeight = sel[L"MachRoomHeight"];
	MachRoomExt = sel[L"MachRoomExt"];

	// These variables are phasing out
	if (MachRoomSlab < 0)
		MachRoomSlab = sel[L"MachRoomSlab"];
	if (LiftBeamHeight < 0)
		LiftBeamHeight = sel[L"LiftBeamHeight"];
	if (Structure < 0)
		Structure = (LIFT_STRUCTURE)(ULONG)sel[L"Structure"];
	if (IntDivBeamWidth < 0)
	{
		IntDivBeamWidth = sel[L"IntDivBeam"];
		IntDivBeamHeight = IntDivBeamWidth * 250.0f / 150.0f;
	}
	return S_OK;
}

HRESULT CBuilding::STOREY::LoadFromVisualisation(dbtools::CDataBase::SELECT &sel)
{
	StoreyID = sel[L"FloorID"];
	HeightValue = sel[L"HeightValue"];
	return S_OK;
}

HRESULT CBuilding::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) throw db;
	CDataBase::SELECT sel;

	// Query for Lobby Data
	sel = db.select("SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
	if (!sel) throw ERROR_DATA_NOT_FOUND;
	sel >> *this;

	// setup variables!
	AVULONG nBuildingID = ME["ID"];
	BuildingName = ME["BuildingName"];
	LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME["LobbyArrangement"];
	PosLiftBookM = ME["PosLiftBookM"];
	NoOfBook = ME["NoOfBook"];
	LobbyCeilingSlabHeight = ME["LobbyCeilingSlabHeight"];
	LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME["LiftShaftArrangement"];
	LobbyDepth = ME["LobbyDepth"];
	FrontWallThickness = ME["FrontWallThickness"];
	Structure = (LIFT_STRUCTURE)(ULONG)ME["Structure"];
	MachRoomSlab = ME["MachRoomSlab"];
	LiftBeamHeight = ME["LiftBeamHeight"];
	IntDivBeamWidth = ME["IntDivBeamWidth"];
	IntDivBeamHeight = ME["IntDivBeamHeight"];
	NoOfShafts = ME["NoOfShafts"];
	StoreysAboveGround = ME["FloorsAboveGround"];
	StoreysBelowGround = ME["FloorsBelowGround"];

	// prepare buffers for shafts and storeys
	CreateShafts();
	CreateStoreys();

	// Query for Lobby Data
	sel = db.select("SELECT * FROM AVShafts WHERE BuildingID=%d ORDER BY ShaftID", nBuildingID);
	for (AVULONG i = 0; i < NoOfShafts && sel; i++, sel++)
		GetShaft(i)->LoadFromVisualisation(sel, MachRoomSlab, LiftBeamHeight, Structure, IntDivBeamWidth, IntDivBeamHeight);

	// Query for Storey Data
	sel = db.select("SELECT * FROM AVFloors WHERE BuildingID=%d ORDER BY FloorID", nBuildingID);
	for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		GetStorey(i)->LoadFromVisualisation(sel);

	// Resolve and test
	Resolve();
	if (!IsValid())
		throw ERROR_DATA_NOT_FOUND;

	return S_OK;
}

