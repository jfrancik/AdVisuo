// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "IfcProject.h"
#include "ifcscanner.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemIfc

// GLOBAL VARIABLES!!!
bool bBrep = true;
bool bPresentation = false;

int hMachine, hLift, hBuffer, hDoorCar, hDoorLanding;
CIFCModelScanner::BB bbMachine;
CIFCModelScanner::BB bbLift;
CIFCModelScanner::BB bbBuffer;


CElemIfc::~CElemIfc()
{
	if (m_pBone) delete m_pBone;
	m_pBone = NULL;
}

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

std::wstring CElemIfc::onCreateName(AVULONG nElemId, std::wstring name,  AVLONG i)
{
	OLECHAR _name[257]; 
	_snwprintf_s(_name, 256, (AVSTRING)name.c_str(), LOWORD(i), HIWORD(i));
	return _name;
}

void CElemIfc::onCreate(AVULONG nElemId, AVVECTOR &vec)
{
	USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)GetName().c_str());

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	matrix._41 = vec.x;
	matrix._42 = vec.y;
	matrix._43 = vec.z;

	CIFCRoot *pParentNode = GetParent() ? GetParent()->GetNode() : NULL;
	if (nElemId != ELEM_PROJECT && !pParentNode) return;

	switch (nElemId)
	{
	case ELEM_PROJECT:
		{
			CIFCProject *pPrj = new CIFCProject("IFC2X3_TC1.exp", "MILLI");
			pPrj->setInfo(pName, pName);
			if (!pPrj->build()) throw Logf(ERROR_IFC_PRJ, L"project");
			m_pBone = new CBoneIfc(pPrj);
		}
		break;
	case ELEM_SITE:
		{
			CIFCSite *pSite = new CIFCSite((CIFCProject*)pParentNode, &matrix);
			pSite->setInfo(pName, pName);
			if (!pSite->build()) throw Logf(ERROR_IFC_PRJ, L"site");
			m_pBone = new CBoneIfc(pSite);
		}
		break;
	case ELEM_BUILDING:
		{
			CIFCBuilding *pBuilding = new CIFCBuilding((CIFCSite*)pParentNode, &matrix);
			pBuilding->setInfo(pName, pName);
			if (!pBuilding->build()) throw Logf(ERROR_IFC_PRJ, L"building");
			m_pBone = new CBoneIfc(pBuilding);
		}
		break;
	case ELEM_STOREY:
		{
			CIFCStorey *pStorey = new CIFCStorey((CIFCBuilding*)pParentNode, &matrix);
			pStorey->setInfo(pName, pName);
			if (!pStorey->build()) throw Logf(ERROR_IFC_PRJ, L"storey");
			m_pBone = new CBoneIfc(pStorey);
		}
		break;
	case ELEM_SHAFT:
		{
			matrix._41 = 0;
			matrix._42 = 0;
			matrix._43 = 0;
			CIFCSpace *pSpace = new CIFCSpace((CIFCBuilding*)pParentNode, &matrix);
			pSpace->setInfo(pName, pName);
			if (!pSpace->build()) throw Logf(ERROR_IFC_PRJ, L"sub storey space");
			m_pBone = new CBoneIfc(pSpace);
		}
		break;
	case ELEM_EXTRA:
		break;
	case ELEM_LIFT:
		{
			CIFCSpace *pSpace = new CIFCSpace((CIFCBuilding*)pParentNode, &matrix);
			pSpace->setInfo(pName, pName);
			if (!pSpace->build()) throw Logf(ERROR_IFC_PRJ, L"lift space");
			m_pBone = new CBoneIfc(pSpace);
		}
		break;
	}
}

void CElemIfc::onMove(AVVECTOR &vec)
{
}

CBone *CElemIfc::onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)name);

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	matrix._41 = vec.x;
	matrix._42 = vec.y;
	matrix._43 = vec.z;

	CIFCSpace *pSpace = new CIFCSpace((CIFCBuilding*)GetNode(), &matrix);
	pSpace->setInfo(pName, pName);
	if (!pSpace->build()) throw Logf(ERROR_IFC_PRJ, L"lift space");
	return new CBoneIfc(pSpace);
}

