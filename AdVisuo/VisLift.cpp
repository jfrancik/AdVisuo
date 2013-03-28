// Lift.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisElem.h"
#include "VisLiftGroup.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "VisSim.h"
#include "Engine.h"

#include <freewill.h>
#include <fwaction.h>
#include "freewilltools.h"

using namespace std;

CLiftVis::CLiftVis(CSimVis *pSim, AVULONG nLift, AVULONG nDecks) : CLift(pSim, nLift, nDecks)
{
	m_pEngine = NULL;
}

CLiftVis::~CLiftVis()
{
}

////////////////////////////////////////////////////////////////////////////////
// Animation Functions

void CLiftVis::MoveTo(AVVECTOR &v)
{
	IKineNode *pBone = GetSim()->GetLiftGroup()->GetLiftElement(GetId())->GetBone();
	ITransform *p = NULL;
	pBone->GetLocalTransformRef(&p);
	p->FromTranslationVector((FWVECTOR*)&v);
	p->Release();
	pBone->Invalidate();
}

void CLiftVis::MoveToInitialPosition()
{
	JOURNEY *pJourney = GetJourney(0);
	if (!pJourney) return;
	MoveTo(GetSim()->GetLiftGroup()->GetCarPos(pJourney->m_shaftFrom, pJourney->m_floorFrom));
}

void CLiftVis::AnimateToInitialPosition(CEngine *pEngine, AVULONG nShaftFrom, AVULONG nStoreyFrom, AVULONG timeStart)
{
	IKineNode *pBone = GetSim()->GetLiftGroup()->GetLiftElement(GetId())->GetBone();
	AVVECTOR vFrom = GetSim()->GetLiftGroup()->GetCarPos(nShaftFrom, nStoreyFrom);

	ANIM_HANDLE a = pEngine->StartAnimation(timeStart-1);
	a = pEngine->MoveTo(a, pBone, 1, vFrom.x, vFrom.y, vFrom.z);
}

void CLiftVis::AnimateDoor(CEngine *pEngine, AVULONG nShaft, AVULONG nStorey, bool bOpen, AVULONG timeStart, AVULONG timeDuration)
{
	AVFLOAT nDist = GetSim()->GetLiftGroup()->GetShaft(nShaft)->GetBoxDoor().Width() / 2.0f - 0.1f;
	if (!bOpen) nDist = -nDist;

	IKineNode *pDoorLI = GetSim()->GetLiftGroup()->GetLiftDoor(GetId(), 0)->GetBone();
	IKineNode *pDoorRI = GetSim()->GetLiftGroup()->GetLiftDoor(GetId(), 1)->GetBone();
	IKineNode *pDoorLX = GetSim()->GetLiftGroup()->GetShaftDoor(nStorey, nShaft, 0)->GetBone();
	IKineNode *pDoorRX = GetSim()->GetLiftGroup()->GetShaftDoor(nStorey, nShaft, 1)->GetBone();

	if (pDoorLI) { ANIM_HANDLE a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorLI, timeDuration, -nDist, 0, 0); }
	if (pDoorRI) { ANIM_HANDLE a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorRI, timeDuration, nDist, 0, 0);  }
	if (pDoorLX) { ANIM_HANDLE a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorLX, timeDuration, -nDist, 0, 0); }
	if (pDoorRX) { ANIM_HANDLE a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorRX, timeDuration, nDist, 0, 0);  }
}

void CLiftVis::AnimateJourney(CEngine *pEngine, AVULONG nShaftTo, AVULONG nStoreyTo, AVULONG timeStart, AVULONG timeDuration)
{
	IKineNode *pBone = GetSim()->GetLiftGroup()->GetLiftElement(GetId())->GetBone();
	AVVECTOR vTo   = GetSim()->GetLiftGroup()->GetCarPos(nShaftTo, nStoreyTo);

	ANIM_HANDLE a = pEngine->StartAnimation(timeStart);
	a = pEngine->MoveTo(a, pBone, timeDuration, vTo.x, vTo.y, vTo.z);
	a = pEngine->SetParabolicEnvelopeT(a, 2000, 2000);

	//	FWFLOAT s = sqrt(vTo.z*vTo.z+vTo.y*vTo.y+vTo.x*vTo.x)/1000.0f/0.04f;
//	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 1.0f, 2.5f/s, 1.0f/s, 1.5f/s);
}

void CLiftVis::Go(JOURNEY &j)
{
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateToInitialPosition(m_pEngine, j.m_shaftFrom, j.m_floorFrom, j.FirstOpenedTime());

	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
		for (AVULONG iCycle = 0; iCycle < j.m_doorcycles[iDeck].size(); iCycle++)
		{
			AnimateDoor(m_pEngine, j.m_shaftFrom, j.m_floorFrom+iDeck, true,  j.m_doorcycles[iDeck][iCycle].m_timeOpen , j.m_doorcycles[iDeck][iCycle].m_durationOpen);
			AnimateDoor(m_pEngine, j.m_shaftFrom, j.m_floorFrom+iDeck, false, j.m_doorcycles[iDeck][iCycle].m_timeClose, j.m_doorcycles[iDeck][iCycle].m_durationClose);
		}

	// journey
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateJourney(m_pEngine, j.m_shaftTo, j.m_floorTo, j.m_timeGo, j.m_timeDest - j.m_timeGo);
}

int _callback_journey(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam);

void CLiftVis::Play(CEngine *pEngine, AVLONG nTime)
{
	m_pEngine = pEngine;

	// Plan journeys
	for (AVULONG i = 0; i < GetJourneyCount(); i++)
	{
		JOURNEY *pJ = GetJourney(i);
		if ((AVLONG)pJ->m_timeGo >= nTime)
		{
			if (i == 0) MoveToInitialPosition();	// do it once in every cycle!

			ASSERT(pJ->m_floorFrom != UNDEF && pJ->m_floorTo != UNDEF);

			AVULONG timeStart = pJ->FirstOpenTime();
			if (timeStart == UNDEF) timeStart = pJ->m_timeGo;
			ASSERT(timeStart != UNDEF);

			ANIM_HANDLE a = pEngine->StartAnimation(timeStart);
			a = pEngine->SetAnimationCB(a, _callback_journey, i, (void*)this);

			AVULONG timeFinish = pJ->m_timeDest;
			ASSERT(timeFinish != UNDEF);
		}
	}
}

AVLONG CLiftVis::FastForward(CEngine *pEngine, AVLONG nTime)
{
	m_pEngine = pEngine;

	AVLONG nEarliestTime = 0x7FFFFFFF;

	// Plan journeys
	for (AVULONG i = 0; i < GetJourneyCount(); i++)
	{
		JOURNEY *pJ = GetJourney(i);
		if ((AVLONG)pJ->m_timeDest >= nTime)
		{
			if (i == 0) MoveToInitialPosition();	// do it once in every cycle!

			ASSERT(pJ->m_floorFrom != UNDEF && pJ->m_floorTo != UNDEF);

			AVULONG timeStart = pJ->FirstOpenTime();
			if (timeStart == UNDEF) timeStart = pJ->m_timeGo;
			ASSERT(timeStart != UNDEF);

			ANIM_HANDLE a = pEngine->StartAnimation(timeStart);
			a = pEngine->SetAnimationCB(a, _callback_journey, i, (void*)this);

			nEarliestTime = min(nEarliestTime, (LONG)pJ->FirstOpenTime());
		}
	}
	return nEarliestTime;
}

int _callback_journey(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		CLiftVis *pLift = (CLiftVis*)pParam;
		pLift->Go(nParam);
	}
	return S_OK;
}
