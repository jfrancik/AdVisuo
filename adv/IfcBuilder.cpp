// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "IfcBuilder.h"
#include "IfcProject.h"
#include "ifcscanner.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

	void RotateX(transformationMatrixStruct &A, double a)
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

	extern TCHAR g_pMainPath[MAX_PATH];

	static void IFCPath(LPCTSTR pIFCName, char bufIFCPath[])
	{
		CString path = g_pMainPath;
		int i;
		i = path.ReverseFind('\\');
		if (i >= 0) path = path.Left(i);
		i = path.ReverseFind('\\');
		if (i >= 0) path = path.Left(i);

		path += L"\\ifcfiles\\";
		path += pIFCName;

		WideCharToMultiByte(CP_UTF8, 0, path, -1, bufIFCPath, MAX_PATH, NULL, NULL);
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIfcBuilder

CIfcBuilder::CIfcBuilder(wchar_t *pIFCFile, AVULONG nInstanceIndex)
{
	char pExtFile[MAX_PATH];
	IFCPath(L"IFC2X3_TC1.exp", pExtFile);
	IFCPath(pIFCFile, m_pIFCFile);
	m_revitfile.Open(m_pIFCFile, pExtFile);
	m_revitfile.GetInstance(nInstanceIndex, m_h, m_hRep);
	CIFCModelScanner::GetBB(m_hRep, m_bb);
}

CIfcBuilder::~CIfcBuilder()	
{ 
}

void CIfcBuilder::SaveAsMesh(wchar_t *pMeshFile)
{
	struct VERTEX
	{
		float	x, y, z;
		float	nx, ny, nz;
	};

	int nVertices, nIndices;
	VERTEX *pVertices;
	int *pIndices;
	int startVertex, startIndex, primitiveCount;

	initializeModelling(m_revitfile.m_hModel, &nVertices, &nIndices, 1);
	pVertices = new VERTEX[nVertices];
	pIndices = new int[nIndices];
	finalizeModelling(m_revitfile.m_hModel, (float*)pVertices, pIndices, 0x12);	// 0x12 = D3DFVF_XYZ | D3DFVF_NORMAL
	getInstanceInModelling(m_revitfile.m_hModel, m_h, 2, &startVertex, &startIndex, &primitiveCount);

	// split IFC filename
	wchar_t ifc_path_buffer[_MAX_PATH], ifc_drive[_MAX_DRIVE], ifc_dir[_MAX_DIR], ifc_fname[_MAX_FNAME], ifc_ext[_MAX_EXT];
	MultiByteToWideChar(CP_UTF8, 0, m_pIFCFile, -1, ifc_path_buffer, _MAX_PATH);
	_wsplitpath_s(ifc_path_buffer, ifc_drive, ifc_dir, ifc_fname, ifc_ext);

	// split MESH filename
	if (!pMeshFile) pMeshFile = L"";
	wchar_t mesh_path_buffer[_MAX_PATH], mesh_drive[_MAX_DRIVE], mesh_dir[_MAX_DIR], mesh_fname[_MAX_FNAME], mesh_ext[_MAX_EXT];
	_wsplitpath_s(pMeshFile, mesh_drive, mesh_dir, mesh_fname, mesh_ext);

	if (mesh_drive[0] == L'\0' && mesh_dir[0] == L'\0')
	{
		wcsncpy_s(mesh_drive, ifc_drive, _MAX_DRIVE);
		wcsncpy_s(mesh_dir, ifc_dir, _MAX_DIR);
	}
	if (mesh_fname[0] == L'\0')
	{
		wcsncpy_s(mesh_fname, ifc_fname, _MAX_DIR);
		wcsncpy_s(mesh_ext, L"mesh", _MAX_EXT);
	}

	_wmakepath_s(mesh_path_buffer, mesh_drive, mesh_dir, mesh_fname, mesh_ext);

	CComPtr<IStream> pFileStream;
	HRESULT h = SHCreateStreamOnFile(mesh_path_buffer, STGM_CREATE | STGM_WRITE, &pFileStream); if (FAILED(h)) throw _com_error(h);
	ULONG N;
	pFileStream->Write(&nVertices, sizeof(nVertices), &N);
	pFileStream->Write(&nIndices, sizeof(nIndices), &N);
	pFileStream->Write(&startVertex, sizeof(startVertex), &N);
	pFileStream->Write(&startIndex, sizeof(startIndex), &N);
	pFileStream->Write(&primitiveCount, sizeof(primitiveCount), &N);
	pFileStream->Write(pVertices, nVertices * sizeof(*pVertices), &N);
	pFileStream->Write(pIndices, nIndices * sizeof(*pIndices), &N);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fRot, AVFLOAT fRotX)
{
	build(pElem, nModelId, strName, nIndex, base, 1, 1, 1, fRot, fRotX);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVFLOAT fRotX, bool bIsotropicHeight, bool bIsotropicXY)
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
	else if (bIsotropicXY) dScaleX = dScaleY = min(dScaleX, dScaleY);
	if (h == 0) dScaleZ = bIsotropicHeight ? dScaleX : 1;

	build(pElem, nModelId, strName, nIndex, box.CentreLower(), dScaleX, dScaleY, dScaleZ, fRot, fRotX);
}

