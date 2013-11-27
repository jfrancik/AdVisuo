// ConstrLiftGroup.h - AdVisuo Common Source File

#pragma once

#include "BaseLiftGroup.h"

class CElem;
class CProjectConstr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLiftGroupConstr

class CLiftGroupConstr : public CLiftGroup
{
public:

	struct STOREY : public CLiftGroup::STOREY
	{
		CElem *m_pElem;

	public:
		STOREY(CLiftGroupConstr *pLiftGroup, AVULONG nId) : 	CLiftGroup::STOREY(pLiftGroup, nId), m_pElem(NULL)	{ }
		~STOREY()								{ }

		CLiftGroupConstr *GetLiftGroup()		{ return (CLiftGroupConstr*)CLiftGroup::STOREY::GetLiftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLiftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct(AVULONG iStorey);
		virtual void Deconstruct();
	};

	struct MR : public CLiftGroup::MR
	{
		CElem *m_pElem;
	public:
		MR(CLiftGroupConstr *pLiftGroup) : CLiftGroup::MR(pLiftGroup), m_pElem(NULL)	{ }
		~MR()							{ }

		CLiftGroupConstr *GetLiftGroup()			{ return (CLiftGroupConstr*)CLiftGroup::STOREY::GetLiftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLiftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct();
		virtual void Deconstruct();
	};

	struct PIT : public CLiftGroup::PIT
	{
		CElem *m_pElem;
	public:
		PIT(CLiftGroupConstr *pLiftGroup) : CLiftGroup::PIT(pLiftGroup), m_pElem(NULL)	{ }
		~PIT()									{ }

		CLiftGroupConstr *GetLiftGroup()		{ return (CLiftGroupConstr*)CLiftGroup::STOREY::GetLiftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLiftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct();
		virtual void Deconstruct();
	};

	struct SHAFT : public CLiftGroup::SHAFT
	{
		// storey structures
		struct BONES
		{
			BONES() 	{ m_pElem = m_pElemLobbySide = m_pElemLeft = m_pElemRight = NULL; memset(m_ppDoors, 0, sizeof(m_ppDoors)); }
			CElem *m_pElem;
			CElem *m_pElemLobbySide;
			CElem *m_pElemLeft;
			CElem *m_pElemRight;
			CElem *m_ppDoors[MAX_DOORS];
		};
		
		BONES *m_pStoreyBones;		// regular stories
		CElem *m_pElemMachine;		// machine room zone
		BONES m_PitBones;			// lift pit bones

	public:
		SHAFT(CLiftGroupConstr *pLiftGroup, AVULONG nId) : CLiftGroup::SHAFT(pLiftGroup, nId), m_pStoreyBones(NULL), m_pElemMachine(NULL)		{ }
		~SHAFT()											{ }

		CLiftGroupConstr *GetLiftGroup()					{ return (CLiftGroupConstr*)CLiftGroup::SHAFT::GetLiftGroup(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CLiftGroup::SHAFT::GetProject(); }

		CElem *GetElement(AVULONG nStorey)					{ return m_pStoreyBones[nStorey].m_pElem; }
		CElem *GetElementLobbySide(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_pElemLobbySide; }
		CElem *GetElementLeft(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_pElemLeft; }
		CElem *GetElementRight(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_pElemRight; }
		CElem *GetElementLeftOrRight(AVULONG nStorey, AVULONG n)	{ return n == 0 ? GetElementLeft(nStorey) : GetElementRight(nStorey); }
		CElem *GetDoor(AVULONG nStorey, AVULONG nDoor)		{ return m_pStoreyBones ? m_pStoreyBones[nStorey].m_ppDoors[nDoor] : NULL; }

		CElem *GetMachineElement()							{ return m_pElemMachine; }

		CElem *GetPitElement()								{ return m_PitBones.m_pElem; }
		CElem *GetPitElementLobbySide()						{ return m_PitBones.m_pElemLobbySide; }
		CElem *GetPitElementLeft()							{ return m_PitBones.m_pElemLeft; }
		CElem *GetPitElementRight()							{ return m_PitBones.m_pElemRight; }
		CElem *GetPitElementLeftOrRight(AVULONG n)			{ return n == 0 ? GetPitElementLeft() : GetPitElementRight(); }

		virtual void Construct(AVULONG iStorey);
		virtual void ConstructMREquipment();
		virtual void ConstructPitEquipment();
		virtual void Deconstruct();
	};

	struct LIFT : public CLiftGroup::LIFT
	{
		// lift structure
		CElem *m_pElem;
		CElem *m_ppDecks[DECK_NUM];
		CElem *m_ppDoors[MAX_DOORS * DECK_NUM];

