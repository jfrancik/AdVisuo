// IFCScanner.h - a part of the AdVisuo Server Module

#pragma once
#include <functional>
#include "ifc/baseIfcElement.h"

class CIFCModelScanner
{
public:
	struct ITEM;
	
	typedef int  HINSTANCE;							// instance handle
	typedef int* HAGGREG;							// aggregate handle
	typedef void* VALUE;							// IFC value type
	typedef std::function<void (ITEM*)> CB_FUNC;	// callback function

	// item information record - passed to the callback function
	struct ITEM
	{
		enum { AGGREG, INSTANCE } type;				// aggregate or instance?
		ITEM *pParent;								// parent information
		int nLevel;									// level in the hierarchy (0 = top)
		union
		{
			struct 
			{										// aggregate information:
				HAGGREG hAggreg, hNewAggreg;		// handle - original and clone; set the clone to NULL to prevent deeper scan
				int nIndex;							// -1 for the entire aggregate, 0..n for aggregate elements
			};
			struct
			{										// instance information:
				HINSTANCE hInstance, hNewInstance;	// handle - original and clone; set the clone to NULL to prevent deeper scan
				char *pstrAttrName;					// NULL for the entire instance, name for the attributes
			};
		};
		int nType;									// sdai type code for the value of the aggreg element or instance attribute
		VALUE value;								// value of the aggreg element or instance attribute - for all sdai codes but sdaiREAL
		double dvalue;								// value of the aggreg element or instance attribute - for sdaiREAL only
	};

	CIFCModelScanner()						{ SetCallback(NULL); }
	CIFCModelScanner(CB_FUNC callbackFn)	{ SetCallback(callbackFn); }

	void SetCallback(CB_FUNC callbackFn)	{ m_callbackFn = callbackFn; }
	CB_FUNC GetCallback()					{ return m_callbackFn; }
	
	HAGGREG   Scan(HAGGREG hAggreg, ITEM *pParent = NULL);
	HINSTANCE Scan(HINSTANCE hInstance, ITEM *pParent = NULL);

	void Null();
	HAGGREG Null(HAGGREG hAggreg)			{ Null(); return Scan(hAggreg); }
	HINSTANCE Null(HINSTANCE hInstance)		{ Null(); return Scan(hInstance); }

	void Dump();
	static HAGGREG Dump(HAGGREG hAggreg)			{ CIFCModelScanner s; s.Dump(); return s.Scan(hAggreg); }
	static HINSTANCE Dump(HINSTANCE hInstance)		{ CIFCModelScanner s; s.Dump(); return s.Scan(hInstance); }

	void DumpAsCpp();
	static HAGGREG DumpAsCpp(HAGGREG hAggreg)		{ CIFCModelScanner s; s.DumpAsCpp(); return s.Scan(hAggreg); }
	static HINSTANCE DumpAsCpp(HINSTANCE hInstance)	{ CIFCModelScanner s; s.DumpAsCpp(); return s.Scan(hInstance); }

	void DumpAsPoints();
	static HAGGREG DumpAsPoints(HAGGREG hAggreg)		{ CIFCModelScanner s; s.DumpAsPoints(); return s.Scan(hAggreg); }
	static HINSTANCE DumpAsPoints(HINSTANCE hInstance)	{ CIFCModelScanner s; s.DumpAsPoints(); return s.Scan(hInstance); }

	void Clone(HINSTANCE nTargetModel, CB_FUNC cbUserFilter = NULL);
	static HAGGREG Clone(HAGGREG hAggreg, HINSTANCE nTargetModel, CB_FUNC cbUserFilter = NULL)			
													{ CIFCModelScanner s; s.Clone(nTargetModel, cbUserFilter); return s.Scan(hAggreg); }
	static HINSTANCE Clone(HINSTANCE hInstance, HINSTANCE nTargetModel, CB_FUNC cbUserFilter = NULL)		
													{ CIFCModelScanner s; s.Clone(nTargetModel, cbUserFilter); return s.Scan(hInstance); }

	// bounding box
	struct BB { double x0, x1, y0, y1, z0, z1; };
	void GetBB(BB &bb);
	static HAGGREG GetBB(HAGGREG hAggreg, BB &bb)			{ CIFCModelScanner s; s.GetBB(bb); return s.Scan(hAggreg); }
	static HINSTANCE GetBB(HINSTANCE hInstance, BB &bb)		{ CIFCModelScanner s; s.GetBB(bb); return s.Scan(hInstance); }

protected:
	void GetTypeValue(ITEM &item, std::function<void (int nType, VALUE value)> f);

private:
	CB_FUNC m_callbackFn;
};

class CRevitFile
{
public:
	typedef int INSTANCE;
	
	INSTANCE m_hModel, *m_h;

	CRevitFile() : m_hModel(NULL), m_h(NULL)				{  }
	CRevitFile(char *pIfcFile, char *pExpFile) : m_hModel(NULL), m_h(NULL)	{ Open(pIfcFile, pExpFile); }
	~CRevitFile()											{ Open(NULL, NULL); }
	operator bool()											{ return m_hModel != NULL && m_h != NULL; }

	INSTANCE GetModel()										{ return m_hModel; }

	bool Open(char *pIfcFile, char *pExpFile)
	{
		if (m_hModel) sdaiCloseModel(m_hModel);
		if (pIfcFile)
		{
			m_hModel = sdaiOpenModelBN(0, pIfcFile, pExpFile);
			if (m_hModel) m_h = sdaiGetEntityExtentBN(m_hModel, "IFCBUILDINGELEMENTPROXY");
		}
		else
			m_hModel = NULL;
		return *this;
	}

	void GetInstance(int nIndex, INSTANCE &h, INSTANCE &hRep)
	{
		if (!*this) return;
		engiGetAggrElement(m_h, nIndex, sdaiINSTANCE, &h);
		if (h) sdaiGetAttrBN(h, "Representation", sdaiINSTANCE, &hRep);
	}
};

class CIFCRevitElem : public CIFCElement
{
public:
	typedef int  HINSTANCE;

private:
	char *strName;
	char *strDescription;
	HINSTANCE hCloneInstance;

public:
	CIFCRevitElem(CIFCRoot *pParent, transformationMatrixStruct *pMatrix);
	
	virtual int build(HINSTANCE hSourceInstance, CIFCModelScanner::CB_FUNC cbUserFilter = NULL);
	virtual int build() { return 0; }

	HINSTANCE getCloneInstance()							{ return hCloneInstance; }

	void setInfo(char *name, char *description)		{ strName = name; strDescription = description; }
};

class CIFCPointsElem : public CIFCElement
{
public:
	typedef int  HINSTANCE;

private:
	char *strName;
	char *strDescription;
	HINSTANCE hCloneInstance;

public:
	CIFCPointsElem(CIFCRoot *pParent, transformationMatrixStruct *pMatrix);
	
	virtual int build(AVULONG nFaceSets, AVULONG *pnFacesPerSet, double *pData, AVULONG *pnPointsPerFace = NULL);
	virtual int build() { return 0; }

	void setInfo(char *name, char *description)		{ strName = name; strDescription = description; }

private:
	void _buildCartesianPoint(int *hAggregPolygon, double dX, double dY, double dZ);
	double *_buildFace(int *hAggregCfsFaces, double *pData, AVULONG nData);
	double *_buildConnectedFaceSet(int *hAggregFbsmFaces, double *pData, AVULONG nData, AVULONG *pnPointsPerFace = NULL);
	int _build(AVULONG nFaceSets, AVULONG *pnFacesPerSet, double *pData, AVULONG *pnPointsPerFace = NULL);
};