void CIfcBuilder::build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fScaleX, AVFLOAT fScaleY, AVFLOAT fScaleZ, AVFLOAT fRot, AVFLOAT fRotX)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateY(matrix, fRotX);
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
	int hRes = machine.build(m_hRep, [dShiftX, dShiftY, dShiftZ, fScaleX, fScaleY, fScaleZ] (CIFCModelScanner::ITEM *pItem) 
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

#define __BOX(x1, x2, y1, y2, z1, z2)	\
		x1, y1, z2,		x2, y1, z2,		x2, y2, z2,		x1, y2, z2,	/* upper */ \
		x1, y1, z1,		x1, y2, z1,		x1, y2, z2,		x1, y1, z2,	/* left  */ \
		x1, y1, z1,		x1, y1, z2,		x2, y1, z2,		x2, y1, z1,	/* front */ \
		x2, y1, z1,		x2, y1, z2,		x2, y2, z2,		x2, y2, z1,	/* right */ \
		x1, y2, z1,		x1, y2, z2,		x2, y2, z2,		x2, y2, z1,	/* rear  */ \
		x1, y1, z1,		x1, y2, z1,		x2, y2, z1,		x2, y1, z1,	/* lower */

void CIfcBuilder::buildSill(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVFLOAT fRotX)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	AVVECTOR base = box.CentreLower();
	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateY(matrix, fRotX);
	RotateZ(matrix, fRot);
	matrix._41 += base.x;
	matrix._42 += -base.y;
	matrix._43 += base.z;

	box -= base;

	double x1 = box.Left(), x2 = box.Right();
	double y1 = max(box.Front(), box.Rear()), y2 = min(box.Front(), box.Rear());
	double z1 = box.Lower(), z2 = box.Upper();
	double w = (z2 - z1) / 20;
	double h = (z2 - z1) * 7;
	double dh = (z2 - z1) * 0.5;
	double dw = (z2 - z1) * 0.5;

	AVULONG faces [] = { 6, 10 };

	double pData[] = 
	{
		// face set 1 (beam)
		__BOX(x1, x2, y1, y2, z1, z2)

		// face set 1 (apron)
		
		x1, y1-w, z1,	// upper
		x1, y1  , z1,
		x2, y1  , z1,
		x2, y1-w, z1,

		x1, y1, z1-h,	// front
		x1, y1, z1,
		x2, y1, z1,
		x2, y1, z1-h,

		x1, y1-w, z1-h,	// rear
		x1, y1-w, z1,
		x2, y1-w, z1,
		x2, y1-w, z1-h,

		x1, y1-w, z1-h,	// left side
		x1, y1,   z1-h,
		x1, y1,   z1,
		x1, y1-w, z1,

		x2, y1-w, z1-h,	// right side
		x2, y1,   z1-h,
		x2, y1,   z1,
		x2, y1-w, z1,

		x1, y1, z1-h,	// front diagonal
		x1, y1-dw, z1-h-dh,
		x2, y1-dw, z1-h-dh,
		x2, y1, z1-h,

		x1, y1-w, z1-h,	// rear diagonal
		x1, y1-dw-w, z1-h-dh,
		x2, y1-dw-w, z1-h-dh,
		x2, y1-w, z1-h,

		x1, y1, z1-h,	// left diagonal
		x1, y1-w, z1-h,
		x1, y1-w-dw, z1-h-dh,
		x1, y1-dw, z1-h-dh,

		x2, y1, z1-h,	// right diagonal
		x2, y1-w, z1-h,
		x2, y1-w-dw, z1-h-dh,
		x2, y1-dw, z1-h-dh,

		x1, y1-dw-w, z1-h-dh,	// lower ending
		x1, y1-dw  , z1-h-dh,
		x2, y1-dw  , z1-h-dh,
		x2, y1-dw-w, z1-h-dh,
	};

	CIFCPointsElem machine(pElem->GetBone(), &matrix);
	machine.setInfo(pName, pName);
	int hRes = machine.build(sizeof(faces) / sizeof(AVULONG), faces, pData);
	if (!hRes) throw Logf(ERROR_IFC_PRJ, L"revit");
}

