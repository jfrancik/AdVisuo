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
	AVVECTOR vec = { 0, 0, 0 };
	if (GetLiftElement(nLift)->GetBone())
		GetLiftElement(nLift)->GetBone()->LtoG((FWVECTOR*)&vec);
	return vec;
}

void CLiftGroupVis::StoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PushState();
		for (FWULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	GetLiftDoor(i, k)->PushState();
	}
	for (FWULONG i = 0; i < GetStoreyCount(); i++)
		for (FWULONG j = 0; j < GetShaftCount(); j++)
			for (FWULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
					GetShaftDoor(i, j, k)->PushState();
}

void CLiftGroupVis::RestoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PopState();
		for (FWULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	{ GetLiftDoor(i, k)->PopState(); GetLiftDoor(i, k)->Invalidate(); GetLiftDoor(i, k)->PushState(); }
	}
	for (FWULONG i = 0; i < GetStoreyCount(); i++)
		for (FWULONG j = 0; j < GetShaftCount(); j++)
			for (FWULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
				{
					GetShaftDoor(i, j, k)->PopState();
					GetShaftDoor(i, j, k)->Invalidate();
					GetShaftDoor(i, j, k)->PushState();
				}
}

