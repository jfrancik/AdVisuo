// BaseLiftGroup.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseLiftGroup.h"
#include "BaseSimClasses.h"

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup

CLiftGroup::CLiftGroup(CProject *pProject, AVULONG nIndex) : m_pProject(pProject), m_nIndex(nIndex)
{
	m_nId = 0;
	m_nProjectId = 0;
	m_pSim = NULL;
	m_fScale = 1;
	m_nBasementStoreyCount = 0;
	m_pnShaftCount[0] = m_pnShaftCount[1] = 0;
	m_LiftShaftArrang = SHAFT_INLINE;
	m_LobbyArrangement = LOBBY_OPENPLAN;
	m_pMachineRoom = NULL;
	m_pPit = NULL;
	m_fPitLevel = m_fMachineRoomLevel = 0;
	m_fLiftingBeamHeight = m_fLiftingBeamWidth = 0;
}

CLiftGroup::~CLiftGroup()
{
	DeleteStoreys();
	DeleteShafts();
	DeleteLifts();
	DeleteExtras();
	DeleteSim();
}

BOX CLiftGroup::GetTotalAreaBox()
{
	BOX box = GetBox();
	box.SetLeft(0); box.SetRight(0); box.SetLeftThickness(0); box.SetRightThickness(0);
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		BOX s = GetShaft(i)->GetBox();
		box += s;
	}
	return box;
}

CLiftGroup::STOREY *CLiftGroup::AddStorey()
{
	STOREY *p = CreateStorey(m_storeys.size());
	m_storeys.push_back(p);
	return p;
}

void CLiftGroup::DeleteStoreys()
{
	for each (STOREY *pStorey in m_storeys)
		delete pStorey;
	m_storeys.clear();
}

CLiftGroup::SHAFT *CLiftGroup::AddShaft()
{
	SHAFT *p = CreateShaft(m_shafts.size());
	m_shafts.push_back(p);
	return p;
}

void CLiftGroup::DeleteShafts()
{
	for each (SHAFT *pShaft in m_shafts)
		delete pShaft;
	m_shafts.clear();
}

void CLiftGroup::AddExtras()
{
	DeleteExtras();
	m_pMachineRoom = CreateMachineRoom();
	m_pPit = CreatePit();
}

void CLiftGroup::DeleteExtras()
{
	if (m_pMachineRoom) delete m_pMachineRoom;
	m_pMachineRoom = NULL;
	if (m_pPit) delete m_pPit;
	m_pPit = NULL;
}

CLiftGroup::LIFT *CLiftGroup::AddLift()
{
	LIFT *p = CreateLift(m_lifts.size());
	m_lifts.push_back(p);
	return p;
}

void CLiftGroup::DeleteLifts()
{
	for each (LIFT *pLift in m_lifts)
		delete pLift;
	m_lifts.clear();
}

CSim *CLiftGroup::AddSim()
{
	m_pSim = CreateSim();
	if (m_pSim)
	{
		m_pSim->SetLiftGroup(this);
		m_pSim->SetIndex(GetIndex());
	}
	return m_pSim;
}

void CLiftGroup::DeleteSim()
{
	if (m_pSim) delete m_pSim;
}

void CLiftGroup::ResolveMe()
{
	SetId(ME[L"ID"]);
	SetIndex(ME[L"LiftGroupIndex"]);
	SetProjectId(ME[L"ProjectId"]);
	m_nBasementStoreyCount = (ULONG)ME[L"NumberOfBasementStoreys"];

	AVULONG nLifts = 0; 
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		GetShaft(i)->SetLiftRange(nLifts, max(1, (AVULONG)(*GetShaft(i))[L"LiftsPerShaft"]));
		nLifts += GetShaft(i)->GetLiftCount();
	}
	for (AVULONG i = 0; i < nLifts; i++)
		AddLift();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		for (AVULONG j = GetShaft(i)->GetLiftBegin(); j < GetShaft(i)->GetLiftEnd(); j++)
			GetLift(j)->SetShaftId(i);
}

