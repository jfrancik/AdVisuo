// ConstrLiftGroup.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrLiftGroup.h"
#include "ConstrProject.h"
#include "BaseSimClasses.h"
#include "DBTools.h"
#include <math.h>

#define F_PI	((AVFLOAT)M_PI)
#define F_PI_2	((AVFLOAT)M_PI_2)

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroupConstr

CLiftGroupConstr::CLiftGroupConstr(CProject *pProject, AVULONG nIndex) : CLiftGroup(pProject, nIndex)
{ 
	m_pElem = NULL; 
	bFastLoad = false; 
}

CLiftGroupConstr::~CLiftGroupConstr()
{
	Deconstruct(); 
}

void CLiftGroupConstr::STOREY::Construct(AVULONG iStorey)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	bool bServed = IsStoreyServed();
	bool bExisting = GetId() <= GetLiftGroup()->GetHighestStoreyServed();

	AVLONG iStorey2 = iStorey - GetLiftGroup()->GetBasementStoreyCount();

	// imposed parameters
	AVFLOAT opn = 0;	//2.5f;			// width of the opening around the door

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), iStorey, Vector(0, 0, GetLevel()));

	// collect door information
	std::vector<FLOAT> doordata[2];
	for (AVULONG iLine = 0; iLine < 2; iLine++)
		for (AVULONG iShaft = GetLiftGroup()->GetShaftBegin(iLine); iShaft < GetLiftGroup()->GetShaftEnd(iLine); iShaft++)
		{
			SHAFT *pShaft = GetLiftGroup()->GetShaft(iShaft);
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
		m_pElem->BuildWall(CElem::WALL_FLOOR,   L"Floor",   iStorey2, GetBox().LowerSlab().SetHeight(0.05f));
		m_pElem->BuildWall(CElem::WALL_CEILING, L"Ceiling", iStorey2, GetBox().UpperSlab());

		if (GetLiftGroup()->bFastLoad) return;
	
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
	if (iStorey == GetLiftGroup()->GetHighestStoreyServed() && !GetLiftGroup()->bFastLoad)
	{
		AVFLOAT h1 = GetLiftGroup()->GetMRLevel() - GetLiftGroup()->GetMRSlabThickness() - GetLiftGroup()->GetStorey(iStorey)->GetLevel();
		AVFLOAT h = GetLiftGroup()->GetStorey(iStorey)->GetBox().HeightExt();
		XBOX box = GetBox();
		box.SetHeight(h1 - h);
		box.Move(0, 0, h);
		if (box.FrontThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_REAR, L"RearWall (HR)",   iStorey2, box.FrontWall());
		if (GetLiftGroup()->GetShaftLinesCount() == 1)
		{
			// for in-line arrangements, the Front Wall will follow the MR arrangement rather than the lobby
			box = GetLiftGroup()->GetBoxMR();
			box.SetHeight(h1 - h);
			box.Move(0, 0, h);
		}
		if (box.RearThickness() > 0)
			m_pElem->BuildWall(CElem::WALL_FRONT, L"FrontWall (HR)",	iStorey2, box.RearWall(), Vector(0, 0, F_PI));
	}

}

void CLiftGroupConstr::STOREY::Deconstruct()
{
	delete m_pElem;
}

void CLiftGroupConstr::MR::Construct()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), 9999, Vector(0, 0, GetLevel()));

	// if MRL - just build the main slab and finish
	if (GetLiftGroup()->IsMRL() || GetLiftGroup()->bFastLoad)
	{
		m_pElem->BuildWall(CElem::WALL_FLOOR, L"Slab (MRL)", 0, GetBox().LowerSlab());
		return;
	}

	// auxiliary boxes
	XBOX boxMain = GetBoxMain();
	XBOX boxExt = GetBoxExt();
	AVFLOAT fExt = GetExt();

	// build the slab
	m_pElem->BuildWall(CElem::WALL_FLOOR, L"Machine Room Slab", 0, boxMain.LowerSlab());
	if (fExt > 0)
		m_pElem->BuildWall(CElem::WALL_FLOOR, L"Machine Room Slab Extension", 0, boxExt.LowerSlab());

	// Door Info
	FLOAT doors[] = { GetLiftGroup()->GetMRDoorOffset(), GetLiftGroup()->GetMRDoorWidth(), GetLiftGroup()->GetMRDoorHeight() };

	// build three easy walls
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, boxMain.FrontWall());
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"FrontWall", 0, boxMain.RearWall(), Vector(0, 0, F_PI));
	m_pElem->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, boxMain.LeftWall(), Vector(0, 0, F_PI_2));
	if (fExt <= 0)
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, GetBox().RightWall(), Vector(0, 0, -F_PI_2), 1, doors);
	else
	{
		// indented walls of the MR extension
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWallExt", 0, boxExt.RightWall(), Vector(0, 0, -F_PI_2), 1, doors);
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RearWallExt", 0, boxExt.FrontWall());
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"FrontWallExt", 0, boxExt.RearWall(), Vector(0, 0, F_PI));

		XBOX boxAux = boxMain;
		boxAux.SetRear(GetLiftGroup()->GetMRIndentFront()); boxAux.SetRearThickness(0);
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWall_1", 0, boxAux.RightWall(), Vector(0, 0, -F_PI_2));

		boxAux = boxMain;
		boxAux.SetFront(GetLiftGroup()->GetMRIndentRear()); boxAux.SetFrontThickness(0);
		m_pElem->BuildWall(CElem::WALL_SHAFT, L"RightWall_2", 0, boxAux.RightWall(), Vector(0, 0, -F_PI_2));
	}

	// Build the Lifting Beam
	AVFLOAT w = GetLiftGroup()->GetLiftingBeamWidth(), h = GetLiftGroup()->GetLiftingBeamHeight();
	BOX box(GetBox().Left(), GetBox().CentreY() - w/2, GetBox().Height() - h, GetBox().Width(), -w, h);
	m_pElem->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", 0, box);

	// Build the Group Panel
	GetLiftGroup()->GetMR()->GetElement()->BuildModel(CElem::MODEL_CONTROL_PANEL, L"Group Panel", GetId(), GetLiftGroup()->GetBoxPanelGrp(), GetLiftGroup()->GetPanelGrpOrientation());
}

