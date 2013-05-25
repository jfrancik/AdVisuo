// BaseLiftGroup.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseLiftGroup.h"
#include "BaseSimClasses.h"

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup

CLiftGroup::CLiftGroup(CProject *pProject, AVULONG nIndex) : m_pProject(pProject), m_nIndex(nIndex)
{
	m_nId = 0;
	m_nProjectId = 0;
	m_pSim = NULL;
	m_fScale = 1;
	m_nBasementStoreyCount = 0;
	m_pnShaftCount[0] = m_pnShaftCount[1] = 0;
	m_LiftShaftArrang = SHAFT_INLINE;
	m_LobbyArrangement = LOBBY_OPENPLAN;
	m_pMR = NULL;
	m_pPit = NULL;
	m_fPitLevel = m_fMRLevel = 0;
	m_fLiftingBeamHeight = m_fLiftingBeamWidth = 0;
	m_fMRDoorOffset = 0;
	m_fMRIndentFront = m_fMRIndentRear = 0;
}

CLiftGroup::~CLiftGroup()
{
	DeleteStoreys();
	DeleteShafts();
	DeleteLifts();
	DeleteExtras();
	DeleteSim();
}

BOX CLiftGroup::GetTotalAreaBox()
{
	BOX box = GetBox();
	box.SetLeft(0); box.SetRight(0); box.SetLeftThickness(0); box.SetRightThickness(0);
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		BOX s = GetShaft(i)->GetBox();
		box += s;
	}
	return box;
}

CLiftGroup::STOREY *CLiftGroup::AddStorey()
{
	STOREY *p = CreateStorey(m_storeys.size());
	m_storeys.push_back(p);
	return p;
}

void CLiftGroup::DeleteStoreys()
{
	for each (STOREY *pStorey in m_storeys)
		delete pStorey;
	m_storeys.clear();
}

CLiftGroup::SHAFT *CLiftGroup::AddShaft()
{
	SHAFT *p = CreateShaft(m_shafts.size());
	m_shafts.push_back(p);
	return p;
}

void CLiftGroup::DeleteShafts()
{
	for each (SHAFT *pShaft in m_shafts)
		delete pShaft;
	m_shafts.clear();
}

void CLiftGroup::AddExtras()
{
	DeleteExtras();
	m_pMR = CreateMR();
	m_pPit = CreatePit();
}

void CLiftGroup::DeleteExtras()
{
	if (m_pMR) delete m_pMR;
	m_pMR = NULL;
	if (m_pPit) delete m_pPit;
	m_pPit = NULL;
}

AVSIZE CLiftGroup::CalcPanelFootstep(AVULONG iFrom, AVULONG iTo)
{
	if (iTo > GetShaftCount()) iTo = GetShaftCount();
	
	AVSIZE sz(0, 0);
	for (AVULONG iShaft = iFrom; iShaft < iTo; iShaft++)
	{
		sz.x += GetShaft(iShaft)->GetBoxPanelCtrl().Width() + GetShaft(iShaft)->GetBoxPanelDrv().Width();
		sz.y = max(sz.y, max(abs(GetShaft(iShaft)->GetBoxPanelCtrl().Depth()), abs(GetShaft(iShaft)->GetBoxPanelDrv().Depth())));
	}
	return sz;
}

AVSIZE CLiftGroup::CalcPanelFootstepIso(AVULONG iFrom, AVULONG iTo)
{
	if (iTo > GetShaftCount()) iTo = GetShaftCount();
	
	AVSIZE sz(0, 0);
	for (AVULONG iShaft = iFrom; iShaft < iTo; iShaft++)
	{
		sz.x += GetShaft(iShaft)->GetBoxPanelIso().Width();
		sz.y = max(sz.y, abs(GetShaft(iShaft)->GetBoxPanelIso().Depth()));
	}
	return sz;
}

CLiftGroup::LIFT *CLiftGroup::AddLift()
{
	LIFT *p = CreateLift(m_lifts.size());
	m_lifts.push_back(p);
	return p;
}

void CLiftGroup::DeleteLifts()
{
	for each (LIFT *pLift in m_lifts)
		delete pLift;
	m_lifts.clear();
}

CSim *CLiftGroup::AddSim()
{
	m_pSim = CreateSim();
	if (m_pSim)
	{
		m_pSim->SetLiftGroup(this);
		m_pSim->SetIndex(GetIndex());
	}
	return m_pSim;
}

void CLiftGroup::DeleteSim()
{
	if (m_pSim) delete m_pSim;
	m_pSim = NULL;
}

void CLiftGroup::ResolveMe()
{
	SetId(ME[L"ID"]);
	SetIndex(ME[L"LiftGroupIndex"]);
	SetProjectId(ME[L"ProjectId"]);
	m_nBasementStoreyCount = (ULONG)ME[L"NumberOfBasementStoreys"];

	AVULONG nLifts = 0; 
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		GetShaft(i)->SetLiftRange(nLifts, max(1, (AVULONG)(*GetShaft(i))[L"LiftsPerShaft"]));
		nLifts += GetShaft(i)->GetLiftCount();
	}
	for (AVULONG i = 0; i < nLifts; i++)
		AddLift();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		for (AVULONG j = GetShaft(i)->GetLiftBegin(); j < GetShaft(i)->GetLiftEnd(); j++)
			GetLift(j)->SetShaftId(i);
}

