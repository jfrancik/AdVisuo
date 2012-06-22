// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "BaseClasses.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBone

class CBone
{
public:
	CBone()							{ }
	virtual ~CBone()				{ }

	virtual void *GetHandle() = 0;

	operator void*()					{ return GetHandle(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElem

class CElem
{
protected:
	CProject *m_pProject;
	CBuilding *m_pBuilding;
	CBone *m_pBone;

protected:
	// Implementation
	virtual void onCreate(CElem *pParent, AVULONG nElemId, AVSTRING name, AVVECTOR &vec) = 0;
	virtual void onMove(AVVECTOR &vec) = 0;
	virtual CBone *onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec) = 0;
	virtual void onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone) = 0;

	static OLECHAR *_name(OLECHAR *name, LONG i)	{ static OLECHAR buf[257]; _snwprintf_s(buf, 256, name, LOWORD(i), HIWORD(i)); return buf; }

public:
	CElem(CProject *pProject, CBuilding *pBuilding)	: m_pProject(pProject), m_pBuilding(pBuilding), m_pBone(NULL)	{ }
	virtual ~CElem()														{ }

	CBuilding *GetBuilding()												{ return m_pBuilding; }
	CProject *GetProject()													{ return m_pProject; }
	CBone *GetBone()														{ return m_pBone; }
	
	void Create(CElem *pParent, AVULONG nElemId, AVSTRING name, AVVECTOR vec)				{ onCreate(pParent, nElemId, name, vec); }
	void Create(CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)		{ onCreate(pParent, nElemId, _name(name, i), vec); }

	void Move(AVVECTOR vec)													{ onMove(vec); }

	CBone *AddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR vec)			{ return onAddBone(nBoneId, name, vec); }
	CBone *AddBone(AVULONG nBoneId, AVSTRING name, AVLONG i, AVVECTOR vec)	{ return onAddBone(nBoneId, _name(name, i), vec); }

	void AddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBone **ppNewBone = NULL)
					{ onAddWall(pBone, nWallId, _name(strName, nIndex), nIndex, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppNewBone); }
	void AddWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBone **ppNewBone = NULL)
					{ onAddWall(GetBone(), nWallId, _name(strName, nIndex), nIndex, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppNewBone); }
	void AddWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					BOX box, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBone **ppNewBone = NULL)
					{ AddWall(nWallId, strName, nIndex, box.LeftFrontLower(), box.Width(), box.Height(), box.Depth(), vecRot, nDoorNum, pDoorData, ppNewBone); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProjectConstr

class CProjectConstr : public CProject
{
	CElem *m_pElem;

public:
	CProjectConstr(): CProject()						{ m_pElem = NULL; }
	virtual ~CProjectConstr()							{ Deconstruct(); }

	CElem *GetElement()									{ return m_pElem; }

	void Construct();
	void Deconstruct();

	// Implementation
	virtual CElem *CreateElement(CBuilding *pBuilding) = 0;
};