void CLiftGroupConstr::MR::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CLiftGroupConstr::PIT::Construct()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	// create skeletal structure (object & bone)
	m_pElem = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetElement(), CElem::ELEM_STOREY, (LPOLESTR)GetName().c_str(), 9998, Vector(0, 0, GetLevel()));

	if (GetLiftGroup()->bFastLoad) return;

	AVFLOAT fScale = GetLiftGroup()->GetScale();

	// collect door information
	AVFLOAT h = -GetLiftGroup()->GetPitLevel();
	std::vector<FLOAT> doordata[2];
	if (GetLiftGroup()->IsPitDoor())		// high pits have no ladders but doors
		for (AVULONG iLine = 0; iLine < 2; iLine++)
			for (AVULONG iShaft = GetLiftGroup()->GetShaftBegin(iLine); iShaft < GetLiftGroup()->GetShaftEnd(iLine); iShaft++)
			{
				if (iLine == 0)
					doordata[iLine].push_back(GetLiftGroup()->GetShaft(iShaft)->GetBoxCar().CentreX() - 375 * fScale - GetBox().LeftExt());
				else
					doordata[iLine].push_back(GetBox().RightExt() - GetLiftGroup()->GetShaft(iShaft)->GetBoxCar().CentreX() - 375 * fScale);
				doordata[iLine].push_back(750 * fScale);
				doordata[iLine].push_back(1800 * fScale);
			}

	// build walls
	GetBox().SetHeight(h);
	if (GetBox().FrontThickness() > 0)
		m_pElem->BuildWall(CElem::WALL_REAR, L"RearPitWall",   0, GetBox().FrontWall(), Vector(0), doordata[0].size() / 3, doordata[0].size() ? &doordata[0][0] : NULL);
	if (GetBox().RearThickness() > 0)
		m_pElem->BuildWall(CElem::WALL_FRONT, L"FrontPitWall",	0, GetBox().RearWall(), Vector(0, 0, F_PI), doordata[1].size() / 3, doordata[1].size() ? &doordata[1][0] : NULL);
}

void CLiftGroupConstr::PIT::Deconstruct()
{
	if (m_pElem) delete m_pElem;
}