void CIfcBuilder::buildHandrail(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVFLOAT fRotX)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	AVVECTOR base = box.CentreExtLower();
	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateY(matrix, fRotX);
	RotateZ(matrix, fRot);
	matrix._41 += base.x;
	matrix._42 += -base.y;
	matrix._43 += base.z;

	box -= base;

	double x1 = box.LeftExt(), x2 = box.RightExt();
	double y1 = max(box.FrontExt(), box.RearExt()), y2 = min(box.FrontExt(), box.RearExt());
	double z1 = box.Lower(), z2 = box.Upper();
	double dx = 50, dy = -50, dz = 50;

	AVULONG faces [] = { 6, 6, 6, 6,  6, 6, 6,  6, 6, 6 };

	double pData[] = 
	{
		__BOX(x1, x1+dx, y1, y1+dy, z1, z2)
		__BOX(x2, x2-dx, y1, y1+dy, z1, z2)
		__BOX(x1, x1+dx, y2, y2-dy, z1, z2)
		__BOX(x2, x2-dx, y2, y2-dy, z1, z2)
		
		__BOX(x1, x2, y2-dy, y2, z2-dz, z2)
		__BOX(x1, x1+dx, y1, y2, z2-dz, z2)
		__BOX(x2, x2-dx, y1, y2, z2-dz, z2)

		__BOX(x1, x2, y2-dy, y2, (z1 + z2 - dz) / 2, (z1 + z2 + dz) / 2)
		__BOX(x1, x1+dx, y1, y2, (z1 + z2 - dz) / 2, (z1 + z2 + dz) / 2)
		__BOX(x2, x2-dx, y1, y2, (z1 + z2 - dz) / 2, (z1 + z2 + dz) / 2)
	};

	CIFCPointsElem machine(pElem->GetBone(), &matrix);
	machine.setInfo(pName, pName);
	int hRes = machine.build(sizeof(faces) / sizeof(AVULONG), faces, pData);
	if (!hRes) throw Logf(ERROR_IFC_PRJ, L"revit");
}

