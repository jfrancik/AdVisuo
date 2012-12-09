// SimLoader.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvSimLoader.h"
#include "SrvLiftGroup.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// CSimLoader

CSimLoader::CSimLoader()
{
	nVersion = 0;
	nLifts = 0;
	nPassengers = 0;
	pPassengers = NULL;
	pnLiftLogs = NULL;
	ppLiftLogs = NULL;
	pnDeckLogs = NULL;
	ppDeckLogs = NULL;
}

CSimLoader::~CSimLoader()
{
	if (pPassengers) delete [] pPassengers;
	if (pnLiftLogs) delete [] pnLiftLogs;
	if (ppLiftLogs)
	{
		for (int i = 0; i < nLifts; i++)
			if (ppLiftLogs[i]) delete [] ppLiftLogs[i];
		delete [] ppLiftLogs;
	}
	if (pnDeckLogs) delete pnDeckLogs;
	if (ppDeckLogs)
	{
		for (int i = 0; i < nLifts; i++)
			if (ppDeckLogs[i]) delete [] ppDeckLogs[i];
		delete [] ppDeckLogs;
	}
}

int CSimLoader::Load(ifstream &stream, void *pBuf, size_t nSize)
{
	memset(pBuf, 0, nSize);
	stream.read((char*)pBuf, nSize);
	if (!stream) throw ERROR_FILE_READ;
	size_t bytes = (size_t)stream.gcount();
	if (bytes != nSize) throw ERROR_FILE_READ;
	return bytes;
}

int CSimLoader::Load(std::ifstream &stream, size_t nSize)
{
	BYTE *pBuf = new BYTE[nSize];
	int bytes = Load(stream, (void*)pBuf, nSize);
	delete [] pBuf;
	return bytes;
}

