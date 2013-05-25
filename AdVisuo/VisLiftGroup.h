// VisLiftGroup.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/ConstrLiftGroup.h"

class CSimVis;
class CElemVis;
class CProjectVis;

class CLiftGroupVis : public CLiftGroupConstr
{
public:
	CLiftGroupVis(CProject *pProject, AVULONG iIndex) : CLiftGroupConstr(pProject, iIndex)		{ }
	~CLiftGroupVis()	{ }

protected:
	virtual CSim *CreateSim();

public:
	CSimVis *AddSim()																	{ return (CSimVis*)CLiftGroupConstr::AddSim(); }
	CSimVis *GetSim()																	{ return (CSimVis*)CLiftGroupConstr::GetSim(); }

	SHAFT  *AddShaft()																	{ return (SHAFT *)CLiftGroupConstr::AddShaft(); }
	STOREY *AddStorey()																	{ return (STOREY*)CLiftGroupConstr::AddStorey(); }
	LIFT   *AddLift()																	{ return (LIFT  *)CLiftGroupConstr::AddLift(); }

	CProjectVis *GetProject()															{ return (CProjectVis*)CLiftGroupConstr::GetProject(); }
	STOREY *GetStorey(AVULONG i)														{ return (STOREY*)CLiftGroupConstr::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)												{ return (STOREY*)CLiftGroupConstr::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)															{ return (SHAFT*)CLiftGroupConstr::GetShaft(i); }
	LIFT *GetLift(AVULONG i)															{ return (LIFT*)CLiftGroupConstr::GetLift(i); }
	MR *GetMR()																			{ return (MR*)CLiftGroupConstr::GetMR(); }
	PIT *GetPit()																		{ return (PIT*)CLiftGroupConstr::GetPit(); }

	CElemVis *GetElement()																{ return (CElemVis*)CLiftGroupConstr::GetElement(); }

	CElemVis *GetStoreyElement(AVULONG nStorey)											{ return (CElemVis*)CLiftGroupConstr::GetStoreyElement(nStorey); }
	CElemVis *GetMRElement()															{ return (CElemVis*)CLiftGroupConstr::GetMRElement(); }
	CElemVis *GetPitElement()															{ return (CElemVis*)CLiftGroupConstr::GetPitElement(); }

	CElemVis *GetLiftElement(AVULONG nLift)												{ return (CElemVis*)CLiftGroupConstr::GetLiftElement(nLift); }
	CElemVis *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemVis*)CLiftGroupConstr::GetLiftDeck(nLift, nDeck); }
	CElemVis *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemVis*)CLiftGroupConstr::GetLiftDoor(nLift, nDoor); }

	CElemVis *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemVis*)CLiftGroupConstr::GetShaftElement(nStorey, nShaft); }
	CElemVis *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemVis*)CLiftGroupConstr::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemVis *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CLiftGroupConstr::GetShaftElementLeft(nStorey, nShaft); }
	CElemVis *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CLiftGroupConstr::GetShaftElementRight(nStorey, nShaft); }
	CElemVis *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemVis*)CLiftGroupConstr::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemVis *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemVis*)CLiftGroupConstr::GetShaftDoor(nStorey, nShaft, nDoor); }

	CElemVis *GetMachineElement(AVULONG nShaft)											{ return (CElemVis*)CLiftGroupConstr::GetMachineElement(nShaft); }

	CElemVis *GetPitElement(AVULONG nShaft)												{ return (CElemVis*)CLiftGroupConstr::GetPitElement(nShaft); }
	CElemVis *GetPitElementLobbySide(AVULONG nShaft)									{ return (CElemVis*)CLiftGroupConstr::GetPitElementLobbySide(nShaft); }
	CElemVis *GetPitElementLeft(AVULONG nShaft)											{ return (CElemVis*)CLiftGroupConstr::GetPitElementLeft(nShaft); }
	CElemVis *GetPitElementRight(AVULONG nShaft)										{ return (CElemVis*)CLiftGroupConstr::GetPitElementRight(nShaft); }
	CElemVis *GetPitElementLeftOrRight(AVULONG nShaft, AVULONG n)						{ return (CElemVis*)CLiftGroupConstr::GetPitElementLeftOrRight(nShaft, n); }

// Main Implementation
public:
	AVVECTOR GetLiftPos(int nLift);							// returns position of the lift

	void StoreConfig();
	void RestoreConfig();
};
