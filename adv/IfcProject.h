// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvProject.h"
#include "IfcBuilding.h"

#include "ifcscanner.h"

class CProjectIfc;

class CElemIfc : public CElem
{
	CIFCRoot *m_pBone;
public:
	CElemIfc(CProject *pProject, CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElemIfc();
	
	CProjectIfc *GetProject()				{ return (CProjectIfc*)CElem::GetProject(); }
	CBuildingIfc *GetBuilding()				{ return (CBuildingIfc*)CElem::GetBuilding(); }
	CElemIfc *GetParent()					{ return (CElemIfc*)CElem::GetParent(); }
	CIFCRoot *GetBone()						{ if (m_pBone) return m_pBone; if (GetParent()) return GetParent()->GetBone(); return NULL; }

	// implementation
	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL);
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVULONG nParam = 0, AVFLOAT fParam1 = 0, AVFLOAT fParam2 = 0);
	virtual void Move(AVVECTOR vec);
};

class CIfcBuilder
{
	CRevitFile m_revitfile;
	int m_h;
	CIFCModelScanner::BB m_bb;
public:
	CIfcBuilder(char *pFilename, AVULONG nInstanceIndex = 0);

	double Left()	{ return m_bb.x0; }			double Right()	{ return m_bb.x1; }
	double Front()	{ return m_bb.y0; }			double Rear()	{ return m_bb.y1; }
	double Lower()	{ return m_bb.z0; }			double Upper()	{ return m_bb.z1; }

	double Width()	{ return m_bb.x1 - m_bb.x0; }
	double Depth()	{ return m_bb.y1 - m_bb.y0; }
	double Height()	{ return m_bb.z1 - m_bb.z0; }

	int GetHandle()	{ return m_h; }

	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVFLOAT fRotX = 0, bool bIsotropicHeight = true, bool bIsotropicXY = true);
	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fScaleX, AVFLOAT fScaleY, AVFLOAT fScaleZ, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, AVVECTOR base, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);

	static void buildApron(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVFLOAT fRotX = 0);
};

class CProjectIfc : public CProjectSrv
{
public:
	CProjectIfc();

	virtual CElem *CreateElement(CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)
															{ return new CElemIfc(this, pBuilding, pParent, nElemId, name, i, vec); }

	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);

	CElemIfc *GetElement()									{ return (CElemIfc*)CProjectSrv::GetElement(); }
	CElemIfc *GetSiteElement()								{ return (CElemIfc*)CProjectSrv::GetSiteElement(); }

	CBuildingIfc *GetBuilding()								{ return (CBuildingIfc*)CProjectSrv::GetBuilding(); }
	CBuildingIfc *GetBuilding(int i)						{ return (CBuildingIfc*)CProjectSrv::GetBuilding(i); }

	void Construct();

protected:
	virtual CBuilding *CreateBuilding(AVULONG nIndex)		{ return new CBuildingIfc(this, nIndex); }
};
