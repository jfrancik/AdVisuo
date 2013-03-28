// Passenger.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisElem.h"
#include "VisPassenger.h"
#include "VisSim.h"
#include "Engine.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>
#include "freewilltools.h"
#include "VisLiftGroup.h"

#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

using namespace std;

CPassengerVis::CPassengerVis(CSimVis *pSim, AVULONG nPassengerId) 
	: CPassenger(pSim, nPassengerId), m_pBody(NULL), m_pObjBody(NULL)
{
	m_pEngine = NULL;
}

CPassengerVis::~CPassengerVis(void)
{
	if (m_pBody) m_pBody->Release();
	if (m_pObjBody) m_pObjBody->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Playing

void CPassengerVis::Play(CEngine *pEngine)
{
	m_pEngine = pEngine;

	// Plan giving a birth
	ANIM_HANDLE a = pEngine->StartAnimation(GetBornTime());
	a = pEngine->SetAnimationCB(a, _callback_birth, 0, (void*)this);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering

	static AVCOLOR HSV_to_RGB(AVFLOAT h, AVFLOAT s, AVFLOAT v)
	{
		// H is given on [0, 6]. S and V are given on [0, 1].
		float f = h - (int)h;
		if (((int)h & 1) == 0) f = 1 - f; // if i is even
		float m = v * (1 - s);
		float n = v * (1 - s * f);

		AVCOLOR RGB;
		switch ((int)h) 
		{
			case 6:
			case 0: RGB.r = v; RGB.g = n; RGB.b = m; RGB.a = 1; return RGB;
			case 1: RGB.r = n; RGB.g = v; RGB.b = m; RGB.a = 1; return RGB;
			case 2: RGB.r = m; RGB.g = v; RGB.b = n; RGB.a = 1; return RGB;
			case 3: RGB.r = m; RGB.g = n; RGB.b = v; RGB.a = 1; return RGB;
			case 4: RGB.r = n; RGB.g = m; RGB.b = v; RGB.a = 1; return RGB;
			case 5: RGB.r = v; RGB.g = m; RGB.b = n; RGB.a = 1; return RGB;
			default: RGB.r = 0; RGB.g = 0; RGB.b = 0; RGB.a = 1; return RGB;
		}
	}

void CPassengerVis::Render(IRenderer *pRenderer, AVLONG nPhase)
{
	// get the current playing time...
	AVLONG nTime;
	pRenderer->GetPlayTime(&nTime);

	// time params
	AVLONG nAge = nTime - GetBornTime();

	if (nAge < 550 && nPhase == 0 || nAge >= 550 && nPhase == 1)
		return;

	if (m_pObjBody)
	{
		// Set Temperature
		AVULONG time;
		AVCOLOR color;
		switch (GetSim()->GetColouringMode())
		{
		case 0: 
			color.r = color.g = color.b = 1;
			break;
		case 1: 
			if (nTime < GetLoadTime() - GetWaitSpan())
				time = 0;
			else if (nTime >= GetLoadTime())
				time = GetWaitSpan();
			else
				time = nTime + GetWaitSpan() - GetLoadTime();

			color = HSV_to_RGB(2 - (((AVFLOAT)min(time, 55000) * 2) / 55000), 1, 1);
			break;
		case 2: 
			time = GetWaitSpan();
			color = HSV_to_RGB(2 - (((AVFLOAT)min(time, 55000) * 2) / 55000), 1, 1);
			break;
		}
		color.a = 1;

		//switch (GetId())
		//{
		//case 15: color.r = 1; color.g = 0; color.b = 0; break;
		//case 16: color.r = 0; color.g = 1; color.b = 0; break;
		//case 19: color.r = 0; color.g = 0; color.b = 1; break;
		//default: color.r = 1; color.g = 1; color.b = 1; break;
		//}
		
		AVFLOAT fAlpha = 1.0f;

		if (nAge < 50) fAlpha = 0.0;
		else if (nAge < 550) fAlpha = (nAge - 50) / 500.0f;

		IMaterial *pMaterial = NULL;
		m_pObjBody->GetMaterial(&pMaterial);
		if (pMaterial) 
		{
			pMaterial->SetDiffuseColor(color);
			pMaterial->SetAlpha(fAlpha);
			pMaterial->Release();
		}
		m_pObjBody->Render((IRndrGeneric*)pRenderer);
	}
}

int _callback_birth(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassengerVis*)pParam)->BeBorn();
	return S_OK;
}

int _callback_death(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassengerVis*)pParam)->Die();
	return S_OK;
}

int _callback_embark(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassengerVis*)pParam)->Embark((enum ENUM_ACTION)nParam);
	return S_OK;
}

void CPassengerVis::BeBorn()
{
	// Get Body!
	if (!m_pBody) m_pBody = m_pEngine->SpawnBiped();
	if (!m_pBody) return;
	IKineNode *pNode = m_pBody->BodyNode(BODY_OBJECT);
	pNode->QueryInterface(&m_pObjBody);
	pNode->Release();

	// Plan Actions!
	ANIM_HANDLE a = m_pEngine->StartAnimation(GetBornTime());
	Embark(ENTER_ARR_FLOOR, false);
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
		case ENTER_DEST_FLOOR:	a = m_pEngine->SetAnimationCB(a, _callback_embark, (AVULONG)wp->nAction, (void*)this); break;
		}
	}
	a = m_pEngine->SetAnimationCB(a, _callback_death, 0, (void*)this);	// Plan Death
}

void CPassengerVis::Die()
{
	if (!m_pBody) return;

	Embark((IKineNode*)NULL);
	m_pEngine->KillBiped(m_pBody);
	m_pBody = NULL;
	if (m_pObjBody) m_pObjBody->Release();
	m_pObjBody = NULL;
}

void CPassengerVis::Embark(enum ENUM_ACTION nAction, bool bSwitchCoord)
{
	IKineNode *pNode = NULL;
	switch (nAction)
	{
	case ENTER_ARR_FLOOR:
		pNode = GetSim()->GetLiftGroup()->GetStoreyElement(GetArrivalFloor())->GetBone(); break;
	case ENTER_LIFT:
		pNode = GetSim()->GetLiftGroup()->GetLiftDeck(GetLiftId(), GetDeck())->GetBone(); break;
	case ENTER_DEST_FLOOR:
		pNode = GetSim()->GetLiftGroup()->GetStoreyElement(GetDestFloor())->GetBone(); break;
	}
	Embark(pNode, bSwitchCoord);
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
