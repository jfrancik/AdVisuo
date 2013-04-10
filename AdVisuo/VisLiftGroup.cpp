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
	for (AVULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PushState();
		for (AVULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	GetLiftDoor(i, k)->PushState();
	}
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		for (AVULONG j = 0; j < GetShaftCount(); j++)
			for (AVULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
					GetShaftDoor(i, j, k)->PushState();
}

void CLiftGroupVis::RestoreConfig()
{
	for (AVULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PopState();
		for (AVULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	{ GetLiftDoor(i, k)->PopState(); GetLiftDoor(i, k)->Invalidate(); GetLiftDoor(i, k)->PushState(); }
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

