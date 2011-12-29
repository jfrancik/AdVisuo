// Passenger.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Passenger.h"
#include "Sim.h"

using namespace dbtools;

CPassenger::CPassenger(CSim *pSim, AVULONG nPassengerId) : CPassengerBase(pSim, nPassengerId)
{
}

inline AVULONG timeToGo(AVFLOAT x1, AVFLOAT y1, AVFLOAT x2, AVFLOAT y2, AVFLOAT stepLen, AVULONG stepDuration)
{
	AVFLOAT fDist = sqrt((y1 - y2) * (y1 - y2) + (x1 - x2) * (x1 - x2));
	return ((int)(fDist / stepLen) + 1) * stepDuration;
}

void CPassenger::Play()
{
	// Calculate Spatial Points

	CBuilding::SHAFT *pSHAFT = GetSim()->GetBuilding()->GetShaft(GetShaftId());
	ASSERT(pSHAFT);

	// initial parameters: which side to enter?
	enum { FRONT, LEFT, RIGHT } flagEnter = FRONT;
	switch (GetSim()->GetBuilding()->GetLobbyArrangement())
	{
	case CBuilding::LOBBY_OPENPLAN: flagEnter = FRONT; break;
	case CBuilding::LOBBY_DEADEND_RIGHT: flagEnter = LEFT; break;
	case CBuilding::LOBBY_DEADEND_LEFT: flagEnter = RIGHT; break;
	case CBuilding::LOBBY_THROUGH: if (rand() % 2 == 1) flagEnter = LEFT; else flagEnter = RIGHT; break;
	}
	
	// initial parameters: which likft line (0 for inline arrangement, 1 or 2)
	int nLine = 0;
	if (GetSim()->GetBuilding()->GetLiftShaftArrang() != CBuilding::SHAFT_INLINE)
		nLine = pSHAFT->ShaftLine + 1;

	// passengers behaviour
	enum { DESTINATION, NATURAL } flagBehaviour = DESTINATION;
	if (!pSHAFT) flagBehaviour = NATURAL;


	AVFLOAT WLobby = abs(GetSim()->GetBuilding()->GetBox()->Width());
	AVFLOAT DLobby = abs(GetSim()->GetBuilding()->GetBox()->Depth());

	AVFLOAT XLobby = 0;
	AVFLOAT YOutOfView = -DLobby/2 - 20;
	AVFLOAT XOutOfViewL = -WLobby/2 - 20;
	AVFLOAT XOutOfViewR =  WLobby/2 + 20;
	
	AVFLOAT YLobby;
	switch (nLine)
	{
	case 0:	YLobby = DLobby / 10; break;
	case 1: YLobby = DLobby / 8; break;				// 8/8/11 was: 6 changed to: 8
	case 2: YLobby = -DLobby / 8; break;			// 8/8/11 was: 6 changed to: 8
	}

	WLobby -= 10;
	AVFLOAT XLobby0, YLobby0, XLobby1, YLobby1, XLobbyEx, YLobbyEx;
	switch (flagEnter)
	{
	case FRONT:
		XLobby0 = XLobby + 0.6f * ((rand() % (AVULONG)WLobby) - WLobby/2);
		YLobby0 = YOutOfView;
		XLobbyEx = XLobby + 0.6f * ((rand() % (AVULONG)WLobby) - WLobby/2);
		YLobbyEx = YOutOfView;
		break;
	case LEFT:
		XLobby0 = XOutOfViewL;
		YLobby0 = 0.6f * ((rand() % (AVULONG)DLobby) - DLobby/2);
		XLobbyEx = XOutOfViewL;
		YLobbyEx = 0.6f * ((rand() % (AVULONG)DLobby) - DLobby/2);
		break;
	case RIGHT:
		XLobby0 = XOutOfViewR;
		YLobby0 = 0.6f * ((rand() % (AVULONG)DLobby) - DLobby/2);
		XLobbyEx = XOutOfViewR;
		YLobbyEx = 0.6f * ((rand() % (AVULONG)DLobby) - DLobby/2);
		break;
	}

	switch (flagBehaviour)
	{
//	case DESTINATION:
//		XLobby1 = XLobby + 0.9f * ((rand() % (AVULONG)WLobby) - WLobby/2);
//		YLobby1 = YLobby + 0.3f * ((rand() % (AVULONG)DLobby) - DLobby/2);
//		break;
	case NATURAL:
		XLobby1 = XLobby + 0.9f * ((rand() % (AVULONG)WLobby) - WLobby/2);
		YLobby1 = YLobby + 0.4f * ((rand() % (AVULONG)DLobby) - DLobby/2);		// 8/8/11 was: 0.3 changed to: 0.4
		break;
	}

	if (pSHAFT)
	{
		AVFLOAT WShaft = abs(pSHAFT->m_box.Width());
		AVFLOAT WLift = abs(pSHAFT->m_boxCar.Width());
		AVFLOAT DLift = abs(pSHAFT->m_boxCar.Depth());
		AVFLOAT WDoor = abs(pSHAFT->m_boxDoor.Width());
		AVFLOAT WOpt = (WShaft + WLift) / 2;	// more optimal zone (should be even overlapping?) - 8/8/11
	
		AVFLOAT YLiftFront = DLobby * 0.4f;											// 8/8/11: DLobby/3  ==>  DLobby * 0.4
		AVFLOAT YLiftDoor = -pSHAFT->m_boxDoor.Front();
	
		AVFLOAT XLift  = (pSHAFT->m_boxDoor.Left()+pSHAFT->m_boxDoor.Right())/2;
		AVFLOAT YLift = -(pSHAFT->m_boxCar.Front()+pSHAFT->m_boxCar.Rear())/2;
		AVFLOAT XLiftDoor = XLift;

		switch (flagBehaviour)
		{
		case DESTINATION:
			XLobby1 = XLift + 0.9f * ((rand() % (AVULONG)WOpt) - WOpt/2);			// 8/8/11: WLift ==> WOpt
			YLobby1 = YLobby + 0.4f * ((rand() % (AVULONG)DLobby) - DLobby/2);		// 8/8/11 was: 0.3 changed to: 0.4
			break;
		}

		AVFLOAT XLiftDoor0 = (XLiftDoor > XLobby1) ? XLiftDoor - WDoor/4 : XLiftDoor + WDoor/4;	// 8/8/11: was: WDoor is: WDoor/4
		AVFLOAT YLiftDoor0 = YLiftFront;
		if (nLine == 2) YLiftDoor0 = -YLiftDoor0;

		AVFLOAT XLift1 = XLift + 0.9f * ((rand() % (AVULONG)WLift) - WLift/2);
		AVFLOAT YLift1 = YLift + 0.9f * ((rand() % (AVULONG)DLift) - DLift/2);

		// Calculate Time Points

		AVULONG stepDuration = 150;
		AVFLOAT stepLen = 15.0f;

		SetGoTime(GetLoadTime()
			- timeToGo(XLobby1, YLobby1, XLiftDoor0, YLiftDoor0, stepLen, stepDuration)
			- timeToGo(XLiftDoor0, YLiftDoor0, XLiftDoor, YLiftDoor, stepLen, stepDuration));
		SetArrivalTime(min(GetArrivalTime(), GetGoTime()));
		SetBornTime(GetArrivalTime() - timeToGo(XLobby0, YLobby0, XLobby1, YLobby1, stepLen, stepDuration));

		WAYPOINT my_way_points[] =
		{
			{ MOVE, {XLobby0, YLobby0 }, 0, L"" },				// passenger journey start point
			{ WAIT, {0, 0}, GetBornTime(), L"watch;look" },		// wait...
			{ WALK, {XLobby1, YLobby1}, 0,L"" },				// go to lobby wait zone
			{ WAIT, {0, 0}, GetGoTime(), L"watch;look" },		// wait...

			{ WALK, {XLiftDoor0, YLiftDoor0}, 0, L"open" },		// go to the front of the lift
			{ WALK, {XLiftDoor, YLiftDoor}, 0, L"open" },		// go to the door
			{ ENTER_LIFT, {0, 0}, 0, L"" },						// enter the lift
			{ WALK, {XLift1, YLift1}, 0, L"close" },			// go inside the lift

			{ TURN, {0, 0}, 0, L"" },							// turn around
			{ WAIT, {0, 0}, GetUnloadTime(), L"look" },			// wait while in the lift and arrive @ the dest floor

			{ ENTER_DEST_FLOOR, {0, 0}, 0, L"" },				// enter the arrival floor
			{ WALK, {XLift, YLiftDoor}, 0, L"open" },			// go to the door
			{ WALK, {XLobbyEx, YLobbyEx}, 0, L"close" }			// go to the destination point upstairs
		};

		CreateWaypoints(sizeof(my_way_points) / sizeof(WAYPOINT));
		for (int i = 0; i < sizeof(my_way_points) / sizeof(WAYPOINT); i++)
			SetWaypoint(i, my_way_points[i]);
	}
	else
	{
		// Calculate Time Points

		AVULONG stepDuration = 150;
		AVFLOAT stepLen = 15.0f;

		SetGoTime(GetLoadTime());
		SetArrivalTime(min(GetArrivalTime(), GetGoTime()));
		SetBornTime(GetArrivalTime() - timeToGo(XLobby0, YLobby0, XLobby1, YLobby1, stepLen, stepDuration));

		WAYPOINT my_way_points[] =
		{
			{ MOVE, {XLobby0, YLobby0 }, 0, L"" },				// passenger journey start point
			{ WAIT, {0, 0}, GetBornTime(), L"watch;look" },		// wait...
			{ WALK, {XLobby1, YLobby1}, 0,L"" },				// go to lobby wait zone
			//{ WAIT, {0, 0}, GetGoTime(), L"watch;look" },		// wait...
		};

		CreateWaypoints(sizeof(my_way_points) / sizeof(WAYPOINT));
		for (int i = 0; i < sizeof(my_way_points) / sizeof(WAYPOINT); i++)
			SetWaypoint(i, my_way_points[i]);
	}
}

