// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "BaseClasses.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElem

class CElem
{
	CProject *m_pProject;
	CBuilding *m_pBuilding;
	CElem *m_pParent;
	std::wstring m_name;

public:

	enum { ELEM_PROJECT, ELEM_SITE, ELEM_BUILDING, ELEM_STOREY, ELEM_SHAFT, ELEM_EXTRA, ELEM_LIFT, ELEM_DECK, ELEM_BONE, ELEM_DOOR };

	enum { WALL_FRONT, WALL_REAR, WALL_SIDE, WALL_CEILING, WALL_FLOOR,
		WALL_BEAM, WALL_SHAFT, WALL_OPENING, WALL_DOOR, WALL_LIFT, WALL_LIFT_FLOOR, WALL_LIFT_CEILING, WALL_LIFT_DOOR,
		WALL_LIFT_NUMBER_PLATE, WALL_FLOOR_NUMBER_PLATE
	};

	enum { MODEL_MACHINE, MODEL_OVERSPEED, MODEL_CONTROL, MODEL_ISOLATOR, MODEL_CWT, MODEL_RAIL_CAR, MODEL_RAIL_CWT, MODEL_BUFFER, MODEL_PULLEY, MODEL_LADDER };

	CElem(CProject *pProject, CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElem();

	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL) = 0;
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0) = 0;
	virtual void Move(AVVECTOR vec) = 0;

	CBuilding *GetBuilding()			{ return m_pBuilding; }
	CProject *GetProject()				{ return m_pProject; }
	CElem *GetParent()					{ return m_pParent; }
	std::wstring GetName()				{ return m_name; }

	void SetName(std::wstring name)		{ m_name = name; }
	
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
	virtual CElem *CreateElement(CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) = 0;
};
