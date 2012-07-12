// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrBuilding.h"
#include "DBTools.h"
#include <math.h>

#define F_PI	((AVFLOAT)M_PI)
#define F_PI_2	((AVFLOAT)M_PI_2)

//////////////////////////////////////////////////////////////////////////////////
// CBuildingEx

CBuildingConstr::CBuildingConstr(CProject *pProject, AVULONG nIndex) : CBuilding(pProject, nIndex)
{ 
	m_pElem = NULL; 
	bFastLoad = false; 
}

CBuildingConstr::~CBuildingConstr()
{
	Deconstruct(); 
}

void CBuildingConstr::STOREY::Construct(AVULONG iStorey)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	bool bServed = IsStoreyServed();
	bool bExisting = GetId() <= GetBuilding()->GetHighestStoreyServed();

	AVLONG iStorey2 = iStorey - GetBuilding()->GetBasementStoreyCount();

	// imposed parameters
	AVFLOAT opn = 0;	//2.5f;			// width of the opening around the door

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), iStorey, Vector(0, 0, GetLevel()));

	// collect door information
	std::vector<FLOAT> doordata[2];
	for (AVULONG iLine = 0; iLine < 2; iLine++)
		for (AVULONG iShaft = GetBuilding()->GetShaftBegin(iLine); iShaft < GetBuilding()->GetShaftEnd(iLine); iShaft++)
		{
			SHAFT *pShaft = GetBuilding()->GetShaft(iShaft);
			if (!pShaft->IsStoreyServed(iStorey)) continue;
			if (iLine == 0)
				doordata[iLine].push_back(pShaft->GetBoxDoor().Left() - GetBox().LeftExt() - opn);
			else
				doordata[iLine].push_back(GetBox().RightExt() - pShaft->GetBoxDoor().Right() - opn);
			doordata[iLine].push_back(pShaft->GetBoxDoor().Width() + opn + opn);
			doordata[iLine].push_back(pShaft->GetBoxDoor().Height() + opn);
		}

	if (bExisting)
	{
		AVVECTOR v;

		// build walls
		m_pElem->BuildWall(CElem::WALL_FLOOR,   L"Floor",   iStorey2, GetBox().LowerSlab());
		m_pElem->BuildWall(CElem::WALL_CEILING, L"Ceiling", iStorey2, GetBox().UpperSlab());

		if (GetBuilding()->bFastLoad) return;
	
		if (bServed)
		{
			v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+40, 0);
			m_pElem->BuildWall(CElem::WALL_FLOOR_NUMBER_PLATE, L"Left_Nameplate", iStorey2, BOX(v, 2, 40, 80), Vector(F_PI_2));
			v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-40, 0);
			m_pElem->BuildWall(CElem::WALL_FLOOR_NUMBER_PLATE, L"Right_Nameplate", iStorey2, BOX(v, -2, 40, 80), Vector(-F_PI_2, 0, -F_PI));
		}

//		m_pElem->BuildWall(CElem::WALL_FRONT, L"Cube",   iStorey2+1, BOX(Vector(-160, 20, 25), 40, 40, 40));

		if (GetBox().FrontThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_REAR, L"RearWall",   iStorey2, GetBox().FrontWall(), Vector(0), doordata[0].size() / 3, doordata[0].size() ? &doordata[0][0] : NULL);
		if (GetBox().RearThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_FRONT, L"FrontWall",	iStorey2, GetBox().RearWall(), Vector(0, 0, F_PI), doordata[1].size() / 3, doordata[1].size() ? &doordata[1][0] : NULL);
		if (GetBox().LeftThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_SIDE, L"LeftWall",   iStorey2, GetBox().LeftWall(), Vector(0, 0, F_PI_2));
		if (GetBox().RightThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_SIDE, L"RightWall",  iStorey2, GetBox().RightWall(), Vector(0, 0, -F_PI_2));
	}
}

void CBuildingConstr::STOREY::Deconstruct()
{
	delete m_pElem;
}

void CBuildingConstr::MACHINEROOM::Construct()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), 9999, Vector(0, 0, GetLevel()));

	// build walls
	m_pElem->BuildWall(CElem::WALL_FLOOR,   L"Floor", 0, GetBox().LowerSlab());
	if (GetBuilding()->bFastLoad) return;
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, GetBox().FrontWall());
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"FrontWall", 0, GetBox().RearWall(), Vector(0, 0, F_PI));
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, GetBox().LeftWall(), Vector(0, 0, F_PI_2));
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, GetBox().RightWall(), Vector(0, 0, -F_PI_2));
}

