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
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 2.5f;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetBuilding());
	m_pElem->Create(GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), Vector(0, 0, GetLevel()));

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
		m_pElem->AddWall(CElem::WALL_FLOOR,   L"Floor",   iStorey2, GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), GetBox().LowerThickness(), GetBox().DepthExt());
		m_pElem->AddWall(CElem::WALL_CEILING, L"Ceiling", iStorey2, GetBox().LeftExtRearExtUpper(), GetBox().WidthExt(), GetBox().UpperThickness(), GetBox().DepthExt());

		if (GetBuilding()->bFastLoad) return;
	
		if (bServed)
		{
			v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+40, 0);
			m_pElem->AddWall(CElem::WALL_FLOOR_NUMBER_PLATE, L"Left_Nameplate", iStorey2, v, 2, 80, 40, Vector(F_PI_2));
			v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-40, 0);
			m_pElem->AddWall(CElem::WALL_FLOOR_NUMBER_PLATE, L"Right_Nameplate", iStorey2, v, -2, 80, 40, Vector(-F_PI_2, 0, -F_PI));
		}

//		m_pElem->AddWall(CElem::WALL_FRONT, L"Cube",   iStorey2+1, Vector(-160, 20, 25), 40, 40, 40);

		if (GetBox().FrontThickness() > 0)
			m_pElem->AddWall(CElem::WALL_REAR, L"RearWall",		iStorey2, GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0),
				doordata[0].size() / 3, doordata[0].size() ? &doordata[0][0] : NULL);
		if (GetBox().RearThickness() > 0)
			m_pElem->AddWall(CElem::WALL_FRONT, L"FrontWall",	iStorey2, GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(0, 0, F_PI),
				doordata[1].size() / 3, doordata[1].size() ? &doordata[1][0] : NULL);
		if (GetBox().LeftThickness() > 0)
			m_pElem->AddWall(CElem::WALL_SIDE, L"LeftWall", iStorey2, GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(0, 0, F_PI_2));
		if (GetBox().RightThickness() > 0)
			m_pElem->AddWall(CElem::WALL_SIDE, L"RightWall", iStorey2, GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(0, 0, -F_PI_2));
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
	m_pElem = GetProject()->CreateElement(GetBuilding());
	m_pElem->Create(GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), Vector(0, 0, GetLevel()));

	// build walls
	m_pElem->AddWall(CElem::WALL_FLOOR,   L"Floor", 0, GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), -GetBox().LowerThickness(), GetBox().DepthExt());
	if (GetBuilding()->bFastLoad) return;
	m_pElem->AddWall(CElem::WALL_SHAFT, L"RearWall", 0, GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0));
	m_pElem->AddWall(CElem::WALL_SHAFT, L"FrontWall", 0, GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(0, 0, F_PI));
	m_pElem->AddWall(CElem::WALL_SHAFT, L"LeftWall", 0, GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(0, 0, F_PI_2));
	m_pElem->AddWall(CElem::WALL_SHAFT, L"RightWall", 0, GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(0, 0, -F_PI_2));
}

