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

	// Lift Headroom Space
	if (iStorey == GetBuilding()->GetHighestStoreyServed() && !GetBuilding()->bFastLoad)
	{
		AVFLOAT h1 = GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness() - GetBuilding()->GetStorey(iStorey)->GetLevel();
		AVFLOAT h = GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt();
		BOX box = GetBox();
		box.SetHeight(h1 - h);
		box.Move(0, 0, h);
		if (box.FrontThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_REAR, L"RearWall (HR)",   iStorey2, box.FrontWall());
		if (box.RearThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_FRONT, L"FrontWall (HR)",	iStorey2, box.RearWall(), Vector(0, 0, F_PI));
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

	// do nothing if Machine Room-Less Lift
	if (GetBuilding()->IsMRL())
	{
		m_pElem->BuildWall(CElem::WALL_FLOOR,   L"Slab (MRL)", 0, GetBox().LowerSlab());
	}
	else
	{
		// Door Info
		AVFLOAT fScale = GetBuilding()->GetScale();
		FLOAT doors[] = { GetBox().Rear() - GetBuilding()->GetBox().Rear() + 500 * fScale, 750 * fScale, 2000 * fScale };

		// build walls
		m_pElem->BuildWall(CElem::WALL_FLOOR,   L"Machine Room Slab", 0, GetBox().LowerSlab());
		if (GetBuilding()->bFastLoad) return;
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, GetBox().FrontWall());
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"FrontWall", 0, GetBox().RearWall(), Vector(0, 0, F_PI));
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, GetBox().LeftWall(), Vector(0, 0, F_PI_2));
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, GetBox().RightWall(), Vector(0, 0, -F_PI_2), 1, doors);

		// Build the Lifting Beam
		AVFLOAT w = GetBuilding()->GetLiftingBeamWidth(), h = GetBuilding()->GetLiftingBeamHeight();
		BOX box(GetBox().Left(), GetBox().CentreY() - w/2, GetBox().Height() - h, GetBox().Width(), -w, h);
		m_pElem->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", 0, box);
	}
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

	if (GetBuilding()->bFastLoad) return;

	AVFLOAT fScale = GetBuilding()->GetScale();

	// collect door information
	AVFLOAT h = -GetBuilding()->GetPitLevel();
	std::vector<FLOAT> doordata[2];
	if (GetBuilding()->GetPitLadderSteps() == 0)	// high pits have no ladders but doors
		for (AVULONG iLine = 0; iLine < 2; iLine++)
			for (AVULONG iShaft = GetBuilding()->GetShaftBegin(iLine); iShaft < GetBuilding()->GetShaftEnd(iLine); iShaft++)
			{
				if (iLine == 0)
					doordata[iLine].push_back(GetBuilding()->GetShaft(iShaft)->GetBoxCar().CentreX() - 375 * fScale - GetBox().LeftExt());
				else
					doordata[iLine].push_back(GetBox().RightExt() - GetBuilding()->GetShaft(iShaft)->GetBoxCar().CentreX() - 375 * fScale);
				doordata[iLine].push_back(750 * fScale);
				doordata[iLine].push_back(2000 * fScale);
			}

	// build walls
	GetBox().SetHeight(h);
	if (GetBox().FrontThickness() > 0)
		m_pElem->BuildWall(CElem::WALL_REAR, L"RearPitWall",   0, GetBox().FrontWall(), Vector(0), doordata[0].size() / 3, doordata[0].size() ? &doordata[0][0] : NULL);
	if (GetBox().RearThickness() > 0)
		m_pElem->BuildWall(CElem::WALL_FRONT, L"FrontPitWall",	0, GetBox().RearWall(), Vector(0, 0, F_PI), doordata[1].size() / 3, doordata[1].size() ? &doordata[1][0] : NULL);
}

