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

#include "IFCEngine.h"

class CIFCProject;
class CIFCSite;
class CIFCBuilding;
class CIFCStorey;

CIFCProject *IFCGetProject();
int IFCGetModel();

typedef struct POINT3DSTRUCT {
	double			x, y, z;
    int             ifcInstance;
}	point3DStruct;

typedef struct TRANSFORMATIONMATRIXSTRUCT {
	double			_11, _12, _13;
	double			_21, _22, _23;
	double			_31, _32, _33;
	double			_41, _42, _43;
}	transformationMatrixStruct;

class CIFCInstance
{
protected:
	CIFCInstance *pParent;
	CIFCProject *pProject;
	int model;
	transformationMatrixStruct *pMatrix;
	int ifcInstance;
	int ifcPlacementInstance;
	int *pifcRelAggregates;
	int *pifcRelContainedInSpatialStructure;
	char *pchRelName, *pchRelDescription;

public:
	CIFCInstance(CIFCInstance *pParent, transformationMatrixStruct *pMatrix);

	virtual int build() = 0;
	virtual void reset();

	CIFCInstance *getParent()						{ return pParent; }
	CIFCProject *getProject()						{ return pProject; }
	int getModel()									{ return model; }
	transformationMatrixStruct *getMatrix()			{ return pMatrix; }
	void setMatrix(transformationMatrixStruct *p)	{ pMatrix = p; }
	int getInstance()			{ return ifcInstance ? ifcInstance : build(); }
	int getPlacementInstance()	{ return ifcInstance ? ifcPlacementInstance : (build(), ifcPlacementInstance); }

	// RelAggregates, RelContainedInSpatialStructure
	void setRelNameAndDescription(char *name, char *description)	{ pchRelName = name; pchRelDescription = description; }
	int *appendRelAggregate(CIFCInstance *pRelatedInstance);
	int *appendRelContainedInSpatialStructure(CIFCInstance *pRelatedInstance);
	int *appendRelContainedInSpatialStructure(int nRelatedInstance);
};

class CIFCProject : public CIFCInstance
{
	// various instances
	int	ifcProjectInstance;
	int ifcApplicationInstance;
	int ifcOrganizationInstance;
	int ifcBuildOwnerHistoryInstance;
	int ifcPersonAndOrganizationInstance;
	int ifcPersonInstance;
	
	int ifcConversionBasedUnitInstance;
	int ifcDimensionalExponentsInstance;
	int ifcUnitAssignmentInstance;
	int ifcGeometricRepresentationContextInstance;

	// constructor params
	char *ifcSchemaName;
	char *lengthUnitConversion;

	// attributes
	char *projectName;
	char *projectDescription;
	char *personGivenName;
	char *personFamilyName;
	char *orgName;
	char *orgDescription;
	char *appVersion;
	char *appFullName;
	char *appId;

public:
	static CIFCProject *pDefProject;

public:
//	CIFCProject(char *ifcSchemaName = "IFC2X3_TC1.exp", char *lengthUnitConversion = "MILLI");
	CIFCProject(char *ifcSchemaName, char *lengthUnitConversion);
	
	virtual int build();
	virtual void reset();

	bool save(char *ifcFileName);
	bool saveAsXml(char *ifcFileName);

	// Attributes
	void setProjectInfo(char *name, char *description)		{ projectName = name; projectDescription = description; }
	void setPeronName(char *given, char *family)			{ personGivenName = given; personFamilyName = family; }
	void SetOrganisationInfo(char *name, char *description)	{ orgName = name; orgDescription = description; }
	void setApplicationInfo(char *name, char *ver, char *id){ appVersion = ver; appFullName = name; appId = id; }

	// Application, Organization, Person (OwnerHistory, PersonAndOrganization)
	int	getApplicationInstance();
	int	getOrganizationInstance();
	int	getOwnerHistoryInstance();
	int	getPersonAndOrganizationInstance();
	int	getPersonInstance();

	// ConversionBasedUnit, DimensionalExponents, MeasureWithUnit, SIUnit, UnitAssignment
	int	getConversionBasedUnitInstance();
	int	getDimensionalExponentsInstance();
	int	buildMeasureWithUnitInstance();
	int	buildSIUnitInstance(char * UnitType, char * Prefix, char * Name);
	int	getUnitAssignmentInstance(char * lengthUnitConversion);

	// WorldCoordinateSystem, GeometricRepresentationContext
	int getWorldCoordinateSystemInstance();
	int getGeometricRepresentationContextInstance();
private:
	void setupHeader(char *ifcFileName);
};

class CIFCSite : public CIFCInstance
{
	char *siteName;
	char *siteDescription;
	int refLat_x, refLat_y, refLat_z;
	int refLong_x, refLong_y, refLong_z;

public:
	CIFCSite(CIFCProject *pParent, transformationMatrixStruct * pMatrix);

	virtual int build();

	void setSiteInfo(char *name, char *description)		{ siteName = name; siteDescription = description; }
	void setLat(int d, int m, int s)					{ refLat_x = d; refLat_y = m; refLat_z = s; }
	void setLong(int d, int m, int s)					{ refLong_x = d; refLong_y = m; refLong_z = s; }
};

class CIFCBuilding : public CIFCInstance
{
	char *buildingName;
	char *buildingDescription;

public:
	CIFCBuilding(CIFCSite *pParent, transformationMatrixStruct *pMatrix);
	
	virtual int build();

	void setBuildingInfo(char *name, char *description)		{ buildingName = name; buildingDescription = description; }
};

class CIFCStorey : public CIFCInstance
{
	char *storeyName;
	char *storeyDescription;

public:
	CIFCStorey(CIFCBuilding *pParent, transformationMatrixStruct *pMatrix);

	virtual int build();

	void setStoreyInfo(char *name, char *description)		{ storeyName = name; storeyDescription = description; }
};


//
//
//		Matrix & Point
//
//

void	identityMatrix(transformationMatrixStruct * pMatrix);
void	identityPoint(point3DStruct * pPoint);
int		* getTimeStamp();

//
//
//		GUID
//
//

char * CreateCompressedGuidString();
char * getString64FromGuid(const GUID *pGuid, char * buf);
BOOL cv_to_64(const unsigned long number, char *code, int len);

//
//
//		CartesianPoint, Direction, LocalPlacement (Axis2Placement)
//
//

int	buildAxis2Placement3DInstance(transformationMatrixStruct * pMatrix);
int	buildCartesianPointInstance(point3DStruct * pPoint);
int	buildCartesianPointInstance(double x, double y, double z);
int	buildDirectionInstance(point3DStruct * pPoint);
int	buildLocalPlacementInstance(transformationMatrixStruct * pMatrix, int ifcPlacementRelativeTo);

