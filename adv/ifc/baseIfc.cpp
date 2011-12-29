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
#include "baseIfc.h"

#pragma warning(disable:4996)

int model = 0;

CIFCProject *CIFCProject::pDefProject = NULL;

CIFCProject *IFCGetProject()
{
	return CIFCProject::pDefProject;
}

int IFCGetModel()
{
	return CIFCProject::pDefProject ? CIFCProject::pDefProject->getModel() : 0;
}


//////////////////////////////////////////////////////////////////////
//
//
// CIFCInstance Class
//
//

CIFCInstance::CIFCInstance(CIFCInstance *_pParent, transformationMatrixStruct *pMatrix)
{ 
	pParent = _pParent;
	
	pProject = NULL;
	if (pParent) pProject = pParent->getProject(); 
	
	this->pMatrix = pMatrix;

	model = 0;
	if (pProject) model = pProject->getModel();

	ifcInstance = ifcPlacementInstance = 0;
	pifcRelAggregates = pifcRelContainedInSpatialStructure = NULL;
	pchRelName = "UndefinedContainer";
	pchRelDescription = "UndefinedContainer for Unknown Elements";
}

void CIFCInstance::reset()
{ 
	ifcInstance = ifcPlacementInstance = 0; 
	pifcRelAggregates = pifcRelContainedInSpatialStructure = NULL;
}

int *CIFCInstance::appendRelAggregate(CIFCInstance *pRelatedInstance)
{
	if (!pifcRelAggregates)
	{
		int ifcRelAggregatesInstance = sdaiCreateInstanceBN(model, "IFCRELAGGREGATES");
		sdaiPutAttrBN(ifcRelAggregatesInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
		sdaiPutAttrBN(ifcRelAggregatesInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
		sdaiPutAttrBN(ifcRelAggregatesInstance, "Name", sdaiSTRING, pchRelName);
		sdaiPutAttrBN(ifcRelAggregatesInstance, "Description", sdaiSTRING, pchRelDescription);
		sdaiPutAttrBN(ifcRelAggregatesInstance, "RelatingObject", sdaiINSTANCE, (void*) getInstance());
		pifcRelAggregates = sdaiCreateAggrBN(ifcRelAggregatesInstance, "RelatedObjects");
	}
	if (pRelatedInstance)
		sdaiAppend((int) pifcRelAggregates, sdaiINSTANCE, (void*) pRelatedInstance->getInstance());

	return	pifcRelAggregates;
}

int *CIFCInstance::appendRelContainedInSpatialStructure(CIFCInstance *pRelatedInstance)
{
	if (!pifcRelContainedInSpatialStructure)
	{
		int ifcRelContainedInSpatialStructureInstance = sdaiCreateInstanceBN(model, "IFCRELCONTAINEDINSPATIALSTRUCTURE");
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "Name", sdaiSTRING, pchRelName);
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "Description", sdaiSTRING, pchRelDescription);
		pifcRelContainedInSpatialStructure = sdaiCreateAggrBN(ifcRelContainedInSpatialStructureInstance, "RelatedElements");
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "RelatingStructure", sdaiINSTANCE, (void*) getInstance());
	}
	if (pRelatedInstance)
		sdaiAppend((int) pifcRelContainedInSpatialStructure, sdaiINSTANCE, (void *) pRelatedInstance->getInstance());

	return	pifcRelContainedInSpatialStructure;
}

int *CIFCInstance::appendRelContainedInSpatialStructure(int nRelatedInstance)
{
	if (!pifcRelContainedInSpatialStructure)
	{
		int ifcRelContainedInSpatialStructureInstance = sdaiCreateInstanceBN(model, "IFCRELCONTAINEDINSPATIALSTRUCTURE");
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "Name", sdaiSTRING, pchRelName);
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "Description", sdaiSTRING, pchRelDescription);
		pifcRelContainedInSpatialStructure = sdaiCreateAggrBN(ifcRelContainedInSpatialStructureInstance, "RelatedElements");
		sdaiPutAttrBN(ifcRelContainedInSpatialStructureInstance, "RelatingStructure", sdaiINSTANCE, (void*) getInstance());
	}
	if (nRelatedInstance)
		sdaiAppend((int) pifcRelContainedInSpatialStructure, sdaiINSTANCE, (void *) nRelatedInstance);

	return	pifcRelContainedInSpatialStructure;
}