void CElemIfc::onAddWall(CBone *pBone_, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone)
{
#define pBone ((CBoneIfc*)pBone_)
USES_CONVERSION;

	if (!pBone || !pBone->GetNode()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateZ(matrix, vecRot.z);
	RotateX(matrix, vecRot.x);
	RotateY(matrix, vecRot.y);
	matrix._41 += vecPos.x;
	matrix._42 += vecPos.y;
	matrix._43 += vecPos.z;

	CIFCElement *pElem = NULL;

	switch (nWallId)
	{
	case WALL_FLOOR:
	case WALL_CEILING:
	case WALL_LIFT_FLOOR:
	case WALL_LIFT_CEILING:			pElem = new CIFCSlab(pBone->GetNode(), &matrix, l, h, -d, bBrep, bPresentation); break;

	case WALL_MACHINE:

		{
		// EXPERIMENTAL
		double dScale = l / (bbMachine.x1 - bbMachine.x0);
		CIFCRevitElem machine(pBone->GetNode(), &matrix);
		machine.setInfo(OLE2A(strName), OLE2A(strName));
		int h = machine.build(hMachine, [dScale] (CIFCModelScanner::ITEM *pItem) 
								{
									if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
										pItem->dvalue *= dScale;
								}); 
		if (!h) throw Logf(ERROR_IFC_PRJ, L"revit");
		return;
		}

	case WALL_BUFFER:

		{
		// EXPERIMENTAL
		identityMatrix(&matrix);
		matrix._41 = vecPos.x + 300;
		matrix._42 = vecPos.y - 300;
		matrix._43 = vecPos.z + 1800;

		double dScale = l / (bbBuffer.y1 - bbBuffer.y0);	// x0 - x1 doesn't provide correct values!
		CIFCRevitElem buffer(pBone->GetNode(), &matrix);
		buffer.setInfo(OLE2A(strName), OLE2A(strName));
		int h = buffer.build(hBuffer, [dScale] (CIFCModelScanner::ITEM *pItem) 
								{
									if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
										pItem->dvalue *= dScale;
								}); 
		if (!h) throw Logf(ERROR_IFC_PRJ, L"revit");
		return;
		}




	case WALL_FRONT:
	case WALL_REAR:
	case WALL_SIDE:
	case WALL_SHAFT:
	case WALL_LIFT:					pElem = new CIFCWall(pBone->GetNode(), &matrix, l, h, -d, bBrep, bPresentation); break;

	case WALL_DOOR:
	case WALL_LIFT_DOOR:			pElem = new CIFCWall(pBone->GetNode(), &matrix, l, h, -d, bBrep, bPresentation); break;

	case WALL_OPENING:				break;

	case WALL_BEAM:					pElem = new CIFCBeam(pBone->GetNode(), &matrix, l, h, -d, bBrep, bPresentation); break;

	case WALL_FLOOR_NUMBER_PLATE:
	case WALL_LIFT_NUMBER_PLATE:	break;
	}
	if (!pElem) return;
	pElem->setInfo(OLE2A(strName), OLE2A(strName));
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
		CIFCOpening opening(pElem, &matrix, fW, fH, -d, bBrep, bPresentation);
		if (!opening.build()) throw Logf(ERROR_IFC_PRJ, L"opening");
	}

	delete pElem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProjectIfc

CProjectIfc::CProjectIfc() : CProjectSrv()
{
}

HRESULT CProjectIfc::SaveAsIFC(LPCOLESTR pFileName, bool bBrep, bool bPresentation)
{
	USES_CONVERSION;
	CIFCProject *pPrj = (CIFCProject*)(GetElement()->GetNode());
	pPrj->save(OLE2A(pFileName));
	return S_OK;
}

void CProjectIfc::Construct()
{
	//////////////////////////////////////////////////////////////////
	// Prepare for creation of REVIT elements
	CRevitFile revit1("Machine30T_UPSTAND.ifc");
	CRevitFile revit2("21April2011AllComponents.ifc");

	hMachine = revit1.GetInstance(0);
	hLift = revit2.GetInstance(0);
	hBuffer = revit2.GetInstance(1);
	hDoorCar = revit2.GetInstance(4);
	hDoorLanding = revit2.GetInstance(5);

	// Bounding boxes
	CIFCModelScanner::GetBB(hMachine, bbMachine);
	CIFCModelScanner::GetBB(hLift, bbLift);
	CIFCModelScanner::GetBB(hBuffer, bbBuffer);

	CProjectSrv::Construct();
}