void CLiftGroup::ConsoleCreate()
{
	// resolve vars
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangementId"];
	m_strName = ME[L"Name"];

	if (m_LobbyArrangement == LOBBY_OPENPLAN && m_LiftShaftArrang == SHAFT_OPPOSITE)
		m_LobbyArrangement = LOBBY_THROUGH;
	ME[L"LobbyArrangementId"] = (AVLONG)m_LobbyArrangement;

	AVFLOAT fLobbyCeilingSlabHeight = 1000;	// ME[L"LiftLobbyCeilingHeight"];
	AVFLOAT fLobbyDepth = ME[L"FrontLobbyWidth"];
	AVFLOAT fFrontWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fSideWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fShaftWallThicknessRear = ME[L"ShaftWallThicknessRear"];
	AVFLOAT fShaftWallThicknessSide = ME[L"ShaftWallThicknessSide"];
	AVFLOAT fMachineRoomSlabThickness = ME[L"MachineRoomSlabThickness"];

	AVFLOAT fRearLobbyDepth = ME[L"RearLobbyWidth"];	// NOT USED YET!!!
	m_fLiftingBeamHeight = ME[L"LiftingBeamHeight"];
	m_fLiftingBeamWidth = 200;

	
	// calculate width of the lobby if lifts in-line, no side walls
	AVFLOAT fLobbyWidth = 0;
	m_pnShaftCount[0] = GetShaftCount();
	m_pnShaftCount[1] = 0;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		fLobbyWidth += (i == 0 ? 0 : max(GetShaft(i-1)->GetRawBeamWidth(), GetShaft(i)->GetRawBeamWidth())) + GetShaft(i)->GetRawWidth();

	// if lifts opposite, sort the lifts into two lines
	if (GetLiftShaftArrang() == SHAFT_OPPOSITE)
	{
		m_pnShaftCount[0] = 0;
		m_pnShaftCount[1] = GetShaftCount();
		AVFLOAT fLineWidth[] = { 0, fLobbyWidth };			// all lifts in line 1
		for (AVULONG i = 0; i < GetShaftCount(); i++)
		{
			SHAFT *pShaft = GetShaft(i);
			// take the first lift in line 1 to line 0
			if (i > 0) fLineWidth[0] += max(GetShaft(i-1)->GetRawBeamWidth(), GetShaft(i)->GetRawBeamWidth());
			fLineWidth[0] += pShaft->GetRawWidth();
			fLineWidth[1] -= pShaft->GetRawWidth();
			if (i < GetShaftCount() - 1) fLineWidth[1] -= max(GetShaft(i)->GetRawBeamWidth(), GetShaft(i+1)->GetRawBeamWidth());
			if (max(fLineWidth[0], fLineWidth[1]) > fLobbyWidth)
				break;
			fLobbyWidth = max(fLineWidth[0], fLineWidth[1]);
			m_pnShaftCount[0]++;
			m_pnShaftCount[1]--;
		}
	}

	// add shaft side walls thickness
	fLobbyWidth += 2 * fShaftWallThicknessSide;

	// calculate lobby box
	AVFLOAT fLt = 0; if (m_LobbyArrangement == LOBBY_DEADEND_LEFT)  fLt = fSideWallThickness;
	AVFLOAT fRt = 0; if (m_LobbyArrangement == LOBBY_DEADEND_RIGHT) fRt = fSideWallThickness;

	m_box = BOX(-fLobbyWidth/2 + fLt, -fLobbyDepth/2, 0, fLobbyWidth - fLt - fRt, fLobbyDepth, 0);
	m_box.SetThickness(fLt, fRt, fFrontWallThickness, (m_LobbyArrangement == LOBBY_OPENPLAN) ? 0 : fFrontWallThickness, 2, fLobbyCeilingSlabHeight);

	// Resolve Shafts and Storeys

	// 1st line of shafts
	AVFLOAT fShaftPosX = GetBox().LeftExt();
	AVFLOAT fShaftPosY = GetBox().FrontExt();
	AVFLOAT fPrevDepth = 0;
	SHAFT *pPrevShaft = NULL;
	for (AVULONG i = 0; i < GetShaftCount(0); i++)
	{
		SHAFT *pShaft = GetShaft(i);

		// setup basic structure
		pShaft->ConsoleCreate(i, 0, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessSide);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
		}
		else if (fDepth - fPrevDepth < -fShaftWallThicknessRear)	// note: both values are negative here!
		{	// deeper than previous: build a shorter beam and add a wall section
			pPrevShaft->ConsoleCreateBeam(SIDE_RIGHT, pShaft);
			pShaft->Move(pPrevShaft->GetBox().RightThickness(), 0, 0);
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessRear, fPrevDepth - fShaftWallThicknessRear);
		}
		else if (fDepth - fPrevDepth > fShaftWallThicknessRear)	// note: both values are negative here!
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConsoleCreateBeam(SIDE_LEFT, pPrevShaft);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
			GetShaft(i-1)->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessRear, fDepth - fShaftWallThicknessRear);
		}
		else /* if (fDepth == fPrevDepth) */
		{
			// same depth as previous: just build a beam
			pShaft->ConsoleCreateBeam(SIDE_LEFT, pPrevShaft);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
		}
		if (i == GetShaftCount(0) - 1)
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessSide);

		// move on
		fPrevDepth = fDepth;
		fShaftPosX = pShaft->GetBox().Right();
		pPrevShaft = pShaft;
	}
	
	// 2nd line of shafts
	fShaftPosX = GetBox().RightExt();
	fShaftPosY = GetBox().RearExt();
	fPrevDepth = 0;
	pPrevShaft = NULL;
	for (AVULONG i = GetShaftCount(0); i < GetShaftCount(); i++)
	{
		SHAFT *pShaft = GetShaft(i);

		// setup basic structure
		pShaft->ConsoleCreate(i, 1, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessSide);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
		}
		else if (fDepth - fPrevDepth > fShaftWallThicknessRear)
		{	// deeper than previous: build a shorter beam and add a wall section
			pPrevShaft->ConsoleCreateBeam(SIDE_LEFT, pShaft);
			pShaft->Move(-pPrevShaft->GetBox().LeftThickness(), 0, 0);
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessRear, fPrevDepth + fShaftWallThicknessRear);
		}
		else if (fDepth - fPrevDepth < -fShaftWallThicknessRear)
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConsoleCreateBeam(SIDE_RIGHT, pPrevShaft);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
			GetShaft(i-1)->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessRear, fDepth + fShaftWallThicknessRear);
		}
		else /* if (fDepth == fPrevDepth) */
		{
			// same depth as previous: just build a beam
			pShaft->ConsoleCreateBeam(SIDE_RIGHT, pPrevShaft);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
		}
		if (i == GetShaftCount() - 1)
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessSide);

		// move on
		fPrevDepth = fDepth;
		fShaftPosX = pShaft->GetBox().Left();
		pPrevShaft = pShaft;
	}

	// Create Storeys
	AVFLOAT fLevel = 0;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);
		pStorey->ConsoleCreate(i, fLevel);
		fLevel += pStorey->GetHeight();
	}

	// calculate machine room box & level
	m_boxMachineRoom = BOX(0, 0, 0, 0, 0, 0);
	AVFLOAT fHeadroom = 0;
	m_boxMachineRoom = GetTotalAreaBox();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		SHAFT *pShaft = GetShaft(i);
