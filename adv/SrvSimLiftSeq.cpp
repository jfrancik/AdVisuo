// SimLoader.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvSimLiftSeq.h"

//////////////////////////////////////////////////////////////////////////////////
// CSimLiftJourneySeq

AVULONG g_nJourneyId = 0;

CSimLiftJourneySeq::CSimLiftJourneySeq(std::vector<JOURNEY> &J) : journeys(J), pJourney(NULL)
{
}

void CSimLiftJourneySeq::Run(dbtools::CDataBase db, AVULONG nLiftId, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration, AVULONG timeToOpen, AVULONG timeToClose)
{
	g_nJourneyId = (g_nJourneyId / 1000) * 1000 + 1000;

	stCar = CAR_STOP;
	for (AVULONG i = 0; i < DECK_NUM; i++)
	{
		stDoor[i] = DOOR_CLOSED;
		lt[i] = 0;
	}

	journeys.push_back(JOURNEY());
	pJourney = &journeys.back();
	pJourney->m_id = g_nJourneyId++;

	// Lift Logs
	dbtools::CDataBase::SELECT selLiftLogs = db.select(L"SELECT * FROM LiftLogs WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);
	dbtools::CDataBase::SELECT selDeckLogs = db.select(L"SELECT * FROM DeckLogs WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);
	for ( ; selLiftLogs; selLiftLogs++)
	{
		AVULONG t = selLiftLogs[L"Time"].msec();

		while (selDeckLogs && (float)selDeckLogs[L"Time"] <= (float)selLiftLogs[L"Time"])
		{
			AVULONG t = selDeckLogs[L"Time"].msec();
			RecordDeckLog(t, (AVULONG)selDeckLogs[L"Deck"] - 1, (enum DOOR)(int)selDeckLogs[L"DoorStateId"], timeToOpen, timeToClose);
			selDeckLogs++;
		}
		RecordLiftLog(t, (enum CAR)(int)selLiftLogs[L"LiftStateId"], selLiftLogs[L"CurrentFloor"], selLiftLogs[L"DestinationFloor"], nLiftId, timeToOpen, timeToClose);
	}
	journeys.pop_back();
}

void CSimLiftJourneySeq::RecordOpen(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToOpen)
{
	duration = timeToOpen;

	ASSERT(door[iDeck].m_timeOpen == UNDEF);
	door[iDeck].m_timeOpen = time; door[iDeck].m_durationOpen = min(duration, timeToOpen);
}

void CSimLiftJourneySeq::RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose)
{
	duration = timeToClose;

	ASSERT(door[iDeck].m_timeOpen != UNDEF);
	door[iDeck].m_timeClose = time; door[iDeck].m_durationClose = min(duration, timeToClose);
	pJourney->m_doorcycles[iDeck].push_back(door[iDeck]);
	door[iDeck].reset();
}

void CSimLiftJourneySeq::RecordDeckLog(AVULONG t, int iDeck, enum DOOR evDoor, AVULONG timeToOpen, AVULONG timeToClose)
{
	if (evDoor == stDoor[iDeck])
		return;

	switch (evDoor)
	{
	case DOOR_CLOSED:
		if      (stDoor[iDeck] == DOOR_OPENING)	{ AVULONG t2 = (t - lt[iDeck]) / 2; 
												RecordOpen(iDeck, lt[iDeck], t2, timeToOpen); 
												RecordClose(iDeck, t-t2, t2, timeToClose); }
		else if (stDoor[iDeck] == DOOR_CLOSING)	RecordClose(iDeck, lt[iDeck], t - lt[iDeck], timeToClose);
		else /* stDoor[iDeck] == DOOR_OPENED */	RecordClose(iDeck, t, timeToClose, timeToClose);
		break;
	case DOOR_OPENING:
		ASSERT(stDoor[iDeck] != DOOR_OPENED);
		if (stDoor[iDeck] == DOOR_CLOSING)		RecordClose(iDeck, lt[iDeck], t - lt[iDeck], timeToClose);
		break;
	case DOOR_OPENED:
		if (stDoor[iDeck] == DOOR_OPENING)		RecordOpen(iDeck, lt[iDeck], t - lt[iDeck], timeToOpen);
		else if (stDoor[iDeck] == DOOR_CLOSING)	{ AVULONG t2 = (t - lt[iDeck]) / 2; 
												RecordClose(iDeck, lt[iDeck], t2, timeToClose); 
												RecordOpen(iDeck, t-t2, t2, timeToOpen); }
		else /* stDoor[iDeck] == DOOR_CLOSED */ RecordOpen(iDeck, t, timeToOpen, timeToOpen);
		break;
	case DOOR_CLOSING:
		ASSERT(stDoor[iDeck] != DOOR_CLOSED);
		if (stDoor[iDeck] == DOOR_OPENING)		RecordOpen(iDeck, lt[iDeck], t - lt[iDeck], timeToOpen); 
		break;
	}
	stDoor[iDeck] = evDoor;
	lt[iDeck] = t;
}

void CSimLiftJourneySeq::RecordLiftLog(AVULONG t, enum CAR evCar, AVULONG floorFrom, AVULONG floorTo, AVULONG shaft, AVULONG timeToOpen, AVULONG timeToClose)
{
	if (evCar != CAR_MOVE && evCar != CAR_SHAFT_MOVE)
		evCar = CAR_STOP;	// make no difference between idle, hall call and motor

	if (evCar == stCar)
		return;

	switch (evCar)
	{
	case CAR_MOVE:
	case CAR_SHAFT_MOVE:
		if (stCar != CAR_STOP)
		{
			// if change MOVE => SHAFT or SHAFT => MOVE
			pJourney->m_shaftTo = shaft;
			pJourney->m_timeDest = t;
			pJourney->m_floorTo = floorTo;
			ASSERT(pJourney->m_floorFrom != UNDEF && pJourney->m_floorTo != UNDEF);
			ASSERT(pJourney->m_timeDest != UNDEF && pJourney->m_timeGo != UNDEF);
			journeys.push_back(JOURNEY());	// start a new journey
			pJourney = &journeys.back();
			pJourney->m_id = g_nJourneyId++;
		}
		pJourney->m_shaftFrom = shaft;
		pJourney->m_floorFrom = floorFrom;
		pJourney->m_timeGo = t;
		for (AVULONG i = 0; i < DECK_NUM; i++) 
		{
			if (stDoor[i] == DOOR_CLOSING) { RecordClose(i, lt[i], t - lt[i], timeToClose); }
			if (stDoor[i] == DOOR_OPENING) { RecordOpen (i, lt[i], t - lt[i], timeToOpen); }
			stDoor[i] = DOOR_CLOSED;
		}
		break;
	case CAR_STOP:
		pJourney->m_shaftTo = shaft;
		pJourney->m_timeDest = t;
		//pJourney->m_floorTo = floorTo;
		//if (stCar == CAR_SHAFT_MOVE) pJourney->m_floorTo = floorFrom;
		pJourney->m_floorTo = floorFrom;
		ASSERT(pJourney->m_floorFrom != UNDEF && pJourney->m_floorTo != UNDEF);
		ASSERT(pJourney->m_timeDest != UNDEF && pJourney->m_timeGo != UNDEF);
		journeys.push_back(JOURNEY());	// start a new journey
		pJourney = &journeys.back();
		pJourney->m_id = g_nJourneyId++;
		break;
	}
	stCar = evCar;
}

