// Passenger.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/XMLTools.h"
#include <list>

interface IBody;
interface ISceneObject;
interface IAction;
interface IRenderer;
interface IKineNode;

class CSimVis;

class CPassengerVis : public CPassenger
{
	// FreeWill Data
	IBody *m_pBody;
	ISceneObject *m_pObjBody;
	IAction *m_pActionTick;

public:
	CPassengerVis(CSimVis *pSim, AVULONG nPassengerId);
	virtual ~CPassengerVis(void);

	CSimVis *GetSim()		{ return (CSimVis*)CPassenger::GetSim(); }
	ISceneObject *GetBody()	{ return m_pObjBody; }

// Operations

	void Play(IAction *pActionTick);
	void Render(IRenderer *pRenderer, AVLONG nPhase = 0);

	void BeBorn();
	void Die();
	void Embark(enum ENUM_ACTION nAction, bool bSwitchCoord = true);
	void Embark(IKineNode *pNode, bool bSwitchCoord = true);

	friend int _callback_birth(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam);
	friend int _callback_death(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam);
	friend int _callback_embark(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam);
};