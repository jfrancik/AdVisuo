// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase

CBuildingBase::CBuildingBase(void) : m_ppShafts(NULL), m_ppStoreys(NULL), m_nShaftCount(0), m_nStoreyCount(0), m_nBasementStoreyCount(0),
									 m_nId(0), m_LiftShaftArrang(SHAFT_INLINE), m_LobbyArrangement(LOBBY_OPENPLAN), m_fScale(1.0)
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

BOX &CBuildingBase::SHAFT::GetBox(CBuildingBase::SHAFT::SHAFT_BOX n)
{
	switch (n)
	{
	case BOX_SHAFT:	return m_boxShaft;
	case BOX_CAR:	return m_boxCar;
	case BOX_BEAM:	return m_boxBeam;
	case BOX_DOOR:	return m_boxDoor[0];
	default:		return m_boxShaft;
	}
}

void CBuildingBase::PreXxxx()
{
// NEW DATABASE
	//CreateShafts(ME[L"NumberOfLifts"]);
	//CreateStoreys((ULONG)ME[L"NumberOfFloorsAboveMain"] + 1 + (ULONG)ME[L"NumberOfFloorsBelowMain"], (ULONG)ME[L"NumberOfFloorsBelowMain"]);
	//SetId(ME[L"ID"]);

	CreateShafts(ME[L"NoOfLifts"]);
	CreateStoreys((ULONG)ME[L"NoFloorsAboveMain"] + 1 + (ULONG)ME[L"NoFloorsBelowMain"], (ULONG)ME[L"NoFloorsBelowMain"]);
	SetId(ME[L"ID"]);
}

void CBuildingBase::Xxxx()
{
	// resolve vars
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangement"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangement"];

	AVFLOAT fLobbyCeilingSlabHeight = ME[L"LiftLobbyCeilingHeight"];
	AVFLOAT fLobbyDepth = ME[L"LobbyDepth"];
	AVFLOAT fFrontWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fIntDivBeamWidth = ME[L"IntermediateDividingBeam"];
	AVFLOAT fIntDivBeamHeight = (AVFLOAT)ME[L"IntermediateDividingBeam"] * 250 / 150;
	AVFLOAT fSideWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fShaftWallThicknessRear = ME[L"FrontWallThickness"];
	AVFLOAT fShaftWallThicknessSide = ME[L"FrontWallThickness"];

	// calculate width of the lobby if lifts in-line, no side walls
	AVFLOAT w = 0;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		GetShaft(i)->Xxxx(this, i, fFrontWallThickness, fShaftWallThicknessRear);
		w += GetShaft(i)->GetBox().Width();
	}
	w += fIntDivBeamWidth * (GetShaftCount() - 1);

	// calculate ShaftCount, LineWidth
	AVFLOAT fLineWidth[2];
	if (GetLiftShaftArrang() != SHAFT_OPPOSITE)
	{
		// in-line
		m_pnShaftCount[0] = GetShaftCount();
		m_pnShaftCount[1] = 0;
		fLineWidth[0] = w + 2 * fSideWallThickness;
		fLineWidth[1] = 0;
	}
	else
	{
		w -= fIntDivBeamWidth;
		AVFLOAT w0 = -fIntDivBeamWidth;
		AVULONG i;
		for (i = 0; i < GetShaftCount() && w0 + GetShaft(i)->GetBox().Width() / 2 < w/2; i++)
		{
			AVFLOAT wx = GetShaft(i)->GetBox().Width() + fIntDivBeamWidth;
			if (w0 + wx/2 > w / 2) break;
			w0 += wx;
		}
		m_pnShaftCount[0] = i;
		m_pnShaftCount[1] = GetShaftCount() - m_pnShaftCount[0];
		fLineWidth[0] = w0 + 2 * fSideWallThickness;
		fLineWidth[1] = w - w0 + 2 * fSideWallThickness;
	}
	AVFLOAT fLobbyWidth = max(fLineWidth[0], fLineWidth[1]);

	// calculate lobby box
	AVFLOAT fLt = 0; if (m_LobbyArrangement == LOBBY_DEADEND_LEFT)  fLt = fSideWallThickness;
	AVFLOAT fRt = 0; if (m_LobbyArrangement == LOBBY_DEADEND_RIGHT) fRt = fSideWallThickness;
	fLt = fRt = fSideWallThickness;	// temporarily closed space!

	m_box = BOX(-fLobbyWidth/2 + fLt, -fLobbyDepth/2, 0, fLobbyWidth - fLt - fRt, fLobbyDepth, 0);
	m_box.SetThickness(fLt, fRt, fFrontWallThickness, (m_LobbyArrangement == LOBBY_OPENPLAN) ? 0 : fFrontWallThickness, 1, fLobbyCeilingSlabHeight);

	// Resolve Shafts and Storeys

	// 1st line of shafts
	AVFLOAT fShaftPosX = GetBox().LeftExt() + fShaftWallThicknessSide;
	AVFLOAT fShaftPosY = GetBox().FrontExt();
	AVFLOAT fPrevDepth = 0;
	for (AVULONG i = 0; i < GetShaftCount(0); i++)
	{
		SHAFT *pShaft = GetShaft(i);

		// setup basic structure
		pShaft->Xxxx(0, fShaftPosX, fShaftPosY);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
			// no previous shaft: build a wall, not beam
			pShaft->XxxxLeftWall(fShaftWallThicknessSide);
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->XxxxLeftBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
		else if (fDepth < fPrevDepth)	// note: both values are negative here!
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->XxxxLeftBeam(fIntDivBeamWidth, fPrevDepth, fIntDivBeamHeight);
			pShaft->XxxxLeftWall(fShaftWallThicknessRear, fPrevDepth - fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->XxxxLeftBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
			GetShaft(i-1)->XxxxRightWall(fShaftWallThicknessRear, fDepth - fShaftWallThicknessRear);
		}
		fPrevDepth = fDepth;

		// move on
		fShaftPosX += pShaft->GetBox().Width() + fIntDivBeamWidth;
	}
	GetShaft(GetShaftCount(0) - 1)->XxxxRightWall(fShaftWallThicknessSide);

	// 2nd line of shafts
	fShaftPosX = GetBox().RightExt() - fShaftWallThicknessSide;
	fShaftPosY = GetBox().RearExt();
	fPrevDepth = 0;
	for (AVULONG i = GetShaftCount(0); i < GetShaftCount(); i++)
	{
		SHAFT *pShaft = GetShaft(i);

		// onto position:
		fShaftPosX -= pShaft->GetBox().Width();

		// setup basic structure
		pShaft->Xxxx(1, fShaftPosX, fShaftPosY);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
			// no previous shaft: build a wall, not beam
			pShaft->XxxxRightWall(fShaftWallThicknessSide);
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->XxxxRightBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
		else if (fDepth > fPrevDepth)
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->XxxxRightBeam(fIntDivBeamWidth, fPrevDepth, fIntDivBeamHeight);
			pShaft->XxxxRightWall(fShaftWallThicknessRear, fPrevDepth + fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->XxxxRightBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
			GetShaft(i-1)->XxxxLeftWall(fShaftWallThicknessRear, fDepth + fShaftWallThicknessRear);
		}
		fPrevDepth = fDepth;

		// move on
		fShaftPosX -= fIntDivBeamWidth;
	}
	GetShaft(GetShaftCount() - 1)->XxxxLeftWall(fShaftWallThicknessSide);

	AVFLOAT fLevel = 0;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);
		pStorey->Xxxx(this, i, fLevel);
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

