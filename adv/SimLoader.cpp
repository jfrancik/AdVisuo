// SimLoader.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SimLoader.h"
#include "../CommonFiles/BaseData.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// CSimLoader


//#define FILENAME "test4.sim"
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	cout << "Reading file: " << FILENAME << endl;
//	CSimLoader reader;
//	if (reader.Load(FILENAME) != S_OK)
//		cout << "ERROR!" << endl;
//	else
//		reader.Print();
//}

CSimLoader::CSimLoader()
{
	nBytes = 0;
	nSign = 0;
	nVersion = 0;
	nFloors = 0;
	nLifts = 0;
	nSims = 0;
	nPassengers = 0;
	pFloors = NULL;
	pLifts = NULL;
	pIters = NULL;
	pPassengers = NULL;
	ppSimIters = NULL;
}

CSimLoader::~CSimLoader()
{
	if (pFloors) delete [] pFloors;
	if (pLifts) delete [] pLifts;
	if (pPassengers) delete [] pPassengers;
	if (pIters) delete [] pIters;
	if (ppSimIters)
	{
		for (int i = 0; i < nLifts; i++)
			if (ppSimIters[i]) delete [] ppSimIters[i];
		delete [] ppSimIters;
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

DWORD CSimLoader::Load(LPCOLESTR pName)
{
	ifstream myFile (pName, ios::in | ios::binary);
	if (!myFile) return ERROR_FILE_NOTFOUND;

	try
	{
		// read signature and version
		nBytes += Load(myFile, nSign);
		nBytes += Load(myFile, nVersion);

		if (nSign != 0x5a4b5341) throw ERROR_FILE_FORMAT;
		if (nVersion < 106) throw ERROR_FILE_VERSION;
		if (nVersion > 109) throw ERROR_FILE_VERSION;

		// read no of floors
		nBytes += Load(myFile, nFloors);

		// read floor heights
		pFloors = new double[nFloors];
		nBytes += Load(myFile, pFloors, sizeof(double) * nFloors);

		// read no of lifts
		nBytes += Load(myFile, nLifts);

		// read lift decks
		// assume version 1.05 or higher
		pLifts = new Lift[nLifts];
		if (nVersion < 108)
		{
			struct Lift107 	{	int nDecks, nHomeFloor; };
			Lift107 *pLifts107 = new Lift107[nLifts];
			nBytes += Load(myFile, pLifts107, sizeof(Lift107) * nLifts);
			for (int i = 0; i < nLifts; i++)
			{
				pLifts[i].nDecks = pLifts107[i].nDecks;
				pLifts[i].nHomeFloor = pLifts107[i].nHomeFloor;
				pLifts[i].TimeToOpen = 1.8;
				pLifts[i].TimeToClose = 2.6;
				pLifts[i].Speed = 2.5;
				pLifts[i].Acceleration = 1.0;
				pLifts[i].Jerk = 1.6;
			}
			delete [] pLifts107;
		}
		else
			nBytes += Load(myFile, pLifts, sizeof(Lift) * nLifts);

		// read no of simulations --- this value is ignored!
		nBytes += Load(myFile, nSims);

		// read no of passengers
		nBytes += Load(myFile, nPassengers);

		// read passengers
		// assume version 1.01 or higher
		pPassengers = new Passenger[nPassengers];
		nBytes += Load(myFile, pPassengers, sizeof(Passenger) * nPassengers);
				
		// read lift simulation data
		pIters = new int[nLifts];
		memset(pIters, 0, sizeof(int)*nLifts);
		ppSimIters = new SimIter*[nLifts];
		memset(ppSimIters, 0, sizeof(SimIter*)*nLifts);

		for (int i = 0; i < nLifts; i++)	// i = lift index
		{
			// read no of sim iters
			nBytes += Load(myFile, pIters[i]);
			if (pIters[i] == 0) continue;

			// read data
			int nDeckCount = pLifts[i].nDecks;
			if (nDeckCount < 1 || nDeckCount > 2) throw ERROR_FILE_DECKS;
			size_t nSize = sizeof(SimIter_Base) + nDeckCount * sizeof(SimIter::Deck);
			BYTE *pData = new BYTE[pIters[i] * nSize];
			nBytes += Load(myFile, pData, pIters[i] * nSize);

			// create the usable array
			ppSimIters[i] = new SimIter[pIters[i]];
			memset(ppSimIters[i], 0, pIters[i] * sizeof(SimIter));

			// mem copy data & clean up
			BYTE *p = pData;
			for (int j = 0; j < pIters[i]; j++)
			{
				memcpy(&ppSimIters[i][j], p, nSize);
				ppSimIters[i][j].curShaft = i;
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
	cout << nBytes << " bytes read." << endl;
	cout << "Signature: " << nSign << endl;
	cout << "Version: " << nVersion << endl;
	cout << nFloors << " floors" << endl;
	cout << nLifts << " lifts" << endl;
	cout << nSims << " simulations" << endl;
	cout << nPassengers << " passengers" << endl;
	cout << endl;

	// display floor heights
	cout << "Floor Heights:" << endl;
	for (int i = 0; i < nFloors; i++)
		cout << "Floor #" << i << " height = " << pFloors[i] << endl;
	cout << endl;

	// display lift decks
	cout << "Lift Data:" << endl;
	for (int i = 0; i < nLifts; i++)
		cout << "Lift #" << i << " decks = " << pLifts[i].nDecks << " home floor = " << pLifts[i].nHomeFloor << endl;
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
		cout << "LIFT #" << i << " (" << pIters[i] << " iterations)" << endl;
		for (int j = 0; j < pIters[i]; j++)
		{
			cout << "  " << j << "."
			<< " Time = " << ppSimIters[i][j].Time
			<< " Floor: " << ppSimIters[i][j].curFloor
			<< " Dest: " << ppSimIters[i][j].destFloor
			<< " State: " << (int)ppSimIters[i][j].carState
			<< endl << "     - Deck 0:"
			<< " door state: " << (int)ppSimIters[i][j].deck[0].doorState
			<< " pasgr count: " << (int)ppSimIters[i][j].deck[0].passengersCount
			<< endl << "     - Deck 1:"
			<< " door state: " << (int)ppSimIters[i][j].deck[1].doorState
			<< " pasgr count: " << (int)ppSimIters[i][j].deck[1].passengersCount << endl;

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

void CSimJourneyResolver::Run(CSimLoader &loader, AVULONG nLiftId)
{
	stCar = CAR_STOP;

	for (AVULONG i = 0; i < DECK_NUM; i++)
	{
		stDoor[i] = DOOR_CLOSED;
		lt[i] = 0;
	}

	AVULONG nTimeToOpen = (AVULONG)(loader.pLifts[nLiftId].TimeToOpen * 1000);
	AVULONG nTimeToClose = (AVULONG)(loader.pLifts[nLiftId].TimeToClose * 1000);

	tmpShaft = nLiftId / 9;

	journeys.push_back(JOURNEY());
	pJourney = &journeys.back();
	for (int i = 0; i < loader.pIters[nLiftId]; i++)
		Record(loader.ppSimIters[nLiftId][i], loader.pLifts[nLiftId].nDecks > 1, nTimeToOpen, nTimeToClose);
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

void CSimJourneyResolver::Record(CSimLoader::SimIter &simiter, bool bDoubleDeck, AVULONG timeToOpen, AVULONG timeToClose)
{
	AVULONG t = (AVULONG)(simiter.Time * 1000);
	AVULONG floorFrom = simiter.curFloor;
	AVULONG floorTo   = simiter.destFloor;
	AVULONG shaft = simiter.curShaft;

	shaft = tmpShaft;

	// identify car event/state as move, stop or between shafts
	enum CAR  evCar;
	if (simiter.carState == 1) evCar = CAR_MOVE; 
	else if (simiter.carState == 4) evCar = CAR_SHAFT_MOVE;
	else evCar = CAR_STOP;

	// identify door states
	enum DOOR evDoor[DECK_NUM];
	for (AVULONG i = 0; i < DECK_NUM; i++) 
		evDoor[i] = (enum DOOR)simiter.deck[i].doorState;
		
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

	if (simiter.carState == 4) tmpShaft = 1 - tmpShaft;

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
