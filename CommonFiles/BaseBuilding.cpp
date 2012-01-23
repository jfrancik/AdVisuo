// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase

CBuildingBase::CBuildingBase(void) : m_ppShafts(NULL), m_ppStoreys(NULL), m_nShaftCount(0), m_nStoreyCount(0), m_nBasementStoreyCount(0),
									 nBuildingID(0), LiftShaftArrang(SHAFT_INLINE), LobbyArrangement(LOBBY_OPENPLAN), m_fScale(1.0)
{
	ShaftCount[0] = ShaftCount[1] = 0;
	LineWidth[0] = LineWidth[1] = 0;
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

void CBuildingBase::PreCreate()
{
	CreateShafts(ME[L"NoOfShafts"]);
	CreateStoreys((ULONG)ME[L"FloorsAboveGround"] + (ULONG)ME[L"FloorsBelowGround"], (ULONG)ME[L"FloorsBelowGround"]);
	SetId(ME[L"ID"]);
}

void CBuildingBase::Create()
{
	// resolve vars
	LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangement"];
	LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangement"];

	AVFLOAT fLobbyCeilingSlabHeight = ME[L"LobbyCeilingSlabHeight"];
	AVFLOAT fLobbyDepth = ME[L"LobbyDepth"];
	AVFLOAT fFrontWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fIntDivBeamWidth = ME[L"IntDivBeamWidth"];
	AVFLOAT fIntDivBeamHeight = ME[L"IntDivBeamHeight"];
	AVFLOAT fSideWallThickness = fFrontWallThickness;
	AVFLOAT fShaftWallThicknessRear = fFrontWallThickness;
	AVFLOAT fShaftWallThicknessSide = fFrontWallThickness;

	// calculate width of the lobby if lifts in-line, no side walls
	AVFLOAT w = 0;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		GetShaft(i)->Create(this, i, fFrontWallThickness, fShaftWallThicknessRear);
		w += GetShaft(i)->GetBox().Width();
	}
	w += fIntDivBeamWidth * (GetShaftCount() - 1);

	// calculate ShaftCount, LineWidth
	if (GetLiftShaftArrang() != SHAFT_OPPOSITE)
	{
		// in-line
		ShaftCount[0] = GetShaftCount();
		ShaftCount[1] = 0;
		LineWidth[0] = w + 2 * fSideWallThickness;
		LineWidth[1] = 0;
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
		ShaftCount[0] = i;
		ShaftCount[1] = GetShaftCount() - ShaftCount[0];
		LineWidth[0] = w0 + 2 * fSideWallThickness;
		LineWidth[1] = w - w0 + 2 * fSideWallThickness;
	}
	AVFLOAT LobbyWidth = max(LineWidth[0], LineWidth[1]);

	// calculate lobby box
	AVFLOAT fLt = 0; if (LobbyArrangement == LOBBY_DEADEND_LEFT)  fLt = fSideWallThickness;
	AVFLOAT fRt = 0; if (LobbyArrangement == LOBBY_DEADEND_RIGHT) fRt = fSideWallThickness;
	fLt = fRt = fSideWallThickness;	// temporarily closed space!

	m_box = BOX(-LobbyWidth/2 + fLt, -fLobbyDepth/2, 0, LobbyWidth - fLt - fRt, fLobbyDepth, 0);
	m_box.SetThickness(fLt, fRt, fFrontWallThickness, (LobbyArrangement == LOBBY_OPENPLAN) ? 0 : fFrontWallThickness, 1, fLobbyCeilingSlabHeight);


	// Resolve Shafts and Storeys

	// 1st line of shafts
	AVFLOAT fShaftPosX = GetBox().LeftExt() + fShaftWallThicknessSide;
	AVFLOAT fShaftPosY = GetBox().FrontExt();
	AVFLOAT fPrevDepth = 0;
	for (AVULONG i = 0; i < GetShaftCount(0); i++)
	{
		SHAFT *pShaft = GetShaft(i);

		// setup basic structure
		pShaft->Create(0, fShaftPosX, fShaftPosY);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
			// no previous shaft: build a wall, not beam
			pShaft->CreateLeftWall(fShaftWallThicknessSide);
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->CreateLeftBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
		else if (fDepth < fPrevDepth)	// note: both values are negative here!
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->CreateLeftBeam(fIntDivBeamWidth, fPrevDepth, fIntDivBeamHeight);
			pShaft->CreateLeftWall(fShaftWallThicknessRear, fPrevDepth - fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->CreateLeftBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
			GetShaft(i-1)->CreateRightWall(fShaftWallThicknessRear, fDepth - fShaftWallThicknessRear);
		}
		fPrevDepth = fDepth;

		// move on
		fShaftPosX += pShaft->GetBox().Width() + fIntDivBeamWidth;
	}
	GetShaft(GetShaftCount(0) - 1)->CreateRightWall(fShaftWallThicknessSide);

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
		pShaft->Create(1, fShaftPosX, fShaftPosY);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
			// no previous shaft: build a wall, not beam
			pShaft->CreateRightWall(fShaftWallThicknessSide);
		else if (fDepth == fPrevDepth)
			// same depth as previous: just build a beam
			pShaft->CreateRightBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
		else if (fDepth > fPrevDepth)
		{	// deeper than previous: build a shorter beam and add a wall section
			pShaft->CreateRightBeam(fIntDivBeamWidth, fPrevDepth, fIntDivBeamHeight);
			pShaft->CreateRightWall(fShaftWallThicknessRear, fPrevDepth + fShaftWallThicknessRear);
		}
		else
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->CreateRightBeam(fIntDivBeamWidth, fDepth, fIntDivBeamHeight);
			GetShaft(i-1)->CreateLeftWall(fShaftWallThicknessRear, fDepth + fShaftWallThicknessRear);
		}
		fPrevDepth = fDepth;

		// move on
		fShaftPosX -= fIntDivBeamWidth;
	}
	GetShaft(GetShaftCount() - 1)->CreateLeftWall(fShaftWallThicknessSide);

	AVFLOAT fLevel = 0;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);
		pStorey->Create(this, i, fLevel);
		fLevel += pStorey->GetHeight();
	}
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