#define __RUNG(h)						\
		 229,  4.736843 * 2 - 34, 0.000000 + h,	\
		 229,  4.575439 * 2 - 34, 2.465252 + h,	\
		 229,  4.102226 * 2 - 34, 4.762500 + h,	\
		 229,  3.349454 * 2 - 34, 6.735192 + h,	\
		 229,  2.368422 * 2 - 34, 8.248892 + h,	\
		 229,  1.225985 * 2 - 34, 9.200444 + h,	\
		 229,  0.000000 * 2 - 34, 9.525000 + h,	\
		 229, -1.225985 * 2 - 34, 9.200444 + h,	\
		 229, -2.368421 * 2 - 34, 8.248892 + h,	\
		 229, -3.349454 * 2 - 34, 6.735192 + h,	\
		 229, -4.102226 * 2 - 34, 4.762500 + h,	\
		 229, -4.575439 * 2 - 34, 2.465252 + h,	\
		 229, -4.736843 * 2 - 34, 0.000000 + h,	\
		 229, -4.575439 * 2 - 34, -2.465251 + h,	\
		 229, -4.102226 * 2 - 34, -4.762500 + h,	\
		 229, -3.349454 * 2 - 34, -6.735192 + h,	\
		 229, -2.368421 * 2 - 34, -8.248892 + h,	\
		 229, -1.225985 * 2 - 34, -9.200443 + h,	\
		 229,  0.000000 * 2 - 34, -9.525000 + h,	\
		 229,  1.225985 * 2 - 34, -9.200443 + h,	\
		 229,  2.368422 * 2 - 34, -8.248892 + h,	\
		 229,  3.349454 * 2 - 34, -6.735192 + h,	\
		 229,  4.102226 * 2 - 34, -4.762500 + h,	\
		 229,  4.575439 * 2 - 34, -2.465251 + h,	\
													\
		-229,  4.736843 * 2 - 34, 0.000000 + h,	\
		-229,  4.575439 * 2 - 34, 2.465252 + h,	\
		-229,  4.102226 * 2 - 34, 4.762500 + h,	\
		-229,  3.349454 * 2 - 34, 6.735192 + h,	\
		-229,  2.368422 * 2 - 34, 8.248892 + h,	\
		-229,  1.225985 * 2 - 34, 9.200444 + h,	\
		-229,  0.000000 * 2 - 34, 9.525000 + h,	\
		-229, -1.225985 * 2 - 34, 9.200444 + h,	\
		-229, -2.368421 * 2 - 34, 8.248892 + h,	\
		-229, -3.349454 * 2 - 34, 6.735192 + h,	\
		-229, -4.102226 * 2 - 34, 4.762500 + h,	\
		-229, -4.575439 * 2 - 34, 2.465252 + h,	\
		-229, -4.736843 * 2 - 34, 0.000000 + h,	\
		-229, -4.575439 * 2 - 34, -2.465251 + h,	\
		-229, -4.102226 * 2 - 34, -4.762500 + h,	\
		-229, -3.349454 * 2 - 34, -6.735192 + h,	\
		-229, -2.368421 * 2 - 34, -8.248892 + h,	\
		-229, -1.225985 * 2 - 34, -9.200443 + h,	\
		-229,  0.000000 * 2 - 34, -9.525000 + h,	\
		-229,  1.225985 * 2 - 34, -9.200443 + h,	\
		-229,  2.368422 * 2 - 34, -8.248892 + h,	\
		-229,  3.349454 * 2 - 34, -6.735192 + h,	\
		-229,  4.102226 * 2 - 34, -4.762500 + h,	\
		-229,  4.575439 * 2 - 34, -2.465251 + h,	\
										\
		 229, -4.575439 * 2 - 34, -2.465251 + h,	\
		-229, -4.575439 * 2 - 34, -2.465251 + h,	\
		-229, -4.102226 * 2 - 34, -4.762500 + h,	\
		 229, -4.102226 * 2 - 34, -4.762500 + h,	\
										\
		 229, -4.102226 * 2 - 34, -4.762500 + h,	\
		-229, -4.102226 * 2 - 34, -4.762500 + h,	\
		-229, -3.349454 * 2 - 34, -6.735192 + h,	\
		 229, -3.349454 * 2 - 34, -6.735192 + h,	\
										\
		 229, -2.368421 * 2 - 34, -8.248892 + h,	\
		-229, -2.368421 * 2 - 34, -8.248892 + h,	\
		-229, -1.225985 * 2 - 34, -9.200443 + h,	\
		 229, -1.225985 * 2 - 34, -9.200443 + h,	\
										\
		 229, -3.349454 * 2 - 34, -6.735192 + h,	\
		-229, -3.349454 * 2 - 34, -6.735192 + h,	\
		-229, -2.368421 * 2 - 34, -8.248892 + h,	\
		 229, -2.368421 * 2 - 34, -8.248892 + h,	\
										\
		 229, -1.225985 * 2 - 34, -9.200443 + h,	\
		-229, -1.225985 * 2 - 34, -9.200443 + h,	\
		-229,  0.000000 * 2 - 34, -9.525000 + h,	\
		 229,  0.000000 * 2 - 34, -9.525000 + h,	\
										\
		 229, -4.736843 * 2 - 34, 0.000000 + h,	\
		-229, -4.736843 * 2 - 34, 0.000000 + h,	\
		-229, -4.575439 * 2 - 34, -2.465251 + h,	\
		 229, -4.575439 * 2 - 34, -2.465251 + h,	\
										\
		 229,  1.225985 * 2 - 34, -9.200443 + h,	\
		-229,  1.225985 * 2 - 34, -9.200443 + h,	\
		-229,  2.368422 * 2 - 34, -8.248892 + h,	\
		 229,  2.368422 * 2 - 34, -8.248892 + h,	\
										\
		 229,  2.368422 * 2 - 34, -8.248892 + h,	\
		-229,  2.368422 * 2 - 34, -8.248892 + h,	\
		-229,  3.349454 * 2 - 34, -6.735192 + h,	\
		 229,  3.349454 * 2 - 34, -6.735192 + h,	\
										\
		 229,  4.102226 * 2 - 34, -4.762500 + h,	\
		-229,  4.102226 * 2 - 34, -4.762500 + h,	\
		-229,  4.575439 * 2 - 34, -2.465251 + h,	\
		 229,  4.575439 * 2 - 34, -2.465251 + h,	\
										\
		 229,  3.349454 * 2 - 34, -6.735192 + h,	\
		-229,  3.349454 * 2 - 34, -6.735192 + h,	\
		-229,  4.102226 * 2 - 34, -4.762500 + h,	\
		 229,  4.102226 * 2 - 34, -4.762500 + h,	\
										\
		 229,  4.575439 * 2 - 34, -2.465251 + h,	\
		-229,  4.575439 * 2 - 34, -2.465251 + h,	\
		-229,  4.736843 * 2 - 34, 0.000000 + h,	\
		 229,  4.736843 * 2 - 34, 0.000000 + h,	\
										\
		-229,  1.225985 * 2 - 34, -9.200443 + h,	\
		 229,  1.225985 * 2 - 34, -9.200443 + h,	\
		 229,  0.000000 * 2 - 34, -9.525000 + h,	\
		-229,  0.000000 * 2 - 34, -9.525000 + h,	\
										\
		 229,  4.736843 * 2 - 34, 0.000000 + h,	\
		 -229, 4.736843 * 2 - 34, 0.000000 + h,	\
		 -229, 4.575439 * 2 - 34, 2.465252 + h,	\
		 229,  4.575439 * 2 - 34, 2.465252 + h,	\
										\
		 -229, 2.368422 * 2 - 34, 8.248892 + h,	\
		 -229, 1.225985 * 2 - 34, 9.200444 + h,	\
		 229,  1.225985 * 2 - 34, 9.200444 + h,	\
		 229,  2.368422 * 2 - 34, 8.248892 + h,	\
										\
		 -229, 3.349454 * 2 - 34, 6.735192 + h,	\
		 -229, 2.368422 * 2 - 34, 8.248892 + h,	\
		 229,  2.368422 * 2 - 34, 8.248892 + h,	\
		 229,  3.349454 * 2 - 34, 6.735192 + h,	\
										\
		 229,  1.225985 * 2 - 34, 9.200444 + h,	\
		 -229, 1.225985 * 2 - 34, 9.200444 + h,	\
		 -229, 0.000000 * 2 - 34, 9.525000 + h,	\
		 229,  0.000000 * 2 - 34, 9.525000 + h,	\
										\
		-229,  4.102226 * 2 - 34, 4.762500 + h,	\
		 229,  4.102226 * 2 - 34, 4.762500 + h,	\
		 229,  4.575439 * 2 - 34, 2.465252 + h,	\
		-229,  4.575439 * 2 - 34, 2.465252 + h,	\
										\
		 229,  3.349454 * 2 - 34, 6.735192 + h,	\
		 229,  4.102226 * 2 - 34, 4.762500 + h,	\
		-229,  4.102226 * 2 - 34, 4.762500 + h,	\
		-229,  3.349454 * 2 - 34, 6.735192 + h,	\
										\
		 229, -1.225985 * 2 - 34, 9.200444 + h,	\
		-229, -1.225985 * 2 - 34, 9.200444 + h,	\
		-229, -2.368421 * 2 - 34, 8.248892 + h,	\
		 229, -2.368421 * 2 - 34, 8.248892 + h,	\
										\
		 229, -2.368421 * 2 - 34, 8.248892 + h,	\
		-229, -2.368421 * 2 - 34, 8.248892 + h,	\
		-229, -3.349454 * 2 - 34, 6.735192 + h,	\
		 229, -3.349454 * 2 - 34, 6.735192 + h,	\
										\
		 229, -4.102226 * 2 - 34, 4.762500 + h,	\
		-229, -4.102226 * 2 - 34, 4.762500 + h,	\
		-229, -4.575439 * 2 - 34, 2.465252 + h,	\
		 229, -4.575439 * 2 - 34, 2.465252 + h,	\
										\
		 229, -3.349454 * 2 - 34, 6.735192 + h,	\
		-229, -3.349454 * 2 - 34, 6.735192 + h,	\
		-229, -4.102226 * 2 - 34, 4.762500 + h,	\
		 229, -4.102226 * 2 - 34, 4.762500 + h,	\
										\
		 229, -4.575439 * 2 - 34, 2.465252 + h,	\
		-229, -4.575439 * 2 - 34, 2.465252 + h,	\
		-229, -4.736843 * 2 - 34, 0.000000 + h,	\
		 229, -4.736843 * 2 - 34, 0.000000 + h,	\
										\
		-229, -1.225985 * 2 - 34, 9.200444 + h,	\
		 229, -1.225985 * 2 - 34, 9.200444 + h,	\
		 229,  0.000000 * 2 - 34, 9.525000 + h,	\
		-229,  0.000000 * 2 - 34, 9.525000 + h,


