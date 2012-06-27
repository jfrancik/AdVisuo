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
public:
	enum { WALL_FRONT, WALL_REAR, WALL_SIDE, WALL_CEILING, WALL_FLOOR,
		WALL_BEAM, WALL_SHAFT, WALL_OPENING, WALL_DOOR, WALL_LIFT, WALL_LIFT_FLOOR, WALL_LIFT_CEILING, WALL_LIFT_DOOR,
		WALL_LIFT_NUMBER_PLATE, WALL_FLOOR_NUMBER_PLATE
	};
	enum { ELEM_PROJECT, ELEM_SITE, ELEM_BUILDING, ELEM_STOREY, ELEM_SHAFT, ELEM_EXTRA, ELEM_LIFT };
	enum { BONE_DECK };

protected:
	CProject *m_pProject;
	CBuilding *m_pBuilding;
	CBone *m_pBone;
	CElem *m_pParent;
	std::wstring m_name;

protected:
	// Implementation
	virtual std::wstring onCreateName(AVULONG nElemId, std::wstring name,  AVLONG i) = 0;
	virtual void onCreate(AVULONG nElemId, AVVECTOR &vec) = 0;
	virtual void onMove(AVVECTOR &vec) = 0;
	virtual CBone *onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec) = 0;
	virtual void onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone) = 0;

	static OLECHAR *_name(OLECHAR *name, LONG i)	{ static OLECHAR buf[257]; _snwprintf_s(buf, 256, name, LOWORD(i), HIWORD(i)); return buf; }

public:
	CElem(CProject *pProject, CBuilding *pBuilding)	: m_pProject(pProject), m_pBuilding(pBuilding), m_pBone(NULL), m_pParent(NULL)	{ }
	virtual ~CElem()														{ }

	CBuilding *GetBuilding()												{ return m_pBuilding; }
	CProject *GetProject()													{ return m_pProject; }
	CBone *GetBone()														{ if (m_pBone) return m_pBone; if (m_pParent) return m_pParent->GetBone(); return NULL; }
	CElem *GetParent()														{ return m_pParent; }
	std::wstring GetName()													{ return m_name; }
	
	void Create(CElem *pParent, AVULONG nElemId, AVSTRING name, AVVECTOR vec)				{ m_pParent = pParent; m_name = onCreateName(nElemId, name, 0); onCreate(nElemId, vec); }
	void Create(CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)		{ m_pParent = pParent; m_name = onCreateName(nElemId, name, i); onCreate(nElemId, vec); }

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
	CElem *m_pElemSite;

public:
	CProjectConstr(): CProject()						{ m_pElem = m_pElemSite = NULL; }
	virtual ~CProjectConstr()							{ Deconstruct(); }

	CElem *GetElement()									{ return m_pElem; }
	CElem *GetSiteElement()								{ return m_pElemSite; }

	void Construct();
	void Deconstruct();

	// Implementation
	virtual CElem *CreateElement(CBuilding *pBuilding) = 0;
};

