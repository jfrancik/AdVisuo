// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase

CBuildingBase::CBuildingBase(void)
{
	ppShafts = NULL;
	ppStoreys = NULL;

	NoOfShafts = 0;
	PosLiftBookM = 0;
	NoOfBook = 0;
	LobbyDepth = 0;
	FrontWallThickness = 200;
	SideWallThickness = -1;
	ShaftWallThickness = -1;
	IntDivBeamWidth = -1;
	IntDivBeamHeight = -1;
	MachRoomSlab = -1;
	LiftBeamHeight = -1;
	Structure = STRUCT_UNKNOWN;
	LobbyCeilingSlabHeight = 1100;
	StoreysAboveGround = 0;
	StoreysBelowGround = 0;
	LiftShaftArrang = SHAFT_INLINE;
	LobbyArrangement = LOBBY_OPENPLAN;

	LobbyWidth = 0;		// unspecified at this stage

	fScale = 0.0;
}

CBuildingBase::~CBuildingBase(void)
{
	DeleteShafts();
	DeleteStoreys();
}

void CBuildingBase::CreateShafts()
{
	DeleteShafts();
	ppShafts = new SHAFT*[GetShaftCount()];
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		ppShafts[i] = CreateShaft();
}

void CBuildingBase::DeleteShafts()
{
	if (!ppShafts) return;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		if (ppShafts[i]) delete ppShafts[i];
	delete [] ppShafts;
	ppShafts = NULL;
}

void CBuildingBase::CreateStoreys()
{
	DeleteStoreys();
	ppStoreys = new STOREY*[GetStoreyCount()];
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		ppStoreys[i] = CreateStorey();
}

void CBuildingBase::DeleteStoreys()
{
	if (!ppStoreys) return;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		if (ppStoreys[i]) delete ppStoreys[i];
	delete [] ppStoreys;
	ppStoreys = NULL;
}

void CBuildingBase::Resolve()
{
	// calculate width of the lobby if lifts in-line, no side walls
	AVFLOAT w = -IntDivBeamWidth;
	AVULONG i;
	for (i = 0; i < GetShaftCount(); i++)
		w += GetShaft(i)->GetShaftWidth() + IntDivBeamWidth;

	if (LiftShaftArrang != SHAFT_OPPOSITE)
	{
		// in-line
		ShaftLinesCount = 1;
		ShaftCount[0] = GetShaftCount();
		ShaftCount[1] = 0;
		LineWidth[0] = w + 2 * SideWallThickness;
		LineWidth[1] = 0;
		LobbyWidth = LineWidth[0];
	}
	else
	{
		w -= IntDivBeamWidth;
		AVFLOAT w0 = -IntDivBeamWidth;
		for (i = 0; i < GetShaftCount() && w0 + GetShaft(i)->GetShaftWidth() / 2 < w/2; i++)
		{
			AVFLOAT wx = GetShaft(i)->GetShaftWidth() + IntDivBeamWidth;
			if (w0 + wx/2 > w / 2) break;
			w0 += wx;
		}
		ShaftCount[0] = i;
		ShaftCount[1] = GetShaftCount() - ShaftCount[0];
		LineWidth[0] = w0 + 2 * SideWallThickness;
		LineWidth[1] = w - w0 + 2 * SideWallThickness;
		LobbyWidth = max(LineWidth[0], LineWidth[1]);
	}

	for (AVULONG i = 0; i < GetShaftCount(); i++)  GetShaft(i)->Resolve(this, i);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) GetStorey(i)->Resolve(this, i);
}