void CBuildingConstr::PIT::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CBuildingConstr::SHAFT::Construct(AVULONG iStorey, AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	AVFLOAT fScale = GetBuilding()->GetScale();
	AVULONG nShaftLine = GetShaftLine();

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
		// Int Div Beams & Side Wall Segments
		if (GetBox().LeftThickness() != 0)
			if (GetBeamLtHeight() > 0)
			{
				// left int div beam
				BOX box = GetBox().LeftWall();
				box.SetHeight(GetBeamLtHeight());
				AVFLOAT fLadderBracket = GetElement(iStorey)->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 1);
				if (iStorey == 0 && fLadderBracket > 0)	// on ground floor, place the beam to fit the pit ladder top bracket
					box.Move(0, 0, fLadderBracket + GetBuilding()->GetPitLevel() - GetBeamLtHeight()/2);
				else				// on all other floors, hide just below the floor
					box.Move(0, 0, -GetBeamLtHeight());
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamL", nIndex, box, Vector(0, 0, F_PI_2));
			}
			else // lhs wall segment
				GetElementLeft(iStorey)->BuildWall(CElem::WALL_SHAFT, L"LeftWall", nIndex, GetBox().LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));
		
		if (GetBox().RightThickness() != 0)
			if (GetBeamRtHeight() > 0)
			{
				// right int div beam
				BOX box = GetBox().RightWall();
				box.SetHeight(GetBeamRtHeight());
				AVFLOAT fLadderBracket = GetElement(iStorey)->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 1);
				if (iStorey == 0 && fLadderBracket > 0)	// on ground floor, place the beam to fit the pit ladder top bracket
					box.Move(0, 0, fLadderBracket + GetBuilding()->GetPitLevel() - GetBeamRtHeight()/2);
				else				// on all other floors, hide just below the floor
					box.Move(0, 0, -GetBeamRtHeight());
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamR", nIndex, box, Vector(0, 0, -F_PI_2));
			}
			else // rhs wall segment
				GetElementRight(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RightWall", nIndex, GetBox().RightWall(0, -GetWallRtStart()), Vector(0, 0, -F_PI_2));

		// Rear Wall
		if (GetBox().RearThickness() != 0)
			GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall", nIndex, GetBox().RearWall(), Vector(0, 0, F_PI));



		if (GetBuilding()->IsStoreyServed(iStorey, iShaft))
		{
			// Plates
			if (nShaftLine == 0)
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, F_PI));
			else
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(iShaft, iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, 0));

			// Door
			AVFLOAT fGap = nShaftLine ? 5 * fScale : -5 * fScale;
			for (AVULONG iIndex = 0; iIndex < 2; iIndex++)	// 0 = left; 1 = right
			{
				if (iIndex == 0 && GetDoorType() == 2 || iIndex == 1 && GetDoorType() == 1)
					continue;	// no of this type of doors
				for (AVULONG iPanel = 0; iPanel < GetDoorPanelsCount(); iPanel++)
				{
					CElem *&pElem = m_pStoreyBones[iStorey].m_ppDoors[iPanel * 2 + iIndex];
					pElem = GetProject()->CreateElement(GetBuilding(), GetElementLobbySide(iStorey), CElem::ELEM_BONE, L"Landing Door %d", iPanel * 2 + iIndex, Vector(0));
					BOX box = GetBoxDoor(); box.SetFront(box.Front() + fGap); box.SetRear(box.Rear() - 2*fGap); 
					box = box.Door(GetDoorType(), GetDoorPanelsCount(), iIndex, iPanel, false, nShaftLine ? true : false);
					pElem->BuildWall(CElem::WALL_DOOR, L"Landing Door %d", iPanel * 2 + iIndex, box);
				}
			}

			// Jamb
			if (nShaftLine == 0 && GetDoorType() == 1 || nShaftLine == 1 && GetDoorType() == 2)
			{
				BOX box = GetBoxDoor().RightWall(); 
				box.SetDepth(-100 * fScale);
				GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_JAMB, L"Landing Door Jamb", iShaft, box, -F_PI_2);
			}
			if (nShaftLine == 0 && GetDoorType() == 2 || nShaftLine == 1 && GetDoorType() == 1)
			{
				BOX box = GetBoxDoor().LeftWall(); box.SetDepth(-100 * fScale);
				GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_JAMB, L"Landing Door Jamb", iShaft, box, F_PI_2);
			}

			// Heading
			BOX box = GetBoxDoor().DoorExtended(GetDoorType(), GetDoorPanelsCount(), 100 * fScale, nShaftLine ? true : false);
			box.Move(0, 0, box.Height()); box.SetHeight(100 * fScale);
			box.SetRear(box.Rear() - fGap); 
			GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_HEADING, L"Landing Door Heading", iShaft, box);

			// Apron
			box = GetBoxDoor().DoorExtended(GetDoorType(), GetDoorPanelsCount(), 100 * fScale, nShaftLine ? true : false);
			box.Move(0, 0, -100 * fScale); box.SetHeight(100 * fScale);
			box.SetRear(box.Rear() - fGap); 
			GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_APRON, L"Landing Door Apron", iShaft, box);
		}

		// Light
		BOX box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
		AVFLOAT fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
		GetElement(iStorey)->BuildModel(CElem::MODEL_LIGHT, L"Lighting", iShaft, box, fAngle);
	}

	// Lift Headroom Space
	if (iStorey == GetBuilding()->GetHighestStoreyServed() && !GetBuilding()->bFastLoad)
	{
		AVFLOAT h1 = GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness() - GetBuilding()->GetStorey(iStorey)->GetLevel();
		AVFLOAT h = GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt();

		// top level beams & side walls
		BOX box = GetBox();
		box.SetHeight(h1 - h);
		box.Move(0, 0, h);
		if (GetBox().LeftThickness() != 0)
			if (GetBeamLtHeight() > 0)
			{
				// left int div beam
				BOX box = GetBox().LeftWall();
				box.SetHeight(GetBeamLtHeight());
				box.Move(0, 0, h1 - GetBeamLtHeight());
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamL (HR)", nIndex, box, Vector(0, 0, F_PI_2));
			}
			else // lhs wall segment
				GetElementLeft(iStorey)->BuildWall(CElem::WALL_SHAFT, L"LeftWall (HR)", nIndex, box.LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));

		if (GetBox().RightThickness() != 0)
			if (GetBeamRtHeight() > 0)
			{
				// right int div beam
				BOX box = GetBox().RightWall();
				box.SetHeight(GetBeamRtHeight());
				box.Move(0, 0, h1- GetBeamRtHeight());
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamR (HR)", nIndex, box, Vector(0, 0, -F_PI_2));
			}
			else // rhs wall segment
				GetElementRight(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RightWall (HR)", nIndex, box.RightWall(0, -GetWallRtStart()), Vector(0, 0, -F_PI_2));
	
		// Top level Rear & Front Walls
		if (box.RearThickness() != 0)
			GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall (HR)", nIndex, box.RearWall(), Vector(0, 0, F_PI));

		// light
		box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
		AVFLOAT fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
		GetElement(iStorey)->BuildModel(CElem::MODEL_LIGHT, L"Lighting (HR)", iShaft, box, fAngle, 2, /* headroom*/ h, h1);
	}
}