//		if (pShaft->GetBox().Left() < m_boxMachineRoom.Left()) m_boxMachineRoom.SetLeft(pShaft->GetBox().Left());
//		if (pShaft->GetBox().Right() > m_boxMachineRoom.Right()) m_boxMachineRoom.SetRight(pShaft->GetBox().Right());
//		if (pShaft->GetBox().Front() < m_boxMachineRoom.Front()) m_boxMachineRoom.SetFront(pShaft->GetBox().Front());
//		if (pShaft->GetBox().Rear() < m_boxMachineRoom.Front()) m_boxMachineRoom.SetFront(pShaft->GetBox().Rear());
//		if (pShaft->GetBox().Rear() > m_boxMachineRoom.Rear()) m_boxMachineRoom.SetRear(pShaft->GetBox().Rear());
		if (((AVFLOAT)(*pShaft)[L"MachineRoomHeight"]) > m_boxMachineRoom.Height()) m_boxMachineRoom.SetHeight((AVFLOAT)(*pShaft)[L"MachineRoomHeight"]);
		if (((AVFLOAT)(*pShaft)[L"Headroom"]) > fHeadroom) fHeadroom = (AVFLOAT)(*pShaft)[L"Headroom"];
	}
	m_boxMachineRoom.SetThickness(fShaftWallThicknessSide, fShaftWallThicknessSide, fShaftWallThicknessRear, fShaftWallThicknessRear, fMachineRoomSlabThickness, 0);
	m_fMachineRoomLevel = max(GetStorey(GetHighestStoreyServed())->GetRoofLevel(), GetStorey(GetHighestStoreyServed())->GetLevel() + fHeadroom);
	m_fMachineRoomLevel += + fMachineRoomSlabThickness;

	if (GetMachineRoom()) GetMachineRoom()->ConsoleCreate();

	// calculate pit level box & level
	m_boxPit = m_box;
	m_fPitLevel = 0;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		AVFLOAT fPitLevel = (*GetShaft(i))[L"PitDepth"];
		if (fPitLevel > m_fPitLevel) m_fPitLevel = fPitLevel;
	}
	m_fPitLevel = -m_fPitLevel;
	if (GetPit()) GetPit()->ConsoleCreate();

	// Final updates and amendments
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->ConsoleCreateAmend();

	// update ME
	ME[L"Line1"] = m_pnShaftCount[0];
	ME[L"Line2"] = m_pnShaftCount[1];
	ME[L"BoxLobby"] = m_box.Stringify();
	ME[L"BoxMachineRoom"] = m_boxMachineRoom.Stringify();
	ME[L"MachineRoomLevel"] = m_fMachineRoomLevel;
	ME[L"BoxPit"] = m_boxPit.Stringify();
	ME[L"PitLevel"] = m_fPitLevel;

	erase(L"FrontLobbyWidth");
	erase(L"FrontWallThickness");
	erase(L"ShaftWallThicknessRear");
	erase(L"ShaftWallThicknessSide");
	erase(L"MachineRoomSlabThickness");
}

void CLiftGroup::Create()
{
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangementId"];
	m_strName = ME[L"Name"];
	m_pnShaftCount[0] = ME[L"Line1"];
	m_pnShaftCount[1] = ME[L"Line2"];
	m_box.ParseFromString(ME[L"BoxLobby"]);
	m_boxMachineRoom.ParseFromString(ME[L"BoxMachineRoom"]);
	m_fMachineRoomLevel = ME[L"MachineRoomLevel"];
	m_boxPit.ParseFromString(ME[L"BoxPit"]);
	m_fPitLevel = ME[L"PitLevel"];
	m_fLiftingBeamHeight = ME[L"LiftingBeamHeight"];
	m_fLiftingBeamWidth = 200;
	
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Create();

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Create();

	if (GetMachineRoom()) GetMachineRoom()->Create();
	if (GetPit()) GetPit()->Create();
}