void CLiftGroup::ConsoleCreate()
{
	// resolve vars
	m_LobbyArrangement = (LOBBY_ARRANG)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANG)(ULONG)ME[L"LiftShaftArrangementId"];
	m_strName = ME[L"Name"];

	if (m_LobbyArrangement == LOBBY_OPENPLAN && m_LiftShaftArrang == SHAFT_OPPOSITE)
		m_LobbyArrangement = LOBBY_THROUGH;
	ME[L"LobbyArrangementId"] = (AVLONG)m_LobbyArrangement;

	AVFLOAT fLobbyCeilingSlabHeight = 1000;	// ME[L"LiftLobbyCeilingHeight"];
	AVFLOAT fLobbyDepth = ME[L"FrontLobbyWidth"];
	AVFLOAT fFrontWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fSideWallThickness = ME[L"FrontWallThickness"];
	AVFLOAT fShaftWallThicknessRear = ME[L"ShaftWallThicknessRear"];
	AVFLOAT fShaftWallThicknessSide = ME[L"ShaftWallThicknessSide"];
	AVFLOAT fMRSlabThickness = ME[L"MachineRoomSlabThickness"];

	AVFLOAT fRearLobbyDepth = ME[L"RearLobbyWidth"];	// NOT USED YET!!!
	m_fLiftingBeamHeight = ME[L"LiftingBeamHeight"];
	m_fLiftingBeamWidth = 200;

	
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
	m_box.SetThickness(fLt, fRt, fFrontWallThickness, (m_LobbyArrangement == LOBBY_OPENPLAN) ? 0 : fFrontWallThickness, 2, fLobbyCeilingSlabHeight);

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
		pShaft->ConsoleCreate(i, 0, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessSide);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
		}
		else if (fDepth - fPrevDepth < -fShaftWallThicknessRear)	// note: both values are negative here!
		{	// deeper than previous: build a shorter beam and add a wall section
			pPrevShaft->ConsoleCreateBeam(SIDE_RIGHT, pShaft);
			pShaft->Move(pPrevShaft->GetBox().RightThickness(), 0, 0);
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessRear, fPrevDepth - fShaftWallThicknessRear);
		}
		else if (fDepth - fPrevDepth > fShaftWallThicknessRear)	// note: both values are negative here!
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConsoleCreateBeam(SIDE_LEFT, pPrevShaft);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
			GetShaft(i-1)->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessRear, fDepth - fShaftWallThicknessRear);
		}
		else /* if (fDepth == fPrevDepth) */
		{
			// same depth as previous: just build a beam
			pShaft->ConsoleCreateBeam(SIDE_LEFT, pPrevShaft);
			pShaft->Move(pShaft->GetBox().LeftThickness(), 0, 0);
		}
		if (i == GetShaftCount(0) - 1)
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessSide);

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
		pShaft->ConsoleCreate(i, 1, fShaftPosX, fShaftPosY, fFrontWallThickness, fShaftWallThicknessRear);
		
		// build int beams and extra shaft walls
		AVFLOAT fDepth = pShaft->GetBox().Depth();
		if (fPrevDepth == 0)
		{	// no previous shaft: build a wall, not beam
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessSide);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
		}
		else if (fDepth - fPrevDepth > fShaftWallThicknessRear)
		{	// deeper than previous: build a shorter beam and add a wall section
			pPrevShaft->ConsoleCreateBeam(SIDE_LEFT, pShaft);
			pShaft->Move(-pPrevShaft->GetBox().LeftThickness(), 0, 0);
			pShaft->ConsoleCreateSideWall(SIDE_RIGHT, fShaftWallThicknessRear, fPrevDepth + fShaftWallThicknessRear);
		}
		else if (fDepth - fPrevDepth < -fShaftWallThicknessRear)
		{	// smaller than previous: build a beam and add a wall section to the previous shaft
			pShaft->ConsoleCreateBeam(SIDE_RIGHT, pPrevShaft);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
			GetShaft(i-1)->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessRear, fDepth + fShaftWallThicknessRear);
		}
		else /* if (fDepth == fPrevDepth) */
		{
			// same depth as previous: just build a beam
			pShaft->ConsoleCreateBeam(SIDE_RIGHT, pPrevShaft);
			pShaft->Move(-pShaft->GetBox().RightThickness(), 0, 0);
		}
		if (i == GetShaftCount() - 1)
			pShaft->ConsoleCreateSideWall(SIDE_LEFT, fShaftWallThicknessSide);

		// move on
		fPrevDepth = fDepth;
		fShaftPosX = pShaft->GetBox().Left();
		pPrevShaft = pShaft;
	}

	// Create Storeys
	AVFLOAT fLevel = 0;
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);
		pStorey->ConsoleCreate(i, fLevel);
		fLevel += pStorey->GetHeight();
	}

	// calculate machine room box & level; including panel box sizes but not positions
	m_boxMR = BOX(0, 0, 0, 0, 0, 0);
	AVFLOAT fHeadroom = 0;
	m_boxMR = GetTotalAreaBox();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		SHAFT *pShaft = GetShaft(i);
		if (((AVFLOAT)(*pShaft)[L"MachineRoomHeight"]) > m_boxMR.Height()) m_boxMR.SetHeight((AVFLOAT)(*pShaft)[L"MachineRoomHeight"]);
		if (((AVFLOAT)(*pShaft)[L"Headroom"]) > fHeadroom) fHeadroom = (AVFLOAT)(*pShaft)[L"Headroom"];
	}
	m_boxMR.SetThickness(fShaftWallThicknessSide, fShaftWallThicknessSide, fShaftWallThicknessRear, fShaftWallThicknessRear, fMRSlabThickness, 0);
	m_fMRLevel = max(GetStorey(GetHighestStoreyServed())->GetRoofLevel(), GetStorey(GetHighestStoreyServed())->GetLevel() + fHeadroom);
	m_fMRLevel += + fMRSlabThickness;

	// default values for: group panel layout, door offset and indentation of the MR extension (space to accomodate excessive panels)
	m_boxPanelGrp = BOX(0, 0, 0, 700, 1300, 2200);
	m_fPanelGrpOrientation = (AVFLOAT)M_PI;
	m_fMRDoorOffset = 100;
	m_fMRIndentFront = m_box.Front();
	m_fMRIndentRear = m_box.Rear();

	// calculate position and layout of panels
	AVSIZE szAvail(m_boxMR.Width(), m_box.Depth());		// space available for panels
	AVSIZE szExtra;										// extra space required but not (yet) available
	AVSIZE szCtr = CalcPanelFootstep();					// footstep for drive and control panels
	AVSIZE szIso = CalcPanelFootstepIso();				// footstep for isopanels
	AVSIZE szGrp = CalcPanelFootstepGrp();				// footstep for the group panel

	if (GetShaftLinesCount() == 1)
	{
		// inline shafts: default layout: all panels along the wall
		AVULONG iBreak = 0;		// line break point (default: line 0 is empty, line 1 has everything)
		AVULONG iGroup = 1;		// Group Panel position (default: in line 1)
		AVSIZE szLn0;			// size of line 0 (default: 0)
		AVSIZE szLn1 = szCtr;	// size of line 1 (default: all panels)

		// if no space for everything inline - break up into two lines
		if (szIso.x + szCtr.x + szGrp.x > szAvail.x)
		{
			iBreak = GetShaftCount() / 2;			// even break up of the panels
			szLn0 = CalcPanelFootstep(0, iBreak);
			szLn1 = CalcPanelFootstep(iBreak, 9999);

			// if no room in line #1 and line #0 is shorter --> move the group ctrl to line #0
			if (szLn1.x + szGrp.x + szIso.x > szAvail.x && szLn0.x < szLn1.x)
				iGroup = 0;
		}

		// include the size of the group controller
		if (iGroup == 0) szLn0.x += szGrp.x; else szLn1.x += szGrp.x; 

		// check the capacity
		AVFLOAT f = szIso.x + max(szLn0.x, szLn1.x) - szAvail.x;
		if (f > 0)
			szExtra.x = f;

		// make the machine room narrower if possible
		if (szLn0.y + szLn1.y < szAvail.y)
		{
			AVFLOAT fRearWall = m_boxMR.Rear() - szAvail.y + szLn0.y + szLn1.y;
			AVFLOAT f = m_boxMR.RearThickness();
			m_boxMR.SetRear(fRearWall);
			m_fMRIndentRear = fRearWall;
			m_boxMR.SetRearThickness(f);
		}

		// Place the panels!
		AVFLOAT x = m_boxMR.Right();
		AVFLOAT y = m_boxMR.Rear();

		// Isolator Panels
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelIso();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
			GetShaft(iShaft)->SetPanelIsoOrientation((AVFLOAT)M_PI);
		}
		// Line #1 Ctrl and Drive Panels
		for (AVULONG iShaft = iBreak; iShaft < GetShaftCount(); iShaft++)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelDrv();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
			pBox = &GetShaft(iShaft)->GetBoxPanelCtrl();
			w = pBox->Width(); d = pBox->Depth(); h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
		}
		// Optionally - Group Controller (in line #1)
		if (iGroup == 1)
		{
			BOX *pBox = &GetBoxPanelGrp();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
			m_fPanelGrpOrientation = (AVFLOAT)M_PI;
		}
		x = m_boxMR.Right() - szIso.x;
		y -= szLn1.y;
		// Line #0 Ctrl and Drive Panels
		for (AVLONG iShaft = iBreak - 1; iShaft >= 0; iShaft--)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelDrv();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
			pBox = &GetShaft(iShaft)->GetBoxPanelCtrl();
			w = pBox->Width(); d = pBox->Depth(); h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
		}
		// Optionally - Group Controller (in line #1)
		if (iGroup == 0)
		{
			BOX *pBox = &GetBoxPanelGrp();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x - w, y - d, 0, w, d, h);
			x -= w;
			m_fPanelGrpOrientation = (AVFLOAT)M_PI;
		}

		// Door location
		m_fMRDoorOffset = szLn1.y;
	}
	else
	{
		// isolator panels rotated by 90 degrees and made broader by the door!
		AVFLOAT fAux = szIso.x;				// swap x and y!
		szIso.x = szIso.y;
		szIso.y = fAux + GetMRDoorWidth();	// add the width of the door here!

		// opposite shafts
		AVULONG iGroup = 1;				// Group Panel position (default: in line 1)
		AVSIZE szLn0 = CalcPanelFootstepLn(0);	// size of line 0 (default: 0)
		AVSIZE szLn1 = CalcPanelFootstepLn(1);	// size of line 1 (default: all panels)

		// if line 0 is shorter, move the group panel there
		if (szLn0.x < szLn1.x)
			iGroup = 0;

		// inlude the size of the group controller
		if (iGroup == 0) szLn0.x += szGrp.x; else szLn1.x += szGrp.x; 

		// check the capacity
		AVFLOAT f = 500 + szIso.x + max(szLn0.x, szLn1.x) - szAvail.x;
		if (f > 0)
			szExtra.x = f;

		// additionally, if isolator panels too wide to fit in available y space and too deep to fit in available extra x space
		if (szIso.y > szAvail.y && szIso.x > szExtra.x)
			szExtra.x = szIso.x;

		// Place the panels!
		AVFLOAT x = m_boxMR.Left() + 500;
		AVFLOAT y = m_box.CentreY();

		// Line #0 Ctrl and Drive Panels
		for (AVULONG iShaft = 0; iShaft < GetShaftEnd(0); iShaft++)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelCtrl();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
			pBox = &GetShaft(iShaft)->GetBoxPanelDrv();
			w = pBox->Width(); d = pBox->Depth(); h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
		}
		// Optionally - Group Controller (in line #0)
		if (iGroup == 0)
		{
			BOX *pBox = &GetBoxPanelGrp();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
			m_fPanelGrpOrientation = (AVFLOAT)M_PI;
		}

		x = m_boxMR.Left() + 500;
		y += szLn1.y;

		// Line #1 Ctrl and Drive Panels
		for (AVULONG iShaft = GetShaftEnd(1) - 1; iShaft >= GetShaftBegin(1); iShaft--)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelCtrl();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
			pBox = &GetShaft(iShaft)->GetBoxPanelDrv();
			w = pBox->Width(); d = pBox->Depth(); h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
		}
		// Optionally - Group Controller (in line #1)
		if (iGroup == 1)
		{
			BOX *pBox = &GetBoxPanelGrp();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y - d, 0, w, d, h);
			x += w;
			m_fPanelGrpOrientation = 0;
		}

		// Isolator Panels
		x = m_boxMR.Right() - szIso.x + szExtra.x;
		y = m_box.CentreY() - szIso.y / 2;

		m_fMRIndentFront = min(m_fMRIndentFront, y);	// setup minimum indentation to accomodate for the panels
		for (AVULONG iShaft = 0; iShaft < GetShaftEnd(0); iShaft++)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelIso();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y, 0, d, w, h);
			y += w;
			GetShaft(iShaft)->SetPanelIsoOrientation(-(AVFLOAT)M_PI/2);
		}
		y += GetMRDoorWidth();
		AVFLOAT fDoorPos = y;
		for (AVULONG iShaft = GetShaftBegin(1); iShaft < GetShaftEnd(1); iShaft++)
		{
			BOX *pBox = &GetShaft(iShaft)->GetBoxPanelIso();
			AVFLOAT w = pBox->Width(), d = pBox->Depth(), h = pBox->Height();
			*pBox = BOX(x, y, 0, d, w, h);
			y += w;
			GetShaft(iShaft)->SetPanelIsoOrientation(-(AVFLOAT)M_PI/2);
		}
		m_fMRIndentRear = max(m_fMRIndentRear, y);		// setup minimum indentation to accomodate for the panels

		if (szExtra.x > 0)
			m_fMRDoorOffset = m_fMRIndentRear - fDoorPos;
		else
			m_fMRDoorOffset = m_boxMR.Rear() - fDoorPos;
	}

	// Extend Machine Room on the Right (if necessary)
	if (szExtra.x > 0)
	{
		AVFLOAT f = m_boxMR.RightThickness();
		m_boxMR.SetRight(m_boxMR.Right() + szExtra.x);
		m_boxMR.SetRightThickness(f);
	}
		
	if (GetMR()) GetMR()->ConsoleCreate();

	// calculate pit level box & level
	m_boxPit = m_box;
	m_fPitLevel = 0;
	for (AVULONG i = 0; i < GetShaftCount(); i++)
	{
		AVFLOAT fPitLevel = (*GetShaft(i))[L"PitDepth"];
		if (fPitLevel > m_fPitLevel) m_fPitLevel = fPitLevel;
	}
	m_fPitLevel = -m_fPitLevel;
	if (GetPit()) GetPit()->ConsoleCreate();

	// Final updates and amendments
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->ConsoleCreateAmend();

	// update ME
	ME[L"Line1"] = m_pnShaftCount[0];
	ME[L"Line2"] = m_pnShaftCount[1];
	ME[L"BoxLobby"] = m_box.Stringify();
	ME[L"BoxMR"] = m_boxMR.Stringify();
	ME[L"MRLevel"] = m_fMRLevel;
	ME[L"MRDoorOffset"] = m_fMRDoorOffset;
	ME[L"MRIndentFront"] = m_fMRIndentFront;
	ME[L"MRIndentRear"] = m_fMRIndentRear;
	ME[L"BoxPanelGrp"] = m_boxPanelGrp.Stringify();
	ME[L"PanelGrpOrientation"] = m_fPanelGrpOrientation;
	ME[L"BoxPit"] = m_boxPit.Stringify();
	ME[L"PitLevel"] = m_fPitLevel;

	erase(L"FrontLobbyWidth");
	erase(L"FrontWallThickness");
	erase(L"ShaftWallThicknessRear");
	erase(L"ShaftWallThicknessSide");
	erase(L"MachineRoomSlabThickness");
}

