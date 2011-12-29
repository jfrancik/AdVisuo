////////////////////////////////////////////////////////////////////////
//  Author:  Peter Bonsma
//  Date:    30 July 2008
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
#include "baseIfc.h"

//
//
//		PropertySet, PropertySingleValue
//
//


int		buildPropertySet(char * name, int ** aggrHasProperties)
{
	int		ifcPropertySetInstance;

	ifcPropertySetInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPROPERTYSET");

	sdaiPutAttrBN(ifcPropertySetInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcPropertySetInstance, "OwnerHistory", sdaiINSTANCE, (void*) IFCGetProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcPropertySetInstance, "Name", sdaiSTRING, name);

	(* aggrHasProperties) = sdaiCreateAggrBN(ifcPropertySetInstance, "HasProperties");

	return	ifcPropertySetInstance;
}

int		buildPropertySingleValue(char * name, char * description, bool nominalValue)
{
	int		ifcPropertySingleValueInstance;
    void    * nominalValueADB;
    char    bTrue[2] = "T", bFalse[2] = "F"; 

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

    if  (nominalValue) {
	    nominalValueADB = sdaiCreateADB(sdaiENUM, bTrue);
    } else {
	    nominalValueADB = sdaiCreateADB(sdaiENUM, bFalse);
    }
	sdaiPutADBTypePath(nominalValueADB, 1, "IFCBOOLEAN"); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	return	ifcPropertySingleValueInstance;
}

int		buildPropertySingleValue(char * name, char * description, double nominalValue)
{
	int		ifcPropertySingleValueInstance;
    void    * nominalValueADB;

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

	nominalValueADB = sdaiCreateADB(sdaiREAL, (int *) &nominalValue);
	sdaiPutADBTypePath(nominalValueADB, 1, "IFCREAL"); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	return	ifcPropertySingleValueInstance;
}

int		buildPropertySingleValue(char * name, char * description, char * nominalValue)
{
	int		ifcPropertySingleValueInstance;
    void    * nominalValueADB;

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

	nominalValueADB = sdaiCreateADB(sdaiSTRING, nominalValue);
	sdaiPutADBTypePath(nominalValueADB, 1, "IFCTEXT"); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	return	ifcPropertySingleValueInstance;
}


//
//
//		ElementQuantity, QuantityLength, QuantityArea, QuantityVolume
//
//


int		buildElementQuantity(char * name, int ** aggrQuantities)
{
	int		ifcElementQuantityInstance;

	ifcElementQuantityInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCELEMENTQUANTITY");

	sdaiPutAttrBN(ifcElementQuantityInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcElementQuantityInstance, "OwnerHistory", sdaiINSTANCE, (void*) IFCGetProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcElementQuantityInstance, "Name", sdaiSTRING, name);

	(* aggrQuantities) = sdaiCreateAggrBN(ifcElementQuantityInstance, "Quantities");

	return	ifcElementQuantityInstance;
}

int		buildQuantityLength(char * name, char * description, double length)
{
	int		ifcQuantityLengthInstance;

	ifcQuantityLengthInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCQUANTITYLENGTH");

	sdaiPutAttrBN(ifcQuantityLengthInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityLengthInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityLengthInstance, "LengthValue", sdaiREAL, &length);

	return	ifcQuantityLengthInstance;
}

int		buildQuantityArea(char * name, char * description, double area)
{
	int		ifcQuantityAreaInstance;

	ifcQuantityAreaInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCQUANTITYAREA");

	sdaiPutAttrBN(ifcQuantityAreaInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityAreaInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityAreaInstance, "AreaValue", sdaiREAL, &area);

	return	ifcQuantityAreaInstance;
}

int		buildQuantityVolume(char * name, char * description, double volume)
{
	int		ifcQuantityVolumeInstance;

	ifcQuantityVolumeInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCQUANTITYVOLUME");

	sdaiPutAttrBN(ifcQuantityVolumeInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityVolumeInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityVolumeInstance, "VolumeValue", sdaiREAL, &volume);

	return	ifcQuantityVolumeInstance;
}


//
//
//		Pset_WallCommon, BaseQuantities_Wall, BaseQuantities_WallStandardCase, BaseQuantities_Opening, Pset_WindowCommon, BaseQuantities_Window
//
//


