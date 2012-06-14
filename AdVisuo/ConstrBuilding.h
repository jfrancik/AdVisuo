// BaseBuilding.h - AdVisuo Common Source File

#pragma once

#include "../CommonFiles/BaseBuilding.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemBase

class CBoneBase
{
public:
	CBoneBase(void *h)					{ }
	virtual ~CBoneBase()				{ }

	virtual void *GetHandle() = 0;

	operator void*()					{ return GetHandle(); }
};

class CElemBase
{
protected:
	CBuildingBase *m_pBuilding;
	CBoneBase *m_pBone;

protected:
	// Implementation
	virtual void onCreate(AVSTRING name, AVVECTOR &vec) = 0;
	virtual void onMove(AVVECTOR &vec) = 0;
	virtual CBoneBase *onAddBone(AVSTRING name, AVVECTOR &vec) = 0;
	virtual void onAddWall(CBoneBase *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBoneBase **ppNewBone) = 0;

public:
	CElemBase(CBuildingBase *pBuilding = NULL) : m_pBuilding(pBuilding), m_pBone(NULL)	{ }
	virtual ~CElemBase()													{ }

	CBuildingBase *GetBuilding()											{ return m_pBuilding; }
	CBoneBase *GetBone()													{ return m_pBone; }
	
	void Create(AVSTRING name, AVVECTOR vec)								{ onCreate(name, vec); }
	void Create(AVSTRING name, AVLONG i, AVVECTOR vec)						{ onCreate(_name(name, i), vec); }

	void Move(AVVECTOR vec)													{ onMove(vec); }

	CBoneBase *AddBone(AVSTRING name, AVVECTOR vec)							{ return onAddBone(name, vec); }
	CBoneBase *AddBone(AVSTRING name, AVLONG i, AVVECTOR vec)				{ return onAddBone(_name(name, i), vec); }