void CBuildingBase::Yyyy()
{
	m_LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangement"];
	m_LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangement"];
	m_pnShaftCount[0] = ME[L"Line1"];
	m_pnShaftCount[1] = ME[L"Line2"];
	m_box.ParseFromString(ME[L"BoxLobby"]);
	
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Yyyy(this);

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Yyyy(this);
}

void CBuildingBase::Scale(AVFLOAT fScale)
{
	m_fScale = fScale;
	m_box *= m_fScale;
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Scale(m_fScale);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Scale(m_fScale);
}

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::SHAFT

CBuildingBase::SHAFT::SHAFT() : m_nId(0), m_pBuilding(NULL), m_nShaftLine(0), m_nLiftCount(0), m_type(LIFT_SINGLE_DECK), m_fWallLtStart(0), m_fWallRtStart(0)	
{ 
}

void CBuildingBase::SHAFT::Xxxx(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fFrontWall, AVFLOAT fRearWall)
{
	//m_nId = nId;
	m_nId = ME[L"Number"];
	m_pBuilding = pBuilding;
	m_nShaftLine = 0;
	
	m_nLiftCount = ME[L"NumberOfLifts"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftType"];

	AVFLOAT ShaftWidth = ME[L"ShaftWidth"];
	AVFLOAT ShaftDepth = ME[L"ShaftDepth"];
	AVFLOAT CarDepth = ME[L"CarDepth"];
	AVFLOAT CarWidth = ME[L"CarWidth"];
	AVFLOAT CarHeight = ME[L"CarHeight"];
	AVFLOAT LiftDoorHeight = ME[L"DoorHeight"];
	AVFLOAT LiftDoorWidth = ME[L"DoorWidth"];

//	MachRoomExt = ME[L"MachRoomExt"];
//	if (MachRoomExt == -1.0f)
//		if (TypeOfLift == LIFT_DOUBLE_DECK) MachRoomExt = 1000.0; else MachRoomExt = 0;

	AVFLOAT wallThickness = fFrontWall / 2;		// lift wall thickness
	AVFLOAT gap = 1.0f;							// gap between lift & floor

	m_boxShaft = BOX(0, 0, ShaftWidth, ShaftDepth);
	m_boxShaft.SetThickness(0, 0, fFrontWall, fRearWall);
	
	m_boxDoor[0] = BOX(m_boxShaft.CenterX() - LiftDoorWidth / 2, m_boxShaft.FrontExt(), 0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	m_boxDoor[1] = BOX(m_boxShaft.CenterX() - LiftDoorWidth / 2, m_boxShaft.Rear(), 0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	
	m_boxCar = BOX(m_boxShaft.CenterXExt() - CarWidth / 2, m_boxShaft.Front() + wallThickness + gap, 0, CarWidth, CarDepth, CarHeight);
	m_boxCar.SetThickness(wallThickness, wallThickness, wallThickness, wallThickness, wallThickness, wallThickness);

	m_boxCarDoor[0] = BOX(m_boxCar.CenterX() - LiftDoorWidth / 2, m_boxCar.FrontExt(), 0, LiftDoorWidth, wallThickness, LiftDoorHeight);
	m_boxCarDoor[1] = BOX(m_boxCar.CenterX() - LiftDoorWidth / 2, m_boxCar.Rear(), 0, LiftDoorWidth, wallThickness, LiftDoorHeight);

	ME[L"ShaftId"] = GetId();
	ME[L"NumberOfLifts"] = (AVULONG)1;
	erase(L"Number");
}

void CBuildingBase::SHAFT::Xxxx(AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY)
{
	m_nShaftLine = nLine;
	AVVECTOR vec = { fShaftPosX, fShaftPosY, 0 };

	m_boxShaft.ScaleY(GetShaftDir());
	m_boxShaft += vec;
	m_boxDoor[0].ScaleY(GetShaftDir());
	m_boxDoor[0] += vec;
	m_boxDoor[1].ScaleY(GetShaftDir());
	m_boxDoor[1] += vec;
	m_boxCar.ScaleY(GetShaftDir());
	m_boxCar += vec;
	m_boxCarDoor[0].ScaleY(GetShaftDir());
	m_boxCarDoor[0] += vec;
	m_boxCarDoor[1].ScaleY(GetShaftDir());
	m_boxCarDoor[1] += vec;

	ME[L"ShaftLine"] = m_nShaftLine;
	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"BoxBeam"] = m_boxBeam.Stringify();
	ME[L"BoxDoor0"] = m_boxDoor[0].Stringify();
	ME[L"BoxDoor1"] = m_boxDoor[1].Stringify();
	ME[L"BoxCar"] = m_boxCar.Stringify();
	ME[L"BoxCarDoor0"] = m_boxCarDoor[0].Stringify();
	ME[L"BoxCarDoor1"] = m_boxCarDoor[1].Stringify();
	ME[L"LeftWallStart"] = m_fWallLtStart;
	ME[L"RightWallStart"] = m_fWallRtStart;
	erase(L"ShaftWidth");
	erase(L"ShaftDepth");
	erase(L"CarDepth");
	erase(L"CarWidth");
	erase(L"CarHeight");
	erase(L"LiftDoorHeight");
	erase(L"LiftDoorWidth");
}

void CBuildingBase::SHAFT::XxxxLeftBeam(AVFLOAT w, AVFLOAT d, AVFLOAT h)
{
	AVVECTOR pos = m_boxShaft.LeftFrontLower();
	pos.x -= w;
	AVVECTOR size = { w, d, h };
	m_boxBeam = BOX(pos, pos + size);

	ME[L"BoxBeam"] = m_boxBeam.Stringify();
}

void CBuildingBase::SHAFT::XxxxRightBeam(AVFLOAT w, AVFLOAT d, AVFLOAT h)
{
	AVVECTOR pos = m_boxShaft.RightFrontLower();
	AVVECTOR size = { w, d, h };
	m_boxBeam = BOX(pos, pos + size);

	ME[L"BoxBeam"] = m_boxBeam.Stringify();
}

void CBuildingBase::SHAFT::XxxxLeftWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetLeftThickness(fThickness);
	m_fWallLtStart = fStart;

	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"LeftWallStart"] = m_fWallLtStart;
}