void CLiftGroup::Scale(AVFLOAT f)
{
	m_fScale *= f;
	m_box.Scale(f);
	m_boxPit.Scale(f);
	m_fMachineRoomLevel *= f;
	m_fPitLevel *= f;
	m_fLiftingBeamHeight *= f;
	m_fLiftingBeamWidth *= f;
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Scale(f);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Scale(f);
	if (GetMachineRoom()) GetMachineRoom()->Scale(f);
	GetBoxMachineRoom().Scale(f);
	if (GetPit()) GetPit()->Scale(f);
	GetBoxPit().Scale(f);
}

void CLiftGroup::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_box.Move(x, y, z);
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Move(x, y, z);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Move(x, y, z);
	if (GetMachineRoom()) GetMachineRoom()->Move(x, y, z);
	if (GetPit()) GetPit()->Move(x, y, z);
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::STOREY

void CLiftGroup::STOREY::ConsoleCreate(AVULONG nId, AVFLOAT fLevel)
{
//	m_nId = ME[L"GroundIndex"];
	m_fLevel = fLevel;
	m_strName = ME[L"Name"];
	m_box = GetLiftGroup()->m_box;
	m_box.SetHeight((AVFLOAT)ME[L"FloorHeight"] * 1000.0f- m_box.UpperThickness());

	ME[L"FloorId"] = GetId();
	ME[L"FloorLevel"] = m_fLevel;
	ME[L"Box"] = m_box.Stringify();
	erase(L"FloorHeight");
	erase(L"GroundIndex");
}

void CLiftGroup::STOREY::Create()
{
	m_strName = ME[L"Name"];

	m_fLevel = ME[L"FloorLevel"];
	m_box.ParseFromString(ME[L"Box"]);
}

void CLiftGroup::STOREY::Scale(AVFLOAT f)
{
	m_fLevel *= f;
	m_box.Scale(f);
}

void CLiftGroup::STOREY::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_fLevel += z;
	m_box.Move(x, y, z);
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::MACHINEROOM

void CLiftGroup::MACHINEROOM::ConsoleCreate()
{
	m_nId = 9999;
	m_fLevel = GetLiftGroup()->GetMachineRoomLevel();
	m_strName = L"Machine Room";
	m_box = GetLiftGroup()->GetBoxMachineRoom();
}