void CLiftGroupConstr::SHAFT::Construct(AVULONG iStorey)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	AVFLOAT fScale = GetLiftGroup()->GetScale();
	AVULONG nShaftLine = GetShaftLine();

	bool bFlip = (nShaftLine == 1);
	AVULONG doorType = GetDoorType(), panelsCount = GetDoorPanelsCount(), panelsPerDoor = GetDoorPanelsPerDoor();

	BOX box;	AVLONG iStorey2 = iStorey - GetLiftGroup()->GetBasementStoreyCount();

	// create skeletal structure (object & bone)
	if (m_pStoreyBones == NULL)
		m_pStoreyBones = new BONES[GetLiftGroup()->GetStoreyCount()];


	CElem *pParent = GetLiftGroup()->GetStoreyElement(iStorey);
	m_pStoreyBones[iStorey].m_pElem = GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_SHAFT, L"Shaft %c", GetId() + 'A', Vector(0, 0, GetLiftGroup()->GetStorey(iStorey)->GetLevel()));
	pParent = GetLiftGroup()->GetShaftElement(iStorey, GetId());
	m_pStoreyBones[iStorey].m_pElemLobbySide = GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_EXTRA, L"LobbySide", 0, Vector(0, 0, GetLiftGroup()->GetStorey(iStorey)->GetLevel()));
	m_pStoreyBones[iStorey].m_pElemLeft =      GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_EXTRA, L"LeftSide",  0, Vector(0, 0, GetLiftGroup()->GetStorey(iStorey)->GetLevel()));
	m_pStoreyBones[iStorey].m_pElemRight =     GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_EXTRA, L"RightSide", 0, Vector(0, 0, GetLiftGroup()->GetStorey(iStorey)->GetLevel()));

	GetBox().SetHeight(GetLiftGroup()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey2, GetId());

	if (iStorey <= GetLiftGroup()->GetHighestStoreyServed() && !GetLiftGroup()->bFastLoad)
	{
		// Int Div Beams & Side Wall Segments
		if (GetBox().LeftThickness() != 0)
			if (GetBeamLtHeight() > 0)
			{
				// left int div beam
				BOX box = GetBox().LeftWall();
				box.SetHeight(GetBeamLtHeight());

				if (iStorey == 0 && GetLiftGroup()->IsPitLadder())	// on ground floor, place the beam to fit the pit ladder top bracket
					box.Move(0, 0, GetLiftGroup()->GetPitLadderUpperBracket() + GetLiftGroup()->GetPitLevel() - GetBeamLtHeight()/2);
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
				if (iStorey == 0 && GetLiftGroup()->IsPitLadder())	// on ground floor, place the beam to fit the pit ladder top bracket
					box.Move(0, 0, GetLiftGroup()->GetPitLadderUpperBracket() + GetLiftGroup()->GetPitLevel() - GetBeamRtHeight()/2);
				else				// on all other floors, hide just below the floor
					box.Move(0, 0, -GetBeamRtHeight());
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamR", nIndex, box, Vector(0, 0, -F_PI_2));
			}
			else // rhs wall segment
				GetElementRight(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RightWall", nIndex, GetBox().RightWall(0, -GetWallRtStart()), Vector(0, 0, -F_PI_2));

		// Rear Wall
		if (GetBox().RearThickness() != 0)
			GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall", nIndex, GetBox().RearWall(), Vector(0, 0, F_PI));



		if (GetLiftGroup()->IsStoreyServed(iStorey, GetId()))
		{
			// Plates
			if (nShaftLine == 0)
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(GetId(), iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, F_PI));
			else
				GetElementLobbySide(iStorey)->BuildWall(CElem::WALL_LIFT_NUMBER_PLATE, L"Nameplate", MAKELONG(GetId(), iStorey), 
					BOX(GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 0.5, 10), Vector(0, F_PI, 0));

			// Door
			//AVFLOAT fGap = nShaftLine ? 5 * fScale : -5 * fScale;
			for (AVULONG iIndex = 0; iIndex < 2; iIndex++)	// 0 = left; 1 = right
			{
				if (iIndex == 0 && doorType == DOOR_SIDE_RIGHT)
					continue;	// no left door in this configuration
				if (iIndex == 1 && doorType == DOOR_SIDE_LEFT)
					continue;	// no right door in this configuration

				for (AVULONG iPanel = 0; iPanel < panelsPerDoor; iPanel++)
				{
					CElem *&pElem = m_pStoreyBones[iStorey].m_ppDoors[iPanel * 2 + iIndex];
					pElem = GetProject()->CreateElement(GetLiftGroup(), GetElementLobbySide(iStorey), CElem::ELEM_BONE, L"Landing Door %d", iPanel * 2 + iIndex, Vector(0));
					//BOX box = GetBoxDoor(); box.SetFront(box.Front() + fGap); box.SetRear(box.Rear() - 2*fGap); 
					BOX box = GetBoxDoor().DoorPanel(panelsCount, panelsPerDoor, iIndex, iPanel, false, bFlip);
					pElem->BuildWall(CElem::WALL_DOOR, L"Landing Door %d", iPanel * 2 + iIndex, box);
				}
			}

			// Jamb
			if (nShaftLine == 0 && doorType == DOOR_SIDE_LEFT || nShaftLine == 1 && doorType == DOOR_SIDE_RIGHT)
			{
				BOX box = BOX(GetBoxDoor().RightRearLower(), GetBoxDoor().Depth(), -100 * fScale, GetBoxDoor().Height());
				GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_JAMB, L"Landing Door Jamb", GetId(), box, -F_PI_2);
			}
			if (nShaftLine == 0 && doorType == DOOR_SIDE_RIGHT || nShaftLine == 1 && doorType == DOOR_SIDE_LEFT)
			{
				BOX box = BOX(GetBoxDoor().LeftFrontLower(), GetBoxDoor().Depth(), -100 * fScale, GetBoxDoor().Height());
				GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_JAMB, L"Landing Door Jamb", GetId(), box, F_PI_2);
			}

			// Heading
			BOX box = GetBoxDoor().DoorWithExtension(doorType, panelsCount, panelsPerDoor, 100 * fScale, bFlip);
			box.Move(0, 0, box.Height()); box.SetHeight(100 * fScale);
			// box.SetRear(box.Rear() - fGap); 
			GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_HEADING, L"Landing Door Heading", GetId(), box);

			// Sill
			AVFLOAT fAngle = GetShaftLine() ? F_PI : 0;
			box = GetBoxDoor().DoorWithExtension(doorType, panelsCount, panelsPerDoor, 100 * fScale, bFlip);
			box.Move(0, 0, -100 * fScale); box.SetHeight(100 * fScale);
			// box.SetRear(box.Rear() - fGap); 
			GetElementLobbySide(iStorey)->BuildModel(CElem::MODEL_SILL, L"Landing Door Sill", GetId(), box, fAngle);
		}

		// Combination Bracket: meaning of parameters: nParam = LHS bracket; fParam1 = bracket width
		BOX box = GetCombBox().GetInnerBox();
		if (iStorey == 0 && GetLiftGroup()->IsPitLadder())	// on ground floor, place the beam to fit the pit ladder top bracket
		{
			box.Move(0, 0, -box.Lower());	// cancel any previous elevation...
			box.Move(0, 0, GetLiftGroup()->GetPitLadderUpperBracket() + GetLiftGroup()->GetPitLevel() - box.Height() / 2);	// re-fit the vertical position here
		}
		GetElement(iStorey)->BuildModel(CElem::MODEL_COMBINATION_BRACKET, L"Combination Bracket", GetId(), box, 0, GetCombBox().LeftThickness() == 0, abs(GetCombBox().FrontThickness()));

		// Light
		box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
		AVFLOAT fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
		GetElement(iStorey)->BuildModel(CElem::MODEL_LIGHT, L"Lighting", GetId(), box, fAngle);
	}

	// Lift Headroom Space
	if (iStorey == GetLiftGroup()->GetHighestStoreyServed() && !GetLiftGroup()->bFastLoad)
	{
		AVFLOAT h1 = GetLiftGroup()->GetMRLevel() - GetLiftGroup()->GetMRSlabThickness() - GetLiftGroup()->GetStorey(iStorey)->GetLevel();
		AVFLOAT h = GetLiftGroup()->GetStorey(iStorey)->GetBox().HeightExt();

		// top level beams & side walls
		XBOX boxShaft = GetBox();
		boxShaft.SetHeight(h1 - h);
		boxShaft.Move(0, 0, h);
		if (GetBox().LeftThickness() != 0)
			if (GetBeamLtHeight() > 0)
			{
				// left int div beam
				BOX box = GetBox().LeftWall();
				box.SetHeight(GetBeamLtHeight());
				box.Move(0, 0, h1 - GetBeamLtHeight() - 500 * fScale);
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamL (HR)", nIndex, box, Vector(0, 0, F_PI_2));
			}
			else // lhs wall segment
				GetElementLeft(iStorey)->BuildWall(CElem::WALL_SHAFT, L"LeftWall (HR)", nIndex, boxShaft.LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));

		if (GetBox().RightThickness() != 0)
			if (GetBeamRtHeight() > 0)
			{
				// right int div beam
				BOX box = GetBox().RightWall();
				box.SetHeight(GetBeamRtHeight());
				box.Move(0, 0, h1- GetBeamRtHeight() - 500 * fScale);
				GetElement(iStorey)->BuildWall(CElem::WALL_BEAM, L"BeamR (HR)", nIndex, box, Vector(0, 0, -F_PI_2));
			}
			else // rhs wall segment
				GetElementRight(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RightWall (HR)", nIndex, boxShaft.RightWall(0, -GetWallRtStart()), Vector(0, 0, -F_PI_2));
	
		// Top level Rear & Front Walls
		if (boxShaft.RearThickness() != 0)
			GetElement(iStorey)->BuildWall(CElem::WALL_SHAFT, L"RearWall (HR)", nIndex, boxShaft.RearWall(), Vector(0, 0, F_PI));

		// Combination Bracket: meaning of parameters: nParam = LHS bracket; fParam1 = bracket width
		BOX box = GetCombBox().GetInnerBox();
		box.Move(0, 0, h1 - 500 * fScale);	// re-fit the vertical position here
		GetElement(iStorey)->BuildModel(CElem::MODEL_COMBINATION_BRACKET, L"Combination Bracket (HR)", GetId(), box, 0, GetCombBox().LeftThickness() == 0, abs(GetCombBox().FrontThickness()));

		// light
		box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
		AVFLOAT fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
		GetElement(iStorey)->BuildModel(CElem::MODEL_LIGHT, L"Lighting (HR)", GetId(), box, fAngle, 2, /* headroom*/ h, h1);
	}
}