void CBuildingConstr::SHAFT::ConstructMachine(AVULONG iShaft)
{
	m_pElemMachine = GetProject()->CreateElement(GetBuilding(), GetBuilding()->GetMachineRoomElement(), CElem::ELEM_SHAFT, L"Machine %c", iShaft + 'A', Vector(0, 0, GetBuilding()->GetMachineRoomLevel()));

	// do nothing if Machine Room-Less Lift
	if (IsMRL())
		return;

	AVFLOAT fAngle = M_PI * (GetShaftLine() ? 3 : 1) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_MACHINE, L"Machine", iShaft, GetBoxCar(), fAngle, GetMachineType());
	fAngle = M_PI * (GetShaftLine() ? 3 : 1) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_OVERSPEED, L"Overspeed Governor", iShaft, GetBoxGovernor(), fAngle);
	fAngle = M_PI * (GetShaftLine() ? 0 : 2) / 2;
	GetMachineElement()->BuildModel(CElem::MODEL_CONTROL, L"Control Panel", iShaft, GetBuilding()->GetBox(), fAngle);
	GetMachineElement()->BuildModel(CElem::MODEL_ISOLATOR, L"Isolator Panel", iShaft, GetBuilding()->GetBox(), -F_PI/2);

	// Build the Lifting Beam
	AVFLOAT w = GetBuilding()->GetLiftingBeamWidth(), h = GetBuilding()->GetLiftingBeamHeight();
	MACHINEROOM *pmr = GetBuilding()->GetMachineRoom();
	if (GetShaftLine() == 0)
	{
		BOX box(GetBoxCar().CentreX() + w/2, pmr->GetBox().CentreY() - w/2, pmr->GetBox().Height() - h, -(pmr->GetBox().Front() - pmr->GetBox().CentreY()) - w/2, w, h);
		GetMachineElement()->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", iShaft, box, Vector(0, 0, -F_PI_2));
	}
	else
	{
		BOX box(GetBoxCar().CentreX() - w/2, pmr->GetBox().CentreY() + w/2, pmr->GetBox().Height() - h, pmr->GetBox().Rear() - pmr->GetBox().CentreY() - w/2, w, h);
		GetMachineElement()->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", iShaft, box, Vector(0, 0, F_PI_2));
	}
}

