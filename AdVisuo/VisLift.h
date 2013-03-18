// Lift.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseSimClasses.h"

class CSimVis;
class CEngine;

class CLiftVis : public CLift
{
protected:
	CEngine *m_pEngine;			// FreeWill engine

public:
	CLiftVis(CSimVis *pSim, AVULONG nLiftId, AVULONG nDecks = 1);
	virtual ~CLiftVis(void);

	CSimVis *GetSim()		{ return (CSimVis*)CLift::GetSim(); }

// Operations

	void MoveTo(AVVECTOR &v);
	void MoveToInitialPosition();

	void AnimateToInitialPosition(CEngine *pEngine, AVULONG nShaftFrom, AVULONG nStoreyFrom, AVULONG timeStart);
	void AnimateDoor(CEngine *pEngine, AVULONG nShaft, AVULONG nStorey, bool bOpen, AVULONG timeStart, AVULONG timeDuration = 1000);
	void AnimateJourney(CEngine *pEngine, AVULONG nShaftTo, AVULONG nStoreyTo, AVULONG timeStart, AVULONG timeDuration);
	
	void Go(AVULONG i)				{ Go(*GetJourney(i)); }
	void Go(JOURNEY &j);

	void Play(CEngine *pEngine, AVLONG nTime = 0);
	AVLONG FastForward(CEngine *pEngine, AVLONG nTime);
};