void CLiftGroupConstr::SHAFT::ConstructMREquipment()
{
	if (!GetMachineElement())
		m_pElemMachine = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetMRElement(), CElem::ELEM_SHAFT, L"Machine %c", GetId() + 'A', Vector(0, 0, GetLiftGroup()->GetMRLevel()));

	// do nothing if Machine Room-Less Lift
	if (IsMRL())
		return;

	OLECHAR _name[100];

	_snwprintf_s(_name, 256, L"Control Panel Type %d", GetPanelCtrlType());
	GetMachineElement()->BuildModel(CElem::MODEL_CONTROL_PANEL, _name, GetId(), GetBoxPanelCtrl(), GetPanelCtrlOrientation(), GetPanelCtrlType());

	_snwprintf_s(_name, 256, L"Drive Panel Type %d", GetPanelDrvType());
	GetMachineElement()->BuildModel(CElem::MODEL_DRIVE_PANEL, _name, GetId(), GetBoxPanelDrv(), GetPanelDrvOrientation(), GetPanelDrvType());

	_snwprintf_s(_name, 256, L"Isolator Panel Type %d", GetPanelIsoType());
	if (GetBoxPanelIso().Width() > 1)
		GetMachineElement()->BuildModel(CElem::MODEL_ISOLATOR, _name, GetId(), GetBoxPanelIso(), GetPanelIsoOrientation(), GetPanelIsoType());

	_snwprintf_s(_name, 256, L"Machine Type %d", GetMachineType());
	GetMachineElement()->BuildModel(CElem::MODEL_MACHINE, _name, GetId(), GetBoxCar().GetLargeBox(), GetMachineOrientation() + M_PI_2, GetMachineType());

	GetMachineElement()->BuildModel(CElem::MODEL_OVERSPEED, L"Overspeed Governor", GetId(), GetBoxGovernor(), GetMachineOrientation() + M_PI_2);
	
	// Build the Lifting Beam
	AVFLOAT w = GetLiftGroup()->GetLiftingBeamWidth(), h = GetLiftGroup()->GetLiftingBeamHeight();
	MR *pmr = GetLiftGroup()->GetMR();
	if (GetShaftLine() == 0)
	{
		BOX box(GetBoxCar().CentreX() + w/2, pmr->GetBox().CentreY() - w/2, pmr->GetBox().Height() - h, -(pmr->GetBox().Front() - pmr->GetBox().CentreY()) - w/2, w, h);
		GetMachineElement()->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", GetId(), box, Vector(0, 0, -F_PI_2));
	}
	else
	{
		BOX box(GetBoxCar().CentreX() - w/2, pmr->GetBox().CentreY() + w/2, pmr->GetBox().Height() - h, pmr->GetBox().Rear() - pmr->GetBox().CentreY() - w/2, w, h);
		GetMachineElement()->BuildWall(CElem::WALL_BEAM, L"LiftingBeam", GetId(), box, Vector(0, 0, F_PI_2));
	}
}

