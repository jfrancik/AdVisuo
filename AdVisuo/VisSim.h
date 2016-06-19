// Sim.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "repos.h"

class CLiftGroupVis;
class CLiftVis;
class CPassengerVis;
class CEngine;

class CSimVis : public CSim
{
public:
	CSimVis();
	virtual ~CSimVis();

	CLiftVis *GetLift(int i)			{ return (CLiftVis*)CSim::GetLift(i); }
	CPassengerVis *GetPassenger(int i)	{ return (CPassengerVis*)CSim::GetPassenger(i); }
	CLiftGroupVis *GetLiftGroup()		{ return (CLiftGroupVis*)CSim::GetLiftGroup(); }

	void Play(CEngine *pEngine, AVLONG nTime = INT_MIN);
	AVLONG FastForward(CEngine *pEngine, AVLONG nTime);					// returns the earliest time that must be scanned before FF (usually < nTime)
	void RenderPassengers(AVLONG nPhase = 0);
	void Stop();

protected:
	virtual CPassenger *CreatePassenger(AVULONG nId);
	virtual CLift *CreateLift(AVULONG nId);

	friend class CProjectVis;
	friend class CAdVisuoLoader;
	friend class CTempLoader;
};