//////////////////////////////////////////////////////////////////////
//
//
// CIFCProject Class
//
//


CIFCProject::CIFCProject(char *ifcSchemaName, char *lengthUnitConversion) : CIFCInstance(NULL, NULL)
{
	reset();
	pParent = NULL;
	pProject = this;
	this->ifcSchemaName = strdup(ifcSchemaName);
	this->lengthUnitConversion = lengthUnitConversion;

	projectName = "Lifts";
	projectDescription = "Lifts Project";
	personGivenName = "Jarek";
	personFamilyName = "Francik";
	orgName = "LBA";
	orgDescription = "Lerch, Bates and Associates Ltd";
	appVersion = "0.9";
	appFullName = "FreeWill+ Lift Visualiser";
	appId = "FW_LIFT_09";
	setRelNameAndDescription("ProjectContainer", "ProjectContainer for Sites");
}

int CIFCProject::build()
{
	// initialise & create model (if necessary)
	reset();
    if (!model) model = sdaiCreateModelBN(1, NULL, ifcSchemaName);
	if (!model) return 0;

	// set the global variables
	pDefProject = this;
	::model = this->model;

	// create instance
	ifcInstance = sdaiCreateInstanceBN(model, "IFCPROJECT");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, projectName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, projectDescription);
	sdaiPutAttrBN(ifcInstance, "UnitsInContext", sdaiINSTANCE, (void*) getUnitAssignmentInstance(lengthUnitConversion));
    int *aggrRepresentationContexts = sdaiCreateAggrBN(ifcInstance, "RepresentationContexts");
	sdaiAppend((int) aggrRepresentationContexts, sdaiINSTANCE, (void*) getGeometricRepresentationContextInstance());

	return	ifcInstance;
}

void CIFCProject::reset()
{
	CIFCInstance::reset();
	ifcBuildOwnerHistoryInstance = 0;
	ifcPersonAndOrganizationInstance = 0;
	ifcPersonInstance = 0;
	ifcOrganizationInstance = 0;
	ifcApplicationInstance = 0;
	ifcUnitAssignmentInstance = 0;
	ifcConversionBasedUnitInstance = 0;
	ifcDimensionalExponentsInstance = 0;
	ifcGeometricRepresentationContextInstance = 0;
}

bool CIFCProject::save(char * ifcFileName)
{
	if	(!model) return false;
	setupHeader(ifcFileName);
	sdaiSaveModelBN(model, ifcFileName);
	return  true;
}

bool CIFCProject::saveAsXml(char * ifcFileName)
{
	if	(!model) return false;
	setupHeader(ifcFileName);
	sdaiSaveModelAsXmlBN(model, ifcFileName);
	return  true;
}

//
//
//		CIFCProject:
//		Application, Organization, Person (OwnerHistory, PersonAndOrganization)
//
//

int CIFCProject::getApplicationInstance()
{
	if	(!ifcApplicationInstance) {
		ifcApplicationInstance = sdaiCreateInstanceBN(model, "IFCAPPLICATION");

		sdaiPutAttrBN(ifcApplicationInstance, "ApplicationDeveloper", sdaiINSTANCE, (void*) getOrganizationInstance());
		sdaiPutAttrBN(ifcApplicationInstance, "Version", sdaiSTRING, appVersion);
		sdaiPutAttrBN(ifcApplicationInstance, "ApplicationFullName", sdaiSTRING, appFullName);
		sdaiPutAttrBN(ifcApplicationInstance, "ApplicationIdentifier", sdaiSTRING, appId);
	}

	return	ifcApplicationInstance;
}

int CIFCProject::getOrganizationInstance()
{
	if	(!ifcOrganizationInstance) {
		ifcOrganizationInstance = sdaiCreateInstanceBN(model, "IFCORGANIZATION");

		sdaiPutAttrBN(ifcOrganizationInstance, "Name", sdaiSTRING, orgName);
		sdaiPutAttrBN(ifcOrganizationInstance, "Description", sdaiSTRING, orgDescription);
	}

	return	ifcOrganizationInstance;
}

