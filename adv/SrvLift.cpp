// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvSim.h"
#include <algorithm>

using namespace dbtools;

extern bool g_bOnScreen;

CLiftSrv::CLiftSrv(CSimSrv *pSim, AVULONG nLift, AVULONG nDecks) : CLift((CSim*)pSim, nLift, nDecks)
{
}

DWORD CLiftSrv::Load(dbtools::CDataBase db)
{
	AVULONG nLiftNativeId = GetSim()->GetLiftGroup()->GetLift(GetId())->GetShaft()->GetNativeId();
	AVULONG nTrafficScenarioId = (*GetSim())[L"TrafficScenarioId"];
	AVULONG nIteration = 0;

	std::deque<CPassenger*> &collLoading = m_collPassengers;	// simply alias
	std::deque<CPassenger*> collUnloading;

	collUnloading.assign(m_collPassengers.begin(), m_collPassengers.end());
	std::sort(collUnloading.begin(), collUnloading.end(), [](CPassenger *p1, CPassenger *p2) -> bool { return p1->GetUnloadTime() < p2->GetUnloadTime(); });

	SetDecks(GetSHAFT()->GetDeckCount());

	// collect some data...
	CLiftGroup::SHAFT *pShaft = GetSHAFT();
	AVLONG nOpeningTime = pShaft->GetOpeningTime();
	AVLONG nClosingTime = pShaft->GetClosingTime();
	AVLONG nLoadingTime = pShaft->GetLoadingTime();
	AVLONG nUnloadingTime = pShaft->GetUnloadingTime();
	AVLONG nPreOpeningTime = pShaft->GetPreOpeningTime();
	AVLONG nMotorStartDelayTime = pShaft->GetMotorStartDelayTime();
	bool bDestination = ((AVULONG)(*pShaft->GetLiftGroup())[L"TrafficControlTypeId"]) == 2;

	static AVULONG nJourneyId = 0;
	nJourneyId = (nJourneyId / 1000) * 1000 + 1000;

	// iterators for the passenger collections
	auto iUnloading = collUnloading.begin();
	auto iLoading = collLoading.begin();

	// Database Query
	dbtools::CDataBase::SELECT selLiftStops = db.select(L"SELECT * FROM LiftStops WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);
	if (!selLiftStops) return S_OK;	// no stops?!


//	STOP stop = m_collStops.front(); m_collStops.pop_front();
//	ASSERT(stop.nFloor == (AVULONG)selLiftStops[L"Floor"] && stop.nTime == selLiftStops[L"Time"].msec() && stop.nDuration == selLiftStops[L"Duration"].msec());

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
	if (!bDestination && nDuration > 0)
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
		{
			nTimeOpen[iDeck] = nTime;
			nTimeClose[iDeck] = nTime + nOpeningTime + pShaft->GetDwellTime(pJourney->m_floorFrom) - nPreOpeningTime;
		}
		
	selLiftStops++;

	for ( ; selLiftStops; selLiftStops++)
	{
//	STOP stop = m_collStops.front(); m_collStops.pop_front();
//	ASSERT(stop.nFloor == (AVULONG)selLiftStops[L"Floor"] && stop.nTime == selLiftStops[L"Time"].msec() && stop.nDuration == selLiftStops[L"Duration"].msec());

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
		//AVULONG idTrafficScenario = (*GetSim())[L"TrafficScenarioId"];
		//AVULONG idLift = (*pShaft)[L"LiftId"];
		//AVULONG idJourney = pJourney->m_id;

		bool flagClosing = nFloor != pJourney->m_floorFrom;

		if (!flagClosing)
		{
			// if still on the same floor - postpone, just update the time
			pJourney->m_timeGo = nTime + nDuration;
			if (nDuration <= nMotorStartDelayTime)
				continue;
			
			// initially plan open cycles
			for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
				if (!bDestination && nTimeOpen[iDeck] < 0)
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
				dc.m_timeClose = nTimeClose[iDeck];
				pJourney->m_doorcycles[iDeck].push_back(dc);

				double fClosing = min((double)(t - nTimeClose[iDeck]) / (nOpeningTime + nClosingTime), 1.0);
				nTimeOpen[iDeck] = t - fClosing * nOpeningTime;
				nTimeClose[iDeck] = t + nDwellTime;
			}
			if (!bDestination && t == nTimeOpen[iDeck] + nOpeningTime + nDwellTime && nPassengers[iDeck] == 1)
				// special case --- passengers waiting for the lift to finish its first dwell time
				nTimeClose[iDeck] = t + nDwellTime;
			else
				nTimeClose[iDeck] = max(nTimeClose[iDeck], t + nLoadingTime);

			// Additional check & intervention - if door closed after motor started...
			if (nTimeClose[iDeck] + nClosingTime > pJourney->m_timeGo - nMotorStartDelayTime)
			{
				//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time trimmed *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
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
			if (!bDestination && nTimeOpen[iDeck] >= 0 && flagClosing && nTimeClose[iDeck] + nClosingTime <= pJourney->m_timeGo - nMotorStartDelayTime - 100 && nPassengers[iDeck] > 0)
			{
				//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time extended *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
				nTimeClose[iDeck] = pJourney->m_timeGo - nMotorStartDelayTime - nClosingTime;
			}

			// send out the door cycle (if avail)
			if (nTimeOpen[iDeck] >= 0)
			{
				JOURNEY::DOOR dc;
				dc.m_timeOpen = nTimeOpen[iDeck];
				dc.m_timeClose = nTimeClose[iDeck];
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
						//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Short door cycle eliminated *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
						pJourney->m_doorcycles[iDeck].pop_back();
					}
					else
						;//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": WTF CONDITION: Door cycle too short *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
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
			if (!bDestination && nTime > nTimeClose[iDeck] && nDuration > 0)
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

DWORD CLiftSrv::Load()
{
	AVULONG nLiftNativeId = GetSim()->GetLiftGroup()->GetLift(GetId())->GetShaft()->GetNativeId();
	AVULONG nTrafficScenarioId = (*GetSim())[L"TrafficScenarioId"];
	AVULONG nIteration = 0;

	std::deque<CPassenger*> &collLoading = m_collPassengers;	// simply alias
	std::deque<CPassenger*> collUnloading;

	collUnloading.assign(m_collPassengers.begin(), m_collPassengers.end());
	std::sort(collUnloading.begin(), collUnloading.end(), [](CPassenger *p1, CPassenger *p2) -> bool { return p1->GetUnloadTime() < p2->GetUnloadTime(); });

	SetDecks(GetSHAFT()->GetDeckCount());

	// collect some data...
	CLiftGroup::SHAFT *pShaft = GetSHAFT();
	AVLONG nOpeningTime = pShaft->GetOpeningTime();
	AVLONG nClosingTime = pShaft->GetClosingTime();
	AVLONG nLoadingTime = pShaft->GetLoadingTime();
	AVLONG nUnloadingTime = pShaft->GetUnloadingTime();
	AVLONG nPreOpeningTime = pShaft->GetPreOpeningTime();
	AVLONG nMotorStartDelayTime = pShaft->GetMotorStartDelayTime();
	bool bDestination = ((AVULONG)(*pShaft->GetLiftGroup())[L"TrafficControlTypeId"]) == 2;

	static AVULONG nJourneyId = 0;
	nJourneyId = (nJourneyId / 1000) * 1000 + 1000;

	// iterators for the passenger collections
	auto iUnloading = collUnloading.begin();
	auto iLoading = collLoading.begin();

	// Database Query
//	dbtools::CDataBase::SELECT selLiftStops = db.select(L"SELECT * FROM LiftStops WHERE LiftId = %d AND TrafficScenarioId=%d AND Iteration=%d ORDER BY [Time]", nLiftNativeId, nTrafficScenarioId, nIteration);
//	if (!selLiftStops) return S_OK;	// no stops?!

	// first journey
	if (m_collStops.empty()) return S_OK;
	STOP stop = m_collStops.front();

	m_journeys.push_back(JOURNEY());
	JOURNEY *pJourney = &m_journeys.back();
	pJourney->m_id = nJourneyId++;
	pJourney->m_floorFrom = stop.nFloor;
	pJourney->m_timeGo = stop.nTime + stop.nDuration;
	
	AVULONG nPassengers[DECK_NUM] = { 0, 0 };
	AVLONG nTimeLiftArrive[DECK_NUM] = { 0, 0 };
	AVLONG nTimeOpen[DECK_NUM] = { -1, -1 }, nTimeClose[DECK_NUM] = { -1, -1 };

	AVLONG nTime = stop.nTime;
	AVLONG nDuration = stop.nDuration;
	AVLONG nDwellTime = pShaft->GetDwellTime(pJourney->m_floorFrom);
	if (!bDestination && nDuration > 0)
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
		{
			nTimeOpen[iDeck] = nTime;
			nTimeClose[iDeck] = nTime + nOpeningTime + pShaft->GetDwellTime(pJourney->m_floorFrom) - nPreOpeningTime;
		}
	
	m_collStops.pop_front();
	for ( ; !m_collStops.empty(); m_collStops.pop_front())
	{
		stop = m_collStops.front();

		// data from lift stops
		AVLONG nTime = stop.nTime;
		AVLONG nDuration = stop.nDuration;
		AVLONG nDwellTime = pShaft->GetDwellTime(pJourney->m_floorFrom);
		AVULONG nFloor = stop.nFloor;

		// update pJourney...
		pJourney->m_shaftFrom = pJourney->m_shaftTo = GetId();
		pJourney->m_floorTo = nFloor;
		pJourney->m_timeDest = nTime;

		// identification
		//AVULONG idTrafficScenario = (*GetSim())[L"TrafficScenarioId"];
		//AVULONG idLift = (*pShaft)[L"LiftId"];
		//AVULONG idJourney = pJourney->m_id;

		bool flagClosing = nFloor != pJourney->m_floorFrom;

		if (!flagClosing)
		{
			// if still on the same floor - postpone, just update the time
			pJourney->m_timeGo = nTime + nDuration;
			if (nDuration <= nMotorStartDelayTime)
				continue;
			
			// initially plan open cycles
			for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
				if (!bDestination && nTimeOpen[iDeck] < 0)
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
				dc.m_timeClose = nTimeClose[iDeck];
				pJourney->m_doorcycles[iDeck].push_back(dc);

				double fClosing = min((double)(t - nTimeClose[iDeck]) / (nOpeningTime + nClosingTime), 1.0);
				nTimeOpen[iDeck] = t - fClosing * nOpeningTime;
				nTimeClose[iDeck] = t + nDwellTime;
			}
			if (!bDestination && t == nTimeOpen[iDeck] + nOpeningTime + nDwellTime && nPassengers[iDeck] == 1)
				// special case --- passengers waiting for the lift to finish its first dwell time
				nTimeClose[iDeck] = t + nDwellTime;
			else
				nTimeClose[iDeck] = max(nTimeClose[iDeck], t + nLoadingTime);

			// Additional check & intervention - if door closed after motor started...
			if (nTimeClose[iDeck] + nClosingTime > pJourney->m_timeGo - nMotorStartDelayTime)
			{
				//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time trimmed *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
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
			if (!bDestination && nTimeOpen[iDeck] >= 0 && flagClosing && nTimeClose[iDeck] + nClosingTime <= pJourney->m_timeGo - nMotorStartDelayTime - 100 && nPassengers[iDeck] > 0)
			{
				//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Door close time extended *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
				nTimeClose[iDeck] = pJourney->m_timeGo - nMotorStartDelayTime - nClosingTime;
			}

			// send out the door cycle (if avail)
			if (nTimeOpen[iDeck] >= 0)
			{
				JOURNEY::DOOR dc;
				dc.m_timeOpen = nTimeOpen[iDeck];
				dc.m_timeClose = nTimeClose[iDeck];
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
						//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": Short door cycle eliminated *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
						pJourney->m_doorcycles[iDeck].pop_back();
					}
					else
						;//if (g_bOnScreen) { wcout << L"*** " << pJourney->m_id << L": WTF CONDITION: Door cycle too short *** " << (bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
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
			if (!bDestination && nTime > nTimeClose[iDeck] && nDuration > 0)
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

