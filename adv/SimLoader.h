// SimLoader.h - a part of the AdVisuo Server Module

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "../CommonFiles/BaseClasses.h"

#pragma pack(1)

/////////////////////////////////////////////////////////////
// Read data from a SIM file, stores in internal data format.
// Passenger and floor data are straightforward.
// Use CSimJourneyResolver to export data to CLift objects.

class CSimLoader
{
public:
	struct Passenger
	{
		int ArrivalFloor;
		int DestinationFloor;
		double ArrivalTime;
		int CarID;
		double WaitingTime;
		double TransitTime;
		double JourneyTime;
		double ToDestTime;
		double CarArrivalTime;
		double CarDepTime;
		double CarDestTime;
		double StartLoadingTime;		// added in 1.01
		double StartUnloadingTime;		// added in 1.01
	};
	
	struct Lift
	{
		int nDecks;
		int nHomeFloor;

		double TimeToOpen;				// new to version 1.08
		double TimeToClose;				// new to version 1.08
		double Speed;					// new to version 1.08
		double Acceleration;			// new to version 1.08
		double Jerk;					// new to version 1.08
	};

	// Car States:
	//  0  Idle,                waiting for new task
	//  1  MoveToTask,          on the way to task (lift is moving)
	//  2  MotorStarting       
	//  3  HallCallsTransfer,door is open: loading or unloadaing
	// Door States:
	//  0  Closed
	//  1  Closing
	//  2  Opening
	//  3  Open
	
	struct SimIter_Base
	{
		double Time;					// (in seconds)
		int curShaft;					// shaft id value (new to version 1.06)!
		int curFloor;					// location of the lift
		int destFloor;					// destination where is  the lift heading to
		BYTE carState;					// Car states
	};

	struct SimIter : public SimIter_Base
	{
		struct Deck 
		{
			BYTE doorState;				// Door States
			BYTE passengersCount;		// passenger count (per deck)
		} deck[DECK_NUM];
	};

	int nBytes;					// number of bytes read
	int nSign;					// file signature
	int nVersion;				// file version

	int nFloors;				// number of floors
	int nLifts;					// number of lifts
	int nSims;					// number of simulations (ignored now)
	int nPassengers;			// number of passengers (hall calls)
	
	double		*pFloors;		// height for each floor
	Lift		*pLifts;		// lifts info: decks & home floor
	Passenger	*pPassengers;	// passenger info for each passenger
	int			*pIters;		// sim iters for each lift
	SimIter		**ppSimIters;	// sim iter info for each sim iter for each lift

	CSimLoader();
	~CSimLoader();

	DWORD Load(LPCOLESTR pName);	// returns S_OK if successful
	void Print();

private:
	int Load(std::ifstream &stream, void *pBuf, size_t nSize);
	int Load(std::ifstream &stream, int &v)		{ return Load(stream, &v, sizeof(int)); }
	int Load(std::ifstream &stream, double &v)	{ return Load(stream, &v, sizeof(int)); }
};

/////////////////////////////////////////////////////////////
// Converts simulation data loaded by a SimLoader (SimIter)
// to an array of JOURNEYs.

class CSimJourneyResolver
{
	std::vector<JOURNEY> &journeys;
	JOURNEY *pJourney;

	JOURNEY::DOOR door[DECK_NUM];

	enum CAR  { CAR_STOP, CAR_MOVE, CAR_SHAFT };
	enum DOOR { DOOR_CLOSING = 1, DOOR_CLOSED = 0, DOOR_OPENING = 2, DOOR_OPENED = 3 };
	enum CAR  stCar;
	enum DOOR stDoor[DECK_NUM];
	AVULONG lt[DECK_NUM];

public:
	CSimJourneyResolver(std::vector<JOURNEY>&);
	void Run(CSimLoader &loader, AVULONG nLiftId);

private:
	void Record(CSimLoader::SimIter &simiter, bool bDoubleDeck, AVULONG timeToOpen, AVULONG timeToClose);
	void RecordOpen(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToOpen);
	void RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose);
};

#pragma pack(8)