int	CIFCProject::getOwnerHistoryInstance()
{
	if	(!ifcBuildOwnerHistoryInstance) {
		ifcBuildOwnerHistoryInstance = sdaiCreateInstanceBN(model, "IFCOWNERHISTORY");

		sdaiPutAttrBN(ifcBuildOwnerHistoryInstance, "OwningUser", sdaiINSTANCE, (void*) getPersonAndOrganizationInstance());
		sdaiPutAttrBN(ifcBuildOwnerHistoryInstance, "OwningApplication", sdaiINSTANCE, (void*) getApplicationInstance());
		sdaiPutAttrBN(ifcBuildOwnerHistoryInstance, "ChangeAction", sdaiENUM, "ADDED");
		sdaiPutAttrBN(ifcBuildOwnerHistoryInstance, "CreationDate", sdaiINTEGER, (void*) getTimeStamp());
	}

	return	ifcBuildOwnerHistoryInstance;
}

int CIFCProject::getPersonAndOrganizationInstance()
{
	if	(!ifcPersonAndOrganizationInstance) {
		ifcPersonAndOrganizationInstance = sdaiCreateInstanceBN(model, "IFCPERSONANDORGANIZATION");

		sdaiPutAttrBN(ifcPersonAndOrganizationInstance, "ThePerson", sdaiINSTANCE, (void*) getPersonInstance());
		sdaiPutAttrBN(ifcPersonAndOrganizationInstance, "TheOrganization", sdaiINSTANCE, (void*) getOrganizationInstance());
	}

	return	ifcPersonAndOrganizationInstance;
}

int	CIFCProject::getPersonInstance()
{
	if	(!ifcPersonInstance) {
		ifcPersonInstance = sdaiCreateInstanceBN(model, "IFCPERSON");

		sdaiPutAttrBN(ifcPersonInstance, "GivenName", sdaiSTRING, personGivenName);
		sdaiPutAttrBN(ifcPersonInstance, "Id", sdaiSTRING, "ID001");
		sdaiPutAttrBN(ifcPersonInstance, "FamilyName", sdaiSTRING, personFamilyName);
	}

	return	ifcPersonInstance;
}

//
//
//		CIFCProject:
//		ConversionBasedUnit, DimensionalExponents, MeasureWithUnit, SIUnit, UnitAssignment
//
//


int CIFCProject::getConversionBasedUnitInstance()
{
	if	(!ifcConversionBasedUnitInstance) {
		ifcConversionBasedUnitInstance = sdaiCreateInstanceBN(model, "IFCCONVERSIONBASEDUNIT");

		sdaiPutAttrBN(ifcConversionBasedUnitInstance, "Dimensions", sdaiINSTANCE, (void*) getDimensionalExponentsInstance());
		sdaiPutAttrBN(ifcConversionBasedUnitInstance, "UnitType", sdaiENUM, "PLANEANGLEUNIT");
		sdaiPutAttrBN(ifcConversionBasedUnitInstance, "Name", sdaiSTRING, "DEGREE");
		sdaiPutAttrBN(ifcConversionBasedUnitInstance, "ConversionFactor", sdaiINSTANCE, (void*) buildMeasureWithUnitInstance());
	}

	return	ifcConversionBasedUnitInstance;
}

int	CIFCProject::getDimensionalExponentsInstance()
{
	int		LengthExponent = 0,
			MassExponent = 0,
			TimeExponent = 0,
			ElectricCurrentExponent = 0,
			ThermodynamicTemperatureExponent = 0,
			AmountOfSubstanceExponent = 0,
			LuminousIntensityExponent = 0;

	if	(!ifcDimensionalExponentsInstance) {
		ifcDimensionalExponentsInstance = sdaiCreateInstanceBN(model, "IFCDIMENSIONALEXPONENTS");

		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "LengthExponent", sdaiINTEGER, &LengthExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "MassExponent", sdaiINTEGER, &MassExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "TimeExponent", sdaiINTEGER, &TimeExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "ElectricCurrentExponent", sdaiINTEGER, &ElectricCurrentExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "ThermodynamicTemperatureExponent", sdaiINTEGER, &ThermodynamicTemperatureExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "AmountOfSubstanceExponent", sdaiINTEGER, &AmountOfSubstanceExponent);
		sdaiPutAttrBN(ifcDimensionalExponentsInstance, "LuminousIntensityExponent", sdaiINTEGER, &LuminousIntensityExponent);
	}

	return	ifcDimensionalExponentsInstance;
}

