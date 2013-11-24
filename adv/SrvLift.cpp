// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "SrvSim.h"
#include "SrvSimLiftSeq.h"

using namespace dbtools;

extern bool g_bOnScreen;

CLiftSrv::CLiftSrv(CSimSrv *pSim, AVULONG nLift, AVULONG nDecks) : CLift(pSim, nLift, nDecks)
{
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
	AVULONG nTrafficControlType = (*pShaft->GetLiftGroup())[L"TrafficControlTypeId"];

	static AVULONG nJourneyId = 0;
	nJourneyId = (nJourneyId / 1000) * 1000 + 1000;

	// iterators for the passenger collections
	std::vector<CPassenger*>::iterator iUnloading = collUnloading.begin();
	std::vector<CPassenger*>::iterator iLoading = collLoading.begin();

	// Database Query
	dbtools::CDataBase::SELECT selLiftStops = db.select(L"SELECT * FROM LiftStops WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);
	if (!selLiftStops) return S_OK;	// no stops?!

	// first journey
	m_journeys.push_back(JOURNEY());
	JOURNEY *pJourney = &m_journeys.back();
	pJourney->m_id = nJourneyId++;
	pJourney->m_floorFrom = selLiftStops[L"Floor"];
	pJourney->m_timeGo = selLiftStops[L"Time"].msec() + selLiftStops[L"Duration"].msec();
	
	AVULONG nPassengers[DECK_NUM] = { 0, 0 };
	AVLONG nTimeLiftArrive[DECK_NUM] = { 0, 0 };
	AVLONG nTimeOpen[DECK_NUM] = { -1, -1 }, nTimeClose[DECK_NUM] = { -1, -1 };

	AVLONG nTime = selLiftStops[L"Time"].msec();
	AVLONG nDuration = selLiftStops[L"Duration"].msec();
	AVLONG nDwellTime = pShaft->GetDwellTime(pJourney->m_floorFrom);
	if (nDuration > 0)
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
		{
			nTimeOpen[iDeck] = nTime;
			nTimeClose[iDeck] = nTime + nOpeningTime + pShaft->GetDwellTime(pJourney->m_floorFrom) - nPreOpeningTime;
		}
		
	selLiftStops++;

	for ( ; selLiftStops; selLiftStops++)
	{
		// data from lift stops
		AVLONG nTime = selLiftStops[L"Time"].msec();
		AVLONG nDuration = selLiftStops[L"Duration"].msec();
		AVLONG nDwellTime = pShaft->GetDwellTime(pJourney->m_floorFrom);
		AVULONG nFloor = selLiftStops[L"Floor"];

		// update pJourney...
		pJourney->m_shaftFrom = pJourney->m_shaftTo = GetId();
		pJourney->m_floorTo = nFloor;
		pJourney->m_timeDest = nTime;

		// identification
		AVULONG idTrafficScenario = (*GetSim())[L"TrafficScenarioId"];
		AVULONG idLift = (*this->GetSHAFT())[L"LiftId"];
		AVULONG idJourney = pJourney->m_id;

		bool flagClosing = nFloor != pJourney->m_floorFrom;

		if (!flagClosing)
		{
			// if still on the same floor - postpone, just update the time
			pJourney->m_timeGo = nTime + nDuration;
			if (nDuration <= nMotorStartDelayTime)
				continue;
			
			// initially plan open cycles
			for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
				if (nTimeOpen[iDeck] < 0)
				{
					nTimeOpen[iDeck] = nTime;
					nTimeClose[iDeck] = nTime + nOpeningTime + nDwellTime - nPreOpeningTime;
				}
		}

		// unloading passengers
		while (iUnloading != collUnloading.end() && (*iUnloading)->GetUnloadTime() + 10 < (AVLONG)pJourney->m_timeGo)
		{
			LONG t = (*iUnloading)->GetUnloadTime();
			AVULONG iDeck = (*iUnloading)->GetDeck();
			if (nTimeOpen[iDeck] < 0) 
			{
				// first opening
				nTimeOpen[iDeck] = t - nOpeningTime;
				nTimeClose[iDeck] = t + nDwellTime;
			}
			nTimeClose[iDeck] = max(nTimeClose[iDeck], t + nUnloadingTime);
			iUnloading++;
		}

		// loading passengers
		while (iLoading != collLoading.end() && (*iLoading)->GetLoadTime() + 10 < (AVLONG)pJourney->m_timeGo)
		{
			LONG t = (*iLoading)->GetLoadTime();
			LONG tarr = (*iLoading)->GetArrivalTime();
			AVULONG iDeck = (*iLoading)->GetDeck();
			nPassengers[iDeck]++;

			if (nTimeOpen[iDeck] < 0) 
			{
				// first opening
				nTimeOpen[iDeck] = t - nOpeningTime;
				nTimeClose[iDeck] = t + nDwellTime;
			}
			else if (t > nTimeClose[iDeck])
			{
				// reopening
				JOURNEY::DOOR dc;
				dc.m_timeOpen = nTimeOpen[iDeck];
				dc.m_durationOpen = nOpeningTime;
				dc.m_timeClose = nTimeClose[iDeck];
				dc.m_durationClose = nClosingTime;
				pJourney->m_doorcycles[iDeck].push_back(dc);

				double fClosing = min((double)(t - nTimeClose[iDeck]) / (nOpeningTime + nClosingTime), 1.0);
				nTimeOpen[iDeck] = t - fClosing * nOpeningTime;
				nTimeClose[iDeck] = t + nDwellTime;
			}
			if (t == nTimeOpen[iDeck] + nOpeningTime + nDwellTime && nPassengers[iDeck] == 1 && nTrafficControlType == 1)
				// special case --- passengers waiting for the lift to finish its first dwell time
				nTimeClose[iDeck] = t + nDwellTime;
			else
				nTimeClose[iDeck] = max(nTimeClose[iDeck], t + nLoadingTime);

			// Additional check & intervention - if door closed after motor started...
			if (nTimeClose[iDeck] + nClosingTime > pJourney->m_timeGo - nMotorStartDelayTime)
			{
				if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time trimmed *** " << nTrafficControlType << endl; LogProgress(0); }
				nTimeClose[iDeck] = pJourney->m_timeGo - nClosingTime - nMotorStartDelayTime;
				t = nTimeClose[iDeck] - nDwellTime;
				if (pJourney->m_doorcycles[iDeck].size() == 0)
					nTimeOpen[iDeck] = min(nTimeOpen[iDeck], t - nOpeningTime);	// this is the first opening - just trim it!
				else if (t >= (AVLONG)pJourney->m_doorcycles[iDeck].back().m_timeClose)
				{	// still reopening - calculate it!
					double fClosing = (double)(t - pJourney->m_doorcycles[iDeck].back().m_timeClose) / (double)(nOpeningTime + nClosingTime);
					nTimeOpen[iDeck] = min(nTimeOpen[iDeck], t - fClosing * nOpeningTime);
				}
				else
				{	// door cycle overlaps with the previous one
					nTimeOpen[iDeck] = pJourney->m_doorcycles[iDeck].back().m_timeOpen;
					pJourney->m_doorcycles[iDeck].pop_back();
				}
			}

			iLoading++;
		}

		// TO DO:
		// 4. Double deckers

		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)
		{
			// Additional check and intervention - if door with passengers closed way before motor started...
			if (nTrafficControlType == 1 && nTimeOpen[iDeck] >= 0 && flagClosing && nTimeClose[iDeck] + nClosingTime <= pJourney->m_timeGo - nMotorStartDelayTime - 100 && nPassengers[iDeck] > 0)
			{
				if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time extended *** " << nTrafficControlType << endl; LogProgress(0); }
				nTimeClose[iDeck] = pJourney->m_timeGo - nMotorStartDelayTime - nClosingTime;
			}

			// send out the door cycle (if avail)
			if (nTimeOpen[iDeck] >= 0)
			{
				JOURNEY::DOOR dc;
				dc.m_timeOpen = nTimeOpen[iDeck];
				dc.m_durationOpen = nOpeningTime;
				dc.m_timeClose = nTimeClose[iDeck];
				dc.m_durationClose = nClosingTime;
				pJourney->m_doorcycles[iDeck].push_back(dc);
			}
		
			// some final amendments - to be completed only near the end of the cycle!
			if (flagClosing)
			{
				// amend for pre-opening
				if (pJourney->m_doorcycles[iDeck].size() && pJourney->m_doorcycles[iDeck][0].m_timeOpen == nTimeLiftArrive[iDeck])
					pJourney->m_doorcycles[iDeck][0].m_timeOpen -= nPreOpeningTime;
				nTimeLiftArrive[iDeck] = pJourney->m_timeDest;

				// amend for open-close cycle too tight (dwell time not kept)
				if (nTimeOpen[iDeck] >= 0 && nTimeClose[iDeck] + nClosingTime > pJourney->m_timeGo - nMotorStartDelayTime)
					if (nPassengers[iDeck] == 0)	// no problem --- remove the last cycle
					{
						if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Short door cycle eliminated *** " << nTrafficControlType << endl; LogProgress(0); }
						pJourney->m_doorcycles[iDeck].pop_back();
					}
					else
						if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": WTF CONDITION: Door cycle too short *** " << nTrafficControlType << endl; LogProgress(0); }
			}
		}

		// if journey completed - send it out!
		if (flagClosing)
		{
			// start a new journey from now
			m_journeys.push_back(JOURNEY());
			pJourney = &m_journeys.back();
			pJourney->m_id = nJourneyId++;
			pJourney->m_floorFrom = nFloor;
			pJourney->m_timeGo = nTime + nDuration;
			for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)
				nPassengers[iDeck] = 0;

			nDwellTime = (AVLONG)pShaft->GetDwellTime(pJourney->m_floorFrom);
		}

		// initialise the cycle...
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
			if (nTime > nTimeClose[iDeck] && nDuration > 0)
			{
				nTimeOpen[iDeck] = nTime;
				nTimeClose[iDeck] = nTime + nOpeningTime + nDwellTime - nPreOpeningTime;
			}
			else
				nTimeOpen[iDeck] = -1;
	}
	m_journeys.pop_back();
	
	return S_OK;
}

bool CLiftSrv::ReportDifferences(CLiftSrv *p)
{
	static AVULONG nCount = 0;
	bool b = true;

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
				wcout << jThis.m_id << L" " << jThis.m_floorFrom << L"->" << jThis.m_floorTo << " in " << jThis.m_timeGo << L"-" << jThis.m_timeDest << " (" << ++nCount << L")" << endl;
			else
			{
				wcout << "  old: " << jThis.m_id << L" " << jThis.m_floorFrom << L"->" << jThis.m_floorTo << " in " << jThis.m_timeGo << L"-" << jThis.m_timeDest << " (" <<   nCount << L")" << endl;
				wcout << "  new: " << jThat.m_id << L" " << jThat.m_floorFrom << L"->" << jThat.m_floorTo << " in " << jThat.m_timeGo << L"-" << jThat.m_timeDest << " (" << ++nCount << L")" << endl;
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