	public:
		LIFT(CLiftGroupConstr *pLiftGroup, AVULONG nId) : CLiftGroup::LIFT(pLiftGroup, nId)
		{
			m_pElem = NULL;
			memset(m_ppDecks, 0, sizeof(m_ppDecks));
			memset(m_ppDoors, 0, sizeof(m_ppDoors));
		}
		~LIFT()												{ }

		CLiftGroupConstr *GetLiftGroup()					{ return (CLiftGroupConstr*)CLiftGroup::LIFT::GetLiftGroup(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CLiftGroup::LIFT::GetProject(); }

		CElem *GetElement()									{ return m_pElem; }
		CElem *GetDeck(AVULONG nDeck)						{ return m_ppDecks[nDeck]; }
		CElem *GetDoor(AVULONG nDeck, AVULONG nDoor)		{ return m_ppDoors[nDeck * MAX_DOORS + nDoor]; }

		virtual void Construct(AVULONG iShaft);
		virtual void Deconstruct();
	};

// Main Implementation
private:
	bool bFastLoad;	// press Shift+Esc to stop constructing most of the structures
	CElem *m_pElem;

public:
	CLiftGroupConstr(CProject *pProject, AVULONG nIndex);
	virtual ~CLiftGroupConstr();

protected:
	virtual STOREY *CreateStorey(AVULONG nId){ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)	{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)	{ return new LIFT(this, nId); }
	virtual MR *CreateMR()					{ return new MR(this); }
	virtual PIT *CreatePit()				{ return new PIT(this); }

public:
	CProjectConstr *GetProject()			{ return (CProjectConstr*)CLiftGroup::GetProject(); }

	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CLiftGroup::GetStorey(i); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CLiftGroup::GetShaft(i); }
	LIFT *GetLift(AVULONG i)				{ return (LIFT*)CLiftGroup::GetLift(i); }
	MR *GetMR()								{ return (MR*)CLiftGroup::GetMR(); }
	PIT *GetPit()							{ return (PIT*)CLiftGroup::GetPit(); }

	CElem *GetElement()																{ return m_pElem; }
	
	CElem *GetStoreyElement(AVULONG nStorey)										{ return GetStorey(nStorey)->GetElement(); }
	CElem *GetMRElement()															{ return GetMR()->GetElement(); }
	CElem *GetPitElement()															{ return GetPit()->GetElement(); }
	
	CElem *GetLiftElement(AVULONG nLift)											{ return GetLift(nLift)->GetElement(); }
	CElem *GetLiftDeck(AVULONG nLift, AVULONG nDeck)								{ return GetLift(nLift)->GetDeck(nDeck); }
	CElem *GetLiftDoor(AVULONG nLift, AVULONG nDeck, AVULONG nDoor)					{ return GetLift(nLift)->GetDoor(nDeck, nDoor); }

	CElem *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return GetShaft(nShaft)->GetElement(nStorey); }
	CElem *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)				{ return GetShaft(nShaft)->GetElementLobbySide(nStorey); }
	CElem *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return GetShaft(nShaft)->GetElementLeft(nStorey); }
	CElem *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)					{ return GetShaft(nShaft)->GetElementRight(nStorey); }
	CElem *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return GetShaft(nShaft)->GetElementLeftOrRight(nStorey, n); }
	CElem *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return GetShaft(nShaft)->GetDoor(nStorey, nDoor); }

	CElem *GetMachineElement(AVULONG nShaft)										{ return GetShaft(nShaft)->GetMachineElement(); }

	CElem *GetPitElement(AVULONG nShaft)											{ return GetShaft(nShaft)->GetPitElement(); }
	CElem *GetPitElementLobbySide(AVULONG nShaft)									{ return GetShaft(nShaft)->GetPitElementLobbySide(); }
	CElem *GetPitElementLeft(AVULONG nShaft)										{ return GetShaft(nShaft)->GetPitElementLeft(); }
	CElem *GetPitElementRight(AVULONG nShaft)										{ return GetShaft(nShaft)->GetPitElementRight(); }
	CElem *GetPitElementLeftOrRight(AVULONG nShaft, AVULONG n)						{ return GetShaft(nShaft)->GetPitElementLeftOrRight(n); }

	virtual void Construct(AVVECTOR vec);
	virtual void Deconstruct();

	// necessary overrides for new versions of SHAFT/STOREY/LIFT
	SHAFT  *AddShaft()						{ return (SHAFT *)CLiftGroup::AddShaft(); }
	STOREY *AddStorey()						{ return (STOREY*)CLiftGroup::AddStorey(); }
	LIFT   *AddLift()						{ return (LIFT  *)CLiftGroup::AddLift(); }
};