#define __BRACKET(h)			\
		/* left */			\
		-305, 66, 100+h,	\
		-305, 56, 100+h,	\
		-248, 56, 100+h,	\
		-248, -66, 100+h,	\
		-238, -66, 100+h,	\
		-238, 66, 100+h,	\
							\
		-305, 66, 0+h,		\
		-305, 56, 0+h,		\
		-248, 56, 0+h,		\
		-248, -66, 0+h,		\
		-238, -66, 0+h,		\
		-238, 66, 0+h,		\
							\
		-305, 56, 100+h,	\
		-305, 56, 0+h,		\
		-305, 66, 0+h,		\
		-305, 66, 100+h,	\
							\
		-248, 56, 100+h,	\
		-248, 56, 0+h,		\
		-305, 56, 0+h,		\
		-305, 56, 100+h,	\
							\
		-248, -66, 100+h,	\
		-248, -66, 0+h,		\
		-248, 56, 0+h,		\
		-248, 56, 100+h,	\
							\
		-238, -66, 100+h,	\
		-248, -66, 100+h,	\
		-248, -66, 0+h,		\
		-238, -66, 0+h,		\
							\
		-238, 66, 100+h,	\
		-238, -66, 100+h,	\
		-238, -66, 0+h,		\
		-238, 66, 0+h,		\
							\
		-305, 66, 100+h,	\
		-238, 66, 100+h,	\
		-238, 66, 0+h,		\
		-305, 66, 0+h,		\
							\
		/* right */			\
							\
		305, 66, 100+h,		\
		238, 66, 100+h,		\
		238, -66, 100+h,	\
		248, -66, 100+h,	\
		248, 56, 100,		\
		305, 56, 100,		\
							\
		305, 66, 0+h,		\
		238, 66, 0+h,		\
		238, -66, 0+h,		\
		248, -66, 0+h,		\
		248, 56, 0,			\
		305, 56, 0,			\
							\
		238, 66, 100+h,		\
		305, 66, 100+h,		\
		305, 66, 0+h,		\
		238, 66, 0+h,		\
							\
		238, -66, 100+h,	\
		238, -66, 0+h,		\
		238, 66, 0+h,		\
		238, 66, 100+h,		\
							\
		248, -66, 100+h,	\
		238, -66, 100+h,	\
		238, -66, 0+h,		\
		248, -66, 0+h,		\
							\
		248, 56, 100+h,		\
		248, -66, 100+h,	\
		248, -66, 0+h,		\
		248, 56, 0+h,		\
							\
		305, 56, 100+h,		\
		305, 56, 0+h,		\
		248, 56, 0+h,		\
		248, 56, 100+h,		\
							\
		305, 66, 100+h,		\
		305, 56, 100+h,		\
		305, 56, 0+h,		\
		305, 66, 0+h,


