// Passenger.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "SrvPassenger.h"
#include "SrvSim.h"

using namespace dbtools;

CPassengerSrv::CPassengerSrv(CSimSrv *pSim, AVULONG nPassengerId) : CPassenger(pSim, nPassengerId)
{
}

inline AVULONG timeToGo(AVFLOAT x1, AVFLOAT y1, AVFLOAT x2, AVFLOAT y2, AVFLOAT stepLen, AVULONG stepDuration)
{
	AVFLOAT fDist = sqrt((y1 - y2) * (y1 - y2) + (x1 - x2) * (x1 - x2));
	return ((int)(fDist / stepLen) + 1) * stepDuration;
}

void CPassengerSrv::Play()
{
	// Calculate Spatial Points

	CBuildingSrv::SHAFT *pSHAFT = GetSim()->GetBuilding()->GetShaft(GetShaftId());
	ASSERT(pSHAFT);

	// initial parameters: which side to enter?
	enum { FRONT, LEFT, RIGHT } flagEnter = FRONT;
	switch (GetSim()->GetBuilding()->GetLobbyArrangement())
	{
	case CBuildingSrv::LOBBY_OPENPLAN: flagEnter = FRONT; break;
	case CBuildingSrv::LOBBY_DEADEND_RIGHT: flagEnter = LEFT; break;
	case CBuildingSrv::LOBBY_DEADEND_LEFT: flagEnter = RIGHT; break;
	case CBuildingSrv::LOBBY_THROUGH: if (rand() % 2 == 1) flagEnter = LEFT; else flagEnter = RIGHT; break;
	}
	
	// initial parameters: which likft line (0 for inline arrangement, 1 or 2)
	int nLine = 0;
	if (GetSim()->GetBuilding()->GetLiftShaftArrang() != CBuildingSrv::SHAFT_INLINE)
		nLine = pSHAFT->GetShaftLine() + 1;

	// passengers behaviour
	enum { DESTINATION, NATURAL } flagBehaviour = DESTINATION;
	if (!pSHAFT) flagBehaviour = NATURAL;


	AVFLOAT WLobby = abs(GetSim()->GetBuilding()->GetBox().Width());
	AVFLOAT DLobby = abs(GetSim()->GetBuilding()->GetBox().Depth());

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
		AVFLOAT WShaft = abs(pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_SHAFT).Width());
		AVFLOAT WLift = abs(pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_CAR).Width());
		AVFLOAT DLift = abs(pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_CAR).Depth());
		AVFLOAT WDoor = abs(pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_DOOR).Width());
		AVFLOAT WOpt = (WShaft + WLift) / 2;	// more optimal zone (should be even overlapping?) - 8/8/11
	
		AVFLOAT YLiftFront = DLobby * 0.4f;											// 8/8/11: DLobby/3  ==>  DLobby * 0.4
		AVFLOAT YLiftDoor = -pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_DOOR).Front();
	
		AVFLOAT XLift  = (pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_DOOR).Left()+pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_DOOR).Right())/2;
		AVFLOAT YLift = -(pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_CAR).Front()+pSHAFT->GetBox(CBuildingSrv::SHAFT::BOX_CAR).Rear())/2;
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

		SetGoTime(GetLoadTime() + 1600
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

DWORD CPassengerSrv::Load(AVULONG nId, CSimLoader::Passenger &P)
{
	SetId(nId);
	SetArrivalTime((AVULONG)(P.ArrivalTime * 1000));
	SetArrivalFloor(P.ArrivalFloor + GetSim()->GetBuilding()->GetBasementStoreyCount());
	SetDestFloor(P.DestinationFloor + GetSim()->GetBuilding()->GetBasementStoreyCount());
	SetLiftId(P.CarID);
	SetShaftId(P.CarID);	// provisionary setting
	SetDeck(0);				// provisionary setting

	SetWaitSpan((AVULONG)(P.WaitingTime * 1000));
	SetLoadTime(GetArrivalTime() + (AVULONG)(P.WaitingTime * 1000));
	SetUnloadTime(GetArrivalTime() + (AVULONG)(P.JourneyTime * 1000));

	return S_OK;
}

HRESULT CPassengerSrv::Store(CDataBase db)
{
	if (!db) throw db;
	CDataBase::INSERT ins = db.insert(L"AVPassengers");
	ins[L"PassengerId"] = GetId();
	ins[L"SimID"] = GetSimId();
	ins[L"ShaftId"] = GetShaftId();
	ins[L"LiftId"] = GetLiftId();
	ins[L"DeckId"] = GetDeck();
	ins[L"FloorArrival"] = GetArrivalFloor();
	ins[L"FloorDest"] = GetDestFloor();
	ins[L"TimeBorn"] = GetBornTime();
	ins[L"TimeArrival"] = GetArrivalTime();
	ins[L"TimeGo"] = GetGoTime();
	ins[L"TimeLoad"] = GetLoadTime();
	ins[L"TimeUnload"] = GetUnloadTime();
	ins[L"SpanWait"] = GetWaitSpan();
	ins[L"WP"] = StringifyWayPoints();
	ins.execute();
	return S_OK;
}