void CBuildingConstr::MACHINEROOM::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CBuildingConstr::PIT::Construct()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), 9998, Vector(0, 0, GetLevel()));

	// build walls - what walls?
	if (GetBuilding()->bFastLoad) return;

	// for walls - look at SHAFT::ConstructPit
}

void CBuildingConstr::PIT::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CBuildingConstr::SHAFT::Construct(AVULONG iStorey, AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// imposed parameters
//	AVFLOAT opn = 2.5f;			// width of the opening around the door
//	AVFLOAT fOpeningThickness = GetBoxDoor().Depth() * 0.4f;

	AVLONG iStorey2 = iStorey - GetBuilding()->GetBasementStoreyCount();

	// create skeletal structure (object & bone)
	if (m_pStoreyBones == NULL)
		m_pStoreyBones = new BONES[GetBuilding()->GetStoreyCount()];


	CElem *pParent = GetBuilding()->GetStoreyElement(iStorey);
	m_pStoreyBones[iStorey].m_pElem = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_SHAFT, L"Shaft %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	pParent = GetBuilding()->GetShaftElement(iStorey, iShaft);
	m_pStoreyBones[iStorey].m_pElemLobbySide = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_EXTRA, L"LobbySide", 0, Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	m_pStoreyBones[iStorey].m_pElemLeft =      GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_EXTRA, L"LeftSide",  0, Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	m_pStoreyBones[iStorey].m_pElemRight =     GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_EXTRA, L"RightSide", 0, Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));

	GetBox().SetHeight(GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey2, iShaft);

	if (iStorey <= GetBuilding()->GetHighestStoreyServed() && !GetBuilding()->bFastLoad)
	{
		if (GetBoxBeam().Width() > 0)
		{
			BOX box = GetBoxBeam();
			box.SetDepth(-box.Depth());
			box.SetHeight(-box.Height());
			GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"Beam", nIndex, box);
		}

		if (GetBox().LeftThickness() > 0)
			GetElementLeft(iStorey)->BuildWall(CElem::WALL_SHAFT, L"LeftWall", nIndex, GetBox().LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));
		if (GetBox().RightThickness() > 0)
			GetElementRight(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RightWall", nIndex, GetBox().RightWall(GetWallRtStart()), Vector(0, 0, -F_PI_2));
		if (GetBox().RearThickness() != 0)
			if (GetBoxBeam().Left() < GetBox().LeftExt())
				GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall", nIndex, GetBox().RearWall(0, GetBoxBeam().Width()), Vector(0, 0, F_PI));
			else
				GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall", nIndex, GetBox().RearWall(GetBoxBeam().Width(), 2*GetBoxBeam().Width()), Vector(0, 0, F_PI));

		if (GetBuilding()->IsStoreyServed(iStorey, iShaft))
		{
			// The Opening
//			AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
//			GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_OPENING, L"Opening", nIndex, 
//				BOX(GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), GetBoxDoor().Width() + opn + opn, fOpeningThickness, GetBoxDoor().Height() + opn),
//				Vector(0), 1, door);

			// Plates
			if (GetShaftLine() == 0)
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, F_PI));
			else
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, 0));

			// Door
			m_pStoreyBones[iStorey].m_ppDoors[0] = GetProject()->CreateElement(GetBuilding(), GetElementLobbySide(iStorey), CElem::ELEM_BONE, L"Door_Left", MAKELONG(iStorey, iShaft), Vector(0));
			m_pStoreyBones[iStorey].m_ppDoors[0]->BuildWall(CElem::WALL_DOOR, L"DoorL" , MAKELONG(iStorey, iShaft), BOX(GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Depth(), GetBoxDoor().Height()));
			m_pStoreyBones[iStorey].m_ppDoors[1] = GetProject()->CreateElement(GetBuilding(), GetElementLobbySide(iStorey), CElem::ELEM_BONE, L"Door_Right", MAKELONG(iStorey, iShaft), Vector(0));
			m_pStoreyBones[iStorey].m_ppDoors[1]->BuildWall(CElem::WALL_DOOR, L"DoorR", MAKELONG(iStorey, iShaft), BOX(GetBoxDoor().LeftRearLower() + Vector(GetBoxDoor().Width()/2), GetBoxDoor().Width()/2, GetBoxDoor().Depth(), GetBoxDoor().Height()));
		}
	}
}

void CBuildingConstr::SHAFT::ConstructMachine(AVULONG iShaft)
{
	m_pElemMachine = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetMachineRoomElement(), CElem::ELEM_SHAFT, L"Machine %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetMachineRoomLevel()));

	AVFLOAT fAngle = M_PI * (GetShaftLine() ? 3 : 1) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_MACHINE, L"Machine", iShaft, GetBoxCar(), fAngle);
	fAngle = M_PI * (GetShaftLine() ? 3 : 1) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_OVERSPEED, L"Overspeed Governor", iShaft, GetBoxGovernor(), fAngle);
	fAngle = M_PI * (GetShaftLine() ? 0 : 2) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_CONTROL, L"Control Panel", iShaft, GetBuilding()->GetBox(), fAngle);
}

