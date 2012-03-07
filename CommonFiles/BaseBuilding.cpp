// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase

CBuildingBase::CBuildingBase(void) : m_ppShafts(NULL), m_ppStoreys(NULL), m_nShaftCount(0), m_nStoreyCount(0), m_nBasementStoreyCount(0),
									 m_nId(0), m_LiftShaftArrang(SHAFT_INLINE), m_LobbyArrangement(LOBBY_OPENPLAN)
{
	m_pnShaftCount[0] = m_pnShaftCount[1] = 0;
}

CBuildingBase::~CBuildingBase(void)
{
	DeleteShafts();
	DeleteStoreys();
}

void CBuildingBase::CreateShafts(AVULONG nShaftCount)
{
	DeleteShafts();
	m_nShaftCount = nShaftCount;
	m_ppShafts = new SHAFT*[GetShaftCount()];
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		m_ppShafts[i] = CreateShaft();
}

void CBuildingBase::DeleteShafts()
{
	if (!m_ppShafts) return;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		if (m_ppShafts[i]) delete m_ppShafts[i];
	delete [] m_ppShafts;
	m_ppShafts = NULL;
}

void CBuildingBase::CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount)
{
	DeleteStoreys();
	m_nStoreyCount = nStoreyCount;
	m_nBasementStoreyCount = nBasementStoreyCount;
	m_ppStoreys = new STOREY*[GetStoreyCount()];
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		m_ppStoreys[i] = CreateStorey();
}

void CBuildingBase::DeleteStoreys()
{
	if (!m_ppStoreys) return;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		if (m_ppStoreys[i]) delete m_ppStoreys[i];
	delete [] m_ppStoreys;
	m_ppStoreys = NULL;
}

BOX &CBuildingBase::SHAFT::GetBox(CBuildingBase::SHAFT::SHAFT_BOX n, AVULONG i)
{
	switch (n)
	{
	case BOX_SHAFT:	return m_boxShaft;
	case BOX_BEAM:	return m_boxBeam;
	case BOX_DOOR:	return m_boxDoor[i];
	case BOX_CAR:	return m_boxCar;
	case BOX_CARDOOR:	return m_boxCarDoor[i];
	default:		return m_boxShaft;
	}
}

BOX CBuildingBase::SHAFT::GetLeftWallBox()
{
	return BOX(GetBox().LeftExt(), GetBox().Front() + m_fWallLtStart, GetBox().Lower(), GetBox().Depth() - m_fWallLtStart, GetBox().LeftThickness(), GetBox().Height());
}

BOX CBuildingBase::SHAFT::GetRightWallBox()
{
	return BOX(GetBox().Right(), GetBox().Front() + m_fWallRtStart, GetBox().Lower(), GetBox().Depth() - m_fWallRtStart, GetBox().RightThickness(), GetBox().Height());
}

void CBuildingBase::PreCreate()
{
	CreateShafts(ME[L"NumberOfLifts"]);
	CreateStoreys((ULONG)ME[L"NumberOfFloorsAboveMain"] + 1 + (ULONG)ME[L"NumberOfFloorsBelowMain"], (ULONG)ME[L"NumberOfFloorsBelowMain"]);
	SetId(ME[L"ID"]);
}