DWORD CSimLoader::Load(CLiftGroupSrv *pLiftGroup, dbtools::CDataBase db, ULONG nSimulationId)
{
	if (!db) throw db;
	dbtools::CDataBase::SELECT sel;

	bDB = true;

	// analyse the lifts in the lift group
	nLifts = pLiftGroup->GetLiftCount();

	sel = db.select(L"SELECT * FROM SimulationLogs WHERE SimulationId=%d", nSimulationId);
	if (!sel) return ERROR_SIM_MISSING;
	nVersion = (int)((float)sel[L"AdSimuloVersion"] * 10 + 100.5);

	// count passengers - on the lift by lift basis
	nPassengers = 0;
	for (int iLift = 0; iLift < nLifts; iLift++)
	{
		sel = db.select(L"SELECT COUNT(HallCallId) As NumPassengers FROM HallCalls WHERE LiftId = %d AND SimulationId=%d", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		if (!sel) return ERROR_SIM_MISSING;
		nPassengers += (int)sel[L"NumPassengers"];
	}
	pPassengers = new Passenger[nPassengers];

	// load passengers (lift by lift)
	int iPassenger = 0;
	for (int iLift = 0; iLift < nLifts; iLift++)
	{
		sel = db.select(L"SELECT * FROM HallCalls WHERE LiftId = %d AND SimulationId=%d", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		for ( ; sel; sel++, iPassenger++)
		{
			pPassengers[iPassenger].ArrivalFloor = (int)sel[L"ArrivalFloor"];
			pPassengers[iPassenger].DestinationFloor = (int)sel[L"DestinationFloor"];
			pPassengers[iPassenger].ArrivalTime = (float)sel[L"ArrivalTime"];
			pPassengers[iPassenger].CarID = iLift;
			// (int)sel[L"Deck"];
			pPassengers[iPassenger].WaitingTime = (float)sel[L"WaitingTime"];
			pPassengers[iPassenger].TransitTime = (float)sel[L"TransitTime"];
			pPassengers[iPassenger].JourneyTime = (float)sel[L"JourneyTime"];
			pPassengers[iPassenger].ToDestTime = (float)sel[L"TimeToDestination"];
			pPassengers[iPassenger].CarArrivalTime = (float)sel[L"CarArrivalAtArrivalFloor"];
			pPassengers[iPassenger].CarDepTime = (float)sel[L"DepartureTimeFromArrivalFloor"];
			pPassengers[iPassenger].CarDestTime = (float)sel[L"CarArrivalAtDestinationFloor"];
			pPassengers[iPassenger].StartLoadingTime = (float)sel[L"StartLoading"];
			pPassengers[iPassenger].StartUnloadingTime = (float)sel[L"StartUnloading"];
		}
	}

	pnLiftLogs = new int[nLifts];
	memset(pnLiftLogs, 0, sizeof(int)*nLifts);
	ppLiftLogs = new LiftLog*[nLifts];
	memset(ppLiftLogs, 0, sizeof(LiftLog*)*nLifts);
	
	pnDeckLogs = new int[nLifts];
	memset(pnDeckLogs, 0, sizeof(int)*nLifts);
	ppDeckLogs = new DeckLog*[nLifts];
	memset(ppDeckLogs, 0, sizeof(DeckLog*)*nLifts);

	for (int iLift = 0; iLift < nLifts; iLift++)
	{
		// Lift Logs

		// number of logs
		sel = db.select(L"SELECT COUNT(LiftLogId) As NumLiftLogs FROM LiftLogs WHERE Iteration=0 AND LiftId = %d AND SimulationId=%d", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		if (!sel) return ERROR_SIM_MISSING;
		pnLiftLogs[iLift] = (int)sel[L"NumLiftLogs"];

		// allocate
		ppLiftLogs[iLift] = new LiftLog[pnLiftLogs[iLift]];
		memset(ppLiftLogs[iLift], 0, pnLiftLogs[iLift] * sizeof(LiftLog));
		
		sel = db.select(L"SELECT * FROM LiftLogs WHERE Iteration=0 AND LiftId = %d AND SimulationId=%d ORDER BY [Time]", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		for (int iIter = 0; sel; sel++, iIter++)
		{
			ppLiftLogs[iLift][iIter].Time = (float)sel[L"Time"];
			ppLiftLogs[iLift][iIter].curShaft = iLift;	//pLiftGroup->GetLift(iLift)->GetShaftId();
			ppLiftLogs[iLift][iIter].curFloor = (int)sel[L"CurrentFloor"];
			ppLiftLogs[iLift][iIter].destFloor = (int)sel[L"DestinationFloor"];
			ppLiftLogs[iLift][iIter].carState = (int)sel[L"LiftStateId"];
		}

		// Deck Logs
		
		// number of deck logs
		sel = db.select(L"SELECT COUNT(DeckLogId) As NumDeckLogs FROM DeckLogs WHERE Iteration=0 AND LiftId = %d AND SimulationId=%d", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		if (!sel) return ERROR_SIM_MISSING;
		pnDeckLogs[iLift] = (int)sel[L"NumDeckLogs"];

		if (pnDeckLogs[iLift] == 0) continue;

		// allocate
		ppDeckLogs[iLift] = new DeckLog[pnDeckLogs[iLift]];
		memset(ppDeckLogs[iLift], 0, pnDeckLogs[iLift] * sizeof(DeckLog));
		
		sel = db.select(L"SELECT * FROM DeckLogs WHERE Iteration=0 AND LiftId = %d AND SimulationId=%d ORDER BY [Time]", pLiftGroup->GetLift(iLift)->GetShaft()->GetNativeId(), nSimulationId);
		for (int iIter = 0; sel; sel++, iIter++)
		{
			ppDeckLogs[iLift][iIter].Time = (float)sel[L"Time"];
			ppDeckLogs[iLift][iIter].deck = (int)sel[L"Deck"] - 1;
			ppDeckLogs[iLift][iIter].doorState = (int)sel[L"DoorStateId"];
		}
	}

	return 0;
}

DWORD CSimLoader::Load(CLiftGroupSrv *pLiftGroup, LPCOLESTR pName)
{
	ifstream myFile (pName, ios::in | ios::binary);
	if (!myFile) return ERROR_FILE_NOTFOUND;

	bDB = false;
	nLifts = pLiftGroup->GetLiftCount();

	try
	{
		int nBytes = 0;					// number of bytes read
		int nSign = 0;					// file signature

		// read signature and version
		nBytes += Load(myFile, nSign);
		nBytes += Load(myFile, nVersion);

		if (nSign != 0x5a4b5341) throw ERROR_FILE_FORMAT;
		if (nVersion < 108) throw ERROR_FILE_VERSION;
		if (nVersion > 109) throw ERROR_FILE_VERSION;

		// floors: read and ignore
		int dummy;
		nBytes += Load(myFile, dummy);
		nBytes += Load(myFile, sizeof(double) * dummy);

		// lifts: read and ignore
		nBytes += Load(myFile, dummy);
		nBytes += Load(myFile, (2 * sizeof(int) + 5 * sizeof(double)) * dummy);

		// no of simulations: read and ignore
		nBytes += Load(myFile, dummy);	// no of simulations - disused

		// read no of passengers
		nBytes += Load(myFile, nPassengers);

		// read passengers
		// assume version 1.01 or higher
		pPassengers = new Passenger[nPassengers];
		nBytes += Load(myFile, pPassengers, sizeof(Passenger) * nPassengers);
				
		// read lift simulation data
		pnLiftLogs = new int[nLifts];
		memset(pnLiftLogs, 0, sizeof(int)*nLifts);
		ppLiftLogs = new LiftLog*[nLifts];
		memset(ppLiftLogs, 0, sizeof(LiftLog*)*nLifts);

		for (int i = 0; i < nLifts; i++)	// i = lift index
		{
			// read no of sim iters
			nBytes += Load(myFile, pnLiftLogs[i]);
			if (pnLiftLogs[i] == 0) continue;

			// read data
			int nDeckCount = pLiftGroup->GetLift(i)->GetShaft()->GetDeckCount();
			if (nDeckCount < 1 || nDeckCount > 2) throw ERROR_FILE_DECKS;
			size_t nSize = sizeof(LiftLog_Base) + nDeckCount * sizeof(LiftLog::Deck);
			BYTE *pData = new BYTE[pnLiftLogs[i] * nSize];
			nBytes += Load(myFile, pData, pnLiftLogs[i] * nSize);

			// create the usable array
			ppLiftLogs[i] = new LiftLog[pnLiftLogs[i]];
			memset(ppLiftLogs[i], 0, pnLiftLogs[i] * sizeof(LiftLog));

			// mem copy data & clean up
			BYTE *p = pData;
			for (int j = 0; j < pnLiftLogs[i]; j++)
			{
				memcpy(&ppLiftLogs[i][j], p, nSize);
				ppLiftLogs[i][j].curShaft = i;
				p += nSize;
			}
			delete [] pData;
		}
	}
	catch(DWORD n)
	{
		myFile.close();
		return n;
	}
	myFile.close();
	return 0;
}

void CSimLoader::Print()
{
	// display header
	cout << "HEADER:" << endl;
	cout << "Version: " << nVersion << endl;
	cout << nLifts << " lifts" << endl;
	cout << nPassengers << " passengers" << endl;
	cout << endl;

	// display passengers
	cout << "Passenger Info:" << endl;
	for (int i = 0; i < nPassengers; i++)
	{
		cout << "Passenger #" << i 
				<< " time: " << pPassengers[i].ArrivalTime 
				<< " travelling from " << pPassengers[i].ArrivalFloor 
				<< " to " << pPassengers[i].DestinationFloor 
				<< " by lift #" << pPassengers[i].CarID + 1 << endl;
		if (i >= 10)
		{
			cout << "Truncated after 10 items..." << endl;
			break;
		}
	}
	cout << endl;

	// display simulation iterations
	cout << "Simulation Data:" << endl;
	for (int i = 0; i < nLifts; i++)
	{
		cout << "LIFT #" << i << " (" << pnLiftLogs[i] << " iterations)" << endl;
		for (int j = 0; j < pnLiftLogs[i]; j++)
		{
			cout << "  " << j << "."
			<< " Time = " << ppLiftLogs[i][j].Time
			<< " Floor: " << ppLiftLogs[i][j].curFloor
			<< " Dest: " << ppLiftLogs[i][j].destFloor
			<< " State: " << (int)ppLiftLogs[i][j].carState
			<< endl << "     - Deck 0:"
			<< " door state: " << (int)ppLiftLogs[i][j].deck[0].doorState
			<< " pasgr count: " << (int)ppLiftLogs[i][j].deck[0].passengersCount
			<< endl << "     - Deck 1:"
			<< " door state: " << (int)ppLiftLogs[i][j].deck[1].doorState
			<< " pasgr count: " << (int)ppLiftLogs[i][j].deck[1].passengersCount << endl;

			if (j >= 10)
			{
				cout << "Truncated after 10 items..." << endl;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
// CSimJourneyResolver

CSimJourneyResolver::CSimJourneyResolver(std::vector<JOURNEY> &J) : journeys(J), pJourney(NULL)
{
}

void CSimJourneyResolver::Run(CLiftGroupSrv::LIFT *pLIFT, CSimLoader &loader, AVULONG nLiftId)
{
	stCar = CAR_STOP;

	for (AVULONG i = 0; i < DECK_NUM; i++)
	{
		stDoor[i] = DOOR_CLOSED;
		lt[i] = 0;
	}

	tmpShaft = nLiftId / 9;

	journeys.push_back(JOURNEY());
	pJourney = &journeys.back();
	int iDL = 0;
	for (int iLL = 0; iLL < loader.pnLiftLogs[nLiftId]; iLL++)
	{
		CSimLoader::LiftLog ll = loader.ppLiftLogs[nLiftId][iLL];
		while (loader.pnDeckLogs && iDL < loader.pnDeckLogs[nLiftId] && loader.ppDeckLogs[nLiftId][iDL].Time <= loader.ppLiftLogs[nLiftId][iLL].Time)
		{
			CSimLoader::DeckLog dl = loader.ppDeckLogs[nLiftId][iDL];
			ll.Time = dl.Time;
			ll.deck[dl.deck].doorState = dl.doorState;
			Record(ll, loader.bDB, pLIFT->GetShaft()->GetDeckCount() > 1, pLIFT->GetShaft()->GetOpeningTime(), pLIFT->GetShaft()->GetClosingTime());
			iDL++;
		}
		ll.Time = loader.ppLiftLogs[nLiftId][iLL].Time;
		Record(ll, loader.bDB, pLIFT->GetShaft()->GetDeckCount() > 1, pLIFT->GetShaft()->GetOpeningTime(), pLIFT->GetShaft()->GetClosingTime());
	}
	journeys.pop_back();
}

void CSimJourneyResolver::RecordOpen(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToOpen)
{
	ASSERT(door[iDeck].m_timeOpen == UNDEF);
	door[iDeck].m_timeOpen = time; door[iDeck].m_durationOpen = min(duration, timeToOpen);
}

void CSimJourneyResolver::RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose)
{
	ASSERT(door[iDeck].m_timeOpen != UNDEF);
	door[iDeck].m_timeClose = time; door[iDeck].m_durationClose = min(duration, timeToClose);
	pJourney->m_doorcycles[iDeck].push_back(door[iDeck]);
	door[iDeck].reset();
}

void CSimJourneyResolver::Record(CSimLoader::LiftLog &liftlog, bool bDB, bool bDoubleDeck, AVULONG timeToOpen, AVULONG timeToClose)
{
	AVULONG t = (AVULONG)(liftlog.Time * 1000);
	AVULONG floorFrom = liftlog.curFloor;
	AVULONG floorTo   = liftlog.destFloor;
	AVULONG shaft = liftlog.curShaft;

	// shaft = tmpShaft;	// this was added in Circular Lifts edition...

	// identify car event/state as move, stop or between shafts
	enum CAR  evCar;
	if (bDB)
	{
		if (liftlog.carState == 0) evCar = CAR_MOVE; 
		else if (liftlog.carState == 4) evCar = CAR_SHAFT_MOVE;
		else evCar = CAR_STOP;
	}
	else
	{
		if (liftlog.carState == 1) evCar = CAR_MOVE; 
		else if (liftlog.carState == 4) evCar = CAR_SHAFT_MOVE;
		else evCar = CAR_STOP;
	}

	// identify door states
	enum DOOR evDoor[DECK_NUM];
	for (AVULONG i = 0; i < DECK_NUM; i++) 
		evDoor[i] = (enum DOOR)liftlog.deck[i].doorState;
		
	ASSERT(evDoor[1] == DOOR_CLOSED || bDoubleDeck);	// second door always closed unless double deck

	if (evCar != stCar)
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
			}
			pJourney->m_shaftFrom = shaft;
			pJourney->m_floorFrom = floorFrom;
			pJourney->m_timeGo = t;
			for (AVULONG i = 0; i < DECK_NUM; i++) 
			{
				ASSERT(evDoor[i] == DOOR_CLOSED);
				if (stDoor[i] == DOOR_CLOSING) { RecordClose(i, lt[i], t - lt[i], timeToClose); }
				if (stDoor[i] == DOOR_OPENING) { RecordOpen (i, lt[i], t - lt[i], timeToOpen); }
				stDoor[i] = DOOR_CLOSED;
				ASSERT(evDoor[i] == stDoor[i]);
			}
			break;
		case CAR_STOP:
			pJourney->m_shaftTo = shaft;
			pJourney->m_timeDest = t;
			pJourney->m_floorTo = floorTo;
			if (stCar == CAR_SHAFT_MOVE) pJourney->m_floorTo = floorFrom;
			ASSERT(pJourney->m_floorFrom != UNDEF && pJourney->m_floorTo != UNDEF);
			ASSERT(pJourney->m_timeDest != UNDEF && pJourney->m_timeGo != UNDEF);
			journeys.push_back(JOURNEY());	// start a new journey
			pJourney = &journeys.back();
			break;
		}
	stCar = evCar;

	if (liftlog.carState == 4) tmpShaft = 1 - tmpShaft;

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (evDoor[i] != stDoor[i])
		{
//			ASSERT(pJourney->m_timeGo == UNDEF);
			switch (evDoor[i])
			{
			case DOOR_CLOSED:
				ASSERT(stDoor[i] != DOOR_CLOSED);
				if      (stDoor[i] == DOOR_OPENING)	{ AVULONG t2 = (t - lt[i]) / 2; RecordOpen(i, lt[i], t2, timeToOpen); RecordClose(i, t-t2, t2, timeToClose); }
				else if (stDoor[i] == DOOR_CLOSING)	RecordClose(i, lt[i], t - lt[i], timeToClose);
				else 								RecordClose(i, t, timeToClose, timeToClose);		// only if stDoor[i] == DOOR_OPENED
				break;
			case DOOR_OPENING:
				ASSERT(stDoor[i] != DOOR_OPENED && stDoor[i] != DOOR_OPENING);
				if (stDoor[i] == DOOR_CLOSING)		RecordClose(i, lt[i], t - lt[i], timeToClose);
				break;
			case DOOR_OPENED:
				ASSERT(stDoor[i] != DOOR_OPENED);
				if (stDoor[i] == DOOR_OPENING)		RecordOpen(i, lt[i], t - lt[i], timeToOpen);
				else if (stDoor[i] == DOOR_CLOSING)	{ AVULONG t2 = (t - lt[i]) / 2; RecordClose(i, lt[i], t2, timeToClose); RecordOpen(i, t-t2, t2, timeToOpen); }
				else 								RecordOpen(i, t, timeToOpen, timeToOpen);		// only if stDoor[i] == DOOR_CLOSED
				break;
			case DOOR_CLOSING:
				ASSERT(stDoor[i] != DOOR_CLOSED && stDoor[i] != DOOR_CLOSING);
				if (stDoor[i] == DOOR_OPENING)		RecordOpen(i, lt[i], t - lt[i], timeToOpen); 
				break;
			}
			stDoor[i] = evDoor[i];
			lt[i] = t;
		}
}