void CLiftGroupConstr::SHAFT::ConstructPitEquipment()
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	AVFLOAT fScale = GetLiftGroup()->GetScale();
	AVFLOAT h = -GetLiftGroup()->GetPitLevel();

	// create skeletal structure (object & bone)
	CElem *pParent = GetLiftGroup()->GetPitElement();
	m_PitBones.m_pElem = GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_SHAFT, L"Pit %c", GetId() + 'A', Vector(0, 0, -h));
	pParent = GetLiftGroup()->GetPitElement(GetId());
	m_PitBones.m_pElemLobbySide = GetProject()->CreateElement(GetLiftGroup(), pParent, CElem::ELEM_EXTRA, L"LobbySide", 0, Vector(0, 0, -h));
	m_PitBones.m_pElemLeft = GetProject()->CreateElement(GetLiftGroup(),      pParent, CElem::ELEM_EXTRA, L"LeftSide",  0, Vector(0, 0, -h));
	m_PitBones.m_pElemRight = GetProject()->CreateElement(GetLiftGroup(),     pParent, CElem::ELEM_EXTRA, L"RightSide", 0, Vector(0, 0, -h));

	XBOX boxPit = GetBox();
	boxPit.SetHeight(h);
	boxPit.SetFrontThickness(boxPit.RearThickness());

	if (GetBox().LeftThickness() != 0)
		if (GetBeamLtHeight() > 0)
		{
			// left int div beam
			BOX box = GetBox().LeftWall();
			box.SetHeight(GetBeamLtHeight());
			if (GetLiftGroup()->IsPitLadder())
				box.Move(0, 0, GetLiftGroup()->GetPitLadderLowerBracket() - GetBeamLtHeight()/2);
			else
				box.Move(0, 0, 500 * fScale);
			GetPitElement()->BuildWall(CElem::WALL_BEAM, L"BeamL", 0, box, Vector(0, 0, F_PI_2));
		}
		else // lhs wall segment
			GetPitElementLeft()->BuildWall(CElem::WALL_SHAFT, L"LeftWall", 0, boxPit.LeftWall(GetWallLtStart()), Vector(0, 0, F_PI_2));

	if (GetBox().RightThickness() != 0)
		if (GetBeamRtHeight() > 0)
		{
			// right int div beam
			BOX box = GetBox().RightWall();
			box.SetHeight(GetBeamRtHeight());
			if (GetLiftGroup()->IsPitLadder())
				box.Move(0, 0, GetLiftGroup()->GetPitLadderLowerBracket() - GetBeamRtHeight()/2);
			else
				box.Move(0, 0, 500 * fScale);
			GetPitElement()->BuildWall(CElem::WALL_BEAM, L"BeamR", 0, box, Vector(0, 0, -F_PI_2));
		}
		else // rhs wall segment
			GetPitElementRight()->BuildWall(CElem::WALL_SHAFT, L"RightWall", 0, boxPit.RightWall(0, -(GetWallRtStart())), Vector(0, 0, -F_PI_2));

	// Front & Rear Wall
	if (boxPit.RearThickness() != 0)
		GetPitElement()->BuildWall(CElem::WALL_SHAFT, L"RearWall", 0, boxPit.RearWall(), Vector(0, 0, F_PI));

	// Components

	bool bCwtRear = abs(GetBoxCwt().Width()) > abs(GetBoxCwt().Depth());

	// Governor Tension Pulley
	GetPitElement()->BuildModel(CElem::MODEL_PULLEY, L"Governor Tension Pulley", GetId(), GetBoxGovernor(), GetMachineOrientation() + F_PI/2);

	// Buffers
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER_CAR, L"Car Buffer", GetId(), GetBoxCar().GetLargeBox(), 0, GetBufferNum(), GetBufferDiameter(), min(GetBufferHeight(), h - 250)); 
	GetPitElement()->BuildModel(CElem::MODEL_BUFFER_CWT, L"Counterweight Buffer", GetId(), GetBoxCwt(), 0, GetBufferNum(), GetBufferDiameter(), min(GetBufferHeight(), h - 250)); 

	// Ladder
	AVFLOAT fAngle = (GetBoxLadder().CentreX() > GetBoxCar().CentreX()) ? M_PI * 3 / 2 : M_PI / 2;
	GetPitElement()->BuildModel(CElem::MODEL_LADDER, L"Pit Ladder", GetId(), GetBoxLadder(), fAngle, GetLiftGroup()->GetPitLadderRungs(), GetLiftGroup()->GetPitLadderLowerBracket(), GetLiftGroup()->GetPitLadderUpperBracket());
	
	// car rails
	AVFLOAT rw = GetRailWidth();
	AVFLOAT rl = GetRailLength();

	BOX box;
	
	box = BOX(GetBoxCar().LeftExt() - 50 * fScale - rl, GetBoxCar().CentreY() - rw/2, 0, rl, rw, 0);
	box.SetHeight(h + GetLiftGroup()->GetMRLevel() - GetLiftGroup()->GetMRSlabThickness());
	GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Car Guide Rail 1", GetId(), box, F_PI);
	box.Move(GetBoxCar().WidthExt() + 100 * fScale + rl, 0, 0);
	GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Car Guide Rail 2", GetId(), box, 0);

	// cwt rails
	if (bCwtRear)
	{
		box = BOX(GetBoxCwt().Left() - rl, GetBoxCwt().CentreY() - rw/2, 0, rl, rw, 0);
		box.SetHeight(h + GetLiftGroup()->GetMRLevel() - GetLiftGroup()->GetMRSlabThickness());
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 1", GetId(), box, F_PI);
		box.Move(GetBoxCwt().Width() + rl, 0, 0);
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 2", GetId(), box, 0);
	}
	else
	{
		if (GetShaftLine() == 0) rl = -rl;

		box = BOX(GetBoxCwt().CentreX() - rw/2, GetBoxCwt().Front() - rl, 0, rw, rl, 0);
		box.SetHeight(h + GetLiftGroup()->GetMRLevel() - GetLiftGroup()->GetMRSlabThickness());
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 1", GetId(), box, 3*F_PI/2);
		box.Move(0, GetBoxCwt().Depth() + rl, 0);
		GetPitElement()->BuildModel(CElem::MODEL_RAIL, L"Counterweight Guide Rail 2", GetId(), box, F_PI/2);
	}

	// cwt
	box = GetBoxCwt();
	box.Move(0, 0, h + GetLiftGroup()->GetStorey(GetLiftGroup()->GetHighestStoreyServed() - GetLiftGroup()->GetBasementStoreyCount())->GetLevel());
	GetPitElement()->BuildModel(CElem::MODEL_CWT, L"Counterweight", GetId(), box);

	// Combination Bracket: meaning of parameters: nParam = LHS bracket; fParam1 = bracket width
	box = GetCombBox().GetInnerBox();
	if (GetLiftGroup()->IsPitLadder())
	{
		box.Move(0, 0, -box.Lower());	// cancel any previous elevation...
		box.Move(0, 0, GetLiftGroup()->GetPitLadderLowerBracket() - box.Height()/2);
	}
	else
		box.Move(0, 0, -2 * box.Lower() - box.Height() + 500 * fScale);
	GetPitElement()->BuildModel(CElem::MODEL_COMBINATION_BRACKET, L"Combination Bracket (HR)", GetId(), box, 0, GetCombBox().LeftThickness() == 0, abs(GetCombBox().FrontThickness()));

	// light
	box = BOX(GetLightingXPos(), GetBox().Rear(), 0, 0, 0, 0);
	fAngle = M_PI * (GetShaftLine() ? -1 : 1) / 2;
	GetPitElement()->BuildModel(CElem::MODEL_LIGHT, L"Lighting (Pit)", GetId(), box, fAngle, 1, /* pit */ h);
}

