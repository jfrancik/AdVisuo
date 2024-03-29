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


#include "stdafx.h"
#include "baseIfcElement.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCElement

CIFCElement::CIFCElement(CIFCRoot *_pParent, transformationMatrixStruct *pMatrix, bool bBrep, bool bPresentation) : CIFCRoot(_pParent, pMatrix)
{
	aggrRepresentations = NULL;
	ifcBrepShapeRepresentationInstance = 0;
	aggrBrepShapeRepresentationItems = NULL;
	pShell = NULL;
	this->bBrep = bBrep;
	this->bPresentation = bPresentation && bBrep;
}

CShell *CIFCElement::createShellStructureForCuboid(double min_x, double min_y, double min_z, double max_x, double max_y, double max_z)
{
    CPoint3D *pPoints[8];
    CFace3D *pFaces[6];

    pPoints[0] = new CPoint3D(min_x, min_y, min_z);
    pPoints[1] = new CPoint3D(max_x, min_y, min_z);
    pPoints[2] = new CPoint3D(min_x, max_y, min_z);
    pPoints[3] = new CPoint3D(max_x, max_y, min_z);
    pPoints[4] = new CPoint3D(min_x, min_y, max_z);
    pPoints[5] = new CPoint3D(max_x, min_y, max_z);
    pPoints[6] = new CPoint3D(min_x, max_y, max_z);
    pPoints[7] = new CPoint3D(max_x, max_y, max_z);

    pShell = new CShell(
		pFaces[0] = new CFace3D(new CVector3D(pPoints[2], pPoints[6], pPoints[7], pPoints[3]),
		pFaces[1] = new CFace3D(new CVector3D(pPoints[1], pPoints[5], pPoints[4], pPoints[0]),
		pFaces[2] = new CFace3D(new CVector3D(pPoints[0], pPoints[2], pPoints[3], pPoints[1]),
		pFaces[3] = new CFace3D(new CVector3D(pPoints[4], pPoints[5], pPoints[7], pPoints[6]),
		pFaces[4] = new CFace3D(new CVector3D(pPoints[0], pPoints[4], pPoints[6], pPoints[2]),
		pFaces[5] = new CFace3D(new CVector3D(pPoints[3], pPoints[7], pPoints[5], pPoints[1])
		)))))));

	return  pShell;
}

CShell *CIFCElement::appendShellStructureForOpening(double min_y, double max_y, double min_x_opening, double min_z_opening, double max_x_opening, double max_z_opening)
{
	if (!pShell) return NULL;
	CFace3D *pFace1 = (CFace3D*)pShell->pFace;
	if (!pFace1) return NULL;
	CFace3D *pFace2 = (CFace3D*)pFace1->next;
	if (!pFace2) return NULL;

	CPoint3D *pPoints[] =
	{
		new CPoint3D(min_x_opening, min_y, min_z_opening),
		new CPoint3D(max_x_opening, min_y, min_z_opening),
		new CPoint3D(min_x_opening, max_y, min_z_opening),
		new CPoint3D(max_x_opening, max_y, min_z_opening),
		new CPoint3D(min_x_opening, min_y, max_z_opening),
		new CPoint3D(max_x_opening, min_y, max_z_opening),
		new CPoint3D(min_x_opening, max_y, max_z_opening),
		new CPoint3D(max_x_opening, max_y, max_z_opening),
	};

	CFace3D *pNewFaces = 
		new CFace3D(new CVector3D(pPoints[0], pPoints[1], pPoints[3], pPoints[2]),
		new CFace3D(new CVector3D(pPoints[4], pPoints[6], pPoints[7], pPoints[5]),
		new CFace3D(new CVector3D(pPoints[0], pPoints[2], pPoints[6], pPoints[4]),
		new CFace3D(new CVector3D(pPoints[3], pPoints[1], pPoints[5], pPoints[7]),
		(CFace3D*)pFace2->next))));
	pFace2->next = pNewFaces;

	pFace1->AddOpening(new CPolygon3D(new CVector3D(pPoints[2], pPoints[3], pPoints[7], pPoints[6])));
	pFace2->AddOpening(new CPolygon3D(new CVector3D(pPoints[1], pPoints[0], pPoints[4], pPoints[5])));

	return pShell;
}

void CIFCElement::appendRepresentation(CIFCRoot *p)
{
	appendRepresentation(p->getInstance());
}

void CIFCElement::appendRepresentation(int instance)
{
	if (!aggrRepresentations)
	{
		int ifcProductDefinitionShapeInstance = sdaiCreateInstanceBN(getModel(), "IFCPRODUCTDEFINITIONSHAPE");
		aggrRepresentations = sdaiCreateAggrBN(ifcProductDefinitionShapeInstance, "Representations");
		sdaiPutAttrBN(ifcInstance, "Representation", sdaiINSTANCE, (void*) ifcProductDefinitionShapeInstance);
	}
	sdaiAppend((int)aggrRepresentations, sdaiINSTANCE, (void*)instance);
}

// for CIFCElement::buildBrepShapeRepresentationInstance() - check BRepIfc.cpp



//		RelVoidsElement, RelFillsElement

