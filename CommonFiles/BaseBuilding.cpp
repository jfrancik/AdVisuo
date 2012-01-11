// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingBase

CBuildingBase::CBuildingBase(void)
{
	m_nShaftCount = 0;
	m_nStoreyCount = 0;
	m_nBasementStoreyCount = 0;

	ppShafts = NULL;
	ppStoreys = NULL;

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

void CBuildingBase::CreateShafts(AVULONG nShaftCount)
{
	DeleteShafts();
	m_nShaftCount = nShaftCount;
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

void CBuildingBase::CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount)
{
	DeleteStoreys();
	m_nStoreyCount = nStoreyCount;
	m_nBasementStoreyCount = nBasementStoreyCount;
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

void CBuildingBase::dupaCorrect()
{
#ifdef __ADV_DLL
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
#endif
}

void CBuildingBase::dupaSetupVars()
{
	BuildingName = ME[L"BuildingName"];
	LobbyArrangement = (LOBBY_ARRANGEMENT)(ULONG)ME[L"LobbyArrangement"];
	PosLiftBookM = ME[L"PosLiftBookM"];
	NoOfBook = ME[L"NoOfBook"];
	LobbyCeilingSlabHeight = ME[L"LobbyCeilingSlabHeight"];
	LiftShaftArrang = (SHAFT_ARRANGEMENT)(ULONG)ME[L"LiftShaftArrangement"];
	LobbyDepth = ME[L"LobbyDepth"];
	FrontWallThickness = ME[L"FrontWallThickness"];
	Structure = (LIFT_STRUCTURE)(ULONG)ME[L"Structure"];
	MachRoomSlab = ME[L"MachRoomSlab"];
	LiftBeamHeight = ME[L"LiftBeamHeight"];
	IntDivBeamWidth = ME[L"IntDivBeamWidth"];
	IntDivBeamHeight = ME[L"IntDivBeamHeight"];
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

void CBuildingBase::SHAFT::dupaCorrect()
{
#ifdef __ADV_DLL
	if      (ME[L"DoorType"].as_wstring() == L"Centre")	ME[L"DoorType"] = (ULONG)DOOR_CENTRE;
	else if (ME[L"DoorType"].as_wstring() == L"Side")	ME[L"DoorType"] = (ULONG)DOOR_SIDE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"door type", (ME[L"DoorType"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME[L"TypeOfLift"].as_wstring() == L"Single Deck") ME[L"TypeOfLift"] = (ULONG)LIFT_SINGLE_DECK;
	else if (ME[L"TypeOfLift"].as_wstring() == L"Double Deck") ME[L"TypeOfLift"] = (ULONG)LIFT_DOUBLE_DECK;
	else if (ME[L"TypeOfLift"].as_wstring() == L"Multi Car")   ME[L"TypeOfLift"] = (ULONG)LIFT_MULTI_CAR;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"type of lift", (ME[L"TypeOfLift"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME[L"NoOfCarEntrances"].as_wstring() == L"Front") ME[L"NoOfCarEntrances"] = (ULONG)CAR_FRONT;
	else if (ME[L"NoOfCarEntrances"].as_wstring() == L"Rear")  ME[L"NoOfCarEntrances"] = (ULONG)CAR_REAR;
	else if (ME[L"NoOfCarEntrances"].as_wstring() == L"Both")  ME[L"NoOfCarEntrances"] = (ULONG)CAR_BOTH;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"configuration of car entrances", (ME[L"NoOfCarEntrances"]).as_wstring().c_str()), ERROR_GENERIC);
	if      (ME[L"CounterWeightPosition"].as_wstring() == L"Rear") ME[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_REAR;
	else if (ME[L"CounterWeightPosition"].as_wstring() == L"Side") ME[L"CounterWeightPosition"] = (ULONG)CNTRWEIGHT_SIDE;
	else throw (Log(ERROR_UNRECOGNISED_STRING, L"counterweight position", (ME[L"CounterWeightPosition"]).as_wstring().c_str()), ERROR_GENERIC);
#endif
}

void CBuildingBase::SHAFT::dupaSetupVars()
{
	Acceleration = ME[L"Acceleration"];
	Capacity = ME[L"Capacity"];
	CarDepth = ME[L"CarDepth"];
	CarHeight = ME[L"CarHeight"];
	CarWidth = ME[L"CarWidth"];
	ClosingTime = ME[L"ClosingTime"];
	CounterWeightPosition = (CNTRWEIGHT_POS)(ULONG)ME[L"CounterWeightPosition"];
	DoorType = (DOOR_TYPE)(ULONG)ME[L"DoorType"];
	FloorsServed = ME[L"FloorsServed"];
	HeadRoom = ME[L"HeadRoom"];
	Jerk = ME[L"Jerk"];
	LiftDoorHeight = ME[L"LiftDoorHeight"];
	LiftDoorWidth = ME[L"LiftDoorWidth"];
	LoadingTime = ME[L"LoadingTime"];
	MachRoomExt = ME[L"MachRoomExt"];
	MachRoomHeight = ME[L"MachRoomHeight"];
	MotorStartDelay = ME[L"MotorStartDelay"];
	NoOfCarEntrances = (CAR_ENTRANCES)(ULONG)ME[L"NoOfCarEntrances"];
	NumberOfLifts = ME[L"NumberOfLifts"];
	OpeningTime = ME[L"OpeningTime"];
	OverallHeight = ME[L"OverallHeight"];
	PitDepth = ME[L"PitDepth"];
	PreOperTime = ME[L"PreOperTime"];
	ShaftDepth = ME[L"ShaftDepth"];
	ShaftID = ME[L"ShaftID"];
	ShaftWidth = ME[L"ShaftWidth"];
	Speed = ME[L"Speed"];
	TypeOfLift = (TYPE_OF_LIFT)(ULONG)ME[L"TypeOfLift"];
	UnloadingTime = ME[L"UnloadingTime"];
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

void CBuildingBase::STOREY::dupaCorrect()
{
}

void CBuildingBase::STOREY::dupaSetupVars()
{
	StoreyID = ME[L"FloorID"];
	HeightValue = ME[L"HeightValue"];
	Area = ME[L"Area"];
	PopDensity = ME[L"PopDensity"];
	Absentee = ME[L"Absentee"];
	StairFactor = ME[L"StairFactor"];
	Escalator = ME[L"Escalator"];
	Name = ME[L"Name"];
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


