// Passenger.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisElem.h"
#include "VisPassenger.h"
#include "VisSim.h"
#include "VisLiftGroup.h"
#include "Engine.h"

#include <freewill.h>

#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

using namespace std;

CPassengerVis::CPassengerVis(CSimVis *pSim, AVULONG nPassengerId) 
	: CPassenger(pSim, nPassengerId), m_pBody(NULL)
{
	m_pEngine = NULL;
}

CPassengerVis::~CPassengerVis(void)
{
	if (m_pBody) m_pBody->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Playing

void CPassengerVis::Play(CEngine *pEngine)
{
	m_pEngine = pEngine;

	// Plan spawning
	ANIM_HANDLE a = pEngine->StartAnimation(GetSpawnTime());
	a = pEngine->SetAnimationListener(a, this, SPAWN);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering

void CPassengerVis::Render(AVLONG nPhase)
{
	if (m_pEngine) 
		m_pEngine->RenderPassenger(m_pBody, GetSim()->GetColouringMode(), nPhase, GetSpawnTime(), GetLoadTime(), GetWaitSpan());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Lifecycle

int CPassengerVis::OnAnimationTick(AVULONG nParam)			
{ 
	IKineNode *pNode = NULL;
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
		Embark(pNode);
		break;
	case ENTER_LIFT:
		pNode = GetSim()->GetLiftGroup()->GetLiftDeck(GetLiftId(), GetDeck())->GetBone(); 
		Embark(pNode);
		break;
	case ENTER_DEST_FLOOR:
		pNode = GetSim()->GetLiftGroup()->GetStoreyElement(GetDestFloor())->GetBone(); 
		Embark(pNode);
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
	ANIM_HANDLE a = m_pEngine->StartAnimation(GetSpawnTime());
	Embark(GetSim()->GetLiftGroup()->GetStoreyElement(GetArrivalFloor())->GetBone(), false);
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

	Embark((IKineNode*)NULL);
	m_pEngine->KillBiped(m_pBody);
	m_pBody = NULL;
}

void CPassengerVis::Embark(IKineNode *pNode, bool bSwitchCoord)
{
	if (!m_pBody) return;
	IKineNode *pMe = m_pBody->BodyNode(BODY_OBJECT);

	if (bSwitchCoord && pNode)
	{
		FWVECTOR v1 = { 0, 0, 0 };
		pMe->LtoG(&v1);

		ITransform *pT;
		pMe->CreateCompatibleTransform(&pT);
		pNode->GetGlobalTransform(pT);
		pMe->Transform(pT, KINE_LOCAL | KINE_INVERTED);
		pMe->GetParentTransform(pT);
		pMe->Transform(pT, KINE_LOCAL | KINE_REGULAR);

		// compensate for Z coordinate
		FWVECTOR v2 = { 0, 0, 0 };
		pNode->LtoG(&v2);
		pT->FromTranslationXYZ(0, 0, v2.z - v1.z);
		pMe->Transform(pT, KINE_LOCAL | KINE_REGULAR);

		pT->Release();
	}
		
	IKineNode *pParent;
	pMe->GetParent(&pParent);
	if (pParent)
	{
		pParent->DelChildPtr(pMe);
		pParent->Release();
	}

	if (pNode)
	{
		OLECHAR buf[257];
		pNode->CreateUniqueLabel(L"Passenger", 256, buf);
		pNode->AddChild(buf, pMe);
	}

	pMe->Release();
}