void CBuildingBase::ConCreate()
{
	// resolve vars
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangementId"];
	
	if (m_LobbyArrangement == LOBBY_OPENPLAN && m_LiftShaftArrang == SHAFT_OPPOSITE)
		m_LobbyArrangement = LOBBY_THROUGH;
	ME[L"LobbyArrangementId"] = (AVLONG)m_LobbyArrangement;

	AVFLOAT fLobbyCeilingSlabHeight = ME[L"LiftLobbyCeilingHeight"];
	AVFLOAT fLobbyDepth = ME[L"FrontLobbyWidth"];
	AVFLOAT fFrontWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fSideWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fShaftWallThicknessRear = ME[L"ShaftWallThicknessRear"];
	AVFLOAT fShaftWallThicknessSide = ME[L"ShaftWallThicknessSide"];
	fLobbyCeilingSlabHeight = 1000;
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
	m_box.SetThickness(fLt, fRt, fFrontWallThickness, (m_LobbyArrangement == LOBBY_OPENPLAN) ? 0 : fFrontWallThickness, 1, fLobbyCeilingSlabHeight);

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
		pShaft->ConCreate(this, i, 0, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConCreateLeftWall(fShaftWallThicknessSide);
			pShaft->Move(fShaftWallThicknessSide, 0, 0);
		}
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->ConCreateLeftBeam(fDepth, pPrevShaft);
		else if (fDepth < fPrevDepth)	// note: both values are negative here!
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->ConCreateLeftBeam(fPrevDepth, pPrevShaft);
			pShaft->ConCreateLeftWall(fShaftWallThicknessRear, fPrevDepth - fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConCreateLeftBeam(fDepth, pPrevShaft);
			GetShaft(i-1)->ConCreateRightWall(fShaftWallThicknessRear, fDepth - fShaftWallThicknessRear);
		}
		if (i == GetShaftCount(0) - 1)
			pShaft->ConCreateRightWall(fShaftWallThicknessSide);

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
		pShaft->ConCreate(this, i, 1, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConCreateRightWall(fShaftWallThicknessSide);
			pShaft->Move(-fShaftWallThicknessSide, 0, 0);
		}
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->ConCreateRightBeam(fDepth, pPrevShaft);
		else if (fDepth > fPrevDepth)
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->ConCreateRightBeam(fPrevDepth, pPrevShaft);
			pShaft->ConCreateRightWall(fShaftWallThicknessRear, fPrevDepth + fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConCreateRightBeam(fDepth, pPrevShaft);
			GetShaft(i-1)->ConCreateLeftWall(fShaftWallThicknessRear, fDepth + fShaftWallThicknessRear);
		}
		if (i == GetShaftCount() - 1)
			pShaft->ConCreateLeftWall(fShaftWallThicknessSide);

		// move on
		fPrevDepth = fDepth;
		fShaftPosX = pShaft->GetBox().Left();
		pPrevShaft = pShaft;
	}

	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->ConCreateAmend();

	AVFLOAT fLevel = 0;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);
		pStorey->ConCreate(this, i, fLevel);
		fLevel += pStorey->GetHeight();
	}

	ME[L"Line1"] = m_pnShaftCount[0];
	ME[L"Line2"] = m_pnShaftCount[1];
	ME[L"BoxLobby"] = m_box.Stringify();
	erase(L"LobbyCeilingSlabHeight");
	erase(L"LobbyDepth");
	erase(L"FrontWallThickness");
	erase(L"IntDivBeamWidth");
	erase(L"IntDivBeamHeight");
	erase(L"SideWallThickness");
	erase(L"ShaftWallThickness");
	erase(L"ShaftWallThickness");
}

void CBuildingBase::Create()
{
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangementId"];
	m_pnShaftCount[0] = ME[L"Line1"];
	m_pnShaftCount[1] = ME[L"Line2"];
	m_box.ParseFromString(ME[L"BoxLobby"]);
	
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Create(this);

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Create(this);
}

void CBuildingBase::Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_box.Scale(x, y, z);
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Scale(x, y, z);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Scale(x, y, z);
}

void CBuildingBase::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_box.Move(x, y, z);
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Move(x, y, z);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Move(x, y, z);
}

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::SHAFT

CBuildingBase::SHAFT::SHAFT() : m_nId(0), m_pBuilding(NULL), m_nShaftLine(0), m_type(LIFT_CONVENTIONAL), m_deck(DECK_SINGLE), m_fWallLtStart(0), m_fWallRtStart(0)	
{ 
}

void CBuildingBase::SHAFT::ConCreate(CBuildingBase *pBuilding, AVULONG nId, AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY, AVFLOAT fFrontWall, AVFLOAT fRearWall)
{
	//m_nId = nId;
	m_nId = ME[L"LiftNumber"];
	m_pBuilding = pBuilding;
	m_nShaftLine = nLine;

	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];

	AVFLOAT ShaftWidth = ME[L"ShaftWidth"];
	AVFLOAT ShaftDepth = ME[L"ShaftDepth"];
	AVFLOAT CarDepth = ME[L"CarDepth"];
	AVFLOAT CarWidth = ME[L"CarWidth"];
	AVFLOAT CarHeight = ME[L"CarCeilingHeight"];
	AVFLOAT LiftDoorHeight = ME[L"DoorHeight"];
	AVFLOAT LiftDoorWidth = ME[L"DoorWidth"];