void CBuildingBase::Scale(AVFLOAT fScale)
{
	this->fScale = fScale;
	if (SideWallThickness <= 0) SideWallThickness = FrontWallThickness;

	AVFLOAT fWidth = LobbyWidth*fScale;
	AVFLOAT fDepth = LobbyDepth*fScale;
	AVFLOAT fLt = 0; if (LobbyArrangement == LOBBY_DEADEND_LEFT)  fLt = SideWallThickness*fScale;
	AVFLOAT fRt = 0; if (LobbyArrangement == LOBBY_DEADEND_RIGHT) fRt = SideWallThickness*fScale;
	AVFLOAT fFt = FrontWallThickness*fScale;
	AVFLOAT fRr = FrontWallThickness*fScale; if (LobbyArrangement == LOBBY_OPENPLAN) fRr = 0;

	// temporarily closed space!
	fLt = fRt = SideWallThickness*fScale;

	m_box = BOX(-fWidth/2 + fLt, -fDepth/2, 0, fWidth - fLt - fRt, fDepth, 0);
	m_box.SetThickness(fLt, fRt, fFt, fRr, 1, LobbyCeilingSlabHeight*fScale);
	for (AVULONG i = 0; i < GetShaftCount(); i++) GetShaft(i)->Scale(fScale);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) GetStorey(i)->Scale(fScale);
}

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::SHAFT

CBuildingBase::SHAFT::SHAFT() : m_pBuilding(NULL)
{
	ShaftID = 0;				// unspecified at this stage
	Capacity = 1275;
	Speed = 2.5f;
	Acceleration = 1.0f;
	Jerk = 1.6f;
	DoorType = DOOR_CENTRE;
	LiftDoorHeight = 2200;
	LiftDoorWidth = 1100;
	CarHeight = 2400;
	TypeOfLift = LIFT_SINGLE_DECK;
	NumberOfLifts = 1;
	NoOfCarEntrances = CAR_FRONT;
	CounterWeightPosition = CNTRWEIGHT_REAR;
	CarWidth = 2000;
	CarDepth = 1400;
	ShaftWidth = 2600;
	ShaftDepth = 2400;
	PitDepth = 1800;
	OverallHeight = 3000;
	HeadRoom = 4950;
	MachRoomHeight = 2450;
	MachRoomExt = -1.0f;
	PreOperTime = 0;
	OpeningTime = 1800;
	ClosingTime = 2600;
	MotorStartDelay = 500;
	LoadingTime = 1100;
	UnloadingTime = 500;
	FloorsServed = L"";
}

CBuildingBase::SHAFT::~SHAFT()
{
}

void CBuildingBase::SHAFT::Resolve(CBuildingBase *pBuilding, AVULONG nIndex)
{
	m_pBuilding = pBuilding;
	AVULONG nLine0 = pBuilding->GetShaftCount(0);

	ShaftIndex = nIndex;
	ShaftLine = nIndex >= nLine0 ? 1 : 0;

	if (ShaftLine == 0)
	{
		ShaftPos = (nIndex > 0)			? pBuilding->GetShaft(nIndex-1)->ShaftRPos + pBuilding->IntDivBeamWidth : pBuilding->ShaftWallThickness;
		ShaftRPos = ShaftPos + ShaftWidth;
	}
	else
	{
		ShaftRPos = (nIndex > nLine0)	? pBuilding->GetShaft(nIndex-1)->ShaftPos - pBuilding->IntDivBeamWidth : pBuilding->LobbyWidth - pBuilding->ShaftWallThickness;
		ShaftPos = ShaftRPos - ShaftWidth;
	}

	if (MachRoomExt == -1.0f)
		if (TypeOfLift == LIFT_DOUBLE_DECK) MachRoomExt = 1000.0; else MachRoomExt = 0;
}

