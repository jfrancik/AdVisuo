// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "IfcProject.h"
#include "ifcscanner.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

	// GLOBAL VARIABLES!!!
	bool bBrep = true;
	bool bPresentation = false;

	static void RotateX(transformationMatrixStruct &A, double a)
	{
		double fSin = sin(a), fCos = cos(a), x2, x3;
		x2 = fCos * A._12 - fSin * A._13;  x3 = fSin * A._12 + fCos * A._13;  A._12 = x2; A._13 = x3;
		x2 = fCos * A._22 - fSin * A._23;  x3 = fSin * A._22 + fCos * A._23;  A._22 = x2; A._23 = x3;
		x2 = fCos * A._32 - fSin * A._33;  x3 = fSin * A._32 + fCos * A._33;  A._32 = x2; A._33 = x3;
		x2 = fCos * A._42 - fSin * A._43;  x3 = fSin * A._42 + fCos * A._43;  A._42 = x2; A._43 = x3;
	}
	
	void RotateY(transformationMatrixStruct &A, double a)
	{
		double fSin = sin(a), fCos = cos(a), x1, x3;
		x1 =  fCos * A._11 + fSin * A._13;  x3 = -fSin * A._11 + fCos * A._13;  A._11 = x1; A._13 = x3;
		x1 =  fCos * A._21 + fSin * A._23;  x3 = -fSin * A._21 + fCos * A._23;  A._21 = x1; A._23 = x3;
		x1 =  fCos * A._31 + fSin * A._33;  x3 = -fSin * A._31 + fCos * A._33;  A._31 = x1; A._33 = x3;
		x1 =  fCos * A._41 + fSin * A._43;  x3 = -fSin * A._41 + fCos * A._43;  A._41 = x1; A._43 = x3;
	}

	void RotateZ(transformationMatrixStruct &A, double a)
	{
		double fSin = sin(a), fCos = cos(a), x1, x2;
		x1 = fCos * A._11 - fSin * A._12;  x2 = fSin * A._11 + fCos * A._12;  A._11 = x1; A._12 = x2;
		x1 = fCos * A._21 - fSin * A._22;  x2 = fSin * A._21 + fCos * A._22;  A._21 = x1; A._22 = x2;
		x1 = fCos * A._31 - fSin * A._32;  x2 = fSin * A._31 + fCos * A._32;  A._31 = x1; A._32 = x2;
		x1 = fCos * A._41 - fSin * A._42;  x2 = fSin * A._41 + fCos * A._42;  A._41 = x1; A._42 = x2;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemIfc

CElemIfc::CElemIfc(CProject *pProject, CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) 
	: CElem(pProject, pBuilding, pParent, nElemId, name, i, vec)
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
	case ELEM_PROJECT:	m_pBone = new CIFCProject("IFC2X3_TC1.exp", "MILLI"); break;
	case ELEM_SITE:		m_pBone = new CIFCSite((CIFCProject*)pParentBone, &matrix); break;
	case ELEM_BUILDING:	m_pBone = new CIFCBuilding((CIFCSite*)pParentBone, &matrix); break;
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
	char *pName = OLE2A((LPOLESTR)strName);

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

	case WALL_FLOOR_NUMBER_PLATE:
	case WALL_LIFT_NUMBER_PLATE:
	case WALL_OPENING:		break;
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

void CElemIfc::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot)
{
	if (!GetBone()) return;
	CIfcBuilder *p = NULL;
	AVFLOAT fScale = GetBuilding()->GetScale();
	AVVECTOR centre = box.CentreLower();
	AVULONG i;

	switch (nModelId)
	{

	case MODEL_MACHINE:
		p = new CIfcBuilder("c:\\IFC\\machine40t.ifc");
		if (!p) return;
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_OVERSPEED:
		p = new CIfcBuilder("c:\\IFC\\overspeed.ifc"); 
		if (!p) return;
		box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		centre = box.CentreLower();
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_CONTROL:
		p = new CIfcBuilder("c:\\IFC\\control.ifc"); 
		if (!p) return;
		i = GetBuilding()->GetShaft(nIndex)->GetShaftLine();	// which shaft line we are
		centre.x += p->Width() * (nIndex - GetBuilding()->GetShaftBegin(i) - (AVFLOAT)(GetBuilding()->GetShaftCount(i) - 1) / 2.0f);
		if (i == 0)
			centre.y -= p->Depth()/2;
		else
			centre.y += p->Depth()/2;
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_ISOLATOR:
		p = new CIfcBuilder("c:\\IFC\\isolator.ifc"); 
		if (!p) return;
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_CWT:
		//p = new CIfcBuilder("c:\\IFC\\cwt.ifc"); 
		//if (!p) return;
		//p->build(this, nModelId, strName, nIndex, centre, fRot);
		//delete p;
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_SHAFT, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_RAIL_CAR:
		p = new CIfcBuilder("c:\\IFC\\rail.ifc"); 
		if (!p) return;
		{
			BOX b = box;
			b.Grow(1, 0, 1);
			b.SetRight(b.Left() + 100);
			p->build(this, nModelId, strName, nIndex, b, fRot + M_PI);
			b = box;
			b.Grow(1, 0, 1);
			b.SetLeft(b.Right() - 100);
			p->build(this, nModelId, strName, nIndex, b, fRot);
		}
		delete p;
		break;
	case MODEL_RAIL_CWT:
		p = new CIfcBuilder("c:\\IFC\\rail.ifc"); 
		if (!p) return;
		{
			BOX b = box;
			b.Grow(1, 0, 1);
			b.SetRight(b.Left() - 75);
			p->build(this, nModelId, strName, nIndex, b, fRot + M_PI);
			b = box;
			b.Grow(1, 0, 1);
			b.SetLeft(b.Right() + 75);
			p->build(this, nModelId, strName, nIndex, b, fRot);
		}
		delete p;
		break;
	case MODEL_BUFFER:
		p = new CIfcBuilder("c:\\IFC\\buffer.ifc"); 
		if (!p) return;
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_PULLEY:
		p = new CIfcBuilder("c:\\IFC\\pulley.ifc"); 
		if (!p) return;
		box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		centre = box.CentreLower();
		p->build(this, nModelId, strName, nIndex, centre, fRot);
		delete p;
		break;
	case MODEL_LADDER:
		p = new CIfcBuilder("c:\\IFC\\ladder.ifc"); 
		if (!p) return;
		box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		centre = box.CentreLower();
		p->build(this, nModelId, strName, nIndex, centre, 0.8, 0.8, 1, fRot);
		delete p;
		break;
	}
}