//	MachRoomExt = ME[L"MachRoomExt"];
//	if (MachRoomExt == -1.0f)
//		if (TypeOfLift == LIFT_DOUBLE_DECK) MachRoomExt = 1000.0; else MachRoomExt = 0;

	AVFLOAT wallThickness = fFrontWall / 2;		// lift wall thickness
	AVFLOAT gap = 1.0f;							// gap between lift & floor

	if (m_nShaftLine == 1) fShaftPosX -= ShaftWidth;
	m_boxShaft = BOX(fShaftPosX, abs(fShaftPosY), ShaftWidth, ShaftDepth);
	m_boxShaft.SetThickness(0, 0, fFrontWall, fRearWall);
	
	m_boxDoor[0] = BOX(m_boxShaft.CenterX() - LiftDoorWidth / 2, m_boxShaft.FrontExt(), 0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	m_boxDoor[1] = BOX(m_boxShaft.CenterX() - LiftDoorWidth / 2, m_boxShaft.Rear(), 0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	
	m_boxCar = BOX(m_boxShaft.CenterXExt() - CarWidth / 2, m_boxShaft.Front() + wallThickness + gap, 0, CarWidth, CarDepth, CarHeight);
	m_boxCar.SetThickness(wallThickness, wallThickness, wallThickness, wallThickness, wallThickness, wallThickness);

	m_boxCarDoor[0] = BOX(m_boxCar.CenterX() - LiftDoorWidth / 2, m_boxCar.FrontExt(), 0, LiftDoorWidth, wallThickness, LiftDoorHeight);
	m_boxCarDoor[1] = BOX(m_boxCar.CenterX() - LiftDoorWidth / 2, m_boxCar.Rear(), 0, LiftDoorWidth, wallThickness, LiftDoorHeight);

	if (fShaftPosY < 0)
		Scale(1, -1, 1);
}

void CBuildingBase::SHAFT::ConCreateLeftBeam(AVFLOAT fDepth, CBuildingBase::SHAFT *pPrev)
{
	AVFLOAT fIntDivBeamWidth = ME[L"DividingBeamWidth"];
	AVFLOAT fIntDivBeamHeight = ME[L"DividingBeamHeight"];

	if (pPrev && (AVFLOAT)(*pPrev)[L"DividingBeamWidth"] > fIntDivBeamWidth) fIntDivBeamWidth = (*pPrev)[L"DividingBeamWidth"];
	if (pPrev && (AVFLOAT)(*pPrev)[L"DividingBeamHeight"] > fIntDivBeamHeight) fIntDivBeamHeight = (*pPrev)[L"DividingBeamHeight"];

	AVVECTOR pos = m_boxShaft.LeftFrontLower();
	pos.x -= fIntDivBeamWidth;
	AVVECTOR size = { fIntDivBeamWidth, fDepth, fIntDivBeamHeight };
	m_boxBeam = BOX(pos, pos + size);

	Move(fIntDivBeamWidth, 0, 0);
}

void CBuildingBase::SHAFT::ConCreateRightBeam(AVFLOAT fDepth, CBuildingBase::SHAFT *pPrev)
{
	AVFLOAT fIntDivBeamWidth = ME[L"DividingBeamWidth"];
	AVFLOAT fIntDivBeamHeight = ME[L"DividingBeamHeight"];

	if (pPrev && (AVFLOAT)(*pPrev)[L"DividingBeamWidth"] > fIntDivBeamWidth) fIntDivBeamWidth = (*pPrev)[L"DividingBeamWidth"];
	if (pPrev && (AVFLOAT)(*pPrev)[L"DividingBeamHeight"] > fIntDivBeamHeight) fIntDivBeamHeight = (*pPrev)[L"DividingBeamHeight"];

	AVVECTOR pos = m_boxShaft.RightFrontLower();
	AVVECTOR size = { fIntDivBeamWidth, fDepth, fIntDivBeamHeight };
	m_boxBeam = BOX(pos, pos + size);

	Move(-fIntDivBeamWidth, 0, 0);
}

void CBuildingBase::SHAFT::ConCreateLeftWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetLeftThickness(fThickness);
	m_fWallLtStart = fStart;
}

