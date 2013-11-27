// Lift.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisElem.h"
#include "VisLiftGroup.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "VisSim.h"
#include "Engine.h"

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
	GetSim()->GetLiftGroup()->GetLiftElement(GetId())->MoveTo(v);
}

void CLiftVis::MoveToInitialPosition()
{
	JOURNEY *pJourney = GetJourney(0);
	if (!pJourney) return;
	MoveTo(GetSim()->GetLiftGroup()->GetCarPos(pJourney->m_shaftFrom, pJourney->m_floorFrom));
}

void CLiftVis::AnimateToInitialPosition(CEngine *pEngine, AVULONG nShaftFrom, AVULONG nStoreyFrom, AVULONG timeStart)
{
	HBONE pBone = GetSim()->GetLiftGroup()->GetLiftElement(GetId())->GetBone();
	AVVECTOR vFrom = GetSim()->GetLiftGroup()->GetCarPos(nShaftFrom, nStoreyFrom);

	HACTION a = pEngine->StartAnimation(timeStart-1);
	a = pEngine->MoveTo(a, pBone, 1, vFrom.x, vFrom.y, vFrom.z);
}

void CLiftVis::AnimateDoor(CEngine *pEngine, AVULONG nShaft, AVULONG nStorey, AVULONG nDeck, AVLONG timeStart, bool bOpen)
{
	AVFLOAT nDist = GetSim()->GetLiftGroup()->GetShaft(nShaft)->GetBoxDoor().Width() / 2.0f - 0.1f;
	if (!bOpen) nDist = -nDist;
	if (GetSHAFT()->GetShaftLine() == 1) nDist = -nDist;

	AVLONG timeDuration = bOpen ? GetSHAFT()->GetOpeningTime() : GetSHAFT()->GetClosingTime();

	CElemVis *pElemLI = GetSim()->GetLiftGroup()->GetLiftDoor(GetId(), nDeck, 0);
	CElemVis *pElemRI = GetSim()->GetLiftGroup()->GetLiftDoor(GetId(), nDeck, 1);
	CElemVis *pElemLX = GetSim()->GetLiftGroup()->GetShaftDoor(nStorey, nShaft, 0);
	CElemVis *pElemRX = GetSim()->GetLiftGroup()->GetShaftDoor(nStorey, nShaft, 1);

	HBONE pDoorLI = pElemLI ? pElemLI->GetBone() : NULL;
	HBONE pDoorRI = pElemRI ? pElemRI->GetBone() : NULL;
	HBONE pDoorLX = pElemLX ? pElemLX->GetBone() : NULL;
	HBONE pDoorRX = pElemRX ? pElemRX->GetBone() : NULL;

	if (pDoorLI) { HACTION a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorLI, timeDuration, -nDist, 0, 0); }
	if (pDoorRI) { HACTION a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorRI, timeDuration, nDist, 0, 0);  }
	if (pDoorLX) { HACTION a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorLX, timeDuration, -nDist, 0, 0); }
	if (pDoorRX) { HACTION a = pEngine->StartAnimation(timeStart); a = pEngine->Move(a, pDoorRX, timeDuration, nDist, 0, 0);  }
}

void CLiftVis::AnimateJourney(CEngine *pEngine, AVULONG nShaftTo, AVULONG nStoreyTo, AVULONG timeStart, AVULONG timeDuration)
{
	HBONE pBone = GetSim()->GetLiftGroup()->GetLiftElement(GetId())->GetBone();
	AVVECTOR vTo   = GetSim()->GetLiftGroup()->GetCarPos(nShaftTo, nStoreyTo);

	HACTION a = pEngine->StartAnimation(timeStart);
	a = pEngine->MoveTo(a, pBone, timeDuration, vTo.x, vTo.y, vTo.z);
	a = pEngine->SetEnvelope(a, 2000, 2000);

	//	FWFLOAT s = sqrt(vTo.z*vTo.z+vTo.y*vTo.y+vTo.x*vTo.x)/1000.0f/0.04f;
//	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 1.0f, 2.5f/s, 1.0f/s, 1.5f/s);
}

void CLiftVis::Go(JOURNEY &j)
{
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateToInitialPosition(m_pEngine, j.m_shaftFrom, j.m_floorFrom, j.FirstOpenTime() + GetSHAFT()->GetOpeningTime());

	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
		for (AVULONG iCycle = 0; iCycle < j.m_doorcycles[iDeck].size(); iCycle++)
		{
			AnimateDoor(m_pEngine, j.m_shaftFrom, j.m_floorFrom+iDeck, iDeck, j.m_doorcycles[iDeck][iCycle].m_timeOpen, true);
			AnimateDoor(m_pEngine, j.m_shaftFrom, j.m_floorFrom+iDeck, iDeck, j.m_doorcycles[iDeck][iCycle].m_timeClose, false);
		}

	// journey
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateJourney(m_pEngine, j.m_shaftTo, j.m_floorTo, j.m_timeGo, j.m_timeDest - j.m_timeGo);
}

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

			HACTION a = pEngine->StartAnimation(timeStart);
			a = pEngine->SetAnimationListener(a, this, i);

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

			HACTION a = pEngine->StartAnimation(timeStart);
			a = pEngine->SetAnimationListener(a, this, i);

			nEarliestTime = min(nEarliestTime, (LONG)pJ->FirstOpenTime());
		}
	}
	return nEarliestTime;
}