void CElemIfc::Move(AVVECTOR vec)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIfcBuilder

CIfcBuilder::CIfcBuilder(char *pFilename, AVULONG nInstanceIndex)
{
	m_revitfile.Open(pFilename);
	m_h = m_revitfile.GetInstance(nInstanceIndex);
	CIFCModelScanner::GetBB(m_h, m_bb);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fRot)
{
	build(pElem, nModelId, strName, nIndex, base, 1, 1, 1, fRot);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, bool bIsotropic)
{
	double w = box.Width();
	double d = box.Depth();
	double h = box.Height();
	double W = abs(cos(fRot)*w - sin(fRot)*d);
	double D = abs(cos(fRot)*d + sin(fRot)*w);
	double dScaleX = W / Width();
	double dScaleY = D / Depth();
	double dScaleZ = h / Height();

	if (w == 0 && d == 0 && h == 0) return;
	else if (w == 0 && d == 0) dScaleX = dScaleY = dScaleZ;
	else if (w == 0 || d == 0) dScaleX = dScaleY = max(dScaleX, dScaleY);
	else if (bIsotropic) dScaleX = dScaleY = min(dScaleX, dScaleY);
	if (h == 0) dScaleZ = dScaleX;

	build(pElem, nModelId, strName, nIndex, box.CentreLower(), dScaleX, dScaleY, dScaleZ, fRot);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fScaleX, AVFLOAT fScaleY, AVFLOAT fScaleZ, AVFLOAT fRot)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateZ(matrix, fRot);
	matrix._41 += base.x;
	matrix._42 += -base.y;
	matrix._43 += base.z;

	// shift
	double dShiftX = Width() / 4;
	double dShiftY = Depth() / 4;
	double dShiftZ = Lower();

	CIFCRevitElem machine(pElem->GetBone(), &matrix);
	machine.setInfo(pName, pName);
	int hRes = machine.build(m_h, [dShiftX, dShiftY, dShiftZ, fScaleX, fScaleY, fScaleZ] (CIFCModelScanner::ITEM *pItem) 
								{
									if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
									{
										switch (pItem->nIndex)
										{
										case 0: pItem->dvalue = (pItem->dvalue - dShiftX) * fScaleX; break;
										case 1: pItem->dvalue = (pItem->dvalue - dShiftY) * fScaleY; break;
										case 2: pItem->dvalue = (pItem->dvalue - dShiftZ) * fScaleZ; break;
										}
									}
								}); 
	if (!hRes) throw Logf(ERROR_IFC_PRJ, L"revit");
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