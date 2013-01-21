// SimLoader.h - a part of the AdVisuo Server Module

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "../CommonFiles/BaseSimClasses.h"
#include "SrvLiftGroup.h"

#pragma pack(1)

/////////////////////////////////////////////////////////////
// Read data from a SIM file, stores in internal data format.
// Passenger and floor data are straightforward.
// Use CSimJourneyResolver to export data to CLift objects.

class CLiftGroupSrv;

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
	
	// Car States:
	// DB:
	//  0  Move
	//  1  MotorStarting
	//  2  Idle
	//  3  Transfer
	// File:
	//  0  Idle,                waiting for new task
	//  1  MoveToTask,          on the way to task (lift is moving)
	//  2  MotorStarting       
	//  3  HallCallsTransfer,door is open: loading or unloadaing
	// Door States:
	//  0  Closed
	//  1  Closing
	//  2  Opening
	//  3  Open
	
	struct LiftLog_Base
	{
		double Time;					// (in seconds)
		int curShaft;					// shaft id value (new to version 1.06)!
		int curFloor;					// location of the lift
		int destFloor;					// destination where is  the lift heading to
		BYTE carState;					// Car states
	};

	struct LiftLog : public LiftLog_Base
	{
		struct Deck 
		{
			BYTE doorState;				// Door States
			BYTE passengersCount;		// passenger count (per deck)
		} deck[DECK_NUM];
	};

	struct DeckLog	// specific for the DB
	{
		double Time;
		int deck;
		BYTE doorState;
	};

public:

	int nVersion;				// file version
	bool bDB;					// true if read from a DB; false for a file
	
	int nPassengers;			// number of passengers (hall calls)
	Passenger	*pPassengers;	// passenger info for each passenger
	
	int			nLifts;			// number of lifts
	
	int			*pnLiftLogs;	// sim iters for each lift
	LiftLog		**ppLiftLogs;	// sim iter info for each sim iter for each lift

	int			*pnDeckLogs;	// deck logs for each lift and deck
	DeckLog		**ppDeckLogs;	// deck log info for each deck log for each lift and deck

	CSimLoader();
	~CSimLoader();

	DWORD Load(CLiftGroupSrv *pLiftGroup, dbtools::CDataBase db, ULONG nSimulationId, ULONG nTrafficScenarioId, ULONG nIteration);	// returns S_OK if successful
	DWORD Load(CLiftGroupSrv *pLiftGroup, LPCOLESTR pName);		// returns S_OK if successful
	void Print();

private:
	int Load(std::ifstream &stream, void *pBuf, size_t nSize);
	int Load(std::ifstream &stream, size_t nSize);
	int Load(std::ifstream &stream, int &v)		{ return Load(stream, &v, sizeof(int)); }
	int Load(std::ifstream &stream, double &v)	{ return Load(stream, &v, sizeof(int)); }
};

/////////////////////////////////////////////////////////////
// Converts simulation data loaded by a SimLoader (LiftLog)
// to an array of JOURNEYs.

class CSimJourneyResolver
{
	std::vector<JOURNEY> &journeys;
	JOURNEY *pJourney;

	JOURNEY::DOOR door[DECK_NUM];

	enum CAR  { CAR_STOP, CAR_MOVE, CAR_SHAFT_MOVE };
	enum DOOR { DOOR_CLOSING = 1, DOOR_CLOSED = 0, DOOR_OPENING = 2, DOOR_OPENED = 3 };
	enum CAR  stCar;
	enum DOOR stDoor[DECK_NUM];
	AVULONG lt[DECK_NUM];
	AVULONG tmpShaft;	// temporary addition!

public:
	CSimJourneyResolver(std::vector<JOURNEY>&);
	void Run(CLiftGroupSrv::LIFT *pLIFT, CSimLoader &loader, AVULONG nLiftId);

private:
	void Record(CSimLoader::LiftLog &liftlog, bool bDB, bool bDoubleDeck, AVULONG timeToOpen, AVULONG timeToClose);
	void RecordOpen(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToOpen);
	void RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose);
};

#pragma pack(8)