void CLiftGroup::Create()
{
	m_LobbyArrangement = (LOBBY_ARRANG)(ULONG)ME[L"LobbyArrangementId"];
	m_LiftShaftArrang = (SHAFT_ARRANG)(ULONG)ME[L"LiftShaftArrangementId"];
	m_strName = ME[L"Name"];
	m_pnShaftCount[0] = ME[L"Line1"];
	m_pnShaftCount[1] = ME[L"Line2"];
	m_box.ParseFromString(ME[L"BoxLobby"]);
	m_boxMR.ParseFromString(ME[L"BoxMR"]);
	m_fMRLevel = ME[L"MRLevel"];
	m_fMRDoorOffset = ME[L"MRDoorOffset"];
	m_fLiftingBeamHeight = ME[L"LiftingBeamHeight"];
	m_fLiftingBeamWidth = 200;
	m_fMRIndentFront = ME[L"MRIndentFront"];
	m_fMRIndentRear = ME[L"MRIndentRear"];
	m_boxPanelGrp.ParseFromString(ME[L"BoxPanelGrp"]);
	m_fPanelGrpOrientation = ME[L"PanelGrpOrientation"];
	m_boxPit.ParseFromString(ME[L"BoxPit"]);
	m_fPitLevel = ME[L"PitLevel"];
	
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Create();

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Create();

	if (GetMR()) GetMR()->Create();
	if (GetPit()) GetPit()->Create();
}

