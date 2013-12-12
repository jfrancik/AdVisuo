// SimLoader.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseSimClasses.h"

/////////////////////////////////////////////////////////////
// Converts simulation data loaded from the DB
// to an array of JOURNEYs.

class CSimLiftJourneySeq
{
	std::vector<JOURNEY> &journeys;
	JOURNEY *pJourney;

	JOURNEY::DOOR door[DECK_NUM];

	enum CAR  { CAR_MOVE, CAR_MOTOR, CAR_IDLE, CAR_HALLCALL, CAR_SHAFT_MOVE, CAR_STOP };
	enum DOOR { DOOR_CLOSING = 1, DOOR_CLOSED = 0, DOOR_OPENING = 2, DOOR_OPENED = 3 };
	enum CAR  stCar;
	enum DOOR stDoor[DECK_NUM];
	AVULONG lt[DECK_NUM];

public:
	CSimLiftJourneySeq(std::vector<JOURNEY>&);
	void Run(dbtools::CDataBase db, AVULONG nLiftId, AVULONG nLiftNativeId, AVULONG nTrafficScenarioId, AVULONG Iteration, AVULONG timeToOpen, AVULONG timeToClose);

private:
	void RecordDeckLog(AVULONG t, int iDeck, enum DOOR evDoor, AVULONG timeToOpen, AVULONG timeToClose);
	void RecordLiftLog(AVULONG t, enum CAR, AVULONG floorFrom, AVULONG floorTo, AVULONG shaft, AVULONG timeToOpen, AVULONG timeToClose);
	void RecordOpen(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToOpen);
	void RecordClose(AVULONG iDeck, AVULONG time, AVULONG duration, AVULONG timeToClose);
};

#pragma pack(8)
