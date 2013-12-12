// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "SrvSim.h"
#include "_legacy_SrvLift.h"

using namespace dbtools;

bool CLiftSrv::legacy_ReportDifferences(CLiftSrv *p)
{
	static AVULONG nCount = 0;
	bool b = true;

	bool bDestination = ((AVULONG)(*GetSHAFT()->GetLiftGroup())[L"TrafficControlTypeId"]) == 2;

	auto iThis = m_journeys.begin();
	auto iThat = p->m_journeys.begin();

	if (m_journeys.size() != p->m_journeys.size())
	{
		#ifdef __ADV_DLL
		wcout << L"**** Size of Journey Collection Differs!!! ****" << endl;
		#endif
		b = false;
	}

	while (iThis != m_journeys.end())
	{
		JOURNEY &jThis = *iThis;
		JOURNEY &jThat = *iThat;

		auto id = jThis.m_id;

		if (jThis != jThat)
		{
			#ifdef __ADV_DLL
			if (jThis.m_id == jThat.m_id && jThis.m_floorFrom == jThat.m_floorFrom && jThis.m_floorTo == jThat.m_floorTo && jThis.m_timeGo == jThat.m_timeGo && jThis.m_timeDest == jThat.m_timeDest)
				wcout << jThis.m_id << L" " << jThis.m_floorFrom << L"->" << jThis.m_floorTo << " in " << jThis.m_timeGo << L"-" << jThis.m_timeDest << " (" << ++nCount << L")" << (bDestination ? L" - destination" : L" - conventional") << endl;
			else
			{
				wcout << "  old: " << jThis.m_id << L" " << jThis.m_floorFrom << L"->" << jThis.m_floorTo << " in " << jThis.m_timeGo << L"-" << jThis.m_timeDest << " (" <<   nCount << L")" << (bDestination ? L" - destination" : L" - conventional") << endl;
				wcout << "  new: " << jThat.m_id << L" " << jThat.m_floorFrom << L"->" << jThat.m_floorTo << " in " << jThat.m_timeGo << L"-" << jThat.m_timeDest << " (" << ++nCount << L")" << (bDestination ? L" - destination" : L" - conventional") << endl;
			}
			wcout << "  old: "; for (unsigned i = 0; i < jThis.m_doorcycles[0].size(); i++) wcout << jThis.m_doorcycles[0][i].m_timeOpen << " - " <<   jThis.m_doorcycles[0][i].m_timeClose << "  |  "; wcout << endl;
			wcout << "  new: "; for (unsigned i = 0; i < jThat.m_doorcycles[0].size(); i++) wcout << jThat.m_doorcycles[0][i].m_timeOpen << " - " <<   jThat.m_doorcycles[0][i].m_timeClose << "  |  "; wcout << endl;
			if (jThis.m_doorcycles[1].size() || jThat.m_doorcycles[1].size())
			{
				wcout << "  DECK 2:" << endl;
				wcout << "  old: "; for (unsigned i = 0; i < jThis.m_doorcycles[1].size(); i++) wcout << jThis.m_doorcycles[1][i].m_timeOpen << " - " <<   jThis.m_doorcycles[1][i].m_timeClose << "  |  "; wcout << endl;
				wcout << "  new: "; for (unsigned i = 0; i < jThat.m_doorcycles[1].size(); i++) wcout << jThat.m_doorcycles[1][i].m_timeOpen << " - " <<   jThat.m_doorcycles[1][i].m_timeClose << "  |  "; wcout << endl;
			}
			LogProgress(0);
			#endif
			b = false;
		}
		iThis++; iThat++;
	}

	return b;
}

DWORD CLiftSrv::legacy_Load(CLiftGroupSrv::LIFT *pLIFT, dbtools::CDataBase db, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration)
{
	SetDecks(pLIFT->GetShaft()->GetDeckCount());
	CSimLiftJourneySeq resolver(m_journeys);
	resolver.Run(db, GetId(), nLiftNativeId, nTrafficScenarioId, nIteration, pLIFT->GetShaft()->GetOpeningTime(), pLIFT->GetShaft()->GetClosingTime());
	return S_OK;
}