void CLiftGroup::Scale(AVFLOAT f)
{
	m_fScale *= f;
	m_box.Scale(f);
	m_boxMR.Scale(f);
	m_fMRLevel *= f;
	m_fMRDoorOffset *= f;
	m_fLiftingBeamHeight *= f;
	m_fLiftingBeamWidth *= f;
	m_fMRIndentFront *= f;
	m_fMRIndentRear *= f;
	m_boxPanelGrp.Scale(f);
	m_boxPit.Scale(f);
	m_fPitLevel *= f;

	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Scale(f);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Scale(f);
	if (GetMR()) GetMR()->Scale(f);
	if (GetPit()) GetPit()->Scale(f);
}

void CLiftGroup::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_box.Move(x, y, z);
	for (AVULONG i = 0; i < GetShaftCount(); i++) 
		GetShaft(i)->Move(x, y, z);
	for (AVULONG i = 0; i < GetStoreyCount(); i++) 
		GetStorey(i)->Move(x, y, z);
	if (GetMR()) GetMR()->Move(x, y, z);
	GetBoxMR().Move(x, y, z);
	GetBoxPanelGrp().Move(x, y, z);
	if (GetPit()) GetPit()->Move(x, y, z);
	GetBoxPit().Move(x, y, z);

	m_fMRLevel += z;
	m_fMRIndentFront += y;
	m_fMRIndentRear += y;
	m_fPitLevel += z;
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::STOREY

void CLiftGroup::STOREY::ConsoleCreate(AVULONG nId, AVFLOAT fLevel)
{
//	m_nId = ME[L"GroundIndex"];
	m_fLevel = fLevel;
	m_strName = ME[L"Name"];
	m_box = GetLiftGroup()->m_box;
	m_box.SetHeight((AVFLOAT)ME[L"FloorHeight"] * 1000.0f- m_box.UpperThickness());

	ME[L"FloorId"] = GetId();
	ME[L"FloorLevel"] = m_fLevel;
	ME[L"Box"] = m_box.Stringify();
	erase(L"FloorHeight");
	erase(L"GroundIndex");
}

void CLiftGroup::STOREY::Create()
{
	m_strName = ME[L"Name"];

	m_fLevel = ME[L"FloorLevel"];
	m_box.ParseFromString(ME[L"Box"]);
}

void CLiftGroup::STOREY::Scale(AVFLOAT f)
{
	m_fLevel *= f;
	m_box.Scale(f);
}

void CLiftGroup::STOREY::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_fLevel += z;
	m_box.Move(x, y, z);
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::MR

void CLiftGroup::MR::ConsoleCreate()
{
	m_nId = 9999;
	m_fLevel = GetLiftGroup()->GetMRLevel();
	m_strName = L"Machine Room";
	m_box = GetLiftGroup()->GetBoxMR();
}

void CLiftGroup::MR::Create()
{
	m_nId = 9999;
	m_fLevel = GetLiftGroup()->GetMRLevel();
	m_strName = L"Machine Room";
	m_box = GetLiftGroup()->GetBoxMR();
}

