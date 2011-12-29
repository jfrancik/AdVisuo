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


class CIFCObject;

void    createIfcBoundingBoxShape(CIFCObject *pObject, double width, double thickness, double height, char * representationIdentifier);


//
//
//		BoundingBox, ShapeRepresentation
//
//


int		buildBoundingBoxInstance(double width, double thickness, double height);
int		buildShapeRepresentationInstance(double width, double thickness, double height, char * representationIdentifier);