int	CIFCElement::buildRelVoidsElementInstance(int ifcBuildingElementInstance, int ifcOpeningElementInstance)
{
	int ifcRelVoidsElementInstance = sdaiCreateInstanceBN(getModel(), "IFCRELVOIDSELEMENT");
	sdaiPutAttrBN(ifcRelVoidsElementInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcRelVoidsElementInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcRelVoidsElementInstance, "RelatingBuildingElement", sdaiINSTANCE, (void*) ifcBuildingElementInstance);
	sdaiPutAttrBN(ifcRelVoidsElementInstance, "RelatedOpeningElement", sdaiINSTANCE, (void*) ifcOpeningElementInstance);
	return	ifcRelVoidsElementInstance;
}

int CIFCElement::buildRelFillsElementInstance(int ifcOpeningElementInstance, int ifcBuildingElementInstance)
{
	int ifcRelFillsElementInstance = sdaiCreateInstanceBN(getModel(), "IFCRELFILLSELEMENT");
	sdaiPutAttrBN(ifcRelFillsElementInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcRelFillsElementInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcRelFillsElementInstance, "RelatingOpeningElement", sdaiINSTANCE, (void*) ifcOpeningElementInstance);
	sdaiPutAttrBN(ifcRelFillsElementInstance, "RelatedBuildingElement", sdaiINSTANCE, (void*) ifcBuildingElementInstance);
	return	ifcRelFillsElementInstance;
}

//      RelAssociatesMaterial, MaterialLayerSetUsage, MaterialLayerSet, MaterialLayer

