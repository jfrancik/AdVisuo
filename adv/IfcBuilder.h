// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/Box.h"
#include "ifcscanner.h"

class CElemIfc;

/////////////////////////////
// Helper functions

	void RotateX(transformationMatrixStruct &A, double a);
	void RotateY(transformationMatrixStruct &A, double a);
	void RotateZ(transformationMatrixStruct &A, double a);


class CIfcBuilder
{
	CRevitFile m_revitfile;
	int m_h, m_hRep;
	CIFCModelScanner::BB m_bb;
	char m_pIFCFile[MAX_PATH];
public:
	CIfcBuilder(wchar_t *pIFCFile, AVULONG nInstanceIndex = 0);
	~CIfcBuilder();

	void SaveAsMesh(wchar_t *pMeshFile);

	double Left()	{ return m_bb.x0; }			double Right()	{ return m_bb.x1; }
	double Front()	{ return m_bb.y0; }			double Rear()	{ return m_bb.y1; }
	double Lower()	{ return m_bb.z0; }			double Upper()	{ return m_bb.z1; }

	double Width()	{ return m_bb.x1 - m_bb.x0; }
	double Depth()	{ return m_bb.y1 - m_bb.y0; }
	double Height()	{ return m_bb.z1 - m_bb.z0; }

	int GetObjectHandle()			{ return m_h; }
	int GetRepresentationHandle()	{ return m_hRep; }

	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVFLOAT fRotX = 0, bool bIsotropicHeight = true, bool bIsotropicXY = true);
	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fScaleX, AVFLOAT fScaleY, AVFLOAT fScaleZ, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);

	static void buildSill(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
	static void buildHandrail(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
	static void buildLadder(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVULONG nRungs, AVFLOAT fLowerBracket, AVFLOAT fUpperBracket, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
	static void buildCombinationBracket(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVULONG bLHS, AVFLOAT fBracketWidth);
};

