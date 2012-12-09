// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "IfcProject.h"
#include "IfcBuilder.h"
#include "ifcscanner.h"
#include "IfcLiftGroup.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLiftGroup *CProjectIfc::CreateLiftGroup(AVULONG nIndex)
{ 
	return new CLiftGroupIfc(this, nIndex); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

	// GLOBAL VARIABLES!!!
	bool bBrep = true;
	bool bPresentation = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemIfc

CElemIfc::CElemIfc(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) 
	: CElem(pProject, pLiftGroup, pParent, nElemId, name, i, vec)
{
	USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)GetName().c_str());

	m_pBone = NULL;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	matrix._41 = vec.x;
	matrix._42 = -vec.y;
	matrix._43 = vec.z;

	CIFCRoot *pParentBone = GetParent() ? GetParent()->GetBone() : NULL;
	if (nElemId != ELEM_PROJECT && !pParentBone) return;

	switch (nElemId)
	{
	case ELEM_PROJECT:	m_pBone = new CIFCProject("c:\\ifc\\IFC2X3_TC1.exp", "MILLI"); break;
	case ELEM_SITE:		m_pBone = new CIFCSite((CIFCProject*)pParentBone, &matrix); break;
	case ELEM_LIFTGROUP:m_pBone = new CIFCBuilding((CIFCSite*)pParentBone, &matrix); break;
	case ELEM_STOREY:	m_pBone = new CIFCStorey((CIFCBuilding*)pParentBone, &matrix); break;
	case ELEM_SHAFT:	matrix._41 = matrix._42 = matrix._43 = 0; m_pBone = new CIFCSpace((CIFCBuilding*)pParentBone, &matrix); break;
	case ELEM_DECK:
	case ELEM_LIFT:		m_pBone = new CIFCSpace((CIFCBuilding*)pParentBone, &matrix); break;
	case ELEM_EXTRA:
	case ELEM_BONE:		break;
	}

	if (m_pBone)
	{
		m_pBone->setInfo(pName, pName);
		if (!m_pBone->build()) throw Logf(ERROR_IFC_PRJ, L"project");
	}
}

CElemIfc::~CElemIfc()
{
	if (m_pBone) delete m_pBone; 
}

