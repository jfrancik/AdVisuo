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


class CIFCObject : public CIFCInstance
{
	bool bBrep, bPresentation;
	int *aggrRepresentations;				// Product Definition Shape Representations aggregate
	int ifcBrepShapeRepresentationInstance;	// 
	int *aggrBrepShapeRepresentationItems;	// Brep Shape Representation Items aggregate
	CShell *pShell;

public:
	CIFCObject(CIFCInstance *_pParent, transformationMatrixStruct *pMatrix, bool bBrep = true, bool bPresentation = true);

	bool IsBrep()			{ return bBrep; }
	bool IsExtruded()		{ return !bBrep; }
	bool IsPresentation()	{ return bPresentation; }
	bool IsCoordination()	{ return !bPresentation; }

	CShell *createShellStructureForCuboid(double min_x, double min_y, double min_z, double max_x, double max_y, double max_z);
	CShell *appendShellStructureForOpening(double min_y, double max_y, double min_x_opening, double min_z_opening, double max_x_opening, double max_z_opening);

	void appendRepresentation(CIFCInstance *p);
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

class CIFCWall : public CIFCObject
{
	char *wallName;
	char *wallDescription;
	double wallWidth, wallHeight, wallThickness;

public:
	CIFCWall(CIFCStorey *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	
	virtual int build();

	void setWallInfo(char *name, char *description)		{ wallName = name; wallDescription = description; }
	void setWallSize(double width, double height, double thickness)
														{ wallWidth = width; wallHeight = height; wallThickness = thickness; }
};

class CIFCSlab : public CIFCObject
{
	char *slabName;
	char *slabDescription;
	double slabWidth, slabHeight, slabThickness;

public:
	CIFCSlab(CIFCStorey *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	
	virtual int build();

	void setSlabInfo(char *name, char *description)		{ slabName = name; slabDescription = description; }
	void setSlabSize(double width, double height, double thickness)
														{ slabWidth = width; slabHeight = height; slabThickness = thickness; }
};

class CIFCOpening : public CIFCObject
{
	char *openingName;
	char *openingDescription;
	double openingWidth, openingHeight, openingThickness;

public:
	CIFCOpening(CIFCWall *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	
	virtual int build();

	void setOpeningInfo(char *name, char *description)		{ openingName = name; openingDescription = description; }
	void setOpeningSize(double width, double height, double thickness)
														{ openingWidth = width; openingHeight = height; openingThickness = thickness; }
};

class CIFCWindow : public CIFCObject
{
	char *windowName;
	char *windowDescription;
	double windowWidth, windowHeight, windowThickness;

public:
	CIFCWindow(CIFCOpening *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
					bool bBrep = true, bool bPresentation = true);
	
	virtual int build();

	void setWindowInfo(char *name, char *description)		{ windowName = name; windowDescription = description; }
	void setWindowSize(double width, double height, double thickness)
														{ windowWidth = width; windowHeight = height; windowThickness = thickness; }
};