void CBuildingBase::SHAFT::Scale(AVFLOAT fScale)
{
	ASSERT(m_pBuilding); if (!m_pBuilding) return;

	BOX lobbyBox = m_pBuilding->m_box;

	AVFLOAT LeftBeam = m_pBuilding->IntDivBeamWidth;
	AVFLOAT RightBeam = m_pBuilding->IntDivBeamWidth;

	switch (ShaftLine)
	{
		case 0:
			if (ShaftIndex == 0) LeftBeam = m_pBuilding->ShaftWallThickness;
			if (ShaftIndex == m_pBuilding->GetShaftCount(0)-1) RightBeam = m_pBuilding->ShaftWallThickness;
			break;
		case 1:
			if (ShaftIndex == m_pBuilding->GetShaftCount(0)) RightBeam = m_pBuilding->ShaftWallThickness;
			if (ShaftIndex == m_pBuilding->GetShaftCount()-1) LeftBeam = m_pBuilding->ShaftWallThickness;
			break;
	}
	

	// imposed parameters
	AVFLOAT wallThickness = m_pBuilding->IntDivBeamWidth*fScale;	// lift wall thickness
	AVFLOAT doorThickness = lobbyBox.FrontThickness() / 4;		// lift (external) door thickness
	AVFLOAT gap = 1.0f;											// gap between lift & floor

	if (ShaftLine == 0)
	{
		m_box = BOX(lobbyBox.Left() + ShaftPos*fScale, lobbyBox.FrontExt(), ShaftWidth*fScale, -ShaftDepth*fScale);
		m_box.SetThickness(LeftBeam*fScale, RightBeam*fScale, 0, -m_pBuilding->ShaftWallThickness*fScale);

		m_boxDoor = BOX(m_box.Left() + (m_box.Width() - LiftDoorWidth*fScale) / 2, m_box.Front(),               0, LiftDoorWidth*fScale, doorThickness,      LiftDoorHeight*fScale);

		m_boxCar = BOX(m_box.Left() + (m_box.Width() - CarWidth     *fScale) / 2, m_box.Front()-wallThickness-gap, 0, CarWidth     *fScale, -CarDepth * fScale, CarHeight     *fScale);
		m_boxCar.SetThickness(wallThickness, wallThickness, -wallThickness, -wallThickness, wallThickness, wallThickness);
	}
	else
	{
		m_box = BOX(lobbyBox.Left() + ShaftPos*fScale, lobbyBox.RearExt(), ShaftWidth*fScale, ShaftDepth*fScale);
		m_box.SetThickness(LeftBeam*fScale, RightBeam*fScale, 0, m_pBuilding->ShaftWallThickness*fScale);

		m_boxDoor = BOX(m_box.Left() + (m_box.Width() - LiftDoorWidth*fScale) / 2, m_box.Front(),               0, LiftDoorWidth*fScale, doorThickness,     LiftDoorHeight*fScale);

		m_boxCar = BOX(m_box.Left() + (m_box.Width() - CarWidth     *fScale) / 2, m_box.Front()+wallThickness+gap, 0, CarWidth     *fScale, CarDepth * fScale, CarHeight     *fScale);
		m_boxCar.SetThickness(wallThickness, wallThickness, wallThickness, wallThickness, wallThickness, wallThickness);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase::STOREY

CBuildingBase::STOREY::STOREY() : m_pBuilding(NULL)
{
	StoreyID = 0;
	HeightValue = 4000;
	Name = L"";
	Area = 0;
	PopDensity = 0;
	Absentee = 0;
	StairFactor = 0;
	Escalator = 0;
}

CBuildingBase::STOREY::~STOREY()
{
}

void CBuildingBase::STOREY::Resolve(CBuildingBase *pBuilding, AVULONG nIndex)
{
	m_pBuilding = pBuilding;
	CBuildingBase::STOREY *pPrev = nIndex > 0 ? m_pBuilding->GetStorey(nIndex - 1) : NULL;

	StoreyLevel = 0;
	if (pPrev)
		StoreyLevel = pPrev->StoreyLevel + pPrev->HeightValue;
}

void CBuildingBase::STOREY::Scale(AVFLOAT fScale)
{ 
	SH = HeightValue*fScale; SL = StoreyLevel*fScale;
	AVFLOAT ceiling = m_pBuilding->LobbyCeilingSlabHeight*fScale;
	m_box = m_pBuilding->m_box;
	m_box.SetHeight(SH - ceiling);
}


