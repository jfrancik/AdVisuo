// BaseClasses.h - AdVisuo Common Source File

#pragma once

#include "BaseProject.h"
#include "Box.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElem

class CElem
{
	CProject *m_pProject;
	CLiftGroup *m_pLiftGroup;
	CElem *m_pParent;
	std::wstring m_name;

public:

	enum { ELEM_PROJECT, ELEM_SITE, ELEM_LIFTGROUP, ELEM_STOREY, ELEM_SHAFT, ELEM_EXTRA, ELEM_LIFT, ELEM_DECK, ELEM_BONE, ELEM_DOOR };

	enum { WALL_FRONT, WALL_REAR, WALL_SIDE, WALL_CEILING, WALL_FLOOR,
		WALL_BEAM, WALL_SHAFT, WALL_OPENING, WALL_DOOR, WALL_LIFT, WALL_LIFT_FLOOR, WALL_LIFT_CEILING, WALL_LIFT_DOOR,
		WALL_LIFT_NUMBER_PLATE, WALL_FLOOR_NUMBER_PLATE,
		WALL_CWT
	};

	enum { MODEL_MACHINE, MODEL_OVERSPEED, MODEL_CONTROL_PANEL, MODEL_DRIVE_PANEL, MODEL_GROUP_PANEL, MODEL_ISOLATOR, MODEL_CWT, MODEL_RAIL, MODEL_BUFFER_CAR, MODEL_BUFFER_CWT, MODEL_PULLEY, MODEL_LADDER, MODEL_LIGHT,
		   MODEL_JAMB, MODEL_JAMB_CAR, MODEL_HEADING, MODEL_HEADING_CAR, MODEL_SILL, MODEL_SILL_CAR, MODEL_HANDRAIL };

	CElem(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElem();

	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL) = 0;
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVULONG nParam = 0, AVFLOAT fParam1 = 0, AVFLOAT fParam2 = 0) = 0;
	virtual void Move(AVVECTOR vec) = 0;
	virtual void MoveTo(AVVECTOR vec) = 0;

	CLiftGroup *GetLiftGroup()			{ return m_pLiftGroup; }
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
	virtual CElem *CreateElement(CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) = 0;
};

