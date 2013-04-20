// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisProject.h"
#include "VisSim.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "Engine.h"
#include "vector"

#pragma warning(disable:4018)

using namespace std;

CSimVis::CSimVis() : m_nColouringMode(0)
{
}

CSimVis::~CSimVis(void)
{
}

void CSimVis::Play(CEngine *pEngine, AVLONG nTime)
{
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Play(pEngine, nTime);

	int nCount = 0;
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetSpawnTime() >= nTime)
		{
			GetPassenger(i)->Play(pEngine);
			nCount++;
		}
	
	// this is a temporary assertion to detect crap
	if (nCount == 0 && nTime <= 600000)
	{
		TRACE(L"Shit happens at %d\n", nTime);
		ASSERT(FALSE);
	}
}

AVLONG CSimVis::FastForward(CEngine *pEngine, AVLONG nTime)
{
	AVLONG nEarliestTime = 0x7FFFFFFF;

	for (AVULONG i = 0; i < GetLiftCount(); i++)
	{
		AVLONG t = GetLift(i)->FastForward(pEngine, nTime);
		nEarliestTime = min(nEarliestTime, t);
	}

	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetUnloadTime() >= nTime)
		{
			GetPassenger(i)->Play(pEngine);

			AVLONG t = GetPassenger(i)->GetSpawnTime();
			nEarliestTime = min(nEarliestTime, t);
		}
	return min(nEarliestTime, GetProject()->GetMaxSimulationTime());
}

void CSimVis::RenderPassengers(AVLONG nPhase)
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Render(nPhase);
}

void CSimVis::Stop()
{
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Die();
}

CPassenger *CSimVis::CreatePassenger(AVULONG nId)	{ return new CPassengerVis(this, nId); }
CLift *CSimVis::CreateLift(AVULONG nId)				{ return new CLiftVis(this, nId); }
