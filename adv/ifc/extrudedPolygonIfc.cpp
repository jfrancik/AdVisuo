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
#include "extrudedPolygonIfc.h"

CPoint2D::CPoint2D(double _x, double _y) 
{ 
	x = _x; 
	y = _y; 
}

CPolygon2D::CPolygon2D(CPoint2D *_pPoint, CPolygon2D *_next)
{
	pPoint = _pPoint;
	next = _next;
}

CPolygon2D::CPolygon2D(double min_x, double min_y, double max_x, double max_y)
{
	pPoint = new CPoint2D(min_x, min_y);
	next = new CPolygon2D(new CPoint2D(min_x, max_y),
		new CPolygon2D(new CPoint2D(max_x, max_y),
		new CPolygon2D(new CPoint2D(max_x, min_y))));
}

CPolygon2D::~CPolygon2D()
{
	delete pPoint;
	if (next) delete next;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void    createIfcExtrudedPolygonShape(CIFCElement *pObject, polygon2DStruct * pPolygon, double depth)
{
	pObject->appendRepresentation(buildShapeRepresentationInstance(pPolygon, depth));
}

void    createIfcPolylineShape(CIFCElement *pObject, double p0x, double p0y, double p1x, double p1y)
{
    pObject->appendRepresentation(buildShapeRepresentationInstance(p0x, p0y, p1x, p1y));
}


//
//
//		ShapeRepresentation
//
//


int		buildShapeRepresentationInstance(polygon2DStruct * pPolygon, double depth)
{
	int		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCSHAPEREPRESENTATION");

	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");
	sdaiAppend((int) aggrItems, sdaiINSTANCE, (void*) buildExtrudedAreaSolidInstance(pPolygon, depth));

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "SweptSolid");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) IFCGetProject()->getGeometricRepresentationContextInstance());

	return	ifcShapeRepresentationInstance;
}

int		buildShapeRepresentationInstance(double p0x, double p0y, double p1x, double p1y)
{
	int		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCSHAPEREPRESENTATION");

	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");
	sdaiAppend((int) aggrItems, sdaiINSTANCE, (void*) buildPolylineInstance(p0x, p0y, p1x, p1y));

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Axis");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Curve2D");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) IFCGetProject()->getGeometricRepresentationContextInstance());

	return	ifcShapeRepresentationInstance;
}


//
//
//		ArbitraryClosedProfileDef, CartesianPoint(2D), ExtrudedAreaSolid, Polyline
//
//


int		buildArbitraryClosedProfileDefInstance(polygon2DStruct * pPolygon)
{
	int		ifcArbitraryClosedProfileDefInstance;

	ifcArbitraryClosedProfileDefInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCARBITRARYCLOSEDPROFILEDEF");

	sdaiPutAttrBN(ifcArbitraryClosedProfileDefInstance, "ProfileType", sdaiENUM, "AREA");
	sdaiPutAttrBN(ifcArbitraryClosedProfileDefInstance, "OuterCurve", sdaiINSTANCE, (void*) buildPolylineInstance(pPolygon));

	return	ifcArbitraryClosedProfileDefInstance;
}

int		buildCartesianPointInstance(double x, double y)
{
	int		ifcCartesianPointInstance, * aggrCoordinates;

	ifcCartesianPointInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCCARTESIANPOINT");

	aggrCoordinates = sdaiCreateAggrBN(ifcCartesianPointInstance, "Coordinates");
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &x);
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &y);

	return	ifcCartesianPointInstance;
}

int		buildExtrudedAreaSolidInstance(polygon2DStruct * pPolygon, double depth)
{
	transformationMatrixStruct  matrix;
	int		ifcExtrudedAreaSolidInstance;

	identityMatrix(&matrix);

	ifcExtrudedAreaSolidInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCEXTRUDEDAREASOLID");

	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "SweptArea", sdaiINSTANCE, (void*) buildArbitraryClosedProfileDefInstance(pPolygon));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "Position", sdaiINSTANCE, (void*) buildAxis2Placement3DInstance(&matrix));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "ExtrudedDirection", sdaiINSTANCE, (void*) buildDirectionInstance((point3DStruct *) &matrix._31));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "Depth", sdaiREAL, (void *) &depth);

	return	ifcExtrudedAreaSolidInstance;
}

int		buildPolylineInstance(polygon2DStruct * pPolygon)
{
	int		ifcPolylineInstance, * aggrPoints;

	ifcPolylineInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPOLYLINE");

	aggrPoints = sdaiCreateAggrBN(ifcPolylineInstance, "Points");
    double  x = pPolygon->pPoint->x,
            y = pPolygon->pPoint->y;
    while  (pPolygon) {
	    sdaiAppend((int) aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(pPolygon->pPoint->x, pPolygon->pPoint->y));

        pPolygon = pPolygon->next;
    }
	sdaiAppend((int) aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(x, y));

	return	ifcPolylineInstance;
}

int		buildPolylineInstance(double p0x, double p0y, double p1x, double p1y)
{
	int		ifcPolylineInstance, * aggrPoints;

	ifcPolylineInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPOLYLINE");

	aggrPoints = sdaiCreateAggrBN(ifcPolylineInstance, "Points");
	sdaiAppend((int) aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(p0x, p0y));
	sdaiAppend((int) aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(p1x, p1y));

	return	ifcPolylineInstance;
}
