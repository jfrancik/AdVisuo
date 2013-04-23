// Passenger.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "../CommonFiles/XMLTools.h"
#include "_base.h"

class CSimVis;
class CEngine;

class CPassengerVis : public CPassenger, public IAnimationListener
{
	// FreeWill Data
	HBODY m_pBody;
	CEngine *m_pEngine;			// FreeWill engine

public:
	CPassengerVis(CSimVis *pSim, AVULONG nPassengerId);
	virtual ~CPassengerVis(void);

	CSimVis *GetSim()		{ return (CSimVis*)CPassenger::GetSim(); }

// Operations

	void Play(CEngine *pEngine);
	void Render(AVLONG nPhase = 0);

	void Spawn();
	void Die();
	
	// implementation of IAnimationListener
	virtual int OnAnimationBegin(AVULONG nParam)		{ return S_OK; }
	virtual int OnAnimationTick(AVULONG nParam);
	virtual int OnAnimationEnd(AVULONG nParam)			{ return S_OK; }
};
