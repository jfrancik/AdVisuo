// BaseBuilding.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrBuilding.h"

//////////////////////////////////////////////////////////////////////////////////
// CBuildingEx

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
	m_pElem = GetBuilding()->CreateElement();
	m_pElem->Create(L"Storey_%d", iStorey2, Vector(0, 0, GetLevel()));

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
		m_pElem->AddWall(MAT_FLOOR,   L"Storey_%d_Floor",   iStorey2, GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), GetBox().LowerThickness(), GetBox().DepthExt());
		m_pElem->AddWall(MAT_CEILING, L"Storey_%d_Ceiling", iStorey2, GetBox().LeftExtRearExtUpper(), GetBox().WidthExt(), GetBox().UpperThickness(), GetBox().DepthExt());

		if (GetBuilding()->bFastLoad) return;
	
		if (bServed)
		{
			v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+40, 0);
			m_pElem->AddWall(MAT_FLOOR_NUMBER_PLATE, L"Storey_%d_Left_Nameplate", iStorey2, v, 2, 80, 40, Vector(0, F_PI_2));
			v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-40, 0);
			m_pElem->AddWall(MAT_FLOOR_NUMBER_PLATE, L"Storey_%d_Right_Nameplate", iStorey2, v, -2, 80, 40, Vector(-F_PI, -F_PI_2));
		}

//		m_pElem->AddWall(MAT_FRONT, L"Storey_%d_Cube",   iStorey2+1, Vector(-160, 20, 25), 40, 40, 40);

		if (GetBox().FrontThickness() > 0)
			m_pElem->AddWall(MAT_REAR, L"Storey_%d_RearWall",		iStorey2, GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0),
				doordata[0].size() / 3, doordata[0].size() ? &doordata[0][0] : NULL);
		if (GetBox().RearThickness() > 0)
			m_pElem->AddWall(MAT_FRONT, L"Storey_%d_FrontWall",	iStorey2, GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(F_PI),
				doordata[1].size() / 3, doordata[1].size() ? &doordata[1][0] : NULL);
		if (GetBox().LeftThickness() > 0)
			m_pElem->AddWall(MAT_SIDE, L"Storey_%d_LeftWall", iStorey2, GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(F_PI_2));
		if (GetBox().RightThickness() > 0)
			m_pElem->AddWall(MAT_SIDE, L"Storey_%d_RightWall", iStorey2, GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(-F_PI_2));
	}
}