void CBuildingConstr::SHAFT::ConstructPit(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	AVFLOAT fScale = GetBuilding()->GetScale();
	AVFLOAT h = -GetBuilding()->GetPitLevel();

	// create skeletal structure (object & bone)
	CElem *pParent = GetBuilding()->GetPitElement();
	m_PitBones.m_pElem = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_SHAFT, L"Pit %c", iShaft + 'A', Vector(0, 0, -h));
	pParent = GetBuilding()->GetPitElement(iShaft);
	m_PitBones.m_pElemLobbySide = GetProject()->CreateElement(GetBuilding(), pParent, CElem::ELEM_EXTRA, L"LobbySide", 0, Vector(0, 0, -h));
	m_PitBones.m_pElemLeft = GetProject()->CreateElement(GetBuilding(),      pParent, CElem::ELEM_EXTRA, L"LeftSide",  0, Vector(0, 0, -h));
	m_PitBones.m_pElemRight = GetProject()->CreateElement(GetBuilding(),     pParent, CElem::ELEM_EXTRA, L"RightSide", 0, Vector(0, 0, -h));

	BOX box = GetBox();
	box.SetHeight(h);
	box.SetFrontThickness(box.RearThickness());

	if (GetBox().LeftThickness() != 0)
		if (GetBeamLtHeight() > 0)
		{
			// left int div beam
			BOX box = GetBox().LeftWall();
			box.SetHeight(GetBeamLtHeight());
			AVFLOAT fLadderBracket = GetPitElement()->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 0);
			if (fLadderBracket > 0)
				box.Move(0, 0, GetPitElement()->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 0) - GetBeamLtHeight()/2);
			GetPitElement()->BuildWall(CElem::WALL_BEAM, L"BeamL", 0, box, Vector(0, 0, F_PI_2));
		}
		else // lhs wall segment
			GetPitElementLeft()->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, box.LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));

	if (GetBox().RightThickness() != 0)
		if (GetBeamRtHeight() > 0)
		{
			// right int div beam
			BOX box = GetBox().RightWall();
			box.SetHeight(GetBeamRtHeight());
			AVFLOAT fLadderBracket = GetPitElement()->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 0);
			if (fLadderBracket > 0)
				box.Move(0, 0, GetPitElement()->GetLadderBracketPos(GetBuilding()->GetPitLadderSteps(), 0) - GetBeamRtHeight()/2);
			GetPitElement()->BuildWall(CElem::WALL_BEAM, L"BeamR", 0, box, Vector(0, 0, -F_PI_2));
		}
		else // rhs wall segment
			GetPitElementRight()->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, box.RightWall(0, -(GetWallRtStart())), Vector(0, 0, -F_PI_2));

	// Front & Rear Wall
	if (box.RearThickness() != 0)
		GetPitElement()->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, box.RearWall(), Vector(0, 0, F_PI));

	// Components

	bool bCwtRear = abs(GetBoxCwt().Width()) > abs(GetBoxCwt().Depth());

	// Governor Tension Pulley
	GetPitElement()->BuildModel(CElem::MODEL_PULLEY, L"Governor Tension Pulley", iShaft, GetBoxGovernor(), F_PI/2);

	// Buffers
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER_CAR, L"Car Buffer", iShaft, GetBoxCar(), 0, GetBufferNum(), GetBufferDiameter(), min(GetBufferHeight(), h - 250)); 
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER_CWT, L"Counterweight Buffer", iShaft, GetBoxCwt(), 0, GetBufferNum(), GetBufferDiameter(), min(GetBufferHeight(), h - 250)); 

	// Ladder
	AVFLOAT fAngle = (GetBoxLadder().CentreX() > GetBoxCar().CentreX()) ? M_PI * 3 / 2 : M_PI / 2;
	GetPitElement()->BuildModel(CElem::MODEL_LADDER, L"Pit Ladder", iShaft, GetBoxLadder(), fAngle, GetBuilding()->GetPitLadderSteps());
	
	// car rails
	AVFLOAT rw = GetRailWidth();
	AVFLOAT rl = GetRailLength();

	box = BOX(GetBoxCar().LeftExt() - 50 * fScale - rl, GetBoxCar().CentreY() - rw/2, 0, rl, rw, 0);
	box.SetHeight(h + GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness());
	GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Car Guide Rail 1", iShaft, box, F_PI);
	box.Move(GetBoxCar().WidthExt() + 100 * fScale + rl, 0, 0);
	GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Car Guide Rail 2", iShaft, box, 0);

	// cwt rails
	if (bCwtRear)
	{
		box = BOX(GetBoxCwt().LeftExt() - rl, GetBoxCwt().CentreY() - rw/2, 0, rl, rw, 0);
		box.SetHeight(h + GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness());
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 1", iShaft, box, F_PI);
		box.Move(GetBoxCwt().WidthExt() + rl, 0, 0);
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 2", iShaft, box, 0);
	}
	else
	{
		if (GetShaftLine() == 0) rl = -rl;

		box = BOX(GetBoxCwt().CentreX() - rw/2, GetBoxCwt().FrontExt() - rl, 0, rw, rl, 0);
		box.SetHeight(h + GetBuilding()->GetMachineRoomLevel() - GetBuilding()->GetMachineRoomSlabThickness());
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 1", iShaft, box, 3*F_PI/2);
		box.Move(0, GetBoxCwt().DepthExt() + rl, 0);
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 2", iShaft, box, F_PI/2);
	}

	// cwt
	box = GetBoxCwt();
	box.Move(0, 0, h + GetBuilding()->GetStorey(GetBuilding()->GetHighestStoreyServed() - GetBuilding()->GetBasementStoreyCount())->GetLevel());
	GetPitElement()->BuildModel(CElem::MODEL_CWT, L"Counterweight", iShaft, box);

	// light
	box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
	fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
	GetPitElement()->BuildModel(CElem::MODEL_LIGHT, L"Lighting", iShaft, box, fAngle, 1, /* pit */ h);
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

	AVFLOAT fScale = GetBuilding()->GetScale();
	AVULONG nShaftLine = GetShaft()->GetShaftLine();
	AVULONG nStartingStorey = GetBuilding()->GetBasementStoreyCount();

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

		// Door
		AVFLOAT fGap = nShaftLine ? 5 * fScale : -5 * fScale;
		for (AVULONG iIndex = 0; iIndex < 2; iIndex++)	// 0 = left; 1 = right
		{
			if (iIndex == 0 && GetShaft()->GetDoorType() == 2 || iIndex == 1 && GetShaft()->GetDoorType() == 1)
				continue;	// no of this type of doors
			for (AVULONG iPanel = 0; iPanel < GetShaft()->GetDoorPanelsCount(); iPanel++)
			{
				CElem *&pElem = m_ppDoors[iPanel * 2 + iIndex];
				pElem = GetProject()->CreateElement(GetBuilding(), m_ppDecks[iDeck], CElem::ELEM_BONE, L"Car Door %d", iPanel * 2 + iIndex, Vector(0));
				BOX box = boxDoor0; box.SetFront(box.Front() + fGap); box.SetRear(box.Rear() - fGap); 
				box = box.Door(GetShaft()->GetDoorType(), GetShaft()->GetDoorPanelsCount(), iIndex, iPanel, true, nShaftLine ? true : false);
				pElem->BuildWall(CElem::WALL_LIFT_DOOR, L"Car Door %d", iPanel * 2 + iIndex, box);
			}
		}

		// Jamb
		if (nShaftLine == 0 && GetShaft()->GetDoorType() == 1 || nShaftLine == 1 && GetShaft()->GetDoorType() == 2)
		{
			BOX box = boxDoor0.RightWall(); 
			box.SetDepth(-100 * fScale);
			m_ppDecks[iDeck]->BuildModel(CElem::MODEL_JAMB, L"Car Door Jamb", iShaft, box, -F_PI_2);
		}
		if (nShaftLine == 0 && GetShaft()->GetDoorType() == 2 || nShaftLine == 1 && GetShaft()->GetDoorType() == 1)
		{
			BOX box = boxDoor0.LeftWall(); box.SetDepth(-100 * fScale);
			m_ppDecks[iDeck]->BuildModel(CElem::MODEL_JAMB, L"Car Door Jamb", iShaft, box, F_PI_2);
		}

		// Heading
		BOX boxH = boxDoor0.DoorExtended(GetShaft()->GetDoorType(), GetShaft()->GetDoorPanelsCount(), 100 * fScale, nShaftLine ? true : false);
		boxH.Move(0, 0, boxH.Height()); boxH.SetHeight(100 * fScale);
		m_ppDecks[iDeck]->BuildModel(CElem::MODEL_HEADING, L"Car Door Heading", iShaft, boxH);

		// Apron
		BOX boxA = boxDoor0.DoorExtended(GetShaft()->GetDoorType(), GetShaft()->GetDoorPanelsCount(), 100 * fScale, nShaftLine ? true : false);
		boxA.Move(0, 0, -100 * fScale); boxA.SetHeight(100 * fScale);
		m_ppDecks[iDeck]->BuildModel(CElem::MODEL_APRON, L"Car Door Apron", iShaft, boxA);

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




