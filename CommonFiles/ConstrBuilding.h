// BaseBuilding.h - AdVisuo Common Source File

#pragma once

#include "ConstrProject.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuildingConstr

class CBuildingConstr : public CBuilding
{
public:

	enum { WALL_FRONT, WALL_REAR, WALL_SIDE, WALL_CEILING, WALL_FLOOR,
		WALL_BEAM, WALL_SHAFT, WALL_OPENING, WALL_DOOR, WALL_LIFT, WALL_LIFT_FLOOR, WALL_LIFT_CEILING, WALL_LIFT_DOOR,
		WALL_LIFT_NUMBER_PLATE, WALL_FLOOR_NUMBER_PLATE
	};
	enum { ELEM_BUILDING, ELEM_STOREY, ELEM_SUB_STOREY, ELEM_LIFT };
	enum { BONE_DECK };

	struct STOREY : public CBuilding::STOREY
	{
		CElem *m_pElem;

	public:
		STOREY(CBuildingConstr *pBuilding, AVULONG nId) : 	CBuilding::STOREY(pBuilding, nId), m_pElem(NULL)	{ }
		~STOREY()								{ }

		CBuildingConstr *GetBuilding()			{ return (CBuildingConstr*)CBuilding::STOREY::GetBuilding(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CBuilding::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct(AVULONG iStorey);
		virtual void Deconstruct();
	};

	struct SHAFT : public CBuilding::SHAFT
	{
		// storey structures
		struct BONES
		{
			BONES() 	{ memset(m_ppDoors, 0, sizeof(m_ppDoors)); }
			CElem *m_pElem;
			CElem *m_pElemLobbySide;
			CElem *m_pElemLeft;
			CElem *m_pElemRight;
			CBone *m_ppDoors[MAX_DOORS];
		} *m_pStoreyBones;

	public:
		SHAFT(CBuildingConstr *pBuilding, AVULONG nId) : CBuilding::SHAFT(pBuilding, nId), m_pStoreyBones(NULL)		{ }
		~SHAFT()											{ }

		CBuildingConstr *GetBuilding()						{ return (CBuildingConstr*)CBuilding::SHAFT::GetBuilding(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CBuilding::SHAFT::GetProject(); }

		CElem *GetElement(AVULONG nStorey)					{ return m_pStoreyBones[nStorey].m_pElem; }
		CElem *GetElementLobbySide(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_pElemLobbySide; }
		CElem *GetElementLeft(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_pElemLeft; }
		CElem *GetElementRight(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_pElemRight; }
		CElem *GetElementLeftOrRight(AVULONG nStorey, AVULONG n)	{ return n == 0 ? GetElementLeft(nStorey) : GetElementRight(nStorey); }
		CBone *GetDoor(AVULONG nStorey, AVULONG nDoor)		{ return m_pStoreyBones ? m_pStoreyBones[nStorey].m_ppDoors[nDoor] : NULL; }

		virtual void Construct(AVULONG iStorey, AVULONG iShaft);
		virtual void Deconstruct();
	};

	struct LIFT : public CBuilding::LIFT
	{
		// lift structure
		CElem *m_pElem;
		CBone *m_ppDecks[DECK_NUM];
		CBone *m_ppDoors[MAX_DOORS];

	public:
		LIFT(CBuildingConstr *pBuilding, AVULONG nId) : CBuilding::LIFT(pBuilding, nId)
		{
			m_pElem = NULL;
			memset(m_ppDecks, 0, sizeof(m_ppDecks));
			memset(m_ppDoors, 0, sizeof(m_ppDoors));
		}
		~LIFT()												{ }

		CBuildingConstr *GetBuilding()						{ return (CBuildingConstr*)CBuilding::LIFT::GetBuilding(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CBuilding::LIFT::GetProject(); }

		CElem *GetElement()									{ return m_pElem; }
		CBone *GetDeck(AVULONG nDeck)						{ return m_ppDecks[nDeck]; }
		CBone *GetDoor(AVULONG nDoor)						{ return m_ppDoors[nDoor]; }

		virtual void Construct(AVULONG iShaft);
		virtual void Deconstruct();
	};

// Main Implementation
private:
	bool bFastLoad;	// press Shift+Esc to stop building most of the building structures
	CElem *m_pElem;

public:
	CBuildingConstr(CProject *pProject): CBuilding(pProject)	{ m_pElem = NULL; bFastLoad = false; }
	virtual ~CBuildingConstr()									{ Deconstruct(); }

	CProjectConstr *GetProject()			{ return (CProjectConstr*)CBuilding::GetProject(); }

	virtual STOREY *CreateStorey(AVULONG nId){ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)	{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)	{ return new LIFT(this, nId); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuilding::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return (STOREY*)CBuilding::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuilding::GetShaft(i); }
	LIFT *GetLift(AVULONG i)				{ return (LIFT*)CBuilding::GetLift(i); }


	CElem *GetElement()																{ return m_pElem; }

	CElem *GetStoreyElement(AVULONG nStorey)										{ return GetStorey(nStorey)->GetElement(); }
	CBone *GetStoreyBone(AVULONG nStorey)											{ return GetStoreyElement(nStorey)->GetBone(); }
	
	CElem *GetLiftElement(AVULONG nLift)											{ return GetLift(nLift)->GetElement(); }
	CBone *GetLiftBone(AVULONG nLift)												{ return GetLiftElement(nLift)->GetBone(); }
	CBone *GetLiftDeck(AVULONG nLift, AVULONG nDeck)								{ return GetLift(nLift)->GetDeck(nDeck); }
	CBone *GetLiftDoor(AVULONG nLift, AVULONG nDoor)								{ return GetLift(nLift)->GetDoor(nDoor); }

	CElem *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return GetShaft(nShaft)->GetElement(nStorey); }
	CElem *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)				{ return GetShaft(nShaft)->GetElementLobbySide(nStorey); }
	CElem *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return GetShaft(nShaft)->GetElementLeft(nStorey); }
	CElem *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)					{ return GetShaft(nShaft)->GetElementRight(nStorey); }
	CElem *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return GetShaft(nShaft)->GetElementLeftOrRight(nStorey, n); }
	CBone *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return GetShaft(nShaft)->GetDoor(nStorey, nDoor); }

	virtual void Construct(AVVECTOR vec);
	virtual void Deconstruct();
};

