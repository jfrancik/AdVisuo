// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvProject.h"
#include "ifc/baseIfcElement.h"

class CProjectIfc;
class CLiftGroupIfc;

class CElemIfc : public CElem
{
	CIFCRoot *m_pBone;
public:
	CElemIfc(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElemIfc();
	
	CProjectIfc *GetProject()				{ return (CProjectIfc*)CElem::GetProject(); }
	CLiftGroupIfc *GetLiftGroup()			{ return (CLiftGroupIfc*)CElem::GetLiftGroup(); }
	CElemIfc *GetParent()					{ return (CElemIfc*)CElem::GetParent(); }
	CIFCRoot *GetBone()						{ if (m_pBone) return m_pBone; if (GetParent()) return GetParent()->GetBone(); return NULL; }

	// implementation
	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL);
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVULONG nParam = 0, AVFLOAT fParam1 = 0, AVFLOAT fParam2 = 0);
	virtual void Move(AVVECTOR vec);
	virtual void MoveTo(AVVECTOR vec);
};

class CProjectIfc : public CProjectSrv
{
public:
	CProjectIfc();

	virtual CElem *CreateElement(CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)
															{ return new CElemIfc(this, pLiftGroup, pParent, nElemId, name, i, vec); }

	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);

	CElemIfc *GetElement()									{ return (CElemIfc*)CProjectSrv::GetElement(); }
	CElemIfc *GetSiteElement()								{ return (CElemIfc*)CProjectSrv::GetSiteElement(); }

	CLiftGroupIfc *GetLiftGroup(int i)						{ return (CLiftGroupIfc*)CProjectSrv::GetLiftGroup(i); }
	CLiftGroupIfc *FindLiftGroup(int id)					{ return (CLiftGroupIfc*)CProjectSrv::FindLiftGroup(id); }
	CLiftGroupIfc *AddLiftGroup()							{ return (CLiftGroupIfc*)CProjectSrv::AddLiftGroup(); }

	void Construct();

protected:
	virtual CLiftGroup *CreateLiftGroup(AVULONG nIndex);
};
