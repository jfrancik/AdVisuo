////////////////////////////////////////////////////////////////////////
//  Author:  Peter Bonsma
//  Date:    11 July 2008
//  Project: IFC Engine Series (example using DLL)
//
//  This code may be used and edited freely,
//  also for commercial projects in open and closed source software
//
//  In case of use of the DLL:
//  be aware of license fee for use of this DLL when used commercially
//  more info for commercial use:  peter.bonsma@tno.nl
//
//  more info for using the IFC Engine DLL in other languages: 
//  see other examples or contact:  pim.vandenhelm@tno.nl
////////////////////////////////////////////////////////////////////////


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "baseIFC.h"

typedef struct VECTOR3DSTRUCT {
	POINT3DSTRUCT   * pPoint;
	VECTOR3DSTRUCT  * next;
}	vector3DStruct;

typedef struct POLYGON3DSTRUCT {
	VECTOR3DSTRUCT  * pVector;
	int ifcInstance;
	POLYGON3DSTRUCT * next;
}	polygon3DStruct;

typedef struct FACE3DSTRUCT {
	VECTOR3DSTRUCT  * pVector;
	POLYGON3DSTRUCT * pOpenings;
	int ifcInstance; int *aggr;
	FACE3DSTRUCT * next;
}	face3DStruct;

typedef struct SHELLSTRUCT {
	FACE3DSTRUCT * pFace;
	int ifcInstance; int *aggr;
	SHELLSTRUCT     * next;
}	shellStruct;



struct CPoint3D : public point3DStruct
{
	CPoint3D(double _x, double _y, double _z)
	{ 
		x = _x; y = _y; z = _z; 
		ifcInstance = 0; 
	}
};

struct CVector3D : public vector3DStruct
{
	CVector3D(CPoint3D *_pPoint, CVector3D *_next = NULL)
	{
		pPoint = _pPoint;
		next = _next;
	}

	CVector3D(CPoint3D *p1, CPoint3D *p2, CPoint3D *p3, CPoint3D *p4, CVector3D *_next = NULL)
	{
		pPoint = p1;
		next = new CVector3D(p2, new CVector3D(p3, new CVector3D(p4)));
	}
};

struct CPolygon3D : public polygon3DStruct
{
	CPolygon3D(CVector3D *_pVector, CPolygon3D *_next = NULL)
	{
		pVector = _pVector;
		ifcInstance = 0;
		next = _next;
	}
};

struct CFace3D : public face3DStruct
{
	CFace3D::CFace3D(CVector3D *_pVector, CPolygon3D *_pOpenings, CFace3D *_next = NULL)
	{
		pVector = _pVector;
		pOpenings = _pOpenings;
		ifcInstance = 0; aggr = NULL;
		next = _next;
	}

	CFace3D::CFace3D(CVector3D *_pVector, CFace3D *_next = NULL)
	{
		pVector = _pVector;
		pOpenings = NULL;
		ifcInstance = 0; aggr = NULL;
		next = _next;
	}

	void CFace3D::AddOpening(CPolygon3D *_pOpenings)
	{
		_pOpenings->next = pOpenings;
		pOpenings = _pOpenings;
	}
};

struct CShell : public shellStruct
{
	CShell(CFace3D *_pFace, CShell *_next = NULL)
	{
		pFace = _pFace;
		ifcInstance = 0; aggr = NULL;
		next = _next;
	}
};


// IMPLEMENTATION:
// partial class CIFCObject:
// int	CIFCObject::buildBrepShapeRepresentationInstance()
// See baseIfcObject.h for details