DWORD CPassenger::Load(AVULONG nId, CSimLoader::Passenger &P)
{
	SetId(nId);
	SetArrivalTime((AVULONG)(P.ArrivalTime * 1000));
	SetArrivalFloor(P.ArrivalFloor);
	SetDestFloor(P.DestinationFloor);
	SetLiftId(P.CarID);
	SetShaftId(P.CarID);	// provisionary setting
	SetDeck(0);				// provisionary setting

	SetWaitSpan((AVULONG)(P.WaitingTime * 1000));
	SetLoadTime(GetArrivalTime() + (AVULONG)(P.WaitingTime * 1000));
	SetUnloadTime(GetArrivalTime() + (AVULONG)(P.JourneyTime * 1000));

	return S_OK;
}

HRESULT CPassenger::Store(CDataBase db, ULONG nProjectID)
{
	if (!db) return db;
	try
	{
		CDataBase::INSERT ins = db.insert("AVPassengers");
		ins["ProjectID"] = nProjectID;
		ins["PassengerId"] = GetId();
		ins["ShaftId"] = GetShaftId();
		ins["LiftId"] = GetLiftId();
		ins["DeckId"] = GetDeck();
		ins["FloorArrival"] = GetArrivalFloor();
		ins["FloorDest"] = GetDestFloor();
		ins["TimeBorn"] = GetBornTime();
		ins["TimeArrival"] = GetArrivalTime();
		ins["TimeGo"] = GetGoTime();
		ins["TimeLoad"] = GetLoadTime();
		ins["TimeUnload"] = GetUnloadTime();
		ins["SpanWait"] = GetWaitSpan();
		ins["WP"] = StringifyWayPoints();
		ins.execute();
	}
    catch(HRESULT h)
    {
		return Log(ERROR_COM, h);
	}
	catch(_com_error &ce)
	{
		return Log(ERROR_DB, ce);
	}

	return S_OK;
}