AVFLOAT CLiftGroup::MR::GetExt()
{
	AVFLOAT f = GetBox().RightExt() - GetLiftGroup()->GetBox().RightExt();
	if (f < 0) ASSERT(FALSE);
	return max(f, 0);
}

BOX CLiftGroup::MR::GetBoxMain()
{
	BOX box = m_box;
	AVFLOAT fExt = GetExt();
	box.SetRight(box.Right() - fExt);
	box.SetRightExt(box.RightExt() - fExt);
	return box;
}

BOX CLiftGroup::MR::GetBoxExt()
{
	AVFLOAT fExt = GetExt();

	BOX box = m_box;
	box.SetLeft(GetLiftGroup()->GetBox().RightExt());
	box.SetLeftThickness(0);

	AVFLOAT f;
	f = box.FrontThickness();
	box.SetFront(GetLiftGroup()->GetMRIndentFront()); box.SetFrontThickness(f);
	f = box.RearThickness();
	box.SetRear(GetLiftGroup()->GetMRIndentRear()); box.SetRearThickness(f);

	return box;
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::PIT

void CLiftGroup::PIT::ConsoleCreate()
{
	m_nId = 9998;
	m_fLevel = GetLiftGroup()->GetPitLevel();
	m_strName = L"Lift Pit Level";
	m_box = GetLiftGroup()->GetBoxPit();
}

void CLiftGroup::PIT::Create()
{
	m_nId = 9998;
	m_fLevel = GetLiftGroup()->GetPitLevel();
	m_strName = L"Lift Pit Level";
	m_box = GetLiftGroup()->GetBoxPit();
}

//////////////////////////////////////////////////////////////////////////////////
// CLiftGroup::SHAFT

BOX &CLiftGroup::SHAFT::GetBox(CLiftGroup::SHAFT::SHAFT_BOX n, AVULONG i)
{
	switch (n)
	{
	case BOX_SHAFT:		return m_boxShaft;
	case BOX_DOOR:		return m_boxDoor[i];
	case BOX_CAR:		return m_boxCar;
	case BOX_CARDOOR:	return m_boxCarDoor[i];
	case BOX_CW:		return m_boxCwt;
	case BOX_GOVERNOR:	return m_boxGovernor;
	case BOX_LADDER:	return m_boxLadder;
	case BOX_PANEL_CTRL:return m_boxPanelCtrl;
	case BOX_PANEL_DRV:	return m_boxPanelDrv;
	case BOX_PANEL_ISO:	return m_boxPanelIso;
	default:			return m_boxShaft;
	}
}

void CLiftGroup::SHAFT::ConsoleCreate(AVULONG nId, AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY, AVFLOAT fFrontWall, AVFLOAT fRearWall)
{
	m_nId = nId;
	//m_nId = ME[L"LiftNumber"];
	//m_nId = ME[L"LiftIndex"];
	m_nShaftLine = nLine;

	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];
	m_nLiftType = ME[L"LiftTypeId"];
	m_nDoorType = ME[L"DoorTypeId"];

	m_nOpeningTime = (AVULONG)((float)ME[L"OpeningTime"] * 1000);
	m_nClosingTime = (AVULONG)((float)ME[L"ClosingTime"] * 1000);

	m_strStoreysServed = ME[L"StoreysServed"];

	// ### To resolve the problem quickly
	ME[L"LiftsPerShaft"] = (AVULONG)1;

	m_nLiftCount = max(1, (AVULONG)ME[L"LiftsPerShaft"]);

	AVFLOAT ShaftWidth = ME[L"ShaftWidth"];
	AVFLOAT ShaftDepth = ME[L"ShaftDepth"];
	AVFLOAT CarDepth = ME[L"CarDepth"];
	AVFLOAT CarWidth = ME[L"CarWidth"];
	AVFLOAT CarHeight = ME[L"CarCeilingHeight"];
	AVFLOAT LiftDoorHeight = ME[L"DoorHeight"];
	AVFLOAT LiftDoorWidth = ME[L"DoorWidth"];

	AVFLOAT fVoidRear = ME[L"ClearanceCarToShaftWallOnDepth"];
	AVFLOAT fVoidLeft = ME[L"ClearanceCarToShaftWallOnWidthLHS"];
	AVFLOAT fVoidRight = ME[L"ClearanceCarToShaftWallOnWidthRHS"];
	AVFLOAT fCarRearWallThickness = ME[L"CarDepthWallThickness"];
	AVFLOAT fCarFrontWallThickness = ME[L"CarFrontReturnDepth"];
	AVFLOAT fCarSideWallThickness = ME[L"CarSideWallThickness"];
	AVFLOAT fLightingVoidHeight = ME[L"LightingVoidHeight"];
	AVFLOAT fCarDoorDepth = ME[L"CarDoorDepth"];
	AVFLOAT fLandingDoorDepth = ME[L"LandingDoorDepth"];

	AVFLOAT fCarFloorThickness = 150;

	// Capacity/Speed related values
	AVFLOAT fCapacity = ME[L"Capacity"];
	AVFLOAT fSpeed = ME[L"Speed"];
	AVULONG nCwtPos = ME[L"CounterWeightPositionId"];

	AVFLOAT cwtw = 200;		// cwt width (depth)
	AVFLOAT cwtl = 1000;	// cwt length (width)
	AVFLOAT cwtx = 50;		// cwt extra from the shaft wall
	AVFLOAT slingb = 400;	// sling bottom height
	AVFLOAT slingt = 1000;	// sling top height
	if (fSpeed >= 2.5) cwtl = 1200; if (fSpeed >= 4) cwtl = 1500;
	if (fSpeed >= 4) cwtx = 100;
	if (fCapacity >= 1275) slingb = 600; if (fCapacity >= 1800) slingb = 800;

	// determine machine & panel types
	                          m_nMachineType = 1; m_nPanelCtrlType = 1; m_nPanelDrvType = 1; m_nPanelIsoType = 1; m_boxPanelCtrl = BOX(0, 0, 0, 700, 1300, 2200); m_boxPanelDrv = BOX(0, 0, 0, 1000, 1300, 2200); m_boxPanelIso = BOX(0, 0, 0, 1000, 1300, 2200);
	if (fCapacity >= 1800)	{ m_nMachineType = 2; m_nPanelCtrlType = 1; m_nPanelDrvType = 2; m_nPanelIsoType = 2; m_boxPanelCtrl = BOX(0, 0, 0, 700, 1300, 2200); m_boxPanelDrv = BOX(0, 0, 0, 1500, 1300, 2200); m_boxPanelIso = BOX(0, 0, 0, 1250, 1300, 2200); }
	if (fSpeed >= 2.5)		{ m_nMachineType = 3; m_nPanelCtrlType = 1; m_nPanelDrvType = 3; m_nPanelIsoType = 3; m_boxPanelCtrl = BOX(0, 0, 0, 700, 1300, 2200); m_boxPanelDrv = BOX(0, 0, 0, 1700, 1300, 2200); m_boxPanelIso = BOX(0, 0, 0, 1600, 1300, 2200); }
	if (fSpeed >= 5)		{ m_nMachineType = 4; m_nPanelCtrlType = 1; m_nPanelDrvType = 3; m_nPanelIsoType = 3; m_boxPanelCtrl = BOX(0, 0, 0, 700, 1300, 2200); m_boxPanelDrv = BOX(0, 0, 0, 1700, 1300, 2200); m_boxPanelIso = BOX(0, 0, 0, 1600, 1300, 2200); }

	// additional tuning for the isolator type: isolators are set for every fourth shaft
	AVULONG nId4th = (GetId() / 4) * 4;	// index of the master: first in each four (0, 4, 8 ...)
	if (GetId() != nId4th)
	{
		// for all but the master shaft, zero the isolator, but modify the master type (which must be the maximum in the four)
		if (m_nPanelIsoType > GetLiftGroup()->GetShaft(nId4th)->m_nPanelIsoType)
		{
			GetLiftGroup()->GetShaft(nId4th)->m_nPanelIsoType = m_nPanelIsoType;
			GetLiftGroup()->GetShaft(nId4th)->m_boxPanelIso = m_boxPanelIso;
		}
		m_nPanelIsoType = 0;
		m_boxPanelIso = BOX(0, 0, 0, 0, 0, 0);
	}

	// determine rails size
	m_fRailWidth = 125; m_fRailLength = 82;
	if (fCapacity >= 1275) { m_fRailWidth = 127; m_fRailLength = 89; }
	if (fCapacity >= 1800) { m_fRailWidth = 140; m_fRailLength = 102; }

	// determine buffer num & size
	m_nBufferNum = 1;
	if (fSpeed >= 3.5) m_nBufferNum = 2;
	m_nBufferDiameter = 200; m_nBufferHeight = 540;
	if (fSpeed >= 2.0) m_nBufferHeight = 777;
	if (fSpeed >= 2.5) m_nBufferHeight = 1203;
	if (fSpeed >= 3.0) m_nBufferHeight = 1705;
	if (fSpeed >= 3.5) m_nBufferHeight = 2100;
	if (fSpeed >= 4.0) m_nBufferHeight = 2100;
	if (fSpeed >= 5.0) m_nBufferHeight = 2692;
	if (fSpeed >= 6.0) m_nBufferHeight = 2692;
	if (fSpeed >= 7.0) m_nBufferHeight = 4216;
	if (fSpeed >= 8.0) m_nBufferHeight = 6181;
	if (fSpeed >= 9.0) m_nBufferHeight = 6181;
	if (fSpeed >= 10.0) m_nBufferHeight = 7374;
	
//	MachRoomExt = ME[L"MachRoomExt"];
//	if (MachRoomExt == -1.0f)
//		if (TypeOfLift == LIFT_DOUBLE_DECK) MachRoomExt = 1000.0; else MachRoomExt = 0;

	if (m_nShaftLine == 1) fShaftPosX -= ShaftWidth;
	m_boxShaft = BOX(fShaftPosX, abs(fShaftPosY), ShaftWidth, ShaftDepth);
	m_boxShaft.SetThickness(0, 0, fFrontWall, fRearWall);
	
	AVFLOAT fVoid = (m_nShaftLine == 0) ? fVoidLeft : fVoidRight;
	m_boxCar = BOX(m_boxShaft.Left() + fVoid + fCarSideWallThickness, m_boxShaft.Front() + fLandingDoorDepth + fCarDoorDepth + fCarFrontWallThickness, 0, CarWidth, CarDepth, CarHeight);
	m_boxCar.SetThickness(fCarSideWallThickness, fCarSideWallThickness, fCarFrontWallThickness, fCarRearWallThickness, fCarFloorThickness, fLightingVoidHeight);

	m_boxDoor[0] =    BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxShaft.Front(),    0, LiftDoorWidth, fLandingDoorDepth, LiftDoorHeight);
	m_boxDoor[1] =    BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxShaft.Rear(),     0, LiftDoorWidth, fFrontWall, LiftDoorHeight);
	m_boxCarDoor[0] = BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxCar.FrontExt() - fCarDoorDepth,   0, LiftDoorWidth, fCarDoorDepth, LiftDoorHeight);
	m_boxCarDoor[1] = BOX(m_boxCar.CentreX() - LiftDoorWidth / 2, m_boxCar.Rear(),       0, LiftDoorWidth, fCarRearWallThickness, LiftDoorHeight);

	// resolve left-right side

	AVLONG nMachineOrientation = 0;	// 0 = forward, 1 = left, 2 = backward, 3 = right
	switch (nCwtPos)
	{
	case 1: nMachineOrientation = 0; break;
	case 2: nMachineOrientation = 1; break;
	case 3: nMachineOrientation = 3; break;
	}
	if (m_nShaftLine == 1) nMachineOrientation = (nMachineOrientation + 2) % 4;

	m_fShaftOrientation = (m_nShaftLine == 0) ? 0 : M_PI;
	m_fMachineOrientation = nMachineOrientation * M_PI_2;
	m_fPanelIsoOrientation = (AVFLOAT)M_PI;	

	switch (nMachineOrientation)
	{	case 0:	// forward (first line of lifts): cwt in the rear, gov on the right, light asymetrically, ladder on the left
			m_boxCwt = BOX(m_boxCar.CentreX() - cwtl/2, m_boxShaft.Rear() - cwtx - cwtw, 0, cwtl, cwtw, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.RightExt(), m_boxCar.CentreY(), 0, 200, m_boxCar.DepthExt() / 2, 0);
			m_fLightingXPos = (m_boxCwt.CentreX() >= m_boxShaft.CentreX()) ? (m_boxCwt.Left() + m_boxShaft.Left()) / 2 : (m_boxCwt.Right() + m_boxShaft.Right()) / 2;
			m_boxLadder = BOX(m_boxShaft.Left(), m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 2:	// backward (second line of lifts): cwt in the rear of line 2, gov on the left, light asymetrically, ladder on the right
			m_boxCwt = BOX(m_boxCar.CentreX() - cwtl/2, m_boxShaft.Rear() - cwtx - cwtw, 0, cwtl, cwtw, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.LeftExt() - 200, m_boxCar.CentreY(), 0, 200, m_boxCar.DepthExt() / 2, 0);
			m_fLightingXPos = (m_boxCwt.CentreX() >= m_boxShaft.CentreX()) ? (m_boxCwt.Left() + m_boxShaft.Left()) / 2 : (m_boxCwt.Right() + m_boxShaft.Right()) / 2;
			m_boxLadder = BOX(m_boxShaft.Right() - 132, m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 1:	// heading to the left: cwt on the left, gov in the rear, light centrally, ladder on the right
			m_boxCwt = BOX(m_boxShaft.Left() + cwtx, m_boxCar.CentreY() - cwtl/2, 0, cwtw, cwtl, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt);
			m_boxGovernor = BOX(m_boxCar.CentreX(), m_boxCar.Rear(), 0, m_boxCar.WidthExt() / 2, 200, 0);
			if (m_nShaftLine == 0) m_boxGovernor.MoveY(200); 
			m_fLightingXPos = m_boxDoor[0].CentreX();
			m_boxLadder = BOX(m_boxShaft.Right() - 132, m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
		case 3:	// heading to the right: cwt on the right, gov in the rear, light centrally, ladder on the left
			m_boxCwt = BOX(m_boxShaft.Right() - cwtx - cwtw, m_boxCar.CentreY() - cwtl/2, 0, cwtw, cwtl, CarHeight + fLightingVoidHeight + fCarFloorThickness + slingb + slingt); 
			m_boxGovernor = BOX(m_boxCar.CentreX(), m_boxCar.Rear(), 0, m_boxCar.WidthExt() / 2, 200, 0);
			if (m_nShaftLine == 1) m_boxGovernor.MoveY(200); 
			m_fLightingXPos = m_boxDoor[0].CentreX();
			m_boxLadder = BOX(m_boxShaft.Left(), m_boxCar.FrontExt(), 0, 132, 610, 0);
			break;
	}

	if (fShaftPosY < 0)
		Reflect();
}

void CLiftGroup::SHAFT::ConsoleCreateBeam(AVULONG side, CLiftGroup::SHAFT *pNeighbour)
{
	AVFLOAT fIntDivBeamWidth = ME[L"DividingBeamWidth"];
	AVFLOAT fIntDivBeamHeight = ME[L"DividingBeamHeight"];

	if (pNeighbour && (AVFLOAT)(*pNeighbour)[L"DividingBeamWidth"] > fIntDivBeamWidth) fIntDivBeamWidth = (*pNeighbour)[L"DividingBeamWidth"];
	if (pNeighbour && (AVFLOAT)(*pNeighbour)[L"DividingBeamHeight"] > fIntDivBeamHeight) fIntDivBeamHeight = (*pNeighbour)[L"DividingBeamHeight"];

	//AVVECTOR pos = (side == SIDE_LEFT) ? m_boxShaft.LeftFrontLower() + Vector(-fIntDivBeamWidth, 0, 0): m_boxShaft.RightFrontLower();
	//m_boxBeam = BOX(pos, fIntDivBeamWidth, GetBox().Depth(), fIntDivBeamHeight );

	if (side == SIDE_LEFT)
	{
		m_boxShaft.SetLeftThickness(fIntDivBeamWidth);
		m_fBeamLtHeight = fIntDivBeamHeight;
	}
	else
	{
		m_boxShaft.SetRightThickness(fIntDivBeamWidth);
		m_fBeamRtHeight = fIntDivBeamHeight;
	}
}

void CLiftGroup::SHAFT::ConsoleCreateSideWall(AVULONG side, AVFLOAT fThickness, AVFLOAT fStart)
{
	if (side == SIDE_LEFT)
	{
		m_boxShaft.SetLeftThickness(fThickness);
		m_fWallLtStart = fStart;
	}
	else
	{
		m_boxShaft.SetRightThickness(fThickness);
		m_fWallRtStart = fStart;
	}
}

void CLiftGroup::SHAFT::ConsoleCreateAmend()
{
	ME[L"ShaftId"] = GetId();
	ME[L"ShaftLine"] = m_nShaftLine;

	ME[L"BoxShaft"] = m_boxShaft.Stringify();
	ME[L"LeftWallStart"] = m_fWallLtStart;
	ME[L"RightWallStart"] = m_fWallRtStart;
	ME[L"BeamLtHeight"] = m_fBeamLtHeight;
	ME[L"BeamRtHeight"] = m_fBeamRtHeight;
	ME[L"BoxDoor0"] = m_boxDoor[0].Stringify();
	ME[L"BoxDoor1"] = m_boxDoor[1].Stringify();
	ME[L"BoxCar"] = m_boxCar.Stringify();
	ME[L"BoxCarDoor0"] = m_boxCarDoor[0].Stringify();
	ME[L"BoxCarDoor1"] = m_boxCarDoor[1].Stringify();
	ME[L"BoxCwt"] = m_boxCwt.Stringify();
	ME[L"BoxGovernor"] = m_boxGovernor.Stringify();
	ME[L"BoxLadder"] = m_boxLadder.Stringify();

	ME[L"BoxPanelCtrl"] = m_boxPanelCtrl.Stringify();
	ME[L"BoxPanelDrv"] = m_boxPanelDrv.Stringify();
	ME[L"BoxPanelIso"] = m_boxPanelIso.Stringify();

	ME[L"ShaftOrientation"] = m_fShaftOrientation;
	ME[L"MachineType"] = m_nMachineType;
	ME[L"MachineOrientation"] = m_fMachineOrientation;
	ME[L"PanelCtrlType"] = m_nPanelCtrlType;
	ME[L"PanelDrvType"] = m_nPanelDrvType;
	ME[L"PanelIsoType"] = m_nPanelIsoType;
	ME[L"PanelIsoOrientation"] = m_fPanelIsoOrientation;
	ME[L"RailWidth"] = m_fRailWidth;
	ME[L"RailLength"] = m_fRailLength;
	ME[L"BufferNum"] = m_nBufferNum;
	ME[L"BufferDiameter"] = m_nBufferDiameter;
	ME[L"BufferHeight"] = m_nBufferHeight;
	ME[L"LightingXPos"] = m_fLightingXPos;

	erase(L"Number");
	erase(L"ShaftWidth");
	erase(L"ShaftDepth");
	erase(L"CarDepth");
	erase(L"CarWidth");
	erase(L"CarCeilingHeight");
	erase(L"DoorHeight");
	erase(L"DoorWidth");
	erase(L"DividingBeamWidth");
	erase(L"DividingBeamHeight");

	erase(L"ClearanceCarToShaftWallOnDepth");
	erase(L"ClearanceCarToShaftWallOnWidthLHS");
	erase(L"ClearanceCarToShaftWallOnWidthRHS");
	erase(L"CarDepthWallThickness");
	erase(L"CarFrontReturnDepth");
	erase(L"CarSideWallThickness");
	erase(L"LightingVoidHeight");
	erase(L"CarDoorDepth");
	erase(L"LandingDoorDepth");

	erase(L"Headroom");
	erase(L"HeightAboveCarRoof");

	erase(L"MachineRoomHeight");
	erase(L"PitDepth");
}

void CLiftGroup::SHAFT::Scale(AVFLOAT f)
{
	m_boxShaft.Scale(f);
	m_fWallLtStart *= f;
	m_fWallRtStart *= f;
	m_fBeamLtHeight *= f;
	m_fBeamRtHeight *= f;
	m_boxDoor[0].Scale(f);
	m_boxDoor[1].Scale(f);
	m_boxCar.Scale(f);
	m_boxCarDoor[0].Scale(f);
	m_boxCarDoor[1].Scale(f);
	m_boxCwt.Scale(f);
	m_boxGovernor.Scale(f);
	m_boxLadder.Scale(f);
	m_boxPanelCtrl.Scale(f);
	m_boxPanelDrv.Scale(f);
	m_boxPanelIso.Scale(f);
	m_fRailWidth *= f;
	m_fRailLength *= f;
	m_nBufferDiameter *= f;
	m_nBufferHeight *= f;
	m_fLightingXPos *= f;
}

void CLiftGroup::SHAFT::Reflect()
{
	m_boxShaft.Scale(1, -1, 1);
	m_fWallLtStart *= -1;
	m_fWallRtStart *= -1;
	m_boxDoor[0].Scale(1, -1, 1);
	m_boxDoor[1].Scale(1, -1, 1);
	m_boxCar.Scale(1, -1, 1);
	m_boxCarDoor[0].Scale(1, -1, 1);
	m_boxCarDoor[1].Scale(1, -1, 1);
	m_boxCwt.Scale(1, -1, 1);
	m_boxGovernor.Scale(1, -1, 1);
	m_boxLadder.Scale(1, -1, 1);
//	m_boxPanelCtrl.Scale(1, -1, 1);
//	m_boxPanelDrv.Scale(1, -1, 1);
//	m_boxPanelIso.Scale(1, -1, 1);
}

void CLiftGroup::SHAFT::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	m_boxShaft.Move(x, y, z);
	if (m_fWallLtStart) m_fWallLtStart += y;
	if (m_fWallRtStart) m_fWallRtStart += y;
	m_boxDoor[0].Move(x, y, z);
	m_boxDoor[1].Move(x, y, z);
	m_boxCar.Move(x, y, z);
	m_boxCarDoor[0].Move(x, y, z);
	m_boxCarDoor[1].Move(x, y, z);
	m_boxCwt.Move(x, y, z);
	m_boxGovernor.Move(x, y, z);
	m_boxLadder.Move(x, y, z);
	m_boxPanelCtrl.Move(x, y, z);
	m_boxPanelDrv.Move(x, y, z);
	m_boxPanelIso.Move(x, y, z);
	m_fLightingXPos += x;
}

void CLiftGroup::SHAFT::Create()
{
	m_nId = ME[L"ShaftId"];
	m_type = (TYPE_OF_LIFT)(ULONG)ME[L"LiftTypeId"];
	m_deck = (TYPE_OF_DECK)(ULONG)ME[L"DecksId"];
	m_nDoorType = ME[L"DoorTypeId"];
	m_nLiftType = ME[L"LiftTypeId"];

	m_nOpeningTime = (AVULONG)((float)ME[L"OpeningTime"] * 1000);
	m_nClosingTime = (AVULONG)((float)ME[L"ClosingTime"] * 1000);

	m_nLiftCount = max(1, (AVULONG)ME[L"LiftsPerShaft"]);
	m_nShaftLine = ME[L"ShaftLine"];
	m_boxShaft.ParseFromString(ME[L"BoxShaft"]);
	m_boxDoor[0].ParseFromString(ME[L"BoxDoor0"]);
	m_boxDoor[1].ParseFromString(ME[L"BoxDoor1"]);
	m_boxCar.ParseFromString(ME[L"BoxCar"]);
	m_boxCarDoor[0].ParseFromString(ME[L"BoxCarDoor0"]);
	m_boxCarDoor[1].ParseFromString(ME[L"BoxCarDoor1"]);
	m_boxCwt.ParseFromString(ME[L"BoxCwt"]);
	m_boxGovernor.ParseFromString(ME[L"BoxGovernor"]);
	m_boxLadder.ParseFromString(ME[L"BoxLadder"]);
	m_fWallLtStart = ME[L"LeftWallStart"];
	m_fWallRtStart = ME[L"RightWallStart"];
	m_fBeamLtHeight = ME[L"BeamLtHeight"];
	m_fBeamRtHeight = ME[L"BeamRtHeight"];

	m_boxPanelCtrl.ParseFromString(ME[L"BoxPanelCtrl"]);
	m_boxPanelDrv.ParseFromString(ME[L"BoxPanelDrv"]);
	m_boxPanelIso.ParseFromString(ME[L"BoxPanelIso"]);

	m_fShaftOrientation = ME[L"ShaftOrientation"];
	m_nMachineType = ME[L"MachineType"];
	m_fMachineOrientation = ME[L"MachineOrientation"];
	m_nPanelCtrlType = ME[L"PanelCtrlType"];
	m_nPanelDrvType = ME[L"PanelDrvType"];
	m_nPanelIsoType = ME[L"PanelIsoType"];
	m_fPanelIsoOrientation = ME[L"PanelIsoOrientation"];
	m_fRailWidth = ME[L"RailWidth"];
	m_fRailLength = ME[L"RailLength"];
	m_nBufferNum = ME[L"BufferNum"];
	m_nBufferDiameter = ME[L"BufferDiameter"];
	m_nBufferHeight = ME[L"BufferHeight"];
	m_fLightingXPos = ME[L"LightingXPos"];

	m_strStoreysServed = ME[L"StoreysServed"];
}

