// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "SrvSim.h"
#include "SrvSimLiftSeq.h"

using namespace dbtools;

CLiftSrv::CLiftSrv(CSimSrv *pSim, AVULONG nLift, AVULONG nDecks) : CLift(pSim, nLift, nDecks)
{
}

bool CLiftSrv::Comp(CLiftSrv *p)
{
	bool b = true;

	std::vector<JOURNEY> &m_jThis = m_journeys;
	std::vector<JOURNEY> &m_jThat = p->m_journeys;

	auto iThis = m_jThis.begin();
	auto iThat = m_jThat.begin();
	while (iThis != m_jThis.end())
	{
		if (*iThis != *iThat)
		{
			b = false;
			*iThis == *iThat;
		}
		
		iThis++; iThat++;
	}

	return b;
}

DWORD CLiftSrv::Load2(dbtools::CDataBase db, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration,
						std::vector<CPassenger*> &collUnloading, std::vector<CPassenger*> &collLoading)
{
	SetDecks(GetSHAFT()->GetDeckCount());

	// collect some data...
	CLiftGroup::SHAFT *pShaft = GetSHAFT();
	AVLONG nOpeningTime = pShaft->GetOpeningTime();
	AVLONG nClosingTime = pShaft->GetClosingTime();
	AVLONG nLoadingTime = pShaft->GetLoadingTime();
	AVLONG nUnloadingTime = pShaft->GetUnloadingTime();
	AVLONG nPreOpeningTime = pShaft->GetPreOpeningTime();
	AVLONG nMotorStartDelayTime = pShaft->GetMotorStartDelayTime();
	AVULONG nReopenings = pShaft->GetReopenings();

	static AVULONG nJourneyId = 0;
	nJourneyId = (nJourneyId / 1000) * 1000 + 1000;

	// iterators for the passenger collections
	std::vector<CPassenger*>::iterator iUnloading = collUnloading.begin();
	std::vector<CPassenger*>::iterator iLoading = collLoading.begin();

	// Database Query
	dbtools::CDataBase::SELECT selLiftStops = db.select(L"SELECT * FROM LiftStops WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);

	// first journey
	m_journeys.push_back(JOURNEY());
	JOURNEY *pJourney = &m_journeys.back();
	pJourney->m_id = nJourneyId++;
	pJourney->m_floorFrom = selLiftStops[L"Floor"];
	pJourney->m_timeGo = selLiftStops[L"Time"].msec() + selLiftStops[L"Duration"].msec();
	selLiftStops++;

	AVLONG nTimeOpen = -1, nTimeClose, nTimeClose2;

	for ( ; selLiftStops; selLiftStops++)
	{
		// data from lift stops
		AVULONG nFloor = selLiftStops[L"Floor"];
		AVLONG nTime = selLiftStops[L"Time"].msec();
		AVLONG nDuration = selLiftStops[L"Duration"].msec();

		// dwell time
		AVLONG nDwellTime = pShaft->GetDwellTime(pJourney->m_floorFrom);

		if (nFloor == pJourney->m_floorFrom)
		{
			// if still on the same floor - postpone, just update the time
			pJourney->m_timeGo = nTime + nDuration;

			if (nDuration > nMotorStartDelayTime)
				if (nTimeOpen < 0)
				{
					nTimeOpen = nTime;	// initially plan open cycle
					nTimeClose = nTime + nOpeningTime + nDwellTime - nPreOpeningTime;
					nTimeClose2 = nTimeClose;	// nTime + nDuration;
				}
				else
				{
					// REOPENING!?
					JOURNEY::DOOR dc;
					dc.m_timeOpen = nTimeOpen;
					dc.m_durationOpen = nOpeningTime;
					dc.m_timeClose = nTimeClose;
					dc.m_durationClose = nClosingTime;
					pJourney->m_doorcycles[0].push_back(dc);

//					double f = (double)(nTime - nTimeClose) / (nOpeningTime + nClosingTime);
//					f = min(f, 1.0);

//					nTimeOpen = nTime - f * nOpeningTime;
//					nTimeClose = nTime + nDwellTime;

					nTimeOpen = nTime;	// initially plan open cycle
					nTimeClose = nTime + nOpeningTime + nDwellTime - nPreOpeningTime;
					nTimeClose2 = nTimeClose;
				}

		}
		else
		{
			// a new stop - finish the journey here
			pJourney->m_shaftFrom = pJourney->m_shaftTo = GetId();
			pJourney->m_floorTo = nFloor;
			pJourney->m_timeDest = nTime;

			while (iUnloading != collUnloading.end() && (*iUnloading)->GetUnloadTime() + 10 < (AVLONG)pJourney->m_timeGo)
			{
				LONG t = (*iUnloading)->GetUnloadTime();
				if (nTimeOpen < 0) 
				{
					nTimeOpen = t - nOpeningTime;
					nTimeClose = t + nDwellTime;
					nTimeClose2 = nTimeClose;
				}
				nTimeClose = max(nTimeClose, t + nUnloadingTime);
				iUnloading++;
			}

			while (iLoading != collLoading.end() && (*iLoading)->GetLoadTime() + 10 < (AVLONG)pJourney->m_timeGo)
			{
				LONG t = (*iLoading)->GetLoadTime();
				LONG tarr = (*iLoading)->GetArrivalTime();

				if (nTimeOpen < 0) 
				{
					nTimeOpen = t - nOpeningTime;
					nTimeClose = t + nDwellTime;
					nTimeClose2 = nTimeClose;
				}
				if (tarr < nTimeClose)
					nTimeClose = max(nTimeClose, t + nLoadingTime);
				else
				if (tarr < nTimeClose2)
				{
					// reopening
					JOURNEY::DOOR dc;
					dc.m_timeOpen = nTimeOpen;
					dc.m_durationOpen = nOpeningTime;
					dc.m_timeClose = nTimeClose;
					dc.m_durationClose = nClosingTime;
					pJourney->m_doorcycles[0].push_back(dc);

					double f = (double)(t - nTimeClose) / (nOpeningTime + nClosingTime);
					f = min(f, 1.0);

					nTimeOpen = t - f * nOpeningTime;
					nTimeClose = t + nDwellTime;
					nTimeClose2 = nTimeClose;
				}
				else
				{
					// reopening
					JOURNEY::DOOR dc;
					dc.m_timeOpen = nTimeOpen;
					dc.m_durationOpen = nOpeningTime;
					dc.m_timeClose = nTimeClose;
					dc.m_durationClose = nClosingTime;
					pJourney->m_doorcycles[0].push_back(dc);

					double f = (double)(t - nTimeClose) / (nOpeningTime + nClosingTime);
					f = min(f, 1.0);

					nTimeOpen = t - f * nOpeningTime;
					nTimeClose = t + nDwellTime;
					nTimeClose2 = nTimeClose;
				}

				iLoading++;
			}

	//		ASSERT(nTimeOpen + 10 >= nLiftArrivalTime - nPreOpeningTime);
	//		ASSERT(nTimeClose + nClosingTime + nMotorStartDelay <= (AVLONG)J.m_timeGo + 10);

			// TO DO:
			// 1. Find out discrepancies
			// 2. Add more assertions
			// 3. Reopenings
			// 4. Double deckers
//			DON'T COMPILE WITHOUT READING THIS!!!

			if (nTimeOpen >= 0)
			{
				JOURNEY::DOOR dc;
				dc.m_timeOpen = nTimeOpen;
				dc.m_durationOpen = nOpeningTime;
				dc.m_timeClose = nTimeClose;
				dc.m_durationClose = nClosingTime;
				pJourney->m_doorcycles[0].push_back(dc);
			}

			// Verification of the journey
			if (nTimeOpen >= 0 && nTimeClose + nClosingTime > pJourney->m_timeGo + nMotorStartDelayTime)
				wcerr << L"PROBLEM!!! id = " << pJourney->m_id << L"  TrafficScenarioId = " << (ULONG)(*GetSim())[L"TrafficScenarioId"] << L"  LiftId = " << (ULONG)(*this->GetSHAFT())[L"LiftId"] << endl;

			// start a new journey from now
			m_journeys.push_back(JOURNEY());
			pJourney = &m_journeys.back();
			pJourney->m_id = nJourneyId++;
			pJourney->m_floorFrom = nFloor;
			pJourney->m_timeGo = nTime + nDuration;

			nDwellTime = (AVLONG)pShaft->GetDwellTime(pJourney->m_floorFrom);

			if (nDuration == 0)
				nTimeOpen = -1;		// idle cycle - we don't know when the doors open
			else
			{
				nTimeOpen = nTime;	// initially plan open cycle
				nTimeClose = nTime + nOpeningTime + nDwellTime - nPreOpeningTime;
				nTimeClose2 = nTime + nDuration - nClosingTime - nMotorStartDelayTime;
			}

			//if (rand() % 500 == 1) pJourney->m_timeGo += 100;
		}
	}
	m_journeys.pop_back();
	
	return S_OK;
}

DWORD CLiftSrv::Load(CLiftGroupSrv::LIFT *pLIFT, dbtools::CDataBase db, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG nIteration)
{
	SetDecks(pLIFT->GetShaft()->GetDeckCount());
	CSimLiftJourneySeq resolver(m_journeys);
	resolver.Run(db, GetId(), nLiftNativeId, nTrafficScenarioId, nIteration, pLIFT->GetShaft()->GetOpeningTime(), pLIFT->GetShaft()->GetClosingTime());
	return S_OK;
}

DWORD CLiftSrv::Adjust()
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
		AVULONG timeClose = pJourney->LastClosedTime();

		// transfer timelines
		AVULONG t[DECK_NUM];
		for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
			t[iDeck] = pJourney->FirstOpenedTime(iDeck);

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
			else if (tUnload+1 < pJourney->FirstOpenedTime(nDeck))
				dRes = Log(WARNING_PASSENGER_2, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			else if (tUnload > pJourney->LastCloseTime(nDeck)+1)
				dRes = Log(WARNING_PASSENGER_3, false, pPassenger->GetId(), tUnload, nLiftFloor, myId, nDeck);
			else if (tUnload > pJourney->LastClosedTime(nDeck)+1)
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
			else if (tLoad+1 < pJourney->FirstOpenedTime(nDeck))
				dRes = Log(WARNING_PASSENGER_2, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			else if (tLoad > pJourney->LastCloseTime(nDeck)+1)
				dRes = Log(WARNING_PASSENGER_3, true, pPassenger->GetId(), tLoad, nLiftFloor, myId, nDeck);
			else if (tLoad > pJourney->LastClosedTime(nDeck)+1)
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

HRESULT CLiftSrv::Store(CDataBase db)
{
	if (!db) throw db;

	for (AVULONG iJourney = 0; iJourney < GetJourneyCount(); iJourney++)
	{
		// add journey
		JOURNEY *pJ = GetJourney(iJourney);
			
		CDataBase::INSERT ins = db.insert(L"AVJourneys");
		ins[L"LiftID"] = GetId();
		ins[L"SimID"] = GetSimId();
		ins[L"ShaftFrom"] = pJ->m_shaftFrom;
		ins[L"ShaftTo"] = pJ->m_shaftTo;
		ins[L"FloorFrom"] = pJ->m_floorFrom;
		ins[L"FloorTo"] = pJ->m_floorTo;
		ins[L"TimeGo"] = pJ->m_timeGo;
		ins[L"TimeDest"] = pJ->m_timeDest;
		ins[L"DC"] = pJ->StringifyDoorCycles();
		ins.execute();
	}

	return S_OK;
}

