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


#include "baseIfc.h"
#include "BRepIfc.h"
#include "extrudedPolygonIfc.h"


class CIFCElement : public CIFCRoot
{
	bool bBrep, bPresentation;
	int *aggrRepresentations;				// Product Definition Shape Representations aggregate
	int ifcBrepShapeRepresentationInstance;	// 
	int *aggrBrepShapeRepresentationItems;	// Brep Shape Representation Items aggregate
	CShell *pShell;

protected:
	double fWidth, fHeight, fThickness;

public:
	CIFCElement(CIFCRoot *_pParent, transformationMatrixStruct *pMatrix, bool bBrep = true, bool bPresentation = true);

	bool IsBrep()			{ return bBrep; }
	bool IsExtruded()		{ return !bBrep; }
	bool IsPresentation()	{ return bPresentation; }
	bool IsCoordination()	{ return !bPresentation; }

	void setSize(double width, double height, double thickness)	{ fWidth = width; fHeight = height; fThickness = thickness; }

	CShell *createShellStructureForCuboid(double min_x, double min_y, double min_z, double max_x, double max_y, double max_z);
	CShell *appendShellStructureForOpening(double min_y, double max_y, double min_x_opening, double min_z_opening, double max_x_opening, double max_z_opening);

	void appendRepresentation(CIFCRoot *p);
	void appendRepresentation(int instance);

	int	buildBrepShapeRepresentationInstance();

protected:
	// RelVoidsElement, RelFillsElement
	int buildRelVoidsElementInstance(int ifcBuildingElementInstance, int ifcOpeningElementInstance);
	int buildRelFillsElementInstance(int ifcOpeningElementInstance, int ifcBuildingElementInstance);

	// RelAssociatesMaterial, MaterialLayerSetUsage, MaterialLayerSet, MaterialLayer
	int buildRelAssociatesMaterial(int ifcBuildingElementInstance, double thickness);
	int buildMaterialLayerSetUsage(double thickness);
	int buildMaterialLayerSet(double thickness);
	int buildMaterialLayer(double thickness);
	int buildMaterial();
};

class CIFCWall : public CIFCElement
{
public:
	CIFCWall(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);	
	virtual int build();
};

class CIFCSlab : public CIFCElement
{
public:
	CIFCSlab(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	virtual int build();
};

class CIFCDoor : public CIFCElement
{
public:
	CIFCDoor(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);	
	virtual int build();
};

class CIFCBeam : public CIFCElement
{
public:
	CIFCBeam(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	virtual int build();
};

class CIFCOpening : public CIFCElement
{
public:
	CIFCOpening(CIFCElement *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	virtual int build();
};

class CIFCWindow : public CIFCElement
{
public:
	CIFCWindow(CIFCOpening *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	virtual int build();
};

class CIFCBuildingElementProxy : public CIFCElement
{
public:
	CIFCBuildingElementProxy(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	virtual int build();
};