int	CIFCProject::buildMeasureWithUnitInstance()
{
	int		ifcMeasureWithUnitInstance;
	void	* valueComponentADB;
	double	valueComponent= 0.01745;

	ifcMeasureWithUnitInstance = sdaiCreateInstanceBN(model, "IFCMEASUREWITHUNIT");

	valueComponentADB = sdaiCreateADB(sdaiREAL, &valueComponent);
	sdaiPutADBTypePath(valueComponentADB, 1, "IFCPLANEANGLEMEASURE"); 
	sdaiPutAttrBN(ifcMeasureWithUnitInstance, "ValueComponent", sdaiADB, (void*) valueComponentADB);

	sdaiPutAttrBN(ifcMeasureWithUnitInstance, "UnitComponent", sdaiINSTANCE, (void*) buildSIUnitInstance("PLANEANGLEUNIT", NULL, "RADIAN"));

	return	ifcMeasureWithUnitInstance;
}

int	CIFCProject::buildSIUnitInstance(char * UnitType, char * Prefix, char * Name)
{
	int		ifcSIUnitInstance;

	ifcSIUnitInstance = sdaiCreateInstanceBN(model, "IFCSIUNIT");

	sdaiPutAttrBN(ifcSIUnitInstance, "Dimensions", sdaiINTEGER, NULL);
	sdaiPutAttrBN(ifcSIUnitInstance, "UnitType", sdaiENUM, UnitType);
	if	(Prefix) {
		sdaiPutAttrBN(ifcSIUnitInstance, "Prefix", sdaiENUM, Prefix);
	}
	sdaiPutAttrBN(ifcSIUnitInstance, "Name", sdaiENUM, Name);

	return	ifcSIUnitInstance;
}