DWORD CLiftSrv::legacy_Adjust()
{
	////////////////////////////
	// Adjust part here!


	// Apply Time-Consistency Tests
	// and sequence the passengers departing/arriving...
	AVULONG TT = 700;								// transfer time
	DWORD dRes = S_OK;
	AVULONG nJourneys = GetJourneyCount();
	for (AVULONG i = 0; i < nJourneys; i++)
	{
		JOURNEY *pJourney = GetJourney(i);
		AVULONG nLiftFloor = pJourney->m_floorFrom;
		AVULONG nShaft = pJourney->m_shaftFrom;
		AVULONG timeOpen = pJourney->FirstOpenTime();
		AVULONG timeClose = pJourney->LastCloseTime() + GetSHAFT()->GetClosingTime();

		// transfer timelines
		AVULONG t[DECK_NUM];
		for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
			t[iDeck] = pJourney->FirstOpenTime(iDeck) + GetSHAFT()->GetOpeningTime();

		AVULONG myId = GetId();
		AVULONG nPassengers = GetSim()->GetPassengerCount();

		// adjust t for passengers getting off
		for (AVULONG j = 0; j < nPassengers; j++)
		{
			CPassengerSrv *pPassenger = GetSim()->GetPassenger(j);

			if (pPassenger->GetLiftId() != myId) continue;					// continue, passenger not on this lift
			AVULONG tUnload = pPassenger->GetUnloadTime();					// passenger unload time
			if (tUnload+1 < timeOpen || tUnload > timeClose + 1) continue;	// continue, passenger not on this journey
			AVULONG nPassengerFloor = pPassenger->GetDestFloor();			// passenger destination floor
			AVULONG nDeck = nPassengerFloor - nLiftFloor;					// passenger deck id

			// Error Checking
			if (nDeck >= GetDecks())
				return Log(ERROR_PASSENGER_FLOOR, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);

			tUnload = t[nDeck];

			// Warning Checking
			if (nPassengerFloor < nLiftFloor || nPassengerFloor >= nLiftFloor + DECK_NUM)
				dRes = Log(WARNING_PASSENGER_FLOOR, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			if (tUnload+1 < pJourney->FirstOpenTime(nDeck))
				dRes = Log(WARNING_PASSENGER_1, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			else if (tUnload+1 < pJourney->FirstOpenTime(nDeck) + GetSHAFT()->GetOpeningTime())
				dRes = Log(WARNING_PASSENGER_2, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			else if (tUnload > pJourney->LastCloseTime(nDeck)+1)
				dRes = Log(WARNING_PASSENGER_3, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			else if (tUnload > pJourney->LastCloseTime(nDeck) + GetSHAFT()->GetClosingTime() + 1)
				dRes = Log(WARNING_PASSENGER_4, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);

			pPassenger->SetDeck(nDeck);

			t[nDeck] = max(t[nDeck], tUnload);
			pPassenger->SetUnloadTime(tUnload);
			t[nDeck] += TT;
		}

		// allocate timing for getting on...
		for (AVULONG j = 0; j < nPassengers; j++)
		{
			CPassengerSrv *pPassenger = GetSim()->GetPassenger(j);

			if (pPassenger->GetLiftId() != myId) continue;					// continue, passenger not on this lift
			AVULONG tLoad = pPassenger->GetLoadTime();						// load time
			if (tLoad+1 < timeOpen || tLoad > timeClose + 1) continue;		// continue, passenger not on this journey
			AVULONG nPassengerFloor = pPassenger->GetArrivalFloor();		// passenger arrival floor
			AVULONG nDeck = nPassengerFloor - nLiftFloor;					// passenger deck id

			// Error Checking
			if (nDeck >= GetDecks())
				return Log(ERROR_PASSENGER_FLOOR, false, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);

			tLoad = max(t[nDeck], tLoad);

			// Warning Checking
			if (nPassengerFloor < nLiftFloor || nPassengerFloor >= nLiftFloor + DECK_NUM)
				dRes = Log(WARNING_PASSENGER_FLOOR, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			if (tLoad+1 < pJourney->FirstOpenTime(nDeck))
				dRes = Log(WARNING_PASSENGER_1, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			else if (tLoad+1 < pJourney->FirstOpenTime(nDeck) + GetSHAFT()->GetOpeningTime())
				dRes = Log(WARNING_PASSENGER_2, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			else if (tLoad > pJourney->LastCloseTime(nDeck)+1)
				dRes = Log(WARNING_PASSENGER_3, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			else if (tLoad > pJourney->LastCloseTime(nDeck) + GetSHAFT()->GetClosingTime() + 1)
				dRes = Log(WARNING_PASSENGER_4, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);

			pPassenger->SetDeck(nDeck);
			pPassenger->SetShaftId(nShaft);

			t[nDeck] = max(t[nDeck], tLoad);
			pPassenger->SetLoadTime(t[nDeck]);
			t[nDeck] += TT;
		}
	}

	return dRes == S_OK ? S_OK : WARNING_GENERIC;
}




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
	door[iDeck].m_timeOpen = time;
}

void CSimLiftJourneySeq::RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose)
{
	duration = timeToClose;

	ASSERT(door[iDeck].m_timeOpen != UNDEF);
	door[iDeck].m_timeClose = time;
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

