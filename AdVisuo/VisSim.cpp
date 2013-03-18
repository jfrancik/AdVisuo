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
	for (FWULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Play(pEngine, nTime);

	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetBornTime() >= nTime)
			GetPassenger(i)->Play(pEngine);
}

AVLONG CSimVis::FastForward(CEngine *pEngine, AVLONG nTime)
{
	AVLONG nEarliestTime = 0x7FFFFFFF;

	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		AVLONG t = GetLift(i)->FastForward(pEngine, nTime);
		nEarliestTime = min(nEarliestTime, t);
	}

	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetUnloadTime() >= nTime)
		{
			GetPassenger(i)->Play(pEngine);

			AVLONG t = GetPassenger(i)->GetBornTime();
			nEarliestTime = min(nEarliestTime, t);
		}
	return min(nEarliestTime, GetProject()->GetMaxSimulationTime());
}

void CSimVis::RenderPassengers(IRenderer *pRenderer, AVLONG nPhase)
{
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Render(pRenderer, nPhase);
}

void CSimVis::Stop()
{
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Die();
}

CPassenger *CSimVis::CreatePassenger(AVULONG nId)	{ return new CPassengerVis(this, nId); }
CLift *CSimVis::CreateLift(AVULONG nId)				{ return new CLiftVis(this, nId); }
