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
#include "BRepIfc.h"
#include "baseIfcElement.h"


//////////////////////////////////////////////////////////////////////////////
//
//
//		ShapeRepresentation
//
//

int buildOpeningBoundInstance(VECTOR3DSTRUCT  *pVector);
void buildOpeningRepresentationInstance(polygon3DStruct *pOpening, int *aggrBounds);
int buildFaceOuterBoundInstance(VECTOR3DSTRUCT *pVector);

int	CIFCElement::buildBrepShapeRepresentationInstance()
{
	if (!bBrep) return 0;

	if (!ifcBrepShapeRepresentationInstance)
	{
		ifcBrepShapeRepresentationInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCSHAPEREPRESENTATION");

		aggrBrepShapeRepresentationItems = sdaiCreateAggrBN(ifcBrepShapeRepresentationInstance, "Items");

		sdaiPutAttrBN(ifcBrepShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(ifcBrepShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Brep");
		sdaiPutAttrBN(ifcBrepShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) IFCGetProject()->getGeometricRepresentationContextInstance());

		appendRepresentation(ifcBrepShapeRepresentationInstance);
	}

	shellStruct *pShell = this->pShell;

    while  (pShell) 
	{
	    int ifcClosedShellInstance = pShell->ifcInstance;
	    int *aggrFaces = pShell->aggr;
		if (!ifcClosedShellInstance)
		{
			ifcClosedShellInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCCLOSEDSHELL");
			aggrFaces = sdaiCreateAggrBN(ifcClosedShellInstance, "CfsFaces");
			pShell->ifcInstance = ifcClosedShellInstance;
			pShell->aggr = aggrFaces; 

			int ifcFacetedBrepInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCFACETEDBREP");
			sdaiPutAttrBN(ifcFacetedBrepInstance, "Outer", sdaiINSTANCE, (void *) ifcClosedShellInstance);
	
			sdaiAppend((int) aggrBrepShapeRepresentationItems, sdaiINSTANCE, (void*) ifcFacetedBrepInstance);
		}

		FACE3DSTRUCT *pFace = pShell->pFace;
        while  (pFace) 
		{

			int ifcFaceInstance = pFace->ifcInstance;
	        int *aggrBounds = pFace->aggr;
			if (!ifcFaceInstance)
			{
				int ifcFaceInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCFACE");
		        int *aggrBounds = sdaiCreateAggrBN(ifcFaceInstance, "Bounds");
				pFace->ifcInstance = ifcFaceInstance;
				pFace->aggr = aggrBounds;

				int ifcFaceOuterBoundInstance = buildFaceOuterBoundInstance(pFace->pVector);
		        sdaiAppend((int) aggrBounds, sdaiINSTANCE, (void *) ifcFaceOuterBoundInstance);

			    sdaiAppend((int) aggrFaces, sdaiINSTANCE, (void *) ifcFaceInstance);
			}

			buildOpeningRepresentationInstance(pFace->pOpenings, aggrBounds);

            pFace = pFace->next;
        }
        pShell = pShell->next;
    }

	return	ifcBrepShapeRepresentationInstance;
}

//////////////////////////////////////////////////////////////////////////////
//
//
//		ShapeRepresentation - Helper Functions
//
//

int buildOpeningBoundInstance(VECTOR3DSTRUCT  *pVector)
{
	int	ifcFaceBoundInstance;
	int ifcPolyLoopInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPOLYLOOP");
	int *aggrPolygon = sdaiCreateAggrBN(ifcPolyLoopInstance, "Polygon");

	while  (pVector) {
		POINT3DSTRUCT   * pPoint = pVector->pPoint;
		//
		//  Check if point is already written
		//
		if  (! pPoint->ifcInstance)
			pPoint->ifcInstance = buildCartesianPointInstance(pPoint->x, pPoint->y, pPoint->z);
	            
		sdaiAppend((int) aggrPolygon, sdaiINSTANCE, (void *) pPoint->ifcInstance);

		pVector = pVector->next;
	}

	ifcFaceBoundInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCFACEBOUND");
	sdaiPutAttrBN(ifcFaceBoundInstance, "Bound", sdaiINSTANCE, (void *) ifcPolyLoopInstance);
	sdaiPutAttrBN(ifcFaceBoundInstance, "Orientation", sdaiENUM, "T");

	return ifcFaceBoundInstance;
}


void buildOpeningRepresentationInstance(polygon3DStruct *pOpening, int *aggrBounds)
{
	while  (pOpening) 
	{
		if (!pOpening->ifcInstance)
		{
			int ifcOpeningBoundInstance = buildOpeningBoundInstance(pOpening->pVector);
			pOpening->ifcInstance = ifcOpeningBoundInstance;
			sdaiAppend((int) aggrBounds, sdaiINSTANCE, (void *) ifcOpeningBoundInstance);
		}
		pOpening = pOpening->next;
	}
}

int buildFaceOuterBoundInstance(VECTOR3DSTRUCT *pVector)
{
	int ifcFaceOuterBoundInstance = 0;
	int ifcPolyLoopInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPOLYLOOP");
	int *aggrPolygon = sdaiCreateAggrBN(ifcPolyLoopInstance, "Polygon");

	while  (pVector) {
		POINT3DSTRUCT   * pPoint = pVector->pPoint;
		//
		//  Check if point is already written
		//
		if  (! pPoint->ifcInstance)
			pPoint->ifcInstance = buildCartesianPointInstance(pPoint->x, pPoint->y, pPoint->z);
	                
		sdaiAppend((int) aggrPolygon, sdaiINSTANCE, (void *) pPoint->ifcInstance);

		pVector = pVector->next;
	}

	ifcFaceOuterBoundInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCFACEOUTERBOUND");
	sdaiPutAttrBN(ifcFaceOuterBoundInstance, "Bound", sdaiINSTANCE, (void *) ifcPolyLoopInstance);
	sdaiPutAttrBN(ifcFaceOuterBoundInstance, "Orientation", sdaiENUM, "T");

	return ifcFaceOuterBoundInstance;
}

