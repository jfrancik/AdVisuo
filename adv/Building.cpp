// Building.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"

using namespace dbtools;

//////////////////////////////////////////////////////////////////////////////////
// Database Store

HRESULT CBuilding::Store(CDataBase db, ULONG nProjectID)
{
	if (!db) return db;
	try
	{
		CDataBase::SELECT sel;
		CDataBase::INSERT ins;

		// check if already stored...
		sel = db.select("SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
		if (sel) return S_FALSE;		// already done!
		
		// store lobby layout data
		ins = db.insert("AVBuildings");
		ins["ProjectId"] = nProjectID;
		ins["BuildingName"] = BuildingName;
		ins["NoOfShafts"] = NoOfShafts;
		ins["PosLiftBookM"] = PosLiftBookM;
		ins["NoOfBook"] = NoOfBook;
		ins["LobbyDepth"] = LobbyDepth;
		ins["LobbyWidth"] = LobbyWidth;
		ins["FrontWallThickness"] = FrontWallThickness;
		ins["LobbyCeilingSlabHeight"] = LobbyCeilingSlabHeight;
		ins["FloorsAboveGround"] = StoreysAboveGround;
		ins["FloorsBelowGround"] = StoreysBelowGround;
		ins["LiftShaftArrangement"] = (ULONG)LiftShaftArrang;
		ins["LobbyArrangement"] = (ULONG)LobbyArrangement;
		ins["SideWallThickness"] = SideWallThickness;		// seven variables from here added 15/09/11
		ins["ShaftWallThickness"] = ShaftWallThickness;
		ins["IntDivBeamWidth"] = IntDivBeamWidth;
		ins["IntDivBeamHeight"] = IntDivBeamHeight;
		ins["MachRoomSlab"] = MachRoomSlab;
		ins["LiftBeamHeight"] = LiftBeamHeight;
		ins["Structure"] = (ULONG)Structure;
		ins.execute();

		// retrieve the Building ID
		sel = db.select("SELECT SCOPE_IDENTITY()");
		nBuildingID = sel[(short)0];

		// store lift data
		for (ULONG i = 0; i < NoOfShafts; i++)
		{
			ins = db.insert("AVShafts");
			ins["BuildingID"] = nBuildingID;
			ins["ShaftID"] = GetShaft(i)->ShaftID;
			ins["Capacity"] = GetShaft(i)->Capacity;
			ins["Speed"] = GetShaft(i)->Speed;
			ins["Acceleration"] = GetShaft(i)->Acceleration;
			ins["Jerk"] = GetShaft(i)->Jerk;
			ins["DoorType"] = (ULONG)GetShaft(i)->DoorType;
			ins["LiftDoorHeight"] = GetShaft(i)->LiftDoorHeight;
			ins["LiftDoorWidth"] = GetShaft(i)->LiftDoorWidth;
			ins["CarHeight"] = GetShaft(i)->CarHeight;
			ins["TypeOfLift"] = (ULONG)GetShaft(i)->TypeOfLift;
			ins["NumberOfLifts"] = GetShaft(i)->NumberOfLifts;
			ins["MachRoomSlab"] = MachRoomSlab;		// kept for backward compatibility - 15/09/11
			ins["LiftBeamHeight"] = LiftBeamHeight;	// kept for backward compatibility - 15/09/11
			ins["NoOfCarEntrances"] = (ULONG)GetShaft(i)->NoOfCarEntrances;
			ins["CounterWeightPosition"] = (ULONG)GetShaft(i)->CounterWeightPosition;
			ins["Structure"] = (ULONG)Structure;		// kept for backward compatibility - 15/09/11
			ins["CarWidth"] = GetShaft(i)->CarWidth;
			ins["CarDepth"] = GetShaft(i)->CarDepth;
			ins["ShaftWidth"] = GetShaft(i)->ShaftWidth;
			ins["ShaftDepth"] = GetShaft(i)->ShaftDepth;
			ins["IntDivBeam"] = IntDivBeamWidth;		// kept for backward compatibility - 15/09/11
			ins["PitDepth"] = GetShaft(i)->PitDepth;
			ins["OverallHeight"] = GetShaft(i)->OverallHeight;
			ins["HeadRoom"] = GetShaft(i)->HeadRoom;
			ins["MachRoomHeight"] = GetShaft(i)->MachRoomHeight;
			ins["MachRoomExt"] = GetShaft(i)->MachRoomExt;
			ins["PreOperTime"] = GetShaft(i)->PreOperTime;
			ins["OpeningTime"] = GetShaft(i)->OpeningTime;
			ins["ClosingTime"] = GetShaft(i)->ClosingTime;
			ins["MotorStartDelay"] = GetShaft(i)->MotorStartDelay;
			ins["LoadingTime"] = GetShaft(i)->LoadingTime;
			ins["UnloadingTime"] = GetShaft(i)->UnloadingTime;
			ins["FloorsServed"] = GetShaft(i)->FloorsServed;
			ins.execute();
		}

		// store floor data
		for (ULONG i = 0; i < GetStoreyCount(); i++)
		{
			ins = db.insert("AVFloors");
			ins["BuildingID"] = nBuildingID;
			ins["FloorID"] = GetStorey(i)->StoreyID;
			ins["HeightValue"] = GetStorey(i)->HeightValue;
			ins["Area"] = GetStorey(i)->Area;
			ins["PopDensity"] = GetStorey(i)->PopDensity;
			ins["Absentee"] = GetStorey(i)->Absentee;
			ins["StairFactor"] = GetStorey(i)->StairFactor;
			ins["Escalator"] = GetStorey(i)->Escalator;
			ins["Name"] = GetStorey(i)->Name;
			ins.execute();
		}
	}
    catch(HRESULT h)
    {
		return Log(ERROR_COM, h);
	}
    catch(_com_error &ce)
    {
		return Log(ERROR_DB, ce);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Database Load

HRESULT CBuilding::LoadFromConsole(CDataBase db, ULONG nSimulationID)
{
	if (!db) return db;
	try
	{
		CDataBase::SELECT sel;

		// Query for Building (for building id and name - the latter not very useful)
		sel = db.select("SELECT BuildingId, BuildingName FROM Buildings WHERE SimulationId=%d", nSimulationID);
		if (!sel) return Log(ERROR_BUILDING);
		sel >> *this;
		nBuildingID = (*this)["BuildingId"];
		BuildingName = (*this)["BuildingName"];
		
		// Query for Layout Details (for almost full lobby info)
		sel = db.select("SELECT * FROM LayoutDetailsDataSets WHERE SimulationId=%d", nSimulationID);
		if (!sel) return Log(ERROR_BUILDING);

		std::wstring strLobbyArrangement = sel[L"LobbyArrangement"];
		PosLiftBookM = sel[L"BookingPointPositionM"];
		NoOfBook = sel[L"NoBookingPoints"];
		LobbyCeilingSlabHeight = sel[L"LiftLobbyCeilingHeight"];
		std::wstring strLiftShaftArrang = sel[L"LiftShaftArrangement"];
		LobbyDepth = sel[L"LobbyDepth"];
		FrontWallThickness = sel[L"FrontWallThickness"];
		// additionally, NoOfShafts, StoreysAboveGround and StoreysBelowGround are loaded from other tables

		std::wstring strStructure = sel[L"StructureMaterial"];
		MachRoomSlab = sel[L"MachineRoomSlab"];
		LiftBeamHeight = sel[L"LiftingBeamHeight"];
		IntDivBeamWidth = sel[L"IntermediateDividingBeam"];
		
		// IntDivBeamHeight is calculated while no information in the CONSOLE database yet...
		IntDivBeamHeight = IntDivBeamWidth * 250.0f / 150.0f;

		if (strLobbyArrangement == L"Through") LobbyArrangement = LOBBY_THROUGH;
		else if (strLobbyArrangement == L"Open Plan") LobbyArrangement = LOBBY_OPENPLAN;
		else if (strLobbyArrangement == L"Dead End on the Left") LobbyArrangement = LOBBY_DEADEND_LEFT;
		else if (strLobbyArrangement == L"Dead End on the Right") LobbyArrangement = LOBBY_DEADEND_RIGHT;
		else return Log(ERROR_UNRECOGNISED_STRING, L"lobby arrangement", strLobbyArrangement.c_str());

		if (strLiftShaftArrang == L"Inline") LiftShaftArrang = SHAFT_INLINE;
		else if (strLiftShaftArrang == L"Opposite") LiftShaftArrang = SHAFT_OPPOSITE;
		else return Log(ERROR_UNRECOGNISED_STRING, L"lift shaft arrangement", strLiftShaftArrang.c_str());

		if (strStructure == L"Concrete") Structure = STRUCT_CONCRETE;
		else if (strStructure == L"Steel") Structure = STRUCT_STEEL;
		else return Log(ERROR_UNRECOGNISED_STRING, L"structure material", strStructure.c_str());

		if (LobbyArrangement == LOBBY_OPENPLAN && LiftShaftArrang == SHAFT_OPPOSITE)
			LobbyArrangement = LOBBY_THROUGH;

		// Query for Lift Dataset (for NoOfShafts only)
		sel = db.select("SELECT * FROM LiftDataSets WHERE SimulationId=%d", nSimulationID);
		if (!sel) return Log(ERROR_BUILDING);
		NoOfShafts = sel[L"NoOfLifts"];


		// Query for Floor Dataset (for StoreysAboveGround and StoreysBelowGround)
		sel = db.select("SELECT * FROM FloorDataSets WHERE SimulationId=%d", nSimulationID);
		if (!sel) return Log(ERROR_BUILDING);
		StoreysAboveGround = sel[L"NoFloorsAboveMain"];
		StoreysAboveGround++;
		StoreysBelowGround = sel[L"NoFloorsBelowMain"];
		
		// prepare buffers for shafts and storeys
		CreateShafts();
		CreateStoreys();

		// Query for Shaft Data
		sel = db.select("SELECT * FROM Lifts WHERE BuildingId=%d ORDER BY Number", nBuildingID);
		for (AVULONG i = 0; i < NoOfShafts && sel; i++, sel++)
		{
			SHAFT *pShaft = GetShaft(i);

			pShaft->ShaftID = sel[L"Number"];
			pShaft->Capacity = sel[L"Capacity"];
			pShaft->Speed = sel[L"Speed"];
			pShaft->Acceleration = sel[L"Acceleration"];
			pShaft->Jerk = sel[L"Jerk"];
			std::wstring strDoorType = sel[L"DoorType"];
			pShaft->LiftDoorHeight = sel[L"DoorHeight"];
			pShaft->LiftDoorWidth = sel[L"DoorWidth"];
			pShaft->CarHeight = sel[L"CarHeight"];
			std::wstring strTypeOfLift = sel[L"LiftType"];
			std::wstring strNoOfCarEntrances = sel[L"NoOfCarEntrance"];
			std::wstring strCounterWeightPosition = sel[L"CounterWeightPosition"];
			pShaft->CarWidth = sel[L"CarWidth"];
			pShaft->CarDepth = sel[L"CarDepth"];
			pShaft->ShaftWidth = sel[L"ShaftWidth"];
			pShaft->ShaftDepth = sel[L"ShaftDepth"];
			pShaft->PitDepth = sel[L"LiftPitDepth"];
			pShaft->OverallHeight = sel[L"OverallCarHeight"];
			pShaft->HeadRoom = sel[L"LiftHeadroom"];
			pShaft->MachRoomHeight = sel[L"MachineRoomHeight"];

			// new values (+ Acceleration and Jerk)
			pShaft->PreOperTime = (AVULONG)((AVFLOAT)sel[L"PreOperTime"] * 1000);
			pShaft->OpeningTime = (AVULONG)((AVFLOAT)sel[L"OpeningTime"] * 1000);
			pShaft->ClosingTime = (AVULONG)((AVFLOAT)sel[L"ClosingTime"] * 1000);
			pShaft->MotorStartDelay = (AVULONG)((AVFLOAT)sel[L"MotorStartDelay"] * 1000);
			pShaft->LoadingTime = (AVULONG)((AVFLOAT)sel[L"LoadingTime"] * 1000);
			pShaft->UnloadingTime = (AVULONG)((AVFLOAT)sel[L"UnloadingTime"] * 1000);
			pShaft->FloorsServed = sel[L"FloorsServed"];

			if (strDoorType == L"Centre") pShaft->DoorType = DOOR_CENTRE;
			else if (strDoorType == L"Side") pShaft->DoorType = DOOR_SIDE;
			else return Log(ERROR_UNRECOGNISED_STRING, L"door type", strDoorType.c_str());
			
			if (strTypeOfLift == L"Single Deck") pShaft->TypeOfLift = LIFT_SINGLE_DECK;
			else if (strTypeOfLift == L"Double Deck") pShaft->TypeOfLift = LIFT_DOUBLE_DECK;
			else if (strTypeOfLift == L"Multi Car") pShaft->TypeOfLift = LIFT_MULTI_CAR;
			else return Log(ERROR_UNRECOGNISED_STRING, L"type of lift", strTypeOfLift.c_str());

			if (strNoOfCarEntrances == L"Front") pShaft->NoOfCarEntrances = CAR_FRONT;
			else if (strNoOfCarEntrances == L"Rear") pShaft->NoOfCarEntrances = CAR_REAR;
			else if (strNoOfCarEntrances == L"Both") pShaft->NoOfCarEntrances = CAR_BOTH;
			else return Log(ERROR_UNRECOGNISED_STRING, L"configuration of car entrances", strNoOfCarEntrances.c_str());

			if (strCounterWeightPosition == L"Rear") pShaft->CounterWeightPosition = CNTRWEIGHT_REAR;
			else if (strCounterWeightPosition == L"Side") pShaft->CounterWeightPosition = CNTRWEIGHT_SIDE;
			else return Log(ERROR_UNRECOGNISED_STRING, L"counterweight position", strCounterWeightPosition.c_str());
			
			// not currently supported by the Console database:
			pShaft->NumberOfLifts = 1;
			pShaft->MachRoomExt = -1;
		}

		// Query for Storey Data
		sel = db.select("SELECT * FROM Floors WHERE BuildingId=%d ORDER BY GroundIndex", nBuildingID);
		for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		{
			STOREY *pStorey = GetStorey(i);
			pStorey->StoreyID = sel[L"GroundIndex"];
			pStorey->HeightValue = sel[L"FloorHeight"];
			pStorey->HeightValue *= 1000;

			// new values
			pStorey->Name = sel[L"Name"];
			pStorey->Area = sel[L"Area"];
			pStorey->PopDensity = sel[L"PopDensity"];
			pStorey->Absentee = sel[L"Absentee"];
			pStorey->StairFactor = sel[L"StairFactor"];
			pStorey->Escalator = sel[L"Escalator"];
		}
	}
	catch(_com_error &ce)
	{
		return Log(ERROR_DB, ce);
	}
	catch (HRESULT h)
	{
		return Log(ERROR_COM, h);
	}

	Resolve();

	// Some tests
	if (!IsValid())
		return Log(ERROR_BUILDING);

	return S_OK;
}

HRESULT CBuilding::LoadFromVisualisation(CDataBase db, ULONG nProjectID)
{
	if (!db) return db;
	try
	{
		CDataBase::SELECT sel;

		// Query for Lobby Data
		sel = db.select("SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
		if (!sel) return Log(ERROR_DATA_NOT_FOUND);

		nBuildingID = sel[L"ID"];
		BuildingName = sel[L"BuildingName"];
		BuildingName = sel[L"BuildingName"];
		NoOfShafts = sel[L"NoOfShafts"];
		PosLiftBookM = sel[L"PosLiftBookM"];
		NoOfBook = sel[L"NoOfBook"];
		LobbyDepth = sel[L"LobbyDepth"];
		LobbyWidth = sel[L"LobbyWidth"];
		FrontWallThickness = sel[L"FrontWallThickness"];
		LobbyCeilingSlabHeight = sel[L"LobbyCeilingSlabHeight"];
		StoreysAboveGround = sel[L"FloorsAboveGround"];
		StoreysBelowGround = sel[L"FloorsBelowGround"];
		LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)sel[L"LiftShaftArrangement"];
		LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)sel[L"LobbyArrangement"];

		// prepare buffers for shafts and storeys
		CreateShafts();
		CreateStoreys();

		// Query for Lobby Data
		sel = db.select("SELECT * FROM AVShafts WHERE BuildingID=%d ORDER BY ShaftID", nBuildingID);

		for (AVULONG i = 0; i < NoOfShafts && sel; i++, sel++)
		{
			GetShaft(i)->ShaftID = sel[L"ShaftID"];
			GetShaft(i)->Capacity = sel[L"Capacity"];
			GetShaft(i)->Speed = sel[L"Speed"];
			GetShaft(i)->DoorType = (DOOR_TYPE)(ULONG)sel[L"DoorType"];
			GetShaft(i)->LiftDoorHeight = sel[L"LiftDoorHeight"];
			GetShaft(i)->LiftDoorWidth = sel[L"LiftDoorWidth"];
			GetShaft(i)->CarHeight = sel[L"CarHeight"];
			GetShaft(i)->TypeOfLift = (TYPE_OF_LIFT)(ULONG)sel[L"TypeOfLift"];
			GetShaft(i)->NumberOfLifts = sel[L"NumberOfLifts"];
			GetShaft(i)->NoOfCarEntrances = (CAR_ENTRANCES)(ULONG)sel[L"NoOfCarEntrances"];
			GetShaft(i)->CounterWeightPosition = (CNTRWEIGHT_POS)(ULONG)sel[L"CounterWeightPosition"];
			GetShaft(i)->CarWidth = sel[L"CarWidth"];
			GetShaft(i)->CarDepth = sel[L"CarDepth"];
			GetShaft(i)->ShaftWidth = sel[L"ShaftWidth"];
			GetShaft(i)->ShaftDepth = sel[L"ShaftDepth"];
			GetShaft(i)->PitDepth = sel[L"PitDepth"];
			GetShaft(i)->OverallHeight = sel[L"OverallHeight"];
			GetShaft(i)->HeadRoom = sel[L"HeadRoom"];
			GetShaft(i)->MachRoomHeight = sel[L"MachRoomHeight"];
			GetShaft(i)->MachRoomExt = sel[L"MachRoomExt"];

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
		}

		// Query for Storey Data
		sel = db.select("SELECT * FROM AVFloors WHERE BuildingID=%d ORDER BY FloorID", nBuildingID);

		for (AVULONG i = 0; i < GetStoreyCount() && sel; i++, sel++)
		{
			GetStorey(i)->StoreyID = sel[L"FloorID"];
			GetStorey(i)->HeightValue = sel[L"HeightValue"];
		}
	}
	catch(_com_error &ce)
	{
		return Log(ERROR_DB, ce);
	}

	Resolve();
	
	// Some tests
	if (!IsValid())
		return Log(ERROR_DATA_NOT_FOUND);

	return S_OK;
}

