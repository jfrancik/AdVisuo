// ConstrLftGroup.h - AdVisuo Common Source File

#pragma once

#include "BaseLftGroup.h"

class CElem;
class CProjectConstr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLftGroupConstr

class CLftGroupConstr : public CLftGroup
{
public:

	struct STOREY : public CLftGroup::STOREY
	{
		CElem *m_pElem;

	public:
		STOREY(CLftGroupConstr *pLftGroup, AVULONG nId) : 	CLftGroup::STOREY(pLftGroup, nId), m_pElem(NULL)	{ }
		~STOREY()								{ }

		CLftGroupConstr *GetLftGroup()			{ return (CLftGroupConstr*)CLftGroup::STOREY::GetLftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct(AVULONG iStorey);
		virtual void Deconstruct();
	};

	struct MACHINEROOM : public CLftGroup::MACHINEROOM
	{
		CElem *m_pElem;
	public:
		MACHINEROOM(CLftGroupConstr *pLftGroup) : CLftGroup::MACHINEROOM(pLftGroup), m_pElem(NULL)	{ }
		~MACHINEROOM()							{ }

		CLftGroupConstr *GetLftGroup()			{ return (CLftGroupConstr*)CLftGroup::STOREY::GetLftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct();
		virtual void Deconstruct();
	};

	struct PIT : public CLftGroup::PIT
	{
		CElem *m_pElem;
	public:
		PIT(CLftGroupConstr *pLftGroup) : CLftGroup::PIT(pLftGroup), m_pElem(NULL)	{ }
		~PIT()									{ }

		CLftGroupConstr *GetLftGroup()			{ return (CLftGroupConstr*)CLftGroup::STOREY::GetLftGroup(); }
		CProjectConstr *GetProject()			{ return (CProjectConstr*)CLftGroup::STOREY::GetProject(); }

		CElem *GetElement()						{ return m_pElem; }

		virtual void Construct();
		virtual void Deconstruct();
	};

	struct SHAFT : public CLftGroup::SHAFT
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
		SHAFT(CLftGroupConstr *pLftGroup, AVULONG nId) : CLftGroup::SHAFT(pLftGroup, nId), m_pStoreyBones(NULL), m_pElemMachine(NULL)		{ }
		~SHAFT()											{ }

		CLftGroupConstr *GetLftGroup()						{ return (CLftGroupConstr*)CLftGroup::SHAFT::GetLftGroup(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CLftGroup::SHAFT::GetProject(); }

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

		virtual void Construct(AVULONG iStorey, AVULONG iShaft);
		virtual void ConstructMachine(AVULONG iShaft);
		virtual void ConstructPit(AVULONG iShaft);
		virtual void Deconstruct();
	};

	struct LIFT : public CLftGroup::LIFT
	{
		// lift structure
		CElem *m_pElem;
		CElem *m_ppDecks[DECK_NUM];
		CElem *m_ppDoors[MAX_DOORS];

	public:
		LIFT(CLftGroupConstr *pLftGroup, AVULONG nId) : CLftGroup::LIFT(pLftGroup, nId)
		{
			m_pElem = NULL;
			memset(m_ppDecks, 0, sizeof(m_ppDecks));
			memset(m_ppDoors, 0, sizeof(m_ppDoors));
		}
		~LIFT()												{ }

		CLftGroupConstr *GetLftGroup()						{ return (CLftGroupConstr*)CLftGroup::LIFT::GetLftGroup(); }
		CProjectConstr *GetProject()						{ return (CProjectConstr*)CLftGroup::LIFT::GetProject(); }

		CElem *GetElement()									{ return m_pElem; }
		CElem *GetDeck(AVULONG nDeck)						{ return m_ppDecks[nDeck]; }
		CElem *GetDoor(AVULONG nDoor)						{ return m_ppDoors[nDoor]; }

		virtual void Construct(AVULONG iShaft);
		virtual void Deconstruct();
	};

// Main Implementation
private:
	bool bFastLoad;	// press Shift+Esc to stop constructing most of the structures
	CElem *m_pElem;

public:
	CLftGroupConstr(CProject *pProject, AVULONG nIndex);
	virtual ~CLftGroupConstr();

protected:
	virtual STOREY *CreateStorey(AVULONG nId){ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)	{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)	{ return new LIFT(this, nId); }
	virtual MACHINEROOM *CreateMachineRoom(){ return new MACHINEROOM(this); }
	virtual PIT *CreatePit()				{ return new PIT(this); }

public:
	CProjectConstr *GetProject()			{ return (CProjectConstr*)CLftGroup::GetProject(); }

	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CLftGroup::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return (STOREY*)CLftGroup::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CLftGroup::GetShaft(i); }
	LIFT *GetLift(AVULONG i)				{ return (LIFT*)CLftGroup::GetLift(i); }
	MACHINEROOM *GetMachineRoom()			{ return (MACHINEROOM*)CLftGroup::GetMachineRoom(); }
	PIT *GetPit()							{ return (PIT*)CLftGroup::GetPit(); }


	CElem *GetElement()																{ return m_pElem; }
	
	CElem *GetStoreyElement(AVULONG nStorey)										{ return GetStorey(nStorey)->GetElement(); }
	CElem *GetMachineRoomElement()													{ return GetMachineRoom()->GetElement(); }
	CElem *GetPitElement()															{ return GetPit()->GetElement(); }
	
	CElem *GetLiftElement(AVULONG nLift)											{ return GetLift(nLift)->GetElement(); }
	CElem *GetLiftDeck(AVULONG nLift, AVULONG nDeck)								{ return GetLift(nLift)->GetDeck(nDeck); }
	CElem *GetLiftDoor(AVULONG nLift, AVULONG nDoor)								{ return GetLift(nLift)->GetDoor(nDoor); }

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
};