void CLiftGroupConstr::SHAFT::Deconstruct()
{
	if (m_pStoreyBones == NULL) return;
	for (AVULONG i = 0; i < GetLiftGroup()->GetStoreyCount(); i++)
	{
		delete m_pStoreyBones[i].m_pElem;			m_pStoreyBones[i].m_pElem = NULL;
		delete m_pStoreyBones[i].m_pElemLobbySide;	m_pStoreyBones[i].m_pElemLobbySide = NULL;
		delete m_pStoreyBones[i].m_pElemLeft;		m_pStoreyBones[i].m_pElemLeft = NULL;
		delete m_pStoreyBones[i].m_pElemRight;		m_pStoreyBones[i].m_pElemRight = NULL;
		for (AVULONG j = 0; j < MAX_DOORS; j++)
		{
			if (m_pStoreyBones[i].m_ppDoors[j]) 
				delete m_pStoreyBones[i].m_ppDoors[j];
			m_pStoreyBones[i].m_ppDoors[j] = NULL;
		}
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

void CLiftGroupConstr::LIFT::Construct(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetLiftGroup()->bFastLoad) { GetLiftGroup()->bFastLoad = true; MessageBeep(MB_OK); }

	AVFLOAT fScale = GetLiftGroup()->GetScale();
	AVULONG nShaftLine = GetShaft()->GetShaftLine();
	AVULONG nStartingStorey = GetLiftGroup()->GetBasementStoreyCount();

	bool bFlip = (nShaftLine == 1);
	AVULONG doorType = GetShaft()->GetDoorType(), panelsCount = GetShaft()->GetDoorPanelsCount(), panelsPerDoor = GetShaft()->GetDoorPanelsPerDoor();

	BOX box;

	// Create skeletal elements (entire lift)
	if (!GetProject()->GetRevitCompatibilityMode_())
		m_pElem = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetElement(), CElem::ELEM_LIFT, (LPOLESTR)GetName().c_str(), iShaft, GetShaft()->GetLiftPos(nStartingStorey));

	for (AVULONG iDeck = 0; iDeck < GetShaft()->GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		AVULONG nGroundFloorIndex = GetLiftGroup()->GetBasementStoreyCount();
		AVFLOAT fLowerDeckHeight = GetLiftGroup()->GetStorey(nGroundFloorIndex + iDeck)->GetLevel() - GetLiftGroup()->GetStorey(nGroundFloorIndex)->GetLevel();

		if (GetProject()->GetRevitCompatibilityMode_())
		{
			std::wstring name = GetName() + L" Deck %d";
			m_ppDecks[iDeck] = GetProject()->CreateElement(GetLiftGroup(), GetLiftGroup()->GetElement(), CElem::ELEM_DECK, (LPOLESTR)name.c_str(), iDeck, GetShaft()->GetLiftPos(nStartingStorey) + Vector(0, 0, fLowerDeckHeight)); 
		}
		else
			m_ppDecks[iDeck] = GetProject()->CreateElement(GetLiftGroup(), m_pElem, CElem::ELEM_DECK, L"Deck %d", iDeck, Vector(0, 0, fLowerDeckHeight)); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		XBOX box = GetShaft()->GetBoxCar() - GetShaft()->GetLiftPos(0);
		BOX boxDoor0 = GetShaft()->GetBoxCarDoor(0) - GetShaft()->GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };

		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT_FLOOR,   L"Floor %d",     iShaft + 1000*iDeck, box.LowerSlab());
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT_CEILING, L"Ceiling %d",   iShaft + 1000*iDeck, box.UpperSlab());

		// Door
		// AVFLOAT fGap = nShaftLine ? 5 * fScale : -5 * fScale;
		for (AVULONG iIndex = 0; iIndex < 2; iIndex++)	// 0 = left; 1 = right
		{
			if (iIndex == 0 && doorType == DOOR_SIDE_RIGHT || iIndex == 1 && doorType == DOOR_SIDE_LEFT)
				continue;	// no of this type of doors
			for (AVULONG iPanel = 0; iPanel < panelsPerDoor; iPanel++)
			{
				CElem *&pElem = m_ppDoors[iDeck * MAX_DOORS + iPanel * 2 + iIndex];
				pElem = GetProject()->CreateElement(GetLiftGroup(), m_ppDecks[iDeck], CElem::ELEM_BONE, L"Car Door %d", iPanel * 2 + iIndex, Vector(0));
				// BOX box = boxDoor0; box.SetFront(box.Front() + fGap); box.SetRear(box.Rear() - fGap); 
				BOX box = boxDoor0.DoorPanel(panelsCount, panelsPerDoor, iIndex, iPanel, true, bFlip);
				pElem->BuildWall(CElem::WALL_LIFT_DOOR, L"Car Door %d", iDeck * 100 + iPanel * 2 + iIndex, box);
			}
		}

		// Jamb
		if (nShaftLine == 0 && doorType == 1 || nShaftLine == 1 && doorType == 2)
		{
			BOX box = BOX(boxDoor0.RightRearLower(), boxDoor0.Depth(), -100 * fScale, boxDoor0.Height());
			m_ppDecks[iDeck]->BuildModel(CElem::MODEL_JAMB, L"Car Door Jamb %d", iShaft + 1000*iDeck, box, -F_PI_2);
		}
		if (nShaftLine == 0 && doorType == 2 || nShaftLine == 1 && doorType == 1)
		{
			BOX box = BOX(boxDoor0.LeftFrontLower(), boxDoor0.Depth(), -100 * fScale, boxDoor0.Height());
			m_ppDecks[iDeck]->BuildModel(CElem::MODEL_JAMB, L"Car Door Jamb %d", iShaft + 1000*iDeck, box, F_PI_2);
		}

		// Heading
		BOX boxH = boxDoor0.DoorWithExtension(doorType, panelsCount, panelsPerDoor, 100 * fScale, bFlip);
		boxH.Move(0, 0, boxH.Height()); boxH.SetHeight(100 * fScale);
		m_ppDecks[iDeck]->BuildModel(CElem::MODEL_HEADING, L"Car Door Heading %d", iShaft + 1000*iDeck, boxH);

		// Sill
		AVFLOAT fAngle = GetShaft()->GetShaftLine() ? 0 : F_PI;
		BOX boxA = boxDoor0.DoorWithExtension(doorType, panelsCount, panelsPerDoor, 100 * fScale, bFlip);
		boxA.Move(0, 0, -100 * fScale); boxA.SetHeight(100 * fScale);
		m_ppDecks[iDeck]->BuildModel(CElem::MODEL_SILL_CAR, L"Car Door Sill %d", iShaft + 1000*iDeck, boxA, fAngle);

		// Wall
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"FrontWall %d", iShaft + 1000*iDeck, box.FrontWall(), Vector(0), 1, door);
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"RearWall %d",  iShaft + 1000*iDeck, box.RearWall(), Vector(0, 0, F_PI));
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"LeftWall %d",  iShaft + 1000*iDeck, box.LeftWall(), Vector(0, 0, F_PI_2));
		m_ppDecks[iDeck]->BuildWall(CElem::WALL_LIFT,  L"RightWall %d", iShaft + 1000*iDeck, box.RightWall(), Vector(0, 0, -F_PI_2));

		// Handrail
		if (iDeck == GetShaft()->GetDeckCount() - 1)
		{
			BOX boxHR = box.GetLargeBox();
			boxHR.Move(0, 0, box.Height() + box.UpperThickness());
			boxHR.SetHeight(fScale * 1000);
			m_ppDecks[iDeck]->BuildModel(CElem::MODEL_HANDRAIL, L"Handrail", iShaft + 1000*iDeck, boxHR, fAngle);
		}
	}
}