int	CIFCProject::getUnitAssignmentInstance(char * lengthUnitConversion)
{
	int		* aggrUnits;

	if	(!ifcUnitAssignmentInstance) {
		ifcUnitAssignmentInstance = sdaiCreateInstanceBN(model, "IFCUNITASSIGNMENT");

		aggrUnits = sdaiCreateAggrBN(ifcUnitAssignmentInstance, "Units");
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("LENGTHUNIT", lengthUnitConversion, "METRE"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("AREAUNIT", NULL, "SQUARE_METRE"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("VOLUMEUNIT", NULL, "CUBIC_METRE"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) getConversionBasedUnitInstance());
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("SOLIDANGLEUNIT", NULL, "STERADIAN"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("MASSUNIT", NULL, "GRAM"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("TIMEUNIT", NULL, "SECOND"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("THERMODYNAMICTEMPERATUREUNIT", NULL, "DEGREE_CELSIUS"));
		sdaiAppend((int) aggrUnits, sdaiINSTANCE, (void*) buildSIUnitInstance("LUMINOUSINTENSITYUNIT", NULL, "LUMEN"));
	}

	return	ifcUnitAssignmentInstance;
}

//
//
//		CIFCProject:
//		WorldCoordinateSystem, GeometricRepresentationContext
//
//


int CIFCProject::getWorldCoordinateSystemInstance()
{
    point3DStruct   point;

    identityPoint(&point);
	int ifcWorldCoordinateSystemInstance = sdaiCreateInstanceBN(model, "IFCAXIS2PLACEMENT3D");
    sdaiPutAttrBN(ifcWorldCoordinateSystemInstance, "Location", sdaiINSTANCE, (void*) buildCartesianPointInstance(&point));

    return  ifcWorldCoordinateSystemInstance;
}

int CIFCProject::getGeometricRepresentationContextInstance()
{
    if  (! ifcGeometricRepresentationContextInstance) {
        double  precision = 0.00001;
        int     coordinateSpaceDimension = 3;

		ifcGeometricRepresentationContextInstance = sdaiCreateInstanceBN(model, "IFCGEOMETRICREPRESENTATIONCONTEXT");

		sdaiPutAttrBN(ifcGeometricRepresentationContextInstance, "ContextType", sdaiSTRING, "Model");
		sdaiPutAttrBN(ifcGeometricRepresentationContextInstance, "CoordinateSpaceDimension", sdaiINTEGER, &coordinateSpaceDimension);
		sdaiPutAttrBN(ifcGeometricRepresentationContextInstance, "Precision", sdaiREAL, &precision);
		sdaiPutAttrBN(ifcGeometricRepresentationContextInstance, "WorldCoordinateSystem", sdaiINSTANCE, (void*) getWorldCoordinateSystemInstance());
    }

    return  ifcGeometricRepresentationContextInstance;
}

void CIFCProject::setupHeader(char *ifcFileName)
{
	int i = 0, j = 0;
	while  (ifcFileName[i])
		if  (ifcFileName[i++] == '\\')
			j = i;

	char timeStamp[512];
	time_t t;
	struct tm *tInfo;
	time (&t);
	tInfo = localtime (&t);
	itoa(1900 + tInfo->tm_year, &timeStamp[0], 10); itoa(100 + 1 + tInfo->tm_mon, &timeStamp[4], 10); itoa(100 + tInfo->tm_mday, &timeStamp[7], 10);
	timeStamp[4] = '-'; timeStamp[7] = '-';
	itoa(100 + tInfo->tm_hour, &timeStamp[10], 10); itoa(100 + tInfo->tm_min, &timeStamp[13], 10); itoa(100 + tInfo->tm_sec, &timeStamp[16], 10);
	timeStamp[10] = 'T'; timeStamp[13] = ':'; timeStamp[16] = ':'; timeStamp[19] = 0;

	char full_name[512];
	strcpy(full_name, personGivenName); strcat(full_name, " "); strcat(full_name, personFamilyName);


	SetSPFFHeader(
		model,
		"Lifts and Lobby layout",			//  description
		"2;1",								//  implementationLevel
		&ifcFileName[j],					//  name
		&timeStamp[0],						//  timeStamp
		full_name,							//  author
		orgName,							//  organization
		"IFC Engine DLL version 1.02 beta",	//  preprocessorVersion
		appFullName,						//  originatingSystem
		full_name,							//  authorization
		"IFC2X3"							//  fileSchema
	);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
//
// CIFCSite Class
//
//

CIFCSite::CIFCSite(CIFCProject *pParent, transformationMatrixStruct *pMatrix)
	: CIFCInstance(pParent, pMatrix)
{
	reset();
	siteName = "Default Site";
	siteDescription = "The project default site";
	refLat_x = 51, refLat_y = 19, refLat_z = 45;
	refLong_x = 0, refLong_y = 31, refLong_z = 58;
	setRelNameAndDescription("SiteContainer", "SiteContainer for Buildings");
}

int CIFCSite::build()
{
	if (!getProject() || !model) return false;

	int		*aggrRefLatitude, *aggrRefLongitude;

	ifcInstance = sdaiCreateInstanceBN(model, "IFCSITE");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, siteName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, siteDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);
	sdaiPutAttrBN(ifcInstance, "CompositionType", sdaiENUM, "ELEMENT");

	aggrRefLatitude = sdaiCreateAggrBN(ifcInstance, "RefLatitude");
	sdaiAppend((int) aggrRefLatitude, sdaiINTEGER, &refLat_x);
	sdaiAppend((int) aggrRefLatitude, sdaiINTEGER, &refLat_y);
	sdaiAppend((int) aggrRefLatitude, sdaiINTEGER, &refLat_z);

	aggrRefLongitude = sdaiCreateAggrBN(ifcInstance, "RefLongitude");
	sdaiAppend((int) aggrRefLongitude, sdaiINTEGER, &refLong_x);
	sdaiAppend((int) aggrRefLongitude, sdaiINTEGER, &refLong_y);
	sdaiAppend((int) aggrRefLongitude, sdaiINTEGER, &refLong_z);

	pParent->appendRelAggregate(this);

	return	ifcInstance;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
//
// CIFCBuilding Class
//
//

CIFCBuilding::CIFCBuilding(CIFCSite *pParent, transformationMatrixStruct *pMatrix)
	: CIFCInstance(pParent, pMatrix)
{
	reset();
	buildingName = "Default Building";
	buildingDescription = "The project default building";
	setRelNameAndDescription("BuildingContainer", "BuildingContainer for Building Stories");
}

int CIFCBuilding::build()
{
	if (!getProject() || !model) return false;

	ifcInstance = sdaiCreateInstanceBN(model, "IFCBUILDING");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, buildingName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, buildingDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*)ifcPlacementInstance);
	sdaiPutAttrBN(ifcInstance, "CompositionType", sdaiENUM, "ELEMENT");

	pParent->appendRelAggregate(this);

	return	ifcInstance;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
//
// CIFCStorey Class
//
//

CIFCStorey::CIFCStorey(CIFCBuilding *pParent, transformationMatrixStruct *pMatrix)
	: CIFCInstance(pParent, pMatrix)
{
	reset();
	storeyName = "Default Storey";
	storeyDescription = "The project default building storey";
	setRelNameAndDescription("StoreyContainer", "StoreyContainer for Building Elements");
}

int CIFCStorey::build()
{
	if (!getProject() || !model) return false;

	double	elevation = 0;

	ifcInstance = sdaiCreateInstanceBN(model, "IFCBUILDINGSTOREY");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, storeyName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, storeyDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*)ifcPlacementInstance);
	sdaiPutAttrBN(ifcInstance, "CompositionType", sdaiENUM, "ELEMENT");
	sdaiPutAttrBN(ifcInstance, "Elevation", sdaiREAL, &elevation);

	pParent->appendRelAggregate(this);

	return	ifcInstance;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
//
// Global Tools
//
//



//
//
//	Matrix
//
//



void identityMatrix(transformationMatrixStruct * pMatrix)
{
	pMatrix->_11 = 1;
	pMatrix->_12 = 0;
	pMatrix->_13 = 0;
	pMatrix->_21 = 0;
	pMatrix->_22 = 1;
	pMatrix->_23 = 0;
	pMatrix->_31 = 0;
	pMatrix->_32 = 0;
	pMatrix->_33 = 1;
	pMatrix->_41 = 0;
	pMatrix->_42 = 0;
	pMatrix->_43 = 0;
}

void identityPoint(point3DStruct * pPoint)
{
	pPoint->x = 0;
	pPoint->y = 0;
	pPoint->z = 0;
}

int	*getTimeStamp()
{
	static int timeStamp = (int)time(0);

	return	&timeStamp;
}


//
//
//	GUID
//
//


static const char *cConversionTable64 =
 "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$";

char * CreateCompressedGuidString()
{
	char	* buf = (char *) malloc(23);
	GUID	guid = GUID_NULL;

	//
	// Call to the function from Microsoft
	//
	CoCreateGuid(&guid);

	if (memcmp(&GUID_NULL, &guid, sizeof (GUID)) == 0) {
		return NULL;
	}

	return getString64FromGuid (&guid, buf);
}

char * getString64FromGuid(const GUID *pGuid, char * buf )
{
    unsigned long   num[6];
    char            str[6][5];
    int             i, n;

    //
    // Creation of six 32 Bit integers from the components of the GUID structure
    //
    num[0] = (unsigned long) (pGuid->Data1 / 16777216);                                                 //    16. byte  (pGuid->Data1 / 16777216) is the same as (pGuid->Data1 >> 24)
    num[1] = (unsigned long) (pGuid->Data1 % 16777216);                                                 // 15-13. bytes (pGuid->Data1 % 16777216) is the same as (pGuid->Data1 & 0xFFFFFF)
    num[2] = (unsigned long) (pGuid->Data2 * 256 + pGuid->Data3 / 256);                                 // 12-10. bytes
    num[3] = (unsigned long) ((pGuid->Data3 % 256) * 65536 + pGuid->Data4[0] * 256 + pGuid->Data4[1]);  // 09-07. bytes
    num[4] = (unsigned long) (pGuid->Data4[2] * 65536 + pGuid->Data4[3] * 256 + pGuid->Data4[4]);       // 06-04. bytes
    num[5] = (unsigned long) (pGuid->Data4[5] * 65536 + pGuid->Data4[6] * 256 + pGuid->Data4[7]);       // 03-01. bytes
    //
    // Conversion of the numbers into a system using a base of 64
    //
    buf[0]='\0';
    n = 3;
    for (i = 0; i < 6; i++) {
        if (!cv_to_64(num[i], str[i], n)) {
            return NULL;
        }
        strcat(buf, str[i]);
        n = 5;
    }
    return buf;
}

BOOL cv_to_64(const unsigned long number, char *code, int len)
{
    unsigned long   act;
    int             iDigit, nDigits;
    char            result[5];

    if (len > 5)
        return FALSE;

    act = number;
    nDigits = len - 1;

    for (iDigit = 0; iDigit < nDigits; iDigit++) {
        result[nDigits - iDigit - 1] = cConversionTable64[(int) (act % 64)];
        act /= 64;
    }
    result[len - 1] = '\0';

    if (act != 0)
        return FALSE;

    strcpy(code, result);
    return TRUE;
}


//
//
//		CartesianPoint, Direction, LocalPlacement (Axis2Placement)
//
//


int		buildAxis2Placement3DInstance(transformationMatrixStruct * pMatrix)
{
	int		ifcAxis2Placement3DInstance;

	ifcAxis2Placement3DInstance = sdaiCreateInstanceBN(model, "IFCAXIS2PLACEMENT3D");

	sdaiPutAttrBN(ifcAxis2Placement3DInstance, "Location", sdaiINSTANCE, (void*) buildCartesianPointInstance((point3DStruct *) &pMatrix->_41));
	sdaiPutAttrBN(ifcAxis2Placement3DInstance, "Axis", sdaiINSTANCE, (void*) buildDirectionInstance((point3DStruct *) &pMatrix->_31));
	sdaiPutAttrBN(ifcAxis2Placement3DInstance, "RefDirection", sdaiINSTANCE, (void*) buildDirectionInstance((point3DStruct *) &pMatrix->_11));

	return	ifcAxis2Placement3DInstance;
}

int		buildCartesianPointInstance(point3DStruct * pPoint)
{
	int		ifcCartesianPointInstance, * aggrCoordinates;

	ifcCartesianPointInstance = sdaiCreateInstanceBN(model, "IFCCARTESIANPOINT");

	aggrCoordinates = sdaiCreateAggrBN(ifcCartesianPointInstance, "Coordinates");
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &pPoint->x);
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &pPoint->y);
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &pPoint->z);

	return	ifcCartesianPointInstance;
}

