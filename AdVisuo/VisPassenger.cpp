// Passenger.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisElem.h"
#include "VisPassenger.h"
#include "VisSim.h"
#include "VisLiftGroup.h"
#include "Engine.h"

#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

CPassengerVis::CPassengerVis(CSimVis *pSim, AVULONG nPassengerId) 
	: CPassenger(pSim, nPassengerId), m_pBody(NULL)
{
	m_pEngine = NULL;
}

CPassengerVis::~CPassengerVis(void)
{
	if (m_pBody) ((IUnknown*)m_pBody)->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Playing

void CPassengerVis::Play(CEngine *pEngine)
{
	m_pEngine = pEngine;

	// Plan spawning
	HACTION a = pEngine->StartAnimation(GetSpawnTime());
	a = pEngine->SetAnimationListener(a, this, SPAWN);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering

void CPassengerVis::Render(AVLONG nPhase)
{
//	if (m_pEngine->GetPlayTime() > 69400) return;
//	if (m_pEngine->GetPlayTime() > 50000 && this->GetLiftId() == 0) return;

	if (m_pEngine) 
		m_pEngine->RenderPassenger(m_pBody, nPhase, GetSpawnTime(), GetLoadTime(), GetWaitSpan());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Lifecycle

int CPassengerVis::OnAnimationTick(AVULONG nParam)			
{ 
	HBONE pNode = NULL;
	switch (nParam)
	{
	case SPAWN:
		Spawn();
		break;
	case DIE:
		Die();
		break;
	case ENTER_ARR_FLOOR:
		pNode = GetSim()->GetLiftGroup()->GetStoreyElement(GetArrivalFloor())->GetBone(); 
		m_pEngine->Embark(m_pBody, pNode);
		break;
	case ENTER_LIFT:
		pNode = GetSim()->GetLiftGroup()->GetLiftDeck(GetLiftId(), GetDeck())->GetBone(); 
		m_pEngine->Embark(m_pBody, pNode);
		break;
	case ENTER_DEST_FLOOR:
		pNode = GetSim()->GetLiftGroup()->GetStoreyElement(GetDestFloor())->GetBone(); 
		m_pEngine->Embark(m_pBody, pNode);
		break;
	}
	return S_OK;
}

void CPassengerVis::Spawn()
{
	// Get Body!
	if (!m_pBody) m_pBody = m_pEngine->SpawnBiped();
	if (!m_pBody) return;

	// Plan Actions!
	HACTION a = m_pEngine->StartAnimation(GetSpawnTime());
	m_pEngine->Embark(m_pBody, GetSim()->GetLiftGroup()->GetStoreyElement(GetArrivalFloor())->GetBone(), false);
	for (AVULONG i = 0; i < GetWaypointCount(); i++)
	{
		WAYPOINT *wp = GetWaypoint(i);
		AVVECTOR vector = wp->vector + Vector(GetSim()->GetOffsetVector().x, -GetSim()->GetOffsetVector().y, GetSim()->GetOffsetVector().z);
		switch (wp->nAction)
		{
		case MOVE:				a = m_pEngine->Move(a, m_pBody, 1, vector.y+160.0f, vector.x, 0, wp->wstrStyle); break;
		case WAIT:				a = m_pEngine->Wait(a, m_pBody, wp->nTime, wp->wstrStyle); break;
		case WALK:				a = m_pEngine->Walk(a, m_pBody, vector.x, -vector.y, wp->wstrStyle); break;
		case TURN:				a = m_pEngine->Turn(a, m_pBody, wp->wstrStyle); break;
		case ENTER_ARR_FLOOR:
		case ENTER_LIFT:
		case ENTER_DEST_FLOOR:	a = m_pEngine->SetAnimationListener(a, this, wp->nAction); break;
		}
	}
	a = m_pEngine->SetAnimationListener(a, this, DIE);	// Plan Death
}

void CPassengerVis::Die()
{
	if (!m_pBody) return;

	m_pEngine->Embark(m_pBody, (HBONE)NULL);
	m_pEngine->KillBiped(m_pBody);
	m_pBody = NULL;
}