int	CIFCElement::buildRelAssociatesMaterial(int ifcBuildingElementInstance, double thickness)
{
	int ifcRelAssociatesMaterialInstance = sdaiCreateInstanceBN(getModel(), "IFCRELASSOCIATESMATERIAL");
	sdaiPutAttrBN(ifcRelAssociatesMaterialInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcRelAssociatesMaterialInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	int *aggrRelatedObjects = sdaiCreateAggrBN(ifcRelAssociatesMaterialInstance, "RelatedObjects");
    sdaiAppend((int) aggrRelatedObjects, sdaiINSTANCE, (void*) ifcBuildingElementInstance);
	sdaiPutAttrBN(ifcRelAssociatesMaterialInstance, "RelatingMaterial", sdaiINSTANCE, (void*) buildMaterialLayerSetUsage(thickness));
	return	ifcRelAssociatesMaterialInstance;
}

int CIFCElement::buildMaterialLayerSetUsage(double thickness)
{
	int ifcMaterialLayerSetUsageInstance = sdaiCreateInstanceBN(getModel(), "IFCMATERIALLAYERSETUSAGE");
	sdaiPutAttrBN(ifcMaterialLayerSetUsageInstance, "ForLayerSet", sdaiINSTANCE, (void*) buildMaterialLayerSet(thickness));
	sdaiPutAttrBN(ifcMaterialLayerSetUsageInstance, "LayerSetDirection", sdaiENUM, "AXIS2");
	sdaiPutAttrBN(ifcMaterialLayerSetUsageInstance, "DirectionSense", sdaiENUM, "POSITIVE");
    double  offsetFromReferenceLine = -thickness/2;
	sdaiPutAttrBN(ifcMaterialLayerSetUsageInstance, "OffsetFromReferenceLine", sdaiREAL, &offsetFromReferenceLine);
    return  ifcMaterialLayerSetUsageInstance;
}

int CIFCElement::buildMaterialLayerSet(double thickness)
{
	int ifcMaterialLayerSetInstance = sdaiCreateInstanceBN(getModel(), "IFCMATERIALLAYERSET");
	int *aggrMaterialLayers = sdaiCreateAggrBN(ifcMaterialLayerSetInstance, "MaterialLayers");
    sdaiAppend((int) aggrMaterialLayers, sdaiINSTANCE, (void*) buildMaterialLayer(thickness));
    return  ifcMaterialLayerSetInstance;
}

int CIFCElement::buildMaterialLayer(double thickness)
{
	int ifcMaterialLayerInstance = sdaiCreateInstanceBN(getModel(), "IFCMATERIALLAYER");
	sdaiPutAttrBN(ifcMaterialLayerInstance, "Material", sdaiINSTANCE, (void*) buildMaterial());
	sdaiPutAttrBN(ifcMaterialLayerInstance, "LayerThickness", sdaiREAL, &thickness);
    return  ifcMaterialLayerInstance;
}

int CIFCElement::buildMaterial()
{
	int ifcMaterialInstance = sdaiCreateInstanceBN(getModel(), "IFCMATERIAL");
	sdaiPutAttrBN(ifcMaterialInstance, "Name", sdaiSTRING, (void*) "Name of the material used for the wall");
    return  ifcMaterialInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCWall

CIFCWall::CIFCWall(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Wall";
	strDescription = "The project default wall";
	setRelNameAndDescription("WallContainer", "WallContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCWall::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), IsBrep() ? "IFCWALL" : "IFCWALLSTANDARDCASE");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
		//appendShellStructureForOpening(0, fThickness, 4200, 0, 5200, 1700);
		//buildBrepShapeRepresentationInstance();
	}
	else
	{
		buildRelAssociatesMaterial(ifcInstance, fThickness);
		createIfcPolylineShape(this, 0, fThickness/2, fWidth, fThickness/2);

		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCSlab

CIFCSlab::CIFCSlab(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Slab";
	strDescription = "The project default slab";
	setRelNameAndDescription("SlabContainer", "SlabContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCSlab::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), "IFCSLAB");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
		//appendShellStructureForOpening(0, wallThickness, 4200, 0, 5200, 1700);
		//buildBrepShapeRepresentationInstance();
	}
	else
	{
		buildRelAssociatesMaterial(ifcInstance, fThickness);
		createIfcPolylineShape(this, 0, fThickness/2, fWidth, fThickness/2);

		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCDoor

CIFCDoor::CIFCDoor(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Door";
	strDescription = "The project default door";
	setRelNameAndDescription("DoorContainer", "DoorContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCDoor::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), IsBrep() ? "IFCDOOR" : "IFCDOORSTANDARDCASE");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
		//appendShellStructureForOpening(0, fThickness, 4200, 0, 5200, 1700);
		//buildBrepShapeRepresentationInstance();
	}
	else
	{
		buildRelAssociatesMaterial(ifcInstance, fThickness);
		createIfcPolylineShape(this, 0, fThickness/2, fWidth, fThickness/2);

		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCBeam

CIFCBeam::CIFCBeam(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Beam";
	strDescription = "The project default beam";
	setRelNameAndDescription("BeamContainer", "BeamContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCBeam::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), IsBrep() ? "IFCBEAM" : "IFCBEAMSTANDARDCASE");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	//if (fWidth < 0)				// USE OF THIS CODE IS UNKNOWN - it behaves very badly with RightWall() function!
	//{
	//	fWidth = -fWidth;
	//	pMatrix->_42 += fWidth;
	//}

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
		//appendShellStructureForOpening(0, fThickness, 4200, 0, 5200, 1700);
		//buildBrepShapeRepresentationInstance();
	}
	else
	{
		buildRelAssociatesMaterial(ifcInstance, fThickness);
		createIfcPolylineShape(this, 0, fThickness/2, fWidth, fThickness/2);

		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCOpening

CIFCOpening::CIFCOpening(CIFCElement *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Opening";
	strDescription = "The project default opening";
	setRelNameAndDescription("OpeningContainer", "OpeningContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCOpening::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), "IFCOPENINGELEMENT");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	buildRelVoidsElementInstance(pParent->getInstance(), ifcInstance);

	if (IsExtruded())
	{
		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	else if (IsPresentation())
	{
		CIFCWall *pWall = (CIFCWall*)getParent();
		pWall->appendShellStructureForOpening(pMatrix->_42, pMatrix->_42 + fThickness, pMatrix->_41, pMatrix->_43, pMatrix->_41 + fWidth, pMatrix->_43 + fHeight);
		pWall->buildBrepShapeRepresentationInstance();
	}
	else //if (IsCoordination())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
	}

	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCWindow

CIFCWindow::CIFCWindow(CIFCOpening *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Window";
	strDescription = "The project default window";
	setRelNameAndDescription("WindowContainer", "WindowContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCWindow::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), "IFCWINDOW");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);
	sdaiPutAttrBN(ifcInstance, "OverallHeight", sdaiREAL, &fHeight);
	sdaiPutAttrBN(ifcInstance, "OverallWidth", sdaiREAL, &fWidth);

	pParent->getParent()->getParent()->appendRelContainedInSpatialStructure(ifcInstance);
	buildRelFillsElementInstance(pParent->getInstance(), ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
	}
	else
	{
		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}

	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCBuildingElementProxy

CIFCBuildingElementProxy::CIFCBuildingElementProxy(CIFCRoot *pParent, transformationMatrixStruct *pMatrix, double width, double height, double thickness,
				   bool bBrep, bool bPresentation)  : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Building Element Proxy";
	strDescription = "The Building Element Proxy";
	setRelNameAndDescription("BuildingElementProxyContainer", "BuildingElementProxyContainer");
	fWidth = width; fHeight = height; fThickness = thickness;
}
	
int CIFCBuildingElementProxy::build()
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), IsBrep() ? "IFCBUILDINGELEMENTPROXY" : "IFCBUILDINGELEMENTPROXYSTANDARDCASE");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	if (IsBrep())
	{
		createShellStructureForCuboid(0, 0, 0, fWidth, fThickness, fHeight);
		buildBrepShapeRepresentationInstance();
		//appendShellStructureForOpening(0, fThickness, 4200, 0, 5200, 1700);
		//buildBrepShapeRepresentationInstance();
	}
	else
	{
		buildRelAssociatesMaterial(ifcInstance, fThickness);
		createIfcPolylineShape(this, 0, fThickness/2, fWidth, fThickness/2);

		CPolygon2D poly2D(0, 0, fWidth, fThickness);
		createIfcExtrudedPolygonShape(this, &poly2D, fHeight);
	}
	return ifcInstance;
}