void CLiftGroupConstr::LIFT::Deconstruct()
{
	delete m_pElem;

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (m_ppDecks[i]) delete m_ppDecks[i];
	memset(m_ppDecks, 0, sizeof(m_ppDecks));
	for (AVULONG i = 0; i < MAX_DOORS; i++)
		if (m_ppDoors[i]) delete m_ppDoors[i];
	memset(m_ppDoors, 0, sizeof(m_ppDoors));
}

void CLiftGroupConstr::Construct(AVVECTOR vec)
{
	Move(vec.x, vec.y, vec.z);

	m_pElem = GetProject()->CreateElement(this, GetProject()->GetSiteElement(), CElem::ELEM_LIFTGROUP, (LPOLESTR)GetName().c_str(), 0, Vector(0));

	// apply the shift vector to all sims
	for each (CSim *pSim in GetSims())
		pSim->SetOffsetVector(vec);

	// Lifts
	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
		GetLift(iLift)->Construct(iLift);

	// Pits and its equipment
	if (GetPit())
	{
		GetPit()->Construct();
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->ConstructPitEquipment();
	}

	// Storeys and Shafts
	for (AVULONG iStorey = 0; iStorey < GetStoreyCount(); iStorey++)
	{
		GetStorey(iStorey)->Construct(iStorey);
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->Construct(iStorey);
	}

	// The Machine Room
	if (GetMR())
	{
		GetMR()->Construct();
		if (!IsMRL())	// do nothing if Machine Room-Less Lift
			for (AVULONG iShaft = 0; iShaft < this->GetShaftCount(); iShaft++) 
				GetShaft(iShaft)->ConstructMREquipment();
	}
}

void CLiftGroupConstr::Deconstruct()
{
	if (m_pElem) delete m_pElem;

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Deconstruct();
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Deconstruct();

	if (GetMR())
		GetMR()->Deconstruct();
	if (GetPit())
		GetPit()->Deconstruct();
}