void CLiftGroup::MACHINEROOM::Create()
{
	m_nId = 9999;
	m_fLevel = GetLiftGroup()->GetMachineRoomLevel();
	m_strName = L"Machine Room";
	m_box = GetLiftGroup()->GetBoxMachineRoom();
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::PIT

void CLiftGroup::PIT::ConsoleCreate()
{
	m_nId = 9998;
	m_fLevel = GetLiftGroup()->GetPitLevel();
	m_strName = L"Lift Pit Level";
	m_box = GetLiftGroup()->GetBoxPit();
}

void CLiftGroup::PIT::Create()
{
	m_nId = 9998;
	m_fLevel = GetLiftGroup()->GetPitLevel();
	m_strName = L"Lift Pit Level";
	m_box = GetLiftGroup()->GetBoxPit();
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::SHAFT

BOX &CLiftGroup::SHAFT::GetBox(CLiftGroup::SHAFT::SHAFT_BOX n, AVULONG i)
{
	switch (n)
	{
	case BOX_SHAFT:		return m_boxShaft;
	case BOX_DOOR:		return m_boxDoor[i];
	case BOX_CAR:		return m_boxCar;
	case BOX_CARDOOR:	return m_boxCarDoor[i];
	case BOX_CW:		return m_boxCwt;
	case BOX_GOVERNOR:	return m_boxGovernor;
	case BOX_LADDER:	return m_boxLadder;
	default:			return m_boxShaft;
	}
}

void CLiftGroup::SHAFT::ConsoleCreate(AVULONG nId, AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY, AVFLOAT fFrontWall, AVFLOAT fRearWall)
{
	m_nId = nId;
	//m_nId = ME[L"LiftNumber"];
	//m_nId = ME[L"LiftIndex"];
	m_nShaftLine = nLine;

	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];
	m_nLiftType = ME[L"LiftTypeId"];
	m_nDoorType = ME[L"DoorTypeId"];

	m_nOpeningTime = (AVULONG)((float)ME[L"OpeningTime"] * 1000);
	m_nClosingTime = (AVULONG)((float)ME[L"ClosingTime"] * 1000);

	m_strStoreysServed = ME[L"StoreysServed"];

	// ### To resolve the problem quickly
	ME[L"LiftsPerShaft"] = (AVULONG)1;

	m_nLiftCount = max(1, (AVULONG)ME[L"LiftsPerShaft"]);

	AVFLOAT ShaftWidth = ME[L"ShaftWidth"];
	AVFLOAT ShaftDepth = ME[L"ShaftDepth"];
	AVFLOAT CarDepth = ME[L"CarDepth"];
	AVFLOAT CarWidth = ME[L"CarWidth"];
	AVFLOAT CarHeight = ME[L"CarCeilingHeight"];
	AVFLOAT LiftDoorHeight = ME[L"DoorHeight"];
	AVFLOAT LiftDoorWidth = ME[L"DoorWidth"];

	AVFLOAT fVoidRear = ME[L"ClearanceCarToShaftWallOnDepth"];
	AVFLOAT fVoidLeft = ME[L"ClearanceCarToShaftWallOnWidthLHS"];
	AVFLOAT fVoidRight = ME[L"ClearanceCarToShaftWallOnWidthRHS"];
	AVFLOAT fCarRearWallThickness = ME[L"CarDepthWallThickness"];
	AVFLOAT fCarFrontWallThickness = ME[L"CarFrontReturnDepth"];
	AVFLOAT fCarSideWallThickness = ME[L"CarSideWallThickness"];
	AVFLOAT fLightingVoidHeight = ME[L"LightingVoidHeight"];
	AVFLOAT fCarDoorDepth = ME[L"CarDoorDepth"];
	AVFLOAT fLandingDoorDepth = ME[L"LandingDoorDepth"];

	AVFLOAT fCarFloorThickness = 150;

	// Capacity/Speed related values
	AVFLOAT fCapacity = ME[L"Capacity"];
	AVFLOAT fSpeed = ME[L"Speed"];
	AVULONG nCwtPos = ME[L"CounterWeightPositionId"];

	AVFLOAT cwtw = 200;		// cwt width (depth)
	AVFLOAT cwtl = 1000;	// cwt length (width)
	AVFLOAT cwtx = 50;		// cwt extra from the shaft wall
	AVFLOAT slingb = 400;	// sling bottom height
	AVFLOAT slingt = 1000;	// sling top height
	if (fSpeed >= 2.5) cwtl = 1200; if (fSpeed >= 4) cwtl = 1500;
	if (fSpeed >= 4) cwtx = 100;
	if (fCapacity >= 1275) slingb = 600; if (fCapacity >= 1800) slingb = 800;

	// determine machine & panel types
	                          m_nMachineType = 1; m_nPanelCtrlType = 1; m_nPanelDrvType = 1; m_nPanelIsoType = 1; m_fPanelCtrlWidth = 700; m_fPanelDrvWidth = 1000; m_fPanelIsoWidth = 1000;
	if (fCapacity >= 1800)	{ m_nMachineType = 2; m_nPanelCtrlType = 1; m_nPanelDrvType = 2; m_nPanelIsoType = 2; m_fPanelCtrlWidth = 700; m_fPanelDrvWidth = 1500; m_fPanelIsoWidth = 1250; }
	if (fSpeed >= 2.5)		{ m_nMachineType = 3; m_nPanelCtrlType = 1; m_nPanelDrvType = 3; m_nPanelIsoType = 3; m_fPanelCtrlWidth = 700; m_fPanelDrvWidth = 1700; m_fPanelIsoWidth = 1600; }
	if (fSpeed >= 5)		{ m_nMachineType = 4; m_nPanelCtrlType = 1; m_nPanelDrvType = 3; m_nPanelIsoType = 3; m_fPanelCtrlWidth = 700; m_fPanelDrvWidth = 1700; m_fPanelIsoWidth = 1600; }

	// additional tuning for the isolator type: isolators are set for every fourth shaft
	AVULONG nId4th = (GetId() / 4) * 4;	// index of the master: first in each four (0, 4, 8 ...)
	if (GetId() != nId4th)
	{
		// for all but the master shaft, zero the isolator, but modify the master type (which must be the maximum in the four)
		if (m_nPanelIsoType > GetLiftGroup()->GetShaft(nId4th)->m_nPanelIsoType)
		{
			GetLiftGroup()->GetShaft(nId4th)->m_nPanelIsoType = m_nPanelIsoType;
			GetLiftGroup()->GetShaft(nId4th)->m_fPanelIsoWidth = m_fPanelIsoWidth;
		}
		m_nPanelIsoType = 0;
		m_fPanelIsoWidth = 0;
	}

	// determine rails size
	m_fRailWidth = 125; m_fRailLength = 82;
	if (fCapacity >= 1275) { m_fRailWidth = 127; m_fRailLength = 89; }
	if (fCapacity >= 1800) { m_fRailWidth = 140; m_fRailLength = 102; }

	// determine buffer num & size
	m_nBufferNum = 1;
	if (fSpeed >= 3.5) m_nBufferNum = 2;
	m_nBufferDiameter = 200; m_nBufferHeight = 540;
	if (fSpeed >= 2.0) m_nBufferHeight = 777;
	if (fSpeed >= 2.5) m_nBufferHeight = 1203;
	if (fSpeed >= 3.0) m_nBufferHeight = 1705;
	if (fSpeed >= 3.5) m_nBufferHeight = 2100;
	if (fSpeed >= 4.0) m_nBufferHeight = 2100;
	if (fSpeed >= 5.0) m_nBufferHeight = 2692;
	if (fSpeed >= 6.0) m_nBufferHeight = 2692;
	if (fSpeed >= 7.0) m_nBufferHeight = 4216;
	if (fSpeed >= 8.0) m_nBufferHeight = 6181;
	if (fSpeed >= 9.0) m_nBufferHeight = 6181;
	if (fSpeed >= 10.0) m_nBufferHeight = 7374;
	
//	MachRoomExt = ME[L"MachRoomExt"];
//	if (MachRoomExt == -1.0f)
//		if (TypeOfLift == LIFT_DOUBLE_DECK) MachRoomExt = 1000.0; else MachRoomExt = 0;

	if (m_nShaftLine == 1) fShaftPosX -= ShaftWidth;
	m_boxShaft = BOX(fShaftPosX, abs(fShaftPosY), ShaftWidth, ShaftDepth);
	m_boxShaft.SetThickness(0, 0, fFrontWall, fRearWall);
	
	AVFLOAT fVoid = (m_nShaftLine == 0) ? fVoidLeft : fVoidRight;
	m_boxCar = BOX(m_boxShaft.Left() + fVoid + fCarSideWallThickness, m_boxShaft.Front() + fLandingDoorDepth + fCarDoorDepth + fCarFrontWallThickness, 0, CarWidth, CarDepth, CarHeight);
	m_boxCar.SetThickness(fCarSideWallThickness, fCarSideWallThickness, fCarFrontWallThickness, fCarRearWallThickness, fCarFloorThickness, fLightingVoidHeight);

	m_boxDoor[0] =    BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxShaft.Front(),    0, LiftDoorWidth, fLandingDoorDepth, LiftDoorHeight);
	m_boxDoor[1] =    BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxShaft.Rear(),     0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	m_boxCarDoor[0] = BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxCar.FrontExt() - fCarDoorDepth,   0, LiftDoorWidth, fCarDoorDepth, LiftDoorHeight);
	m_boxCarDoor[1] = BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxCar.Rear(),       0, LiftDoorWidth, fCarRearWallThickness, LiftDoorHeight);

	// resolve left-right side

	AVLONG nMachineOrientation = 0;	// 0 = forward, 1 = left, 2 = backward, 3 = right
	switch (nCwtPos)
	{
	case 1: nMachineOrientation = 0; break;
	case 2: nMachineOrientation = 1; break;
	case 3: nMachineOrientation = 3; break;
	}
	if (m_nShaftLine == 1) nMachineOrientation = (nMachineOrientation + 2) % 4;

	m_fShaftOrientation = (m_nShaftLine == 0) ? 0 : M_PI;
	m_fMachineOrientation = nMachineOrientation * M_PI_2;

	switch (nMachineOrientation)
	{	case 0:	// forward (first line of lifts): cwt in the rear, gov on the right, light asymetrically, ladder on the left
			m_boxCwt = BOX(m_boxCar.CentreX() - cwtl/2, m_boxShaft.Rear() - cwtx - cwtw, 0, cwtl, cwtw, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.RightExt(), m_boxCar.CentreY(), 0, 200, m_boxCar.DepthExt() / 2, 0);
			m_fLightingXPos = (m_boxCwt.CentreX() >= m_boxShaft.CentreX()) ? (m_boxCwt.Left() + m_boxShaft.Left()) / 2 : (m_boxCwt.Right() + m_boxShaft.Right()) / 2;
			m_boxLadder = BOX(m_boxShaft.Left(), m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 2:	// backward (second line of lifts): cwt in the rear of line 2, gov on the left, light asymetrically, ladder on the right
			m_boxCwt = BOX(m_boxCar.CentreX() - cwtl/2, m_boxShaft.Rear() - cwtx - cwtw, 0, cwtl, cwtw, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.LeftExt() - 200, m_boxCar.CentreY(), 0, 200, m_boxCar.DepthExt() / 2, 0);
			m_fLightingXPos = (m_boxCwt.CentreX() >= m_boxShaft.CentreX()) ? (m_boxCwt.Left() + m_boxShaft.Left()) / 2 : (m_boxCwt.Right() + m_boxShaft.Right()) / 2;
			m_boxLadder = BOX(m_boxShaft.Right() - 132, m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 1:	// heading to the left: cwt on the left, gov in the rear, light centrally, ladder on the right
			m_boxCwt = BOX(m_boxShaft.Left() + cwtx, m_boxCar.CentreY() - cwtl/2, 0, cwtw, cwtl, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.CentreX(), m_boxCar.Rear(), 0, m_boxCar.WidthExt() / 2, 200, 0);
			if (m_nShaftLine == 0) m_boxGovernor.MoveY(200); 
			m_fLightingXPos = m_boxDoor[0].CentreX();
			m_boxLadder = BOX(m_boxShaft.Right() - 132, m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 3:	// heading to the right: cwt on the right, gov in the rear, light centrally, ladder on the left
			m_boxCwt = BOX(m_boxShaft.Right() - cwtx - cwtw, m_boxCar.CentreY() - cwtl/2, 0, cwtw, cwtl, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt); 
			m_boxGovernor = BOX(m_boxCar.CentreX(), m_boxCar.Rear(), 0, m_boxCar.WidthExt() / 2, 200, 0);
			if (m_nShaftLine == 1) m_boxGovernor.MoveY(200); 
			m_fLightingXPos = m_boxDoor[0].CentreX();
			m_boxLadder = BOX(m_boxShaft.Left(), m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
	}

	if (fShaftPosY < 0)
		Reflect();
}

void CLiftGroup::SHAFT::ConsoleCreateBeam(AVULONG side, CLiftGroup::SHAFT *pNeighbour)
{
	AVFLOAT fIntDivBeamWidth = ME[L"DividingBeamWidth"];
	AVFLOAT fIntDivBeamHeight = ME[L"DividingBeamHeight"];

	if (pNeighbour && (AVFLOAT)(*pNeighbour)[L"DividingBeamWidth"] > fIntDivBeamWidth) fIntDivBeamWidth = (*pNeighbour)[L"DividingBeamWidth"];
	if (pNeighbour && (AVFLOAT)(*pNeighbour)[L"DividingBeamHeight"] > fIntDivBeamHeight) fIntDivBeamHeight = (*pNeighbour)[L"DividingBeamHeight"];

	//AVVECTOR pos = (side == SIDE_LEFT) ? m_boxShaft.LeftFrontLower() + Vector(-fIntDivBeamWidth, 0, 0): m_boxShaft.RightFrontLower();
	//m_boxBeam = BOX(pos, fIntDivBeamWidth, GetBox().Depth(), fIntDivBeamHeight );

	if (side == SIDE_LEFT)
	{
		m_boxShaft.SetLeftThickness(fIntDivBeamWidth);
		m_fBeamLtHeight = fIntDivBeamHeight;
	}
	else
	{
		m_boxShaft.SetRightThickness(fIntDivBeamWidth);
		m_fBeamRtHeight = fIntDivBeamHeight;
	}
}

void CLiftGroup::SHAFT::ConsoleCreateSideWall(AVULONG side, AVFLOAT fThickness, AVFLOAT fStart)
{
	if (side == SIDE_LEFT)
	{
		m_boxShaft.SetLeftThickness(fThickness);
		m_fWallLtStart = fStart;
	}
	else
	{
		m_boxShaft.SetRightThickness(fThickness);
		m_fWallRtStart = fStart;
	}
}

void CLiftGroup::SHAFT::ConsoleCreateAmend()
{
	ME[L"ShaftId"] = GetId();
	ME[L"ShaftLine"] = m_nShaftLine;

	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"LeftWallStart"] = m_fWallLtStart;
	ME[L"RightWallStart"] = m_fWallRtStart;
	ME[L"BeamLtHeight"] = m_fBeamLtHeight;
	ME[L"BeamRtHeight"] = m_fBeamRtHeight;
	ME[L"BoxDoor0"] = m_boxDoor[0].Stringify();
	ME[L"BoxDoor1"] = m_boxDoor[1].Stringify();
	ME[L"BoxCar"] = m_boxCar.Stringify();
	ME[L"BoxCarDoor0"] = m_boxCarDoor[0].Stringify();
	ME[L"BoxCarDoor1"] = m_boxCarDoor[1].Stringify();
	ME[L"BoxCwt"] = m_boxCwt.Stringify();
	ME[L"BoxGovernor"] = m_boxGovernor.Stringify();
	ME[L"BoxLadder"] = m_boxLadder.Stringify();
	
	ME[L"ShaftOrientation"] = m_fShaftOrientation;
	ME[L"MachineType"] = m_nMachineType;
	ME[L"MachineOrientation"] = m_fMachineOrientation;
	ME[L"PanelCtrlType"] = m_nPanelCtrlType;
	ME[L"PanelDrvType"] = m_nPanelDrvType;
	ME[L"PanelIsoType"] = m_nPanelIsoType;
	ME[L"PanelCtrlWidth"] = m_fPanelCtrlWidth;
	ME[L"PanelDrvWidth"] = m_fPanelDrvWidth;
	ME[L"PanelIsoWidth"] = m_fPanelIsoWidth;
	ME[L"RailWidth"] = m_fRailWidth;
	ME[L"RailLength"] = m_fRailLength;
	ME[L"BufferNum"] = m_nBufferNum;
	ME[L"BufferDiameter"] = m_nBufferDiameter;
	ME[L"BufferHeight"] = m_nBufferHeight;
	ME[L"LightingXPos"] = m_fLightingXPos;

	erase(L"Number");
	erase(L"ShaftWidth");
	erase(L"ShaftDepth");
	erase(L"CarDepth");
	erase(L"CarWidth");
	erase(L"CarCeilingHeight");
	erase(L"DoorHeight");
	erase(L"DoorWidth");
	erase(L"DividingBeamWidth");
	erase(L"DividingBeamHeight");

	erase(L"ClearanceCarToShaftWallOnDepth");
	erase(L"ClearanceCarToShaftWallOnWidthLHS");
	erase(L"ClearanceCarToShaftWallOnWidthRHS");
	erase(L"CarDepthWallThickness");
	erase(L"CarFrontReturnDepth");
	erase(L"CarSideWallThickness");
	erase(L"LightingVoidHeight");
	erase(L"CarDoorDepth");
	erase(L"LandingDoorDepth");

	erase(L"Headroom");
	erase(L"HeightAboveCarRoof");

	erase(L"MachineRoomHeight");
	erase(L"PitDepth");
}

void CLiftGroup::SHAFT::Scale(AVFLOAT f)
{
	m_boxShaft.Scale(f);
	m_fWallLtStart *= f;
	m_fWallRtStart *= f;
	m_fBeamLtHeight *= f;
	m_fBeamRtHeight *= f;
	m_boxDoor[0].Scale(f);
	m_boxDoor[1].Scale(f);
	m_boxCar.Scale(f);
	m_boxCarDoor[0].Scale(f);
	m_boxCarDoor[1].Scale(f);
	m_boxCwt.Scale(f);
	m_boxGovernor.Scale(f);
	m_boxLadder.Scale(f);
	m_fRailWidth *= f;
	m_fRailLength *= f;
	m_nBufferDiameter *= f;
	m_nBufferHeight *= f;
	m_fLightingXPos *= f;
}

void CLiftGroup::SHAFT::Reflect()
{
	m_boxShaft.Scale(1, -1, 1);
	m_fWallLtStart *= -1;
	m_fWallRtStart *= -1;
	m_boxDoor[0].Scale(1, -1, 1);
	m_boxDoor[1].Scale(1, -1, 1);
	m_boxCar.Scale(1, -1, 1);
	m_boxCarDoor[0].Scale(1, -1, 1);
	m_boxCarDoor[1].Scale(1, -1, 1);
	m_boxCwt.Scale(1, -1, 1);
	m_boxGovernor.Scale(1, -1, 1);
	m_boxLadder.Scale(1, -1, 1);
}

void CLiftGroup::SHAFT::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_boxShaft.Move(x, y, z);
	if (m_fWallLtStart) m_fWallLtStart += y;
	if (m_fWallRtStart) m_fWallRtStart += y;
	m_boxDoor[0].Move(x, y, z);
	m_boxDoor[1].Move(x, y, z);
	m_boxCar.Move(x, y, z);
	m_boxCarDoor[0].Move(x, y, z);
	m_boxCarDoor[1].Move(x, y, z);
	m_boxCwt.Move(x, y, z);
	m_boxGovernor.Move(x, y, z);
	m_boxLadder.Move(x, y, z);
	m_fLightingXPos += x;
}

void CLiftGroup::SHAFT::Create()
{
	m_nId = ME[L"ShaftId"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];
	m_nDoorType = ME[L"DoorTypeId"];
	m_nLiftType = ME[L"LiftTypeId"];

	m_nOpeningTime = (AVULONG)((float)ME[L"OpeningTime"] * 1000);
	m_nClosingTime = (AVULONG)((float)ME[L"ClosingTime"] * 1000);

	m_nLiftCount = max(1, (AVULONG)ME[L"LiftsPerShaft"]);
	m_nShaftLine = ME[L"ShaftLine"];
	m_boxShaft.ParseFromString(ME[L"BoxShaft"]);
	m_boxDoor[0].ParseFromString(ME[L"BoxDoor0"]);
	m_boxDoor[1].ParseFromString(ME[L"BoxDoor1"]);
	m_boxCar.ParseFromString(ME[L"BoxCar"]);
	m_boxCarDoor[0].ParseFromString(ME[L"BoxCarDoor0"]);
	m_boxCarDoor[1].ParseFromString(ME[L"BoxCarDoor1"]);
	m_boxCwt.ParseFromString(ME[L"BoxCwt"]);
	m_boxGovernor.ParseFromString(ME[L"BoxGovernor"]);
	m_boxLadder.ParseFromString(ME[L"BoxLadder"]);
	m_fWallLtStart = ME[L"LeftWallStart"];
	m_fWallRtStart = ME[L"RightWallStart"];
	m_fBeamLtHeight = ME[L"BeamLtHeight"];
	m_fBeamRtHeight = ME[L"BeamRtHeight"];

	m_fShaftOrientation = ME[L"ShaftOrientation"];
	m_nMachineType = ME[L"MachineType"];
	m_fMachineOrientation = ME[L"MachineOrientation"];
	m_nPanelCtrlType = ME[L"PanelCtrlType"];
	m_nPanelDrvType = ME[L"PanelDrvType"];
	m_nPanelIsoType = ME[L"PanelIsoType"];
	m_fPanelCtrlWidth = ME[L"PanelCtrlWidth"];
	m_fPanelDrvWidth = ME[L"PanelDrvWidth"];
	m_fPanelIsoWidth = ME[L"PanelIsoWidth"];
	m_fRailWidth = ME[L"RailWidth"];
	m_fRailLength = ME[L"RailLength"];
	m_nBufferNum = ME[L"BufferNum"];
	m_nBufferDiameter = ME[L"BufferDiameter"];
	m_nBufferHeight = ME[L"BufferHeight"];
	m_fLightingXPos = ME[L"LightingXPos"];

	m_strStoreysServed = ME[L"StoreysServed"];
}

