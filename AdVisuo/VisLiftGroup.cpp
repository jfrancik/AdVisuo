// VisLiftGroup.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisLiftGroup.h"
#include "VisElem.h"
#include "Engine.h"
#include "VisSim.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLiftGroupVis

CSim *CLiftGroupVis::CreateSim()
{
	return new CSimVis();
}

AVVECTOR CLiftGroupVis::GetLiftPos(int nLift)
{
	return GetLiftElement(nLift)->GetPos();
}

void CLiftGroupVis::StoreConfig()
{
	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
	{
		GetLiftElement(iLift)->PushState();
		for (AVULONG iDeck = 0; iDeck < GetLift(iLift)->GetShaft()->GetDeckCount(); iDeck++)
			for (AVULONG iDoor = 0; iDoor < MAX_DOORS; iDoor++)
				if (GetLiftDoor(iLift, iDeck, iDoor))	GetLiftDoor(iLift, iDeck, iDoor)->PushState();
	}
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		for (AVULONG j = 0; j < GetShaftCount(); j++)
			for (AVULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
					GetShaftDoor(i, j, k)->PushState();
}

void CLiftGroupVis::RestoreConfig()
{
	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
	{
		GetLiftElement(iLift)->PopState();
		GetLiftElement(iLift)->Invalidate();
		GetLiftElement(iLift)->PushState();
		for (AVULONG iDeck = 0; iDeck < GetLift(iLift)->GetShaft()->GetDeckCount(); iDeck++)
			for (AVULONG iDoor = 0; iDoor < MAX_DOORS; iDoor++)
				if (GetLiftDoor(iLift, iDeck, iDoor))	
				{ 
					GetLiftDoor(iLift, iDeck, iDoor)->PopState(); 
					GetLiftDoor(iLift, iDeck, iDoor)->Invalidate(); 
					GetLiftDoor(iLift, iDeck, iDoor)->PushState(); 
				}
	}
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		for (AVULONG j = 0; j < GetShaftCount(); j++)
			for (AVULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
				{
					GetShaftDoor(i, j, k)->PopState();
					GetShaftDoor(i, j, k)->Invalidate();
					GetShaftDoor(i, j, k)->PushState();
				}
}

