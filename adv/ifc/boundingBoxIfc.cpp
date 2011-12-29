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
#include "boundingBoxIfc.h"
#include "baseIfcObject.h"


void    createIfcBoundingBoxShape(CIFCObject *pObject, double width, double thickness, double height, char * representationIdentifier)
{
    pObject->appendRepresentation(buildShapeRepresentationInstance(width, thickness, height, representationIdentifier));
}


//
//
//		BoundingBox, ShapeRepresentation
//
//


int		buildBoundingBoxInstance(double width, double thickness, double height)
{
	int		ifcBoundingBoxInstance;

	ifcBoundingBoxInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCBOUNDINGBOX");

	sdaiPutAttrBN(ifcBoundingBoxInstance, "Corner", sdaiINSTANCE, (void*) buildCartesianPointInstance(0, 0, 0));
	sdaiPutAttrBN(ifcBoundingBoxInstance, "XDim", sdaiREAL, &width);
	sdaiPutAttrBN(ifcBoundingBoxInstance, "YDim", sdaiREAL, &thickness);
	sdaiPutAttrBN(ifcBoundingBoxInstance, "ZDim", sdaiREAL, &height);

	return	ifcBoundingBoxInstance;
}

int		buildShapeRepresentationInstance(double width, double thickness, double height, char * representationIdentifier)
{
	int		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCSHAPEREPRESENTATION");

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) IFCGetProject()->getGeometricRepresentationContextInstance());
	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, representationIdentifier);
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "BoundingBox");
    sdaiAppend((int) aggrItems, sdaiINSTANCE, (void*) buildBoundingBoxInstance(width, thickness, height));

	return	ifcShapeRepresentationInstance;
}


