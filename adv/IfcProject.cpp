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
	matrix._42 = vec.y;
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
	RotateZ(matrix, vecRot.z);
	RotateX(matrix, vecRot.x);
	RotateY(matrix, vecRot.y);
	matrix._41 += box.Left();
	matrix._42 += box.Front();
	matrix._43 += box.Lower();

	CIFCElement *pElem = NULL;

	switch (nWallId)
	{
	case WALL_FLOOR:
	case WALL_CEILING:
	case WALL_LIFT_FLOOR:
	case WALL_LIFT_CEILING:	pElem = new CIFCSlab(GetBone(), &matrix, box.Width(), box.Height(), -box.Depth(), bBrep, bPresentation); break;

	case WALL_FRONT:
	case WALL_REAR:
	case WALL_SIDE:
	case WALL_SHAFT:
	case WALL_LIFT:			pElem = new CIFCWall(GetBone(), &matrix, box.Width(), box.Height(), -box.Depth(), bBrep, bPresentation); break;

	case WALL_DOOR:
	case WALL_LIFT_DOOR:	pElem = new CIFCWall(GetBone(), &matrix, box.Width(), box.Height(), -box.Depth(), bBrep, bPresentation); break;

	case WALL_BEAM:			pElem = new CIFCBeam(GetBone(), &matrix, box.Width(), box.Height(), -box.Depth(), bBrep, bPresentation); break;

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
		CIFCOpening opening(pElem, &matrix, fW, fH, -box.Depth(), bBrep, bPresentation);
		if (!opening.build()) throw Logf(ERROR_IFC_PRJ, L"opening");
	}

	delete pElem;
}

void CElemIfc::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!GetBone()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateZ(matrix, vecRot.z);
	RotateX(matrix, vecRot.x);
	RotateY(matrix, vecRot.y);
	matrix._41 += box.Left();
	matrix._42 += box.Front();
	matrix._43 += box.Lower();

	CIfcBuilder *p = NULL;

	switch (nModelId)
	{
	case MODEL_MACHINE:	p = new CIfcBuilder("Machine30T_UPSTAND.ifc", 0); break;
	case MODEL_BUFFER:	p = new CIfcBuilder("21April2011AllComponents.ifc", 2); break;
	case MODEL_LIFT:	p = new CIfcBuilder("21April2011AllComponents.ifc", 0); break;
	case MODEL_DOOR_CAR:		p = new CIfcBuilder("21April2011AllComponents.ifc", 4); break;
	case MODEL_DOOR_LANDING:	p = new CIfcBuilder("21April2011AllComponents.ifc", 5); break;
	}

	if (p)
	{
		p->build(this, nModelId, strName, nIndex, box, vecRot.z);
		delete p;
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

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateZ(matrix, fRot);
	matrix._41 += box.Left();
	matrix._42 += box.Front();
	matrix._43 += box.Lower();

	double dScale = box.Width() / (m_bb.x1 - m_bb.x0);

	if (nModelId == CElem::MODEL_BUFFER)
	{
		matrix._41 = box.Left() + 300;
		matrix._42 = box.Front() - 300;
		matrix._43 = box.Lower() + 1800;
		dScale = box.Width() / (m_bb.y1 - m_bb.y0);	// x0 - x1 doesn't provide correct values!
	}

	CIFCRevitElem machine(pElem->GetBone(), &matrix);
	machine.setInfo(pName, pName);
	int h = machine.build(m_h, [dScale] (CIFCModelScanner::ITEM *pItem) 
								{
									if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
										pItem->dvalue *= dScale;
								}); 
	if (!h) throw Logf(ERROR_IFC_PRJ, L"revit");
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