void CBuildingConstr::SHAFT::ConstructPit(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	CElem *pParent = GetBuilding()->GetPitElement();
	m_PitBones.m_pElem = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_SHAFT, L"Pit %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetPitLevel()));
	pParent = GetBuilding()->GetPitElement(iShaft);
	m_PitBones.m_pElemLobbySide = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_EXTRA, L"LobbySide", 0, Vector(0, 0, GetBuilding()->GetPitLevel()));
	m_PitBones.m_pElemLeft = GetProject()->CreateElement(GetBuilding(),      pParent, CElem::ELEM_EXTRA, L"LeftSide",  0, Vector(0, 0, GetBuilding()->GetPitLevel()));
	m_PitBones.m_pElemRight = GetProject()->CreateElement(GetBuilding(),     pParent, CElem::ELEM_EXTRA, L"RightSide", 0, Vector(0, 0, GetBuilding()->GetPitLevel()));
	
	BOX box = GetBox();
	box.SetHeight(-GetBuilding()->GetPitLevel());
	box.SetFrontThickness(box.RearThickness());

	if (GetBoxBeam().Width() > 0)
	{
		BOX box = GetBoxBeam();
		box.SetDepth(-box.Depth());
		GetPitElement()->BuildWall(CElem::WALL_BEAM, L"Beam", 0, box);
	}

	if (box.LeftThickness() > 0)
		GetPitElementLeft()->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, box.LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));
	if (box.RightThickness() > 0)
		GetPitElementRight()->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, box.RightWall(GetWallRtStart()), Vector(0, 0, -F_PI_2));
	if (box.RearThickness() != 0)
		if (GetBoxBeam().Left() < GetBox().LeftExt())
		{
			GetPitElement()->BuildWall(CElem::WALL_SHAFT, L"FrontWall", 0, box.RearWall(0, GetBoxBeam().Width()), Vector(0, 0, F_PI));
			GetPitElementLobbySide()->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, box.FrontWall(-GetBoxBeam().Width(), 0));
		}
		else
		{
			GetPitElement()->BuildWall(CElem::WALL_SHAFT, L"FrontWall", 0, box.RearWall(GetBoxBeam().Width(), 2*GetBoxBeam().Width()), Vector(0, 0, F_PI));
			GetPitElementLobbySide()->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, box.FrontWall(0, GetBoxBeam().Width()));
		}

	// Components
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER, L"Car Buffer", iShaft, GetBoxCar()); 
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER, L"Counterweight Buffer", iShaft, GetBoxCwt());
	GetPitElement()->BuildModel(CElem::MODEL_PULLEY, L"Governor Tension Pulley", iShaft, GetBoxGovernor(), F_PI/2);
	
	AVFLOAT fAngle = M_PI * (GetShaftLine() ? 3 : 1) / 2;
	GetPitElement()->BuildModel(CElem::MODEL_LADDER, L"Pit Ladder", iShaft, GetBoxLadder(), fAngle);
	
	box = GetBoxCarMounting();
	box.SetHeight(abs(GetBuilding()->GetPitLevel()) + GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness());
	GetPitElement()->BuildModel(CElem::MODEL_RAIL_CAR, L"Car Guide Rail", iShaft, box);
	
	box = GetBoxCwt();
	box.SetHeight(abs(GetBuilding()->GetPitLevel()) + GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness());
	GetPitElement()->BuildModel(CElem::MODEL_RAIL_CWT, L"Counterweight Guide Rail", iShaft, box);

	box = GetBoxCwt();
	box.Move(0, 0, abs(GetBuilding()->GetPitLevel()) + GetBuilding()->GetStorey(GetBuilding()->GetHighestStoreyServed())->GetLevel());
	
	GetPitElement()->BuildModel(CElem::MODEL_CWT, L"Counterweight", iShaft, box);
}