void CBuildingConstr::STOREY::Deconstruct()
{
	delete m_pElem;
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
			m_pStoreyBones[i].m_pElem = GetBuilding()->CreateElement();
			m_pStoreyBones[i].m_pElemLobbySide = GetBuilding()->CreateElement();
			m_pStoreyBones[i].m_pElemLeft = GetBuilding()->CreateElement();
			m_pStoreyBones[i].m_pElemRight = GetBuilding()->CreateElement();
		}
	}

	GetElement(iStorey)->Create(L"Storey_%d_Shaft_%d", MAKELONG(iStorey2, iShaft), Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	GetElementLobbySide(iStorey)->Create(L"Storey_%d_Shaft_%d_LobbySide", MAKELONG(iStorey2, iShaft), Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	GetElementLeft(iStorey)->Create(L"Storey_%d_Shaft_%d_LeftSide", MAKELONG(iStorey2, iShaft), Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	GetElementRight(iStorey)->Create(L"Storey_%d_Shaft_%d_RightSide", MAKELONG(iStorey2, iShaft), Vector(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel()));
	
	GetBox().SetHeight(GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey2, iShaft);

	if (iStorey <= GetBuilding()->GetHighestStoreyServed() && !GetBuilding()->bFastLoad)
	{
		if (GetBoxBeam().Width() > 0)
		{
			GetElement(iStorey)->AddWall(MAT_BEAM, L"Storey_%d_Shaft_%d_Beam", nIndex, GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), -GetBoxBeam().Height(), -GetBoxBeam().Depth());
			GetElement(iStorey)->AddWall(MAT_SHAFT, L"Storey_%d_Shaft_%d_BmRr", nIndex, GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
		}

		if (GetBox().LeftThickness() > 0)
			GetElementLeft(iStorey)->AddWall(MAT_SHAFT, L"Storey_%d_Shaft_%d_Lt", nIndex, GetLeftWallBox(), Vector(F_PI_2));
		if (GetBox().RightThickness() > 0)
			GetElementRight(iStorey)->AddWall(MAT_SHAFT, L"Storey_%d_Shaft_%d_Rt", nIndex, GetRightWallBox(), Vector(F_PI_2));
		if (GetBox().RearThickness() != 0)
			GetElement(iStorey)->AddWall(MAT_SHAFT, L"Storey_%d_Shaft_%d_Rr", nIndex, GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());

		if (GetBuilding()->IsStoreyServed(iStorey, iShaft))
		{
			// The Opening
			AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
			GetElementLobbySide(iStorey)->AddWall(MAT_OPENING, L"Storey_%d_Shaft_%d_Opening", nIndex, 
				GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), 
				GetBoxDoor().Width() + opn + opn, GetBoxDoor().Height() + opn, fOpeningThickness,
				Vector(0), 1, door);

			// Plates
			if (GetShaftLine() == 0)
				GetElementLobbySide(iStorey)->AddWall(MAT_LIFT_NUMBER_PLATE, L"Shaft_%d_Storey_%d_Nameplate", MAKELONG(iShaft, iStorey), 
					GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 10, 0.5, Vector(F_PI, 0, F_PI));
			else
				GetElementLobbySide(iStorey)->AddWall(MAT_LIFT_NUMBER_PLATE, L"Shaft_%d_Storey_%d_Nameplate", MAKELONG(iShaft, iStorey), 
					GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 10, 0.5, Vector(0, 0, F_PI));

			// Door
			GetElementLobbySide(iStorey)->AddWall(MAT_DOOR, L"Storey_%d_Shaft_%d_Door_Left" , MAKELONG(iStorey, iShaft), GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(0), 0, NULL, &m_pStoreyBones[iStorey].m_ppDoors[0]);
			GetElementLobbySide(iStorey)->AddWall(MAT_DOOR, L"Storey_%d_Shaft_%d_Door_Right", MAKELONG(iStorey, iShaft), GetBoxDoor().RightRearUpper(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(F_PI, F_PI), 0, NULL, &m_pStoreyBones[iStorey].m_ppDoors[1]);
		}
	}
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
}

void CBuildingConstr::LIFT::Construct(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// Create skeletal elements (entire lift)
	m_pElem = GetBuilding()->CreateElement();
	m_pElem->Create(L"Lift_%d", iShaft, GetShaft()->GetLiftPos(0 /*+ iShaft % 4*/));

	for (AVULONG iDeck = 0; iDeck < GetShaft()->GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_ppDecks[iDeck] = m_pElem->AddBone(L"Deck_%d", iDeck, Vector(0, 0, GetBuilding()->GetGroundStorey(iDeck)->GetLevel() - GetBuilding()->GetGroundStorey()->GetLevel())); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		BOX box = GetShaft()->GetBoxCar() - GetShaft()->GetLiftPos(0);
		BOX boxDoor0 = GetShaft()->GetBoxCarDoor(0) - GetShaft()->GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };
		AVFLOAT fDoorThickness0 = boxDoor0.Depth() * 0.4f;

		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT_FLOOR,   L"Lift_%d_Deck_%d_Floor",     iShaft, box.LeftExtRearExtLowerExt(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT_CEILING, L"Lift_%d_Deck_%d_Ceiling",   iShaft, box.LeftExtRearExtUpper(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT_DOOR,    L"Lift_%d_Deck_%d_Door1",     iShaft, boxDoor0.LeftFrontLower(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(0), 0, NULL, &m_ppDoors[0]);
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT_DOOR,    L"Lift_%d_Deck_%d_Door2",     iShaft, boxDoor0.RightFrontUpper(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(F_PI, F_PI), 0, NULL, &m_ppDoors[1]);
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT,         L"Lift_%d_Deck_%d_FrontWall", iShaft, box.LeftExtFrontLower(), box.WidthExt(), box.Height(), box.FrontThickness(), Vector(0), 1, door);
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT,         L"Lift_%d_Deck_%d_RearWall",  iShaft, box.RightExtRearLower(), box.WidthExt(), box.Height(), box.RearThickness(), Vector(F_PI));
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT,         L"Lift_%d_Deck_%d_LeftWall",  iShaft, box.LeftRearLower(), box.Depth(), box.Height(), box.LeftThickness(), Vector(-F_PI_2));
		m_pElem->AddWall(m_ppDecks[iDeck], MAT_LIFT,         L"Lift_%d_Deck_%d_RightWall", iShaft, box.RightFrontLower(), box.Depth(), box.Height(), box.RightThickness(), Vector(F_PI_2));
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

	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
		GetLift(iLift)->Construct(iLift);

	for (AVULONG iStorey = 0; iStorey < GetStoreyCount(); iStorey++)
	{
		GetStorey(iStorey)->Construct(iStorey);
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->Construct(iStorey, iShaft);
	}
}

void CBuildingConstr::Deconstruct()
{
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Deconstruct();
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Deconstruct();
}




