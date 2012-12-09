// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "SrvSim.h"

using namespace dbtools;

CLiftSrv::CLiftSrv(CSimSrv *pSim, AVULONG nLift, AVULONG nDecks) : CLift(pSim, nLift, nDecks)
{
}

DWORD CLiftSrv::Load(CLftGroupSrv::LIFT *pLIFT, CSimLoader &loader, AVULONG nId, bool bCalcUnload, bool bCalcLoad)
{
	SetId(nId);

	SetDecks(pLIFT->GetShaft()->GetDeckCount());

	CSimJourneyResolver resolver(m_journeys);
	resolver.Run(pLIFT, loader, nId);

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

			if (bCalcUnload) tUnload = t[nDeck];

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
			if (bCalcUnload) pPassenger->SetUnloadTime(tUnload);
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

			if (bCalcLoad) tLoad = max(t[nDeck], tLoad);

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
			if (bCalcLoad) pPassenger->SetLoadTime(t[nDeck]);
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

