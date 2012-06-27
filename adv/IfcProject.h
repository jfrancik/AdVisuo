// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvProject.h"
#include "IfcBuilding.h"

class CProjectIfc;

class CBoneIfc : public CBone
{
	CIFCRoot *m_pNode;
public:
	CBoneIfc(CIFCRoot *pNode) : m_pNode(pNode)	{ }
	virtual ~CBoneIfc()						{ if (m_pNode) delete m_pNode; }
	virtual void *GetHandle()				{ return m_pNode; }

	// implementation specific
	CIFCRoot *GetNode()						{ return m_pNode; }
};

class CElemIfc : public CElem
{
protected:
	// Implementation
	virtual std::wstring onCreateName(AVULONG nElemId, std::wstring name,  AVLONG i);
	virtual void onCreate(AVULONG nElemId, AVVECTOR &vec);
	virtual void onMove(AVVECTOR &vec);
	virtual CBone *onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec);
	virtual void onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone);

public:
	CElemIfc(CProject *pProject, CBuilding *pBuilding) : CElem(pProject, pBuilding) { }
	virtual ~CElemIfc();
	
	CProjectIfc *GetProject()				{ return (CProjectIfc*)CElem::GetProject(); }
	CBuildingIfc *GetBuilding()				{ return (CBuildingIfc*)CElem::GetBuilding(); }
	CBoneIfc *GetBone()						{ return (CBoneIfc*)CElem::GetBone(); }
	CElemIfc *GetParent()					{ return (CElemIfc*)CElem::GetParent(); }

	// implementation specific
	CIFCRoot *GetNode()						{ return GetBone()->GetNode(); }
	CIFCRoot *GetNode(CBone *p)				{ return ((CBoneIfc*)p)->GetNode(); }

	//void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);
};

class CProjectIfc : public CProjectSrv
{
public:
	CProjectIfc();

	virtual CElem *CreateElement(CBuilding *pBuilding)		{ return new CElemIfc(this, pBuilding); }

	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);

	CElemIfc *GetElement()									{ return (CElemIfc*)CProjectSrv::GetElement(); }
	CElemIfc *GetSiteElement()								{ return (CElemIfc*)CProjectSrv::GetSiteElement(); }

	CBuildingIfc *GetBuilding()								{ return (CBuildingIfc*)CProjectSrv::GetBuilding(); }
	CBuildingIfc *GetBuilding(int i)						{ return (CBuildingIfc*)CProjectSrv::GetBuilding(i); }

	void Construct();

protected:
	virtual CBuilding *CreateBuilding(AVULONG nIndex)		{ return new CBuildingIfc(this, nIndex); }
};
