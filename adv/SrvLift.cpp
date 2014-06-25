// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvSim.h"
#include <algorithm>

using namespace dbtools;

extern bool g_bOnScreen;

CLiftSrv::CLiftSrv(CSimSrv *pSim, AVULONG nLift, AVULONG nDecks) : CLift((CSim*)pSim, nLift, nDecks)
{
	m_pJourney = NULL;
}

void CLiftSrv::Feed(AVULONG nFloor, AVLONG nTime, AVLONG nDuration)
{
//	std::sort(m_collLoading.begin(), m_collLoading.end(), [](CPassenger *p1, CPassenger *p2) -> bool { return p1->GetLoadTime() < p2->GetLoadTime(); });
//	std::sort(m_collUnloading.begin(), m_collUnloading.end(), [](CPassenger *p1, CPassenger *p2) -> bool { return p1->GetUnloadTime() < p2->GetUnloadTime(); });

	// collect some parameters (speed up)
	CLiftGroup::SHAFT *pShaft = GetSHAFT();
	AVLONG m_nOpeningTime = pShaft->GetOpeningTime();
	AVLONG m_nClosingTime = pShaft->GetClosingTime();
	AVLONG m_nLoadingTime = pShaft->GetLoadingTime();
	AVLONG m_nUnloadingTime = pShaft->GetUnloadingTime();
	AVLONG m_nPreOpeningTime = pShaft->GetPreOpeningTime();
	AVLONG m_nMotorStartDelayTime = pShaft->GetMotorStartDelayTime();
	bool m_bDestination = ((AVULONG)(*pShaft->GetLiftGroup())[L"TrafficControlTypeId"]) == 2;
	
	if (m_pJourney == NULL)
	{
		SetDecks(GetSHAFT()->GetDeckCount());

		// First Iteration Only!
		m_journeys.push_back(JOURNEY());
		m_pJourney = &m_journeys.back();
		m_pJourney->m_floorFrom = nFloor;
		m_pJourney->m_timeGo = nTime + nDuration;

		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
		{
			m_nPassengers[iDeck] = m_nTimeLiftArrive[iDeck] = 0;
			m_nTimeOpen[iDeck] = m_nTimeClose[iDeck] = -1;

			if (!m_bDestination && nDuration > 0)
			{
				m_nTimeOpen[iDeck] = nTime;
				m_nTimeClose[iDeck] = nTime + m_nOpeningTime + GetSHAFT()->GetDwellTime(m_pJourney->m_floorFrom) - m_nPreOpeningTime;
			}
		}
		return;
	}

	// Find the Dwell Time
	AVLONG nDwellTime = GetSHAFT()->GetDwellTime(m_pJourney->m_floorFrom);

	// update m_pJourney...
	m_pJourney->m_shaftFrom = m_pJourney->m_shaftTo = GetId();
	m_pJourney->m_floorTo = nFloor;
	m_pJourney->m_timeDest = nTime;

	// identification
	//AVULONG idTrafficScenario = (*GetSim())[L"TrafficScenarioId"];
	//AVULONG idLift = (*pShaft)[L"LiftId"];
	//AVULONG idJourney = m_pJourney->m_id;

	bool flagClosing = nFloor != m_pJourney->m_floorFrom;

	if (!flagClosing)
	{
		// if still on the same floor - postpone, just update the time
		m_pJourney->m_timeGo = nTime + nDuration;
		if (nDuration <= m_nMotorStartDelayTime)
			return;
			
		// initially plan open cycles
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
			if (!m_bDestination && m_nTimeOpen[iDeck] < 0)
			{
				m_nTimeOpen[iDeck] = nTime;
				m_nTimeClose[iDeck] = nTime + m_nOpeningTime + nDwellTime - m_nPreOpeningTime;
			}
	}

	// unloading passengers
	while (!m_collUnloading.empty() && (*m_collUnloading.begin())->GetUnloadTime() + 10 < (AVLONG)m_pJourney->m_timeGo)
	{
		LONG t = (*m_collUnloading.begin())->GetUnloadTime();
		AVULONG iDeck = (*m_collUnloading.begin())->GetDeck();
		if (m_nTimeOpen[iDeck] < 0) 
		{
			// first opening
			m_nTimeOpen[iDeck] = t - m_nOpeningTime;
			m_nTimeClose[iDeck] = t + nDwellTime;
		}
		m_nTimeClose[iDeck] = max(m_nTimeClose[iDeck], t + m_nUnloadingTime);
		m_collUnloading.erase(m_collUnloading.begin());
	}

	// loading passengers
	while (!m_collLoading.empty() && (*m_collLoading.begin())->GetLoadTime() + 10 < (AVLONG)m_pJourney->m_timeGo)
	{
		LONG t = (*m_collLoading.begin())->GetLoadTime();
		LONG tarr = (*m_collLoading.begin())->GetArrivalTime();
		AVULONG iDeck = (*m_collLoading.begin())->GetDeck();
		m_nPassengers[iDeck]++;

		if (m_nTimeOpen[iDeck] < 0) 
		{
			// first opening
			m_nTimeOpen[iDeck] = t - m_nOpeningTime;
			m_nTimeClose[iDeck] = t + nDwellTime;
		}
		else if (t > m_nTimeClose[iDeck])
		{
			// reopening
			JOURNEY::DOOR dc;
			dc.m_timeOpen = m_nTimeOpen[iDeck];
			dc.m_timeClose = m_nTimeClose[iDeck];
			m_pJourney->m_doorcycles[iDeck].push_back(dc);

			double fClosing = min((double)(t - m_nTimeClose[iDeck]) / (m_nOpeningTime + m_nClosingTime), 1.0);
			m_nTimeOpen[iDeck] = t - fClosing * m_nOpeningTime;
			m_nTimeClose[iDeck] = t + nDwellTime;
		}
		if (!m_bDestination && t == m_nTimeOpen[iDeck] + m_nOpeningTime + nDwellTime && m_nPassengers[iDeck] == 1)
			// special case --- passengers waiting for the lift to finish its first dwell time
			m_nTimeClose[iDeck] = t + nDwellTime;
		else
			m_nTimeClose[iDeck] = max(m_nTimeClose[iDeck], t + m_nLoadingTime);

		// Additional check & intervention - if door closed after motor started...
		if (m_nTimeClose[iDeck] + m_nClosingTime > m_pJourney->m_timeGo - m_nMotorStartDelayTime)
		{
			//if (g_bOnScreen) { wcout << L"*** " << m_pJourney->m_id << L": Door close time trimmed *** " << (m_bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
			m_nTimeClose[iDeck] = m_pJourney->m_timeGo - m_nClosingTime - m_nMotorStartDelayTime;
			t = m_nTimeClose[iDeck] - nDwellTime;
			if (m_pJourney->m_doorcycles[iDeck].size() == 0)
				m_nTimeOpen[iDeck] = min(m_nTimeOpen[iDeck], t - m_nOpeningTime);	// this is the first opening - just trim it!
			else if (t >= (AVLONG)m_pJourney->m_doorcycles[iDeck].back().m_timeClose)
			{	// still reopening - calculate it!
				double fClosing = (double)(t - m_pJourney->m_doorcycles[iDeck].back().m_timeClose) / (double)(m_nOpeningTime + m_nClosingTime);
				m_nTimeOpen[iDeck] = min(m_nTimeOpen[iDeck], t - fClosing * m_nOpeningTime);
			}
			else
			{	// door cycle overlaps with the previous one
				m_nTimeOpen[iDeck] = m_pJourney->m_doorcycles[iDeck].back().m_timeOpen;
				m_pJourney->m_doorcycles[iDeck].pop_back();
			}
		}

		m_collLoading.erase(m_collLoading.begin());
	}

	// TO DO:
	// 4. Double deckers

	for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)
	{
		// Additional check and intervention - if door with passengers closed way before motor started...
		if (!m_bDestination && m_nTimeOpen[iDeck] >= 0 && flagClosing && m_nTimeClose[iDeck] + m_nClosingTime <= m_pJourney->m_timeGo - m_nMotorStartDelayTime - 100 && m_nPassengers[iDeck] > 0)
		{
			//if (g_bOnScreen) { wcout << L"*** " << m_pJourney->m_id << L": Door close time extended *** " << (m_bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
			m_nTimeClose[iDeck] = m_pJourney->m_timeGo - m_nMotorStartDelayTime - m_nClosingTime;
		}

		// send out the door cycle (if avail)
		if (m_nTimeOpen[iDeck] >= 0)
		{
			JOURNEY::DOOR dc;
			dc.m_timeOpen = m_nTimeOpen[iDeck];
			dc.m_timeClose = m_nTimeClose[iDeck];
			m_pJourney->m_doorcycles[iDeck].push_back(dc);
		}
		
		// some final amendments - to be completed only near the end of the cycle!
		if (flagClosing)
		{
			// amend for pre-opening
			if (m_pJourney->m_doorcycles[iDeck].size() && m_pJourney->m_doorcycles[iDeck][0].m_timeOpen == m_nTimeLiftArrive[iDeck])
				m_pJourney->m_doorcycles[iDeck][0].m_timeOpen -= m_nPreOpeningTime;
			m_nTimeLiftArrive[iDeck] = m_pJourney->m_timeDest;

			// amend for open-close cycle too tight (dwell time not kept)
			if (m_nTimeOpen[iDeck] >= 0 && m_nTimeClose[iDeck] + m_nClosingTime > m_pJourney->m_timeGo - m_nMotorStartDelayTime)
				if (m_nPassengers[iDeck] == 0)	// no problem --- remove the last cycle
				{
					//if (g_bOnScreen) { wcout << L"*** " << m_pJourney->m_id << L": Short door cycle eliminated *** " << (m_bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
					m_pJourney->m_doorcycles[iDeck].pop_back();
				}
				else
					;//if (g_bOnScreen) { wcout << L"*** " << m_pJourney->m_id << L": WTF CONDITION: Door cycle too short *** " << (m_bDestination ? L"dest" : L"conv") << endl; LogProgress(0); }
		}
	}

	// if journey completed - send it out!
	if (flagClosing)
	{
		// start a new journey from now
		m_collJourneyCache.push_back(*m_pJourney);
		m_journeys.push_back(JOURNEY());
		m_pJourney = &m_journeys.back();
		m_pJourney->m_floorFrom = nFloor;
		m_pJourney->m_timeGo = nTime + nDuration;
		for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)
			m_nPassengers[iDeck] = 0;

		nDwellTime = (AVLONG)GetSHAFT()->GetDwellTime(m_pJourney->m_floorFrom);
	}

	// initialise the cycle...
	for (AVULONG iDeck = 0; iDeck < GetDecks(); iDeck++)	// ### indiscriminative iteration through the decks here!
		if (!m_bDestination && nTime > m_nTimeClose[iDeck] && nDuration > 0)
		{
			m_nTimeOpen[iDeck] = nTime;
			m_nTimeClose[iDeck] = nTime + m_nOpeningTime + nDwellTime - m_nPreOpeningTime;
		}
		else
			m_nTimeOpen[iDeck] = -1;
}

HRESULT CLiftSrv::Store(dbtools::CDataBase db, JOURNEY &j)
{
	if (j.m_floorTo == (AVULONG)-1)
		return S_OK;

	CDataBase::INSERT ins = db.insert(L"AVJourneys");
	ins[L"LiftID"] = GetId();
	ins[L"SimID"] = GetSimId();
	ins[L"ShaftFrom"] = j.m_shaftFrom;
	ins[L"ShaftTo"] = j.m_shaftTo;
	ins[L"FloorFrom"] = j.m_floorFrom;
	ins[L"FloorTo"] = j.m_floorTo;
	ins[L"TimeGo"] = j.m_timeGo;
	ins[L"TimeDest"] = j.m_timeDest;
	ins[L"DC"] = j.StringifyDoorCycles();
	ins.execute();

	return S_OK;
}

HRESULT CLiftSrv::Store(CDataBase db)
{
	if (!db) throw db;

	for each (JOURNEY j in GetJourneys())
		Store(db, j);

	return S_OK;
}