void CBuildingBase::SHAFT::ConCreateRightWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetRightThickness(fThickness);
	m_fWallRtStart = fStart;
}

void CBuildingBase::SHAFT::ConCreateAmend()
{
	ME[L"ShaftId"] = GetId();
	ME[L"ShaftLine"] = m_nShaftLine;

	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"LeftWallStart"] = m_fWallLtStart;
	ME[L"RightWallStart"] = m_fWallRtStart;
	ME[L"BoxBeam"] = m_boxBeam.Stringify();
	ME[L"BoxDoor0"] = m_boxDoor[0].Stringify();
	ME[L"BoxDoor1"] = m_boxDoor[1].Stringify();
	ME[L"BoxCar"] = m_boxCar.Stringify();
	ME[L"BoxCarDoor0"] = m_boxCarDoor[0].Stringify();
	ME[L"BoxCarDoor1"] = m_boxCarDoor[1].Stringify();
	
	erase(L"Number");
	erase(L"ShaftWidth");
	erase(L"ShaftDepth");
	erase(L"CarDepth");
	erase(L"CarWidth");
	erase(L"CarHeight");
	erase(L"LiftDoorHeight");
	erase(L"LiftDoorWidth");
}

void CBuildingBase::SHAFT::Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_boxShaft.Scale(x, y, z);
	m_fWallLtStart *= y;
	m_fWallRtStart *= y;
	m_boxBeam.Scale(x, y, z);
	m_boxDoor[0].Scale(x, y, z);
	m_boxDoor[1].Scale(x, y, z);
	m_boxCar.Scale(x, y, z);
	m_boxCarDoor[0].Scale(x, y, z);
	m_boxCarDoor[1].Scale(x, y, z);
}

void CBuildingBase::SHAFT::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_boxShaft.Move(x, y, z);
	m_fWallLtStart += y;
	m_fWallRtStart += y;
	m_boxBeam.Move(x, y, z);
	m_boxDoor[0].Move(x, y, z);
	m_boxDoor[1].Move(x, y, z);
	m_boxCar.Move(x, y, z);
	m_boxCarDoor[0].Move(x, y, z);
	m_boxCarDoor[1].Move(x, y, z);
}

void CBuildingBase::SHAFT::Create(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;
	m_nId = ME[L"ShaftId"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];
	m_nShaftLine = ME[L"ShaftLine"];
	m_boxShaft.ParseFromString(ME[L"BoxShaft"]);
	m_boxBeam.ParseFromString(ME[L"BoxBeam"]);
	m_boxDoor[0].ParseFromString(ME[L"BoxDoor0"]);
	m_boxDoor[1].ParseFromString(ME[L"BoxDoor1"]);
	m_boxCar.ParseFromString(ME[L"BoxCar"]);
	m_boxCarDoor[0].ParseFromString(ME[L"BoxCarDoor0"]);
	m_boxCarDoor[1].ParseFromString(ME[L"BoxCarDoor1"]);
	m_fWallLtStart = ME[L"LeftWallStart"];
	m_fWallRtStart = ME[L"RightWallStart"];
}

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::STOREY

CBuildingBase::STOREY::STOREY() : m_nId(0), m_pBuilding(NULL), m_fLevel(0)
{ 
}

void CBuildingBase::STOREY::ConCreate(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fLevel)
{
	m_nId = ME[L"GroundIndex"];
	m_pBuilding = pBuilding;
	m_fLevel = fLevel;
	m_strName = ME[L"Name"];
	m_box = m_pBuilding->m_box;
	m_box.SetHeight((AVFLOAT)ME[L"FloorHeight"] * 1000.0f- m_box.UpperThickness());

	ME[L"FloorId"] = GetId();
	ME[L"FloorLevel"] = m_fLevel;
	ME[L"Box"] = m_box.Stringify();
	erase(L"FloorHeight");
	erase(L"GroundIndex");
}

void CBuildingBase::STOREY::Create(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;

	m_strName = ME[L"Name"];

	m_fLevel = ME[L"FloorLevel"];
	m_box.ParseFromString(ME[L"Box"]);
}

void CBuildingBase::STOREY::Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_fLevel *= z;
	m_box.Scale(x, y, z);
}

void CBuildingBase::STOREY::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_fLevel += z;
	m_box.Move(x, y, z);
}
