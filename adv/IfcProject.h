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
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0));
	virtual void Move(AVVECTOR vec);
};

class CIfcBuilder
{
	CRevitFile m_revitfile;
	int m_h;
	CIFCModelScanner::BB m_bb;
public:
	CIfcBuilder(char *pFilename, AVULONG nInstanceIndex);
	void build(CElemIfc *pElem, AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0);
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