void CElemIfc::BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData)
{
USES_CONVERSION;
	OLECHAR _name[257];
	_snwprintf_s(_name, 256, strName, nIndex);
	char *pName = OLE2A((LPOLESTR)_name);

	if (!GetBone()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateZ(matrix, -vecRot.z);
	RotateX(matrix, vecRot.x);
	RotateY(matrix, vecRot.y);
	matrix._41 += box.Left();
	matrix._42 += -box.Front();
	matrix._43 += box.Lower();

	CIFCElement *pElem = NULL;

	switch (nWallId)
	{
	case WALL_FLOOR:
	case WALL_CEILING:
	case WALL_LIFT_FLOOR:
	case WALL_LIFT_CEILING:	pElem = new CIFCSlab(GetBone(), &matrix, box.Width(), box.Height(), box.Depth(), bBrep, bPresentation); break;

	case WALL_FRONT:
	case WALL_REAR:
	case WALL_SIDE:
	case WALL_SHAFT:
	case WALL_LIFT:			pElem = new CIFCWall(GetBone(), &matrix, box.Width(), box.Height(), box.Depth(), bBrep, bPresentation); break;

	case WALL_DOOR:
	case WALL_LIFT_DOOR:	pElem = new CIFCDoor(GetBone(), &matrix, box.Width(), box.Height(), box.Depth(), bBrep, bPresentation); break;

	case WALL_BEAM:			pElem = new CIFCBeam(GetBone(), &matrix, box.Width(), box.Height(), box.Depth(), bBrep, bPresentation); break;

	case WALL_OPENING:
	case WALL_CWT:			pElem = new CIFCBuildingElementProxy(GetBone(), &matrix, box.Width(), box.Height(), box.Depth(), bBrep, bPresentation); break;

	case WALL_FLOOR_NUMBER_PLATE:
	case WALL_LIFT_NUMBER_PLATE:
							break;
	}
	if (!pElem) return;
	pElem->setInfo(pName, pName);
	if (!pElem->build()) throw Logf(ERROR_IFC_PRJ, L"wall");

	// openings
	for (AVULONG i = 0; i < nDoorNum; i++)
	{
		FLOAT fPos = *pDoorData++;
		FLOAT fW = *pDoorData++;
		FLOAT fH = *pDoorData++;

		transformationMatrixStruct matrix;
		identityMatrix(&matrix);
		matrix._41 = fPos;
		CIFCOpening opening(pElem, &matrix, fW, fH, box.Depth(), bBrep, bPresentation);
		if (!opening.build()) throw Logf(ERROR_IFC_PRJ, L"opening");
	}

	delete pElem;
}

void CElemIfc::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVULONG nParam, AVFLOAT fParam1, AVFLOAT fParam2)
{
	if (!GetBone()) return;
	CIfcBuilder *p = NULL, *q = NULL;
	AVFLOAT fScale = GetLiftGroup()->GetScale();
	AVVECTOR centre = box.CentreLower();
	AVULONG i;
	bool b;

	switch (nModelId)
	{
	case MODEL_MACHINE:
		switch (nParam)
		{	
		case 1: 
			p = new CIfcBuilder("c:\\IFC\\machine138.ifc"); 
			if (!p) return;
			p->build(this, nModelId, L"Machine - small", 0, centre, fRot);
			delete p;
			break;
		case 2: 
			p = new CIfcBuilder("c:\\IFC\\machine30t.ifc"); 
			if (!p) return;
			p->build(this, nModelId, L"Machine - medium", 0, centre, /*0.25, 0.25, 0.25, */fRot);
			delete p;
			break;
		case 3: 
			p = new CIfcBuilder("c:\\IFC\\machine40t.ifc"); 
			if (!p) return;
			p->build(this, nModelId, L"Machine - large", 0, centre, fRot);
			delete p;
			break;
		case 4: 
			p = new CIfcBuilder("c:\\IFC\\machine70t.ifc"); 
			if (!p) return;
			p->build(this, nModelId, L"Machine - extra large", 0, centre, fRot);
			delete p;
			break;
		}
		break;
	case MODEL_OVERSPEED:
		p = new CIfcBuilder("c:\\IFC\\overspeed.ifc"); 
		if (!p) return;
		centre = box.CentreLower();
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_CONTROL_PANEL:
		p = new CIfcBuilder("c:\\IFC\\control.ifc");	// control panel
		q = new CIfcBuilder("c:\\IFC\\control.ifc");	// drive panel (missing file, control.ifc taken instead)
		if (!p || !q) return;
		i = GetLiftGroup()->GetShaft(nIndex)->GetShaftLine();	// which shaft line we are
		centre.x += (p->Width() + q->Width()) * (nIndex - GetLiftGroup()->GetShaftBegin(i) - (AVFLOAT)(GetLiftGroup()->GetShaftCount(i) - 1) / 2.0f);
		if (i == 0)
			centre.y -= p->Depth()/2;
		else
			centre.y += p->Depth()/2;
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		delete q;
		break;
	case MODEL_DRIVE_PANEL:
		p = new CIfcBuilder("c:\\IFC\\control.ifc");	// control panel
		q = new CIfcBuilder("c:\\IFC\\control.ifc");	// drive panel (missing file, control.ifc taken instead)
		if (!p || !q) return;
		i = GetLiftGroup()->GetShaft(nIndex)->GetShaftLine();	// which shaft line we are
		centre.x += (p->Width() + q->Width()) * (nIndex - GetLiftGroup()->GetShaftBegin(i) - (AVFLOAT)(GetLiftGroup()->GetShaftCount(i) - 1) / 2.0f) + p->Width();
		if (i == 0)
			centre.y -= p->Depth()/2;
		else
			centre.y += p->Depth()/2;
		q->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		delete q;
		break;
	case MODEL_ISOLATOR:
		p = new CIfcBuilder("c:\\IFC\\isolator.ifc"); 
		if (!p) return;
		centre.x = box.Right() - p->Depth() + 35 * fScale;
		centre.y = box.Rear() - p->Width() / 2 - fScale * 1950 - (nIndex / 2) * p->Width();
		centre.z = 1000 * fScale + (nIndex % 2) * p->Height();
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_CWT:
		//p = new CIfcBuilder("c:\\IFC\\cwt.ifc"); 
		//if (!p) return;
		//p->build(this, nModelId, strName, nIndex, centre, fRot);
		//delete p;
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_CWT, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_RAIL:
		p = new CIfcBuilder("c:\\IFC\\rail.ifc"); 
		if (!p) return;
		p->build(this, nModelId, strName, nIndex, box, fRot);
		delete p;
		break;
	case MODEL_BUFFER_CAR:
		p = new CIfcBuilder("c:\\IFC\\buffer.ifc"); 
		if (!p) return;
		box = BOX(box.CentreX() - fParam1/2, box.CentreY() - fParam1/2, 0, fParam1, fParam1, fParam2);
		if (nParam == 1)
			p->build(this, nModelId, strName, nIndex, box, fRot);
		else
		{
			box.Move(-300, 0, 0);
			p->build(this, nModelId, strName, nIndex, box, fRot);
			box.Move(600, 0, 0);
			p->build(this, nModelId, strName, nIndex, box, fRot);
		}
		delete p;
		break;
	case MODEL_BUFFER_CWT:
		b = abs(box.Width()) > abs(box.Depth());
		p = new CIfcBuilder("c:\\IFC\\buffer.ifc"); 
		if (!p) return;
		box = BOX(box.CentreX() - fParam1/2, box.CentreY() - fParam1/2, 0, fParam1, fParam1, fParam2);
		if (nParam == 1)
			p->build(this, nModelId, strName, nIndex, box, fRot);
		else
		{
			if (b) box.Move(-300, 0, 0); else box.Move(0, -300, 0);
			p->build(this, nModelId, strName, nIndex, box, fRot);
			if (b) box.Move(600, 0, 0); else box.Move(0, 600, 0);
			p->build(this, nModelId, strName, nIndex, box, fRot);
		}
		delete p;
		break;
	case MODEL_PULLEY:
		p = new CIfcBuilder("c:\\IFC\\pulley.ifc"); 
		if (!p) return;
		box.Move(0, 0, 500 * fScale);
		centre = box.CentreLower();
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_LIGHT:
		p = new CIfcBuilder("c:\\IFC\\light.ifc"); 
		if (!p) return;
		centre.y = box.Rear() + ((fRot > 0) ? p->Height() : -p->Height());

		if (nParam == 1 && fParam1 < 500 * fScale + p->Width()) { delete p; return; }	// the pit is too shallow
		if (nParam == 2 && fParam2 - fParam1 < 500 * fScale + p->Width()) { delete p; return; }	// the headroom is too shallow
		centre.z = 500 * fScale + p->Width() / 2;
		if (nParam == 2) centre.z = fParam2 - 500 * fScale - p->Width() / 2;	// special location for the headroom light
		p->build(this, nModelId, strName, nIndex, centre, fRot, (AVFLOAT)M_PI/2);
		delete p;
		break;


	case MODEL_LADDER:
		CIfcBuilder::buildLadder(this, nModelId, strName, nIndex, box, nParam, fParam1, fParam2, fRot);
		break;
	case MODEL_JAMB:
	case MODEL_JAMB_CAR:
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_HEADING:
	case MODEL_HEADING_CAR:
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_SILL:
		CIfcBuilder::buildSill(this, nModelId, strName, nIndex, box, fRot);
		break;
	case MODEL_SILL_CAR:
		CIfcBuilder::buildSill(this, nModelId, strName, nIndex, box, fRot);
		break;
	case MODEL_HANDRAIL:
		CIfcBuilder::buildHandrail(this, nModelId, strName, nIndex, box, fRot);
		break;
	}
}

void CElemIfc::Move(AVVECTOR vec)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProjectIfc

CProjectIfc::CProjectIfc() : CProjectSrv()
{
}

HRESULT CProjectIfc::SaveAsIFC(LPCOLESTR pFileName, bool bBrep, bool bPresentation)
{
	USES_CONVERSION;
	CIFCProject *pPrj = (CIFCProject*)(GetElement()->GetBone());
	pPrj->save(OLE2A(pFileName));
	return S_OK;
}

void CProjectIfc::Construct()
{
	CProjectSrv::Construct();
}