void CIfcBuilder::buildLadder(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVULONG nRungs, AVFLOAT fLowerBracket, AVFLOAT fUpperBracket, AVFLOAT fRot, AVFLOAT fRotX)
{
USES_CONVERSION;
	char *pName = OLE2A((LPOLESTR)strName);

	if (!pElem->GetBone()) return;

	if (nRungs == 0) return;	// no ladder if no rungs

	AVVECTOR base = box.CentreExtLower();
	transformationMatrixStruct matrix;
	identityMatrix(&matrix);
	RotateY(matrix, fRotX);
	RotateZ(matrix, fRot);
	matrix._41 += base.x;
	matrix._42 += -base.y;
	matrix._43 += base.z;

	box -= base;

	double x1 = box.LeftExt(), x2 = box.RightExt();
	double y1 = max(box.FrontExt(), box.RearExt()), y2 = min(box.FrontExt(), box.RearExt());
	double z1 = box.Lower(), z2 = box.Upper();
	double dx = 50, dy = -50, dz = 50;

	AVULONG faces [] = { 8, 8, 8, 8, 6, 6, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26 };
	AVULONG points [] = 
	{
		// Brackets
		6, 6, 4, 4, 4, 4, 4, 4,
		6, 6, 4, 4, 4, 4, 4, 4,
		6, 6, 4, 4, 4, 4, 4, 4,
		6, 6, 4, 4, 4, 4, 4, 4,
		// Rails
		4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4,
		// Rungs
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		24, 24, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	};

	double pData [] =
	{

		__BRACKET(fLowerBracket - 50)
		__BRACKET(fUpperBracket - 50)

		__BOX(-238, -228, -66, -3, 0, nRungs * 300 + 300)
		__BOX(228, 238, -66, -3, 0, nRungs * 300 + 300)
					
		__RUNG(300)
		__RUNG(600)
		__RUNG(900)
		__RUNG(1200)
		__RUNG(1500)
		__RUNG(1800)
		__RUNG(2100)
		__RUNG(2400)
		__RUNG(2700)
		__RUNG(3000)
		__RUNG(3300)

	};

//	if (nIndex == 0)
//		for (int i = 0; i < sizeof(pData)/sizeof(double); i += 3)
//			printf("%lf, %lf, %lf\n", pData[i]-125, pData[i+1]+14.210529, pData[i+2]-882.786157);



	CIFCPointsElem machine(pElem->GetBone(), &matrix);
	machine.setInfo(pName, pName);
	int hRes = machine.build(6 + nRungs, faces, pData, points);
	if (!hRes) throw Logf(ERROR_IFC_PRJ, L"revit");






}