void CBuildingConstr::SHAFT::Deconstruct()
{
	if (m_pStoreyBones == NULL) return;
	for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
	{
		delete m_pStoreyBones[i].m_pElem;
		delete m_pStoreyBones[i].m_pElemLobbySide;
		delete m_pStoreyBones[i].m_pElemLeft;
		delete m_pStoreyBones[i].m_pElemRight;
		for (AVULONG j = 0; j < MAX_DOORS; j++)
			if (m_pStoreyBones[i].m_ppDoors[j]) delete m_pStoreyBones[i].m_ppDoors[j];
	}
	delete [] m_pStoreyBones;
	m_pStoreyBones = NULL;

	if (m_pElemMachine) delete m_pElemMachine;
	m_pElemMachine = NULL;

	if (m_PitBones.m_pElem) delete m_PitBones.m_pElem; m_PitBones.m_pElem = NULL;
	if (m_PitBones.m_pElemLobbySide) delete m_PitBones.m_pElemLobbySide; m_PitBones.m_pElemLobbySide = NULL;
	if (m_PitBones.m_pElemLeft) delete m_PitBones.m_pElemLeft; m_PitBones.m_pElemLeft = NULL;
	if (m_PitBones.m_pElemRight) delete m_PitBones.m_pElemRight; m_PitBones.m_pElemRight = NULL;
}

void CBuildingConstr::LIFT::Construct(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	AVULONG nStartingStorey = 0;

	// Create skeletal elements (entire lift)
	m_pElem = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetElement(), CElem::ELEM_LIFT, (LPOLESTR)GetName().c_str(), iShaft, GetShaft()->GetLiftPos(nStartingStorey));

	for (AVULONG iDeck = 0; iDeck < GetShaft()->GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_ppDecks[iDeck] = GetProject()->CreateElement(GetBuilding(), m_pElem, CElem::ELEM_DECK, L"Deck_%d", iDeck, Vector(0, 0, GetBuilding()->GetGroundStorey(iDeck)->GetLevel() - GetBuilding()->GetGroundStorey()->GetLevel())); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		BOX box = GetShaft()->GetBoxCar() - GetShaft()->GetLiftPos(0);
		BOX boxDoor0 = GetShaft()->GetBoxCarDoor(0) - GetShaft()->GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };

		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT_FLOOR,   L"Floor",     iShaft, box.LowerSlab());
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT_CEILING, L"Ceiling",   iShaft, box.UpperSlab());

		m_ppDoors[0] = GetProject()->CreateElement(GetBuilding(), m_ppDecks[iDeck], CElem::ELEM_BONE, L"Door_Left", iShaft, Vector(0));
		m_ppDoors[0]->BuildWall(CElem::WALL_LIFT_DOOR, L"DoorL",     iShaft, BOX(boxDoor0.LeftFrontLower(), boxDoor0.Width()/2, -boxDoor0.Depth(), boxDoor0.Height()));
		m_ppDoors[1] = GetProject()->CreateElement(GetBuilding(), m_ppDecks[iDeck], CElem::ELEM_BONE, L"Door_Right", iShaft, Vector(0));
		m_ppDoors[1]->BuildWall(CElem::WALL_LIFT_DOOR, L"DoorR",     iShaft, BOX(boxDoor0.LeftFrontLower() + Vector(boxDoor0.Width()/2), boxDoor0.Width()/2, -boxDoor0.Depth(), boxDoor0.Height()));
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"FrontWall", iShaft, box.FrontWall(), Vector(0), 1, door);
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"RearWall",  iShaft, box.RearWall(), Vector(0, 0, F_PI));
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"LeftWall",  iShaft, box.LeftWall(), Vector(0, 0, F_PI_2));
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"RightWall", iShaft, box.RightWall(), Vector(0, 0, -F_PI_2));
	}
}

void CBuildingConstr::LIFT::Deconstruct()
{
	delete m_pElem;

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (m_ppDecks[i]) delete m_ppDecks[i];
	memset(m_ppDecks, 0, sizeof(m_ppDecks));
	for (AVULONG i = 0; i < MAX_DOORS; i++)
		if (m_ppDoors[i]) delete m_ppDoors[i];
	memset(m_ppDoors, 0, sizeof(m_ppDoors));
}

void CBuildingConstr::Construct(AVVECTOR vec)
{
	Move(vec.x, vec.y, vec.z);

	m_pElem = GetProject()->CreateElement(this, GetProject()->GetSiteElement(), CElem::ELEM_BUILDING, (LPOLESTR)GetName().c_str(), 0, Vector(0));

	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
		GetLift(iLift)->Construct(iLift);

	if (GetPit())
	{
		GetPit()->Construct();
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->ConstructPit(iShaft);
	}

	for (AVULONG iStorey = 0; iStorey < GetStoreyCount(); iStorey++)
	{
		GetStorey(iStorey)->Construct(iStorey);
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->Construct(iStorey, iShaft);
	}
	if (GetMachineRoom())
	{
		GetMachineRoom()->Construct();
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->ConstructMachine(iShaft);
	}
}

void CBuildingConstr::Deconstruct()
{
	if (m_pElem) delete m_pElem;

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Deconstruct();
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Deconstruct();

	if (GetMachineRoom())
		GetMachineRoom()->Deconstruct();
	if (GetPit())
		GetPit()->Deconstruct();
}