void CBuildingBase::SHAFT::XxxxRightWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetRightThickness(fThickness);
	m_fWallRtStart = fStart;

	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"RightWallStart"] = m_fWallRtStart;
}

void CBuildingBase::SHAFT::Scale(AVFLOAT fScale)
{
	m_boxShaft *= fScale;
	m_fWallLtStart *= fScale;
	m_fWallRtStart *= fScale;
	m_boxBeam *= fScale;
	m_boxDoor[0] *= fScale;
	m_boxDoor[1] *= fScale;
	m_boxCar *= fScale;
	m_boxCarDoor[0] *= fScale;
	m_boxCarDoor[1] *= fScale;
}

void CBuildingBase::SHAFT::Yyyy(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;
	m_nId = ME[L"ShaftId"];
	m_nLiftCount = ME[L"NumberOfLifts"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftType"];
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

void CBuildingBase::STOREY::Xxxx(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fLevel)
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

void CBuildingBase::STOREY::Yyyy(CBuildingBase *pBuilding)
{
	m_pBuilding = pBuilding;

	m_strName = ME[L"Name"];

	m_fLevel = ME[L"FloorLevel"];
	m_box.ParseFromString(ME[L"Box"]);
}

void CBuildingBase::STOREY::Scale(AVFLOAT fScale)
{
	m_fLevel *= fScale;
	m_box *= fScale;
}