	void AddWall(CBoneBase *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBoneBase **ppNewBone = NULL)
					{ onAddWall(pBone, nWallId, _name(strName, nIndex), nIndex, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppNewBone); }
	void AddWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBoneBase **ppNewBone = NULL)
					{ onAddWall(GetBone(), nWallId, _name(strName, nIndex), nIndex, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppNewBone); }
	void AddWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					BOX box, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, CBoneBase **ppNewBone = NULL)
					{ AddWall(nWallId, strName, nIndex, box.LeftFrontLower(), box.Width(), box.Height(), box.Depth(), vecRot, nDoorNum, pDoorData, ppNewBone); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuildingConstr

class CBuildingConstr : public CBuildingBase
{
	bool bFastLoad;	// press Shift+Esc to stop building most of the building structures

public:
	CBuildingConstr(void)										{ bFastLoad = false; }
	~CBuildingConstr(void)										{ Deconstruct(); }

public:
	
	struct STOREY : public CBuildingBase::STOREY
	{
		CElemBase *m_pElem;

	public:
		STOREY(CBuildingConstr *pBuilding, AVULONG nId) : 	CBuildingBase::STOREY(pBuilding, nId), m_pElem(NULL)	{ }
		~STOREY()								{ }

		CBuildingConstr *GetBuilding()			{ return (CBuildingConstr*)CBuildingBase::STOREY::GetBuilding(); }

		CElemBase *GetElement()					{ return m_pElem; }

		void Construct(AVULONG iStorey);
		void Deconstruct();
	};

	struct SHAFT : public CBuildingBase::SHAFT
	{
		// storey structures
		struct BONES
		{
			BONES() 	{ memset(m_ppDoors, 0, sizeof(m_ppDoors)); }
			CElemBase *m_pElem;
			CElemBase *m_pElemLobbySide;
			CElemBase *m_pElemLeft;
			CElemBase *m_pElemRight;
			CBoneBase *m_ppDoors[MAX_DOORS];
		} *m_pStoreyBones;

	public:
		SHAFT(CBuildingConstr *pBuilding, AVULONG nId) : CBuildingBase::SHAFT(pBuilding, nId), m_pStoreyBones(NULL)		{ }
		~SHAFT()											{ }

		CBuildingConstr *GetBuilding()						{ return (CBuildingConstr*)CBuildingBase::SHAFT::GetBuilding(); }

		CElemBase *GetElement(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_pElem; }
		CElemBase *GetElementLobbySide(AVULONG nStorey)		{ return m_pStoreyBones[nStorey].m_pElemLobbySide; }
		CElemBase *GetElementLeft(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_pElemLeft; }
		CElemBase *GetElementRight(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_pElemRight; }
		CElemBase *GetElementLeftOrRight(AVULONG nStorey, AVULONG n)	{ return n == 0 ? GetElementLeft(nStorey) : GetElementRight(nStorey); }
		CBoneBase *GetDoor(AVULONG nStorey, AVULONG nDoor)	{ return m_pStoreyBones ? m_pStoreyBones[nStorey].m_ppDoors[nDoor] : NULL; }

		void Construct(AVULONG iStorey, AVULONG iShaft);
		void Deconstruct();
	};

	struct LIFT : public CBuildingBase::LIFT
	{
		// lift structure
		CElemBase *m_pElem;
		CBoneBase *m_ppDecks[DECK_NUM];
		CBoneBase *m_ppDoors[MAX_DOORS];

	public:
		LIFT(CBuildingConstr *pBuilding, AVULONG nId) : CBuildingBase::LIFT(pBuilding, nId)
		{
			m_pElem = pBuilding->CreateElement();
			memset(m_ppDecks, 0, sizeof(m_ppDecks));
			memset(m_ppDoors, 0, sizeof(m_ppDoors));
		}
		~LIFT()												{ }

		CBuildingConstr *GetBuilding()						{ return (CBuildingConstr*)CBuildingBase::LIFT::GetBuilding(); }

		CElemBase *GetElement()								{ return m_pElem; }
		CBoneBase *GetDeck(AVULONG nDeck)					{ return m_ppDecks[nDeck]; }
		CBoneBase *GetDoor(AVULONG nDoor)					{ return m_ppDoors[nDoor]; }

		void Construct(AVULONG iShaft);
		void Deconstruct();
	};

// Main Implementation
public:
	virtual STOREY *CreateStorey(AVULONG nId){ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)	{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)	{ return new LIFT(this, nId); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return (STOREY*)CBuildingBase::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	LIFT *GetLift(AVULONG i)				{ return (LIFT*)CBuildingBase::GetLift(i); }

	virtual CElemBase *CreateElement() = 0;

public:
	CElemBase *GetStoreyElement(AVULONG nStorey)										{ return GetStorey(nStorey)->GetElement(); }
	CBoneBase *GetStoreyBone(AVULONG nStorey)											{ return GetStoreyElement(nStorey)->GetBone(); }
	
	CElemBase *GetLiftElement(AVULONG nLift)											{ return GetLift(nLift)->GetElement(); }
	CBoneBase *GetLiftBone(AVULONG nLift)												{ return GetLiftElement(nLift)->GetBone(); }
	CBoneBase *GetLiftDeck(AVULONG nLift, AVULONG nDeck)								{ return GetLift(nLift)->GetDeck(nDeck); }
	CBoneBase *GetLiftDoor(AVULONG nLift, AVULONG nDoor)								{ return GetLift(nLift)->GetDoor(nDoor); }

	CElemBase *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return GetShaft(nShaft)->GetElement(nStorey); }
	CElemBase *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)				{ return GetShaft(nShaft)->GetElementLobbySide(nStorey); }
	CElemBase *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return GetShaft(nShaft)->GetElementLeft(nStorey); }
	CElemBase *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)					{ return GetShaft(nShaft)->GetElementRight(nStorey); }
	CElemBase *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return GetShaft(nShaft)->GetElementLeftOrRight(nStorey, n); }
	CBoneBase *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return GetShaft(nShaft)->GetDoor(nStorey, nDoor); }

	void Construct(AVVECTOR vec);
	void Deconstruct();
};

