// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvBuilding.h"
#include "ifc/baseIfcObject.h"

class CBuildingIfc;

class CBoneIfc : public CBone
{
	CIFCInstance *m_pNode;
public:
	CBoneIfc(CIFCInstance *pNode) : m_pNode(pNode)	{ }
	virtual ~CBoneIfc()					{ if (m_pNode) delete m_pNode; }
	virtual void *GetHandle()			{ return m_pNode; }

	// implementation specific
	CIFCInstance *GetNode()				{ return m_pNode; }
};

class CElemIfc : public CElem
{
protected:
	// Implementation
	CIFCProject *m_pPrj;
	CIFCInstance *m_pObj;

	virtual void onCreate(CElem *pParent, AVULONG nElemId, AVSTRING name, AVVECTOR &vec);
	virtual void onMove(AVVECTOR &vec);
	virtual CBone *onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec);
	virtual void onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone);

public:
	CElemIfc(CProject *pProject, CBuilding *pBuilding) : CElem(pProject, pBuilding), m_pObj(NULL), m_pPrj(NULL)	{ }
	virtual ~CElemIfc();
	
	CBuildingIfc *GetBuilding()				{ return (CBuildingIfc*)CElem::GetBuilding(); }
	CBoneIfc *GetBone()						{ return (CBoneIfc*)CElem::GetBone(); }

	// implementation specific
	CIFCInstance *GetNode()					{ return GetBone()->GetNode(); }
	CIFCInstance *GetNode(CBone *p)			{ return ((CBoneIfc*)p)->GetNode(); }

	//void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);
};

class CBuildingIfc : public CBuildingSrv
{
public:
	CBuildingIfc(CProject *pProject) : CBuildingSrv(pProject) { }

//	virtual CElem *CreateElement()	{ return new CElemIfc(this); }

public:
	// IFC
	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);
};