void CBuildingBase::SHAFT::Create(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fFrontWall, AVFLOAT fRearWall)
{
	m_nId = nId;
	m_pBuilding = pBuilding;
	m_nShaftLine = 0;
	
	m_nLiftCount = ME[L"NumberOfLifts"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"TypeOfLift"];

	AVFLOAT ShaftWidth = ME[L"ShaftWidth"];
	AVFLOAT ShaftDepth = ME[L"ShaftDepth"];
	AVFLOAT CarDepth = ME[L"CarDepth"];
	AVFLOAT CarWidth = ME[L"CarWidth"];
	AVFLOAT CarHeight = ME[L"CarHeight"];
	AVFLOAT LiftDoorHeight = ME[L"LiftDoorHeight"];
	AVFLOAT LiftDoorWidth = ME[L"LiftDoorWidth"];

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
}

void CBuildingBase::SHAFT::Create(AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY)
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
}

void CBuildingBase::SHAFT::CreateLeftBeam(AVFLOAT w, AVFLOAT d, AVFLOAT h)
{
	AVVECTOR pos = m_boxShaft.LeftFrontLower();
	pos.x -= w;
	AVVECTOR size = { w, d, h };
	m_boxBeam = BOX(pos, pos + size);
}

void CBuildingBase::SHAFT::CreateRightBeam(AVFLOAT w, AVFLOAT d, AVFLOAT h)
{
	AVVECTOR pos = m_boxShaft.RightFrontLower();
	AVVECTOR size = { w, d, h };
	m_boxBeam = BOX(pos, pos + size);
}

void CBuildingBase::SHAFT::CreateLeftWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetLeftThickness(fThickness);
	m_fWallLtStart = fStart;
}

void CBuildingBase::SHAFT::CreateRightWall(AVFLOAT fThickness, AVFLOAT fStart)
{
	m_boxShaft.SetRightThickness(fThickness);
	m_fWallRtStart = fStart;
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

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::STOREY

CBuildingBase::STOREY::STOREY() : m_nId(0), m_pBuilding(NULL), m_fLevel(0)
{ 
}

void CBuildingBase::STOREY::Create(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fLevel)
{
	m_nId = nId;
	m_pBuilding = pBuilding;
	m_fLevel = fLevel;
	m_strName = ME[L"Name"];
	m_box = m_pBuilding->m_box;
	m_box.SetHeight((AVFLOAT)ME[L"HeightValue"] - m_box.UpperThickness());
}

void CBuildingBase::STOREY::Scale(AVFLOAT fScale)
{
	m_fLevel *= fScale;
	m_box *= fScale;
}