int	buildCartesianPointInstance(double x, double y, double z)
{
	int		ifcCartesianPointInstance, * aggrCoordinates;

	ifcCartesianPointInstance = sdaiCreateInstanceBN(IFCGetModel(), "IFCCARTESIANPOINT");

	aggrCoordinates = sdaiCreateAggrBN(ifcCartesianPointInstance, "Coordinates");
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &x);
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &y);
	sdaiAppend((int) aggrCoordinates, sdaiREAL, &z);

	return	ifcCartesianPointInstance;
}

int		buildDirectionInstance(point3DStruct * pPoint)
{
	int		ifcDirectionInstance, * aggrDirectionRatios;
	double	_null = 0, _one = 1;

	ifcDirectionInstance = sdaiCreateInstanceBN(model, "IFCDIRECTION");

	aggrDirectionRatios = sdaiCreateAggrBN(ifcDirectionInstance, "DirectionRatios");
	sdaiAppend((int) aggrDirectionRatios, sdaiREAL, &pPoint->x);
	sdaiAppend((int) aggrDirectionRatios, sdaiREAL, &pPoint->y);
	sdaiAppend((int) aggrDirectionRatios, sdaiREAL, &pPoint->z);

	return	ifcDirectionInstance;
}

int		buildLocalPlacementInstance(transformationMatrixStruct * pMatrix, int ifcPlacementRelativeTo)
{
	int		ifcLocalPlacementInstance;

	ifcLocalPlacementInstance = sdaiCreateInstanceBN(model, "IFCLOCALPLACEMENT");

	if	(ifcPlacementRelativeTo) {
		sdaiPutAttrBN(ifcLocalPlacementInstance, "PlacementRelTo", sdaiINSTANCE, (void*) ifcPlacementRelativeTo);
	}
	sdaiPutAttrBN(ifcLocalPlacementInstance, "RelativePlacement", sdaiINSTANCE, (void*) buildAxis2Placement3DInstance(pMatrix));

	return	ifcLocalPlacementInstance;
}