int		buildPset_WallCommon()
{
    int     ifcPropertySetInstance, * aggrHasProperties;
    
    ifcPropertySetInstance = buildPropertySet("Pset_WallCommon", &aggrHasProperties);

	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Reference", "Reference", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("AcousticRating", "AcousticRating", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireRating", "FireRating", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Combustible", "Combustible", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SurfaceSpreadOfFlame", "SurfaceSpreadOfFlame", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ThermalTransmittance", "ThermalTransmittance", 0.24));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("IsExternal", "IsExternal", true));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ExtendToStructure", "ExtendToStructure", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("LoadBearing", "LoadBearing", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Compartmentation", "Compartmentation", false));

	return	ifcPropertySetInstance;
}

int		buildBaseQuantities_Wall(double width, double length, double height, double openingArea, double linearConversionFactor)
{
    int     ifcElementQuantityInstance, * aggrQuantities;

    double  grossSideArea = (length / linearConversionFactor) * (height / linearConversionFactor),
            netSideArea = grossSideArea - openingArea;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Lenght", "Lenght", length));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossSideArea", "GrossSideArea", grossSideArea));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("NetSideArea", "NetSideArea", netSideArea));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("GrossVolume", "GrossVolume", grossSideArea * (width / linearConversionFactor)));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("NetVolume", "NetVolume", netSideArea * (width / linearConversionFactor)));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));

	return	ifcElementQuantityInstance;
}

int		buildBaseQuantities_WallStandardCase(double width, double length, double height, double openingArea, double linearConversionFactor)
{
    int     ifcElementQuantityInstance, * aggrQuantities;

    double  grossSideArea = (length / linearConversionFactor) * (height / linearConversionFactor),
            netSideArea = grossSideArea - openingArea;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Lenght", "Lenght", length));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossSideArea", "GrossSideArea", grossSideArea));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("NetSideArea", "NetSideArea", netSideArea));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("GrossVolume", "GrossVolume", grossSideArea * (width / linearConversionFactor)));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("NetVolume", "NetVolume", netSideArea * (width / linearConversionFactor)));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossFootprintArea", "GrossFootprintArea", (length / linearConversionFactor) * (width / linearConversionFactor)));

	return	ifcElementQuantityInstance;
}

int		buildBaseQuantities_Opening(double depth, double height, double width)
{
    int     ifcElementQuantityInstance, * aggrQuantities;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Depth", "Depth", depth));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));

	return	ifcElementQuantityInstance;
}

int		buildPset_WindowCommon()
{
    int     ifcPropertySetInstance, * aggrHasProperties;
    
    ifcPropertySetInstance = buildPropertySet("Pset_WindowCommon", &aggrHasProperties);

	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Reference", "Reference", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireRating", "FireRating", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("AcousticRating", "AcousticRating", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SecurityRating", "SecurityRating", ""));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("IsExternal", "IsExternal", true));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Infiltration", "Infiltration", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ThermalTransmittance", "ThermalTransmittance", 0.24));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("GlazingAresFraction", "GlazingAresFraction", 0.7));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("HandicapAccessible", "HandicapAccessible", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireExit", "FireExit", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SelfClosing", "SelfClosing", false));
	sdaiAppend((int) aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SmokeStop", "SmokeStop", false));

	return	ifcPropertySetInstance;
}

int		buildBaseQuantities_Window(double height, double width)
{
    int     ifcElementQuantityInstance, * aggrQuantities;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend((int) aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));

	return	ifcElementQuantityInstance;
}


//
//
//      RelDefinesByProperties
//
//


int		buildRelDefinesByProperties(int relatedObject, int relatingPropertyDefinition)
{
	int		ifcRelDefinesByPropertiesInstance, * aggrRelatedObjects;

	ifcRelDefinesByPropertiesInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCRELDEFINESBYPROPERTIES");

	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "OwnerHistory", sdaiINSTANCE, (void*) IFCGetProject()->getOwnerHistoryInstance());

	aggrRelatedObjects = sdaiCreateAggrBN(ifcRelDefinesByPropertiesInstance, "RelatedObjects");
	sdaiAppend((int) aggrRelatedObjects, sdaiINSTANCE, (void *) relatedObject);
	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "RelatingPropertyDefinition", sdaiINSTANCE, (void *) relatingPropertyDefinition);

	return	ifcRelDefinesByPropertiesInstance;
}