void CBuildingConstr::MACHINEROOM::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CBuildingConstr::PIT::Construct()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetBuilding());
	m_pElem->Create(GetBuilding()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), Vector(0, 0, GetLevel()));

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
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 2.5f;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)
	AVFLOAT fDoorThickness = GetBoxDoor().Depth() * 0.2f;
	AVFLOAT fOpeningThickness = GetBoxDoor().Depth() * 0.4f;

	AVLONG iStorey2 = iStorey - GetBuilding()->GetBasementStoreyCount();

	// create skeletal structure (object & bone)
	if (m_pStoreyBones == NULL)
	{
		m_pStoreyBones = new BONES[GetBuilding()->GetStoreyCount()];
		for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
		{
			m_pStoreyBones[i].m_pElem = GetProject()->CreateElement(GetBuilding());
			m_pStoreyBones[i].m_pElemLobbySide = GetProject()->CreateElement(GetBuilding());
			m_pStoreyBones[i].m_pElemLeft = GetProject()->CreateElement(GetBuilding());
			m_pStoreyBones[i].m_pElemRight = GetProject()->CreateElement(GetBuilding());
		}
	}

	CElem *pParent = GetBuilding()->GetStoreyElement(iStorey);
	GetElement(iStorey)->Create(pParent, CElem::ELEM_SHAFT, L"Shaft %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	pParent = GetBuilding()->GetShaftElement(iStorey, iShaft);
	GetElementLobbySide(iStorey)->Create(pParent, CElem::ELEM_EXTRA, L"LobbySide", Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	GetElementLeft(iStorey)->Create(pParent, CElem::ELEM_EXTRA, L"LeftSide", Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	GetElementRight(iStorey)->Create(pParent, CElem::ELEM_EXTRA, L"RightSide", Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	
	GetBox().SetHeight(GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey2, iShaft);

	if (iStorey <= GetBuilding()->GetHighestStoreyServed() && !GetBuilding()->bFastLoad)
	{
		if (GetBoxBeam().Width() > 0)
		{
			GetElement(iStorey)->AddWall(CElem::WALL_BEAM, L"Beam", nIndex, GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), -GetBoxBeam().Height(), -GetBoxBeam().Depth());
			GetElement(iStorey)->AddWall(CElem::WALL_SHAFT, L"BmRr", nIndex, GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
		}

		if (GetBox().LeftThickness() > 0)
			GetElementLeft(iStorey)->AddWall(CElem::WALL_SHAFT, L"LeftWall", nIndex, GetLeftWallBox(), Vector(0, 0, F_PI_2));
		if (GetBox().RightThickness() > 0)
			GetElementRight(iStorey)->AddWall(CElem::WALL_SHAFT, L"RightWall", nIndex, GetRightWallBox(), Vector(0, 0, F_PI_2));
		if (GetBox().RearThickness() != 0)
			GetElement(iStorey)->AddWall(CElem::WALL_SHAFT, L"RearWall", nIndex, GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());

		if (GetBuilding()->IsStoreyServed(iStorey, iShaft))
		{
			// The Opening
			AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
			GetElementLobbySide(iStorey)->AddWall(CElem::WALL_OPENING, L"Opening", nIndex, 
				GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), 
				GetBoxDoor().Width() + opn + opn, GetBoxDoor().Height() + opn, fOpeningThickness,
				Vector(0), 1, door);

			// Plates
			if (GetShaftLine() == 0)
				GetElementLobbySide(iStorey)->AddWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 10, 0.5, Vector(0, F_PI, F_PI));
			else
				GetElementLobbySide(iStorey)->AddWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 10, 0.5, Vector(0, F_PI, 0));

			// Door
			GetElementLobbySide(iStorey)->AddWall(CElem::WALL_DOOR, L"Door_Left" , MAKELONG(iStorey, iShaft), GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(0), 0, NULL, &m_pStoreyBones[iStorey].m_ppDoors[0]);
			GetElementLobbySide(iStorey)->AddWall(CElem::WALL_DOOR, L"Door_Right", MAKELONG(iStorey, iShaft), GetBoxDoor().RightRearUpper(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(F_PI, 0, F_PI), 0, NULL, &m_pStoreyBones[iStorey].m_ppDoors[1]);
		}
	}
}

void CBuildingConstr::SHAFT::ConstructMachine(AVULONG iShaft)
{
	m_pElemMachine = GetProject()->CreateElement(GetBuilding());
	GetMachineElement()->Create(GetBuilding()->GetMachineRoomElement(), CElem::ELEM_SHAFT, L"Machine %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetMachineRoomLevel()));

	BOX box = GetBoxCar();
	box.SetHeight(box.Width());
	if (GetShaftLine())
	{
		box.Move(box.Width(), box.Depth(), 0);
		box.SetDepth(-box.Depth());
		GetMachineElement()->AddWall(CElem::WALL_MACHINE, L"Machine", 0, box, Vector(0, 0, M_PI));
	}
	else
	{
		box.Move(0, box.Depth(), 0);
		GetMachineElement()->AddWall(CElem::WALL_MACHINE, L"Machine", 0, box, Vector(0));
	}
}

void CBuildingConstr::SHAFT::ConstructPit(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_PitBones.m_pElem = GetProject()->CreateElement(GetBuilding());
	m_PitBones.m_pElemLobbySide = GetProject()->CreateElement(GetBuilding());
	m_PitBones.m_pElemLeft = GetProject()->CreateElement(GetBuilding());
	m_PitBones.m_pElemRight = GetProject()->CreateElement(GetBuilding());

	CElem *pParent = GetBuilding()->GetPitElement();
	GetPitElement()->Create(pParent, CElem::ELEM_SHAFT, L"Pit %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetPitLevel()));
	pParent = GetBuilding()->GetPitElement(iShaft);
	GetPitElementLobbySide()->Create(pParent, CElem::ELEM_EXTRA, L"LobbySide", Vector(0, 0, GetBuilding()->GetPitLevel()));
	GetPitElementLeft()->Create(pParent, CElem::ELEM_EXTRA, L"LeftSide", Vector(0, 0, GetBuilding()->GetPitLevel()));
	GetPitElementRight()->Create(pParent, CElem::ELEM_EXTRA, L"RightSide", Vector(0, 0, GetBuilding()->GetPitLevel()));
	
	GetBox().SetHeight(-GetBuilding()->GetPitLevel());

	if (GetBoxBeam().Width() > 0)
	{
		GetPitElement()->AddWall(CElem::WALL_BEAM, L"Beam", 0, GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), GetBoxBeam().Height(), -GetBoxBeam().Depth());
		GetPitElement()->AddWall(CElem::WALL_SHAFT, L"BmRr", 0, GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
		GetPitElementLobbySide()->AddWall(CElem::WALL_SHAFT, L"BmFt", 0, GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), GetBox().Height(), GetBox().RearThickness());
	}

	if (GetBox().LeftThickness() > 0)
		GetPitElementLeft()->AddWall(CElem::WALL_SHAFT, L"LeftWall", 0, GetLeftWallBox(), Vector(0, 0, F_PI_2));
	if (GetBox().RightThickness() > 0)
		GetPitElementRight()->AddWall(CElem::WALL_SHAFT, L"RightWall", 0, GetRightWallBox(), Vector(0, 0, F_PI_2));
	if (GetBox().RearThickness() != 0)
	{
		GetPitElement()->AddWall(CElem::WALL_SHAFT, L"RearWall", 0, GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());
		GetPitElementLobbySide()->AddWall(CElem::WALL_SHAFT, L"FrontWall", 0, GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness());
	}

	// Buffer
	AVFLOAT x = GetBoxCar().CenterX();
	AVFLOAT y = GetBoxCar().CenterY();
	GetPitElement()->AddWall(CElem::WALL_BUFFER, L"Buffer", 0, Vector(x-300, y-300, 0), 600, 600, 1800);
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

	// Create skeletal elements (entire lift)
	m_pElem = GetProject()->CreateElement(GetBuilding());
	m_pElem->Create(GetBuilding()->GetElement(), CElem::ELEM_LIFT, (LPOLESTR)GetName().c_str(), GetShaft()->GetLiftPos(0 /*+ iShaft % 4*/));

	for (AVULONG iDeck = 0; iDeck < GetShaft()->GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_ppDecks[iDeck] = m_pElem->AddBone(CElem::BONE_DECK, L"Deck_%d", iDeck, Vector(0, 0, GetBuilding()->GetGroundStorey(iDeck)->GetLevel() - GetBuilding()->GetGroundStorey()->GetLevel())); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		BOX box = GetShaft()->GetBoxCar() - GetShaft()->GetLiftPos(0);
		BOX boxDoor0 = GetShaft()->GetBoxCarDoor(0) - GetShaft()->GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };
		AVFLOAT fDoorThickness0 = boxDoor0.Depth() * 0.4f;

		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT_FLOOR,   L"Floor",     iShaft, box.LeftExtRearExtLowerExt(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT_CEILING, L"Ceiling",   iShaft, box.LeftExtRearExtUpper(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT_DOOR,    L"Door1",     iShaft, boxDoor0.LeftFrontLower(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(0), 0, NULL, &m_ppDoors[0]);
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT_DOOR,    L"Door2",     iShaft, boxDoor0.RightFrontUpper(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(F_PI, 0, F_PI), 0, NULL, &m_ppDoors[1]);
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT,         L"FrontWall", iShaft, box.LeftExtFrontLower(), box.WidthExt(), box.Height(), box.FrontThickness(), Vector(0), 1, door);
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT,         L"RearWall",  iShaft, box.RightExtRearLower(), box.WidthExt(), box.Height(), box.RearThickness(), Vector(0, 0, F_PI));
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT,         L"LeftWall",  iShaft, box.LeftRearLower(), box.Depth(), box.Height(), box.LeftThickness(), Vector(0, 0, -F_PI_2));
		m_pElem->AddWall(m_ppDecks[iDeck], CElem::WALL_LIFT,         L"RightWall", iShaft, box.RightFrontLower(), box.Depth(), box.Height(), box.RightThickness(), Vector(0, 0, F_PI_2));
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

	m_pElem = GetProject()->CreateElement(this);
	m_pElem->Create(GetProject()->GetSiteElement(), CElem::ELEM_BUILDING, (LPOLESTR)GetName().c_str(), Vector(0));

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




