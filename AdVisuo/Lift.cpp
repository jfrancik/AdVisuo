// Lift.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Lift.h"
#include "Passenger.h"
#include "Sim.h"

#include <freewill.h>
#include <fwaction.h>
#include "freewilltools.h"

using namespace std;

CLift::CLift(CSim *pSim, AVULONG nLift, AVULONG nDecks) : CLiftBase(pSim, nLift, nDecks)
{
	m_pActionTick = NULL;
}

CLift::~CLift()
{
	if (m_pActionTick) m_pActionTick->Release();
}

////////////////////////////////////////////////////////////////////////////////
// Animation Functions

void CLift::MoveTo(AVVECTOR &v)
{
	IKineNode *pBone = GetSim()->GetBuilding()->GetLiftBone(GetId())->GetNode();
	ITransform *p = NULL;
	pBone->GetLocalTransformRef(&p);
	p->FromTranslationVector((FWVECTOR*)&v);
	p->Release();
	pBone->Invalidate();
}

void CLift::MoveToInitialPosition()
{
	JOURNEY *pJourney = GetJourney(0);
	if (!pJourney) return;
	MoveTo(GetSim()->GetBuilding()->GetCarPos(pJourney->m_shaftFrom, pJourney->m_floorFrom));
}

void CLift::AnimateToInitialPosition(AVULONG nShaftFrom, AVULONG nStoreyFrom, AVULONG timeStart)
{
	IKineNode *pBone = GetSim()->GetBuilding()->GetLiftBone(GetId())->GetNode();
	AVVECTOR vFrom = GetSim()->GetBuilding()->GetCarPos(nShaftFrom, nStoreyFrom);
	IAction *pAction = NULL;
	pAction = (IAction*)FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"MoveTo", m_pActionTick, timeStart-1, 0, pBone, vFrom.x, vFrom.y, vFrom.z);
}

void CLift::AnimateDoor(AVULONG nShaft, AVULONG nStorey, bool bOpen, AVULONG timeStart, AVULONG timeDuration)
{
	AVFLOAT nDist = GetSim()->GetBuilding()->GetShaft(nShaft)->GetBoxDoor().Width() / 2.0f - 0.1f;
	if (!bOpen) nDist = -nDist;

	IKineNode *pDoorLI = GetSim()->GetBuilding()->GetLiftDoor(GetId(), 0)->GetNode();
	IKineNode *pDoorRI = GetSim()->GetBuilding()->GetLiftDoor(GetId(), 1)->GetNode();
	IKineNode *pDoorLX = GetSim()->GetBuilding()->GetShaftDoor(nStorey, nShaft, 0)->GetNode();
	IKineNode *pDoorRX = GetSim()->GetBuilding()->GetShaftDoor(nStorey, nShaft, 1)->GetNode();

	if (pDoorLI) FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Move", m_pActionTick, timeStart, timeDuration, pDoorLI, -nDist, 0, 0);
	if (pDoorRI) FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Move", m_pActionTick, timeStart, timeDuration, pDoorRI, nDist, 0, 0);
	if (pDoorLX) FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Move", m_pActionTick, timeStart, timeDuration, pDoorLX, -nDist, 0, 0);
	if (pDoorRX) FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Move", m_pActionTick, timeStart, timeDuration, pDoorRX, nDist, 0, 0);
}

void CLift::AnimateJourney(AVULONG nShaftTo, AVULONG nStoreyTo, AVULONG timeStart, AVULONG timeDuration)
{
	IKineNode *pBone = GetSim()->GetBuilding()->GetLiftBone(GetId())->GetNode();
	AVVECTOR vTo   = GetSim()->GetBuilding()->GetCarPos(nShaftTo, nStoreyTo);
	IAction *pAction = NULL;
	pAction = (IAction*)FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"MoveTo", m_pActionTick, timeStart, timeDuration, pBone, vTo.x, vTo.y, vTo.z);
	pAction->SetParabolicEnvelopeT(2000, 2000);
//	FWFLOAT s = sqrt(vTo.z*vTo.z+vTo.y*vTo.y+vTo.x*vTo.x)/1000.0f/0.04f;
//	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 1.0f, 2.5f/s, 1.0f/s, 1.5f/s);
}

void CLift::Go(JOURNEY &j)
{
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateToInitialPosition(j.m_shaftFrom, j.m_floorFrom, j.FirstOpenedTime() - GetSim()->GetTimeLowerBound());

	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
		for (AVULONG iCycle = 0; iCycle < j.m_doorcycles[iDeck].size(); iCycle++)
		{
			AnimateDoor(j.m_shaftFrom, j.m_floorFrom+iDeck, true,  j.m_doorcycles[iDeck][iCycle].m_timeOpen  - GetSim()->GetTimeLowerBound(), j.m_doorcycles[iDeck][iCycle].m_durationOpen);
			AnimateDoor(j.m_shaftFrom, j.m_floorFrom+iDeck, false, j.m_doorcycles[iDeck][iCycle].m_timeClose - GetSim()->GetTimeLowerBound(), j.m_doorcycles[iDeck][iCycle].m_durationClose);
		}

	// journey
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateJourney(j.m_shaftTo, j.m_floorTo, j.m_timeGo - GetSim()->GetTimeLowerBound(), j.m_timeDest - j.m_timeGo);
}

int _callback_journey(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam);

void CLift::Play(IAction *pActionTick, AVLONG nTime)
{
	// store the tick source
	if (m_pActionTick) m_pActionTick->Release();
	m_pActionTick = pActionTick;
	if (m_pActionTick) m_pActionTick->AddRef();

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

			timeStart -= GetSim()->GetTimeLowerBound();
			IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Generic", m_pActionTick, timeStart, 0);
			pAction->SetHandleEventHook(_callback_journey, i, (void*)this);

			AVULONG timeFinish = pJ->m_timeDest;
			ASSERT(timeFinish != UNDEF);
		}
	}
}

AVLONG CLift::FastForward(IAction *pActionTick, AVLONG nTime)
{
	// store the tick source
	if (m_pActionTick) m_pActionTick->Release();
	m_pActionTick = pActionTick;
	if (m_pActionTick) m_pActionTick->AddRef();

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

			timeStart -= GetSim()->GetTimeLowerBound();
			IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Generic", m_pActionTick, timeStart, 0);
			pAction->SetHandleEventHook(_callback_journey, i, (void*)this);

			nEarliestTime = min(nEarliestTime, (LONG)pJ->FirstOpenTime());
		}
	}
	return nEarliestTime;
}

int _callback_journey(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		CLift *pLift = (CLift*)pParam;
		pLift->Go(nParam);
	}
	return S_OK;
}
