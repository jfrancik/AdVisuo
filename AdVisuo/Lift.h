// Lift.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include <Vector>

interface IAction;
class CSim;

class CLift : public CLiftBase
{
protected:
	IAction *m_pActionTick;			// FreeWill action

public:
	CLift(CSim *pSim, AVULONG nLiftId, AVULONG nDecks = 1);
	virtual ~CLift(void);

	CSim *GetSim()		{ return (CSim*)CLiftBase::GetSim(); }

// Operations

	void MoveTo(AVVECTOR &v);
	void MoveToInitialPosition();

	void AnimateToInitialPosition(AVULONG nShaftFrom, AVULONG nStoreyFrom, AVULONG timeStart);
	void AnimateDoor(AVULONG nShaft, AVULONG nStorey, bool bOpen, AVULONG timeStart, AVULONG timeDuration = 1000);
	void AnimateJourney(AVULONG nShaftTo, AVULONG nStoreyTo, AVULONG timeStart, AVULONG timeDuration);
	
	void Go(AVULONG i)				{ Go(*GetJourney(i)); }
	void Go(JOURNEY &j);

	void Play(IAction *pActionTick, AVLONG nTime = 0);
	AVLONG FastForward(IAction *pActionTick, AVLONG nTime);
};
