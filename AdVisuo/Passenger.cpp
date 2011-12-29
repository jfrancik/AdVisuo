// Passenger.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Passenger.h"
#include "Sim.h"

#include <freewill.h>
#include <fwaction.h>
#include "freewilltools.h"
#include "Building.h"

#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

using namespace std;

CPassenger::CPassenger(CSim *pSim, AVULONG nPassengerId) 
	: CPassengerBase(pSim, nPassengerId), m_pBody(NULL), m_pObjBody(NULL), m_pActionTick(NULL)
{
}

CPassenger::~CPassenger(void)
{
	if (m_pBody) m_pBody->Release();
	if (m_pObjBody) m_pObjBody->Release();
	if (m_pActionTick) m_pActionTick->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Playing

void CPassenger::Play(IAction *pActionTick)
{
	// store the tick source
	if (m_pActionTick) m_pActionTick->Release();
	m_pActionTick = pActionTick;
	if (m_pActionTick) m_pActionTick->AddRef();

	// Plan giving a birth
	IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pActionTick->FWDevice(), L"Action", L"Generic", m_pActionTick, GetBornTime() - GetSim()->GetTimeLowerBound(), 0);
	pAction->SetHandleEventHook(_callback_birth, 0, (void*)this);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering

	static FWCOLOR HSV_to_RGB(AVFLOAT h, AVFLOAT s, AVFLOAT v)
	{
		// H is given on [0, 6]. S and V are given on [0, 1].
		float f = h - (int)h;
		if (((int)h & 1) == 0) f = 1 - f; // if i is even
		float m = v * (1 - s);
		float n = v * (1 - s * f);

		FWCOLOR RGB;
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

void CPassenger::Render(IRenderer *pRenderer, AVLONG nPhase)
{
	// time params
	AVLONG nAge = (AVLONG)GetSim()->GetTime() - (GetBornTime() - (AVLONG)GetSim()->GetTimeLowerBound());

	if (nAge < 550 && nPhase == 0 || nAge >= 550 && nPhase == 1)
		return;

	if (m_pObjBody)
	{
		// Set Temperature
		AVULONG time;
		FWCOLOR color;
		switch (GetSim()->GetColouringMode())
		{
		case 0: 
			color.r = color.g = color.b = 1;
			break;
		case 1: 
			if (GetSim()->GetTime() < GetLoadTime() - GetSim()->GetTimeLowerBound() - GetWaitSpan())
				time = 0;
			else if (GetSim()->GetTime() >= (AVULONG)(GetLoadTime() - GetSim()->GetTimeLowerBound()))
				time = GetWaitSpan();
			else
				time = GetSim()->GetTime() + GetWaitSpan() - (GetLoadTime() - GetSim()->GetTimeLowerBound());

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
			pMaterial->Release();
			pMaterial->SetAlpha(fAlpha);
		}
		m_pObjBody->Render((IRndrGeneric*)pRenderer);
	}
}

int _callback_birth(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassenger*)pParam)->BeBorn();
//	Debug(L"Passenger #%d is born!", pPassenger->GetId());
	return S_OK;
}

int _callback_death(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassenger*)pParam)->Die();
//	Debug(L"Passenger #%d is killed!", pPassenger->GetId());
	return S_OK;
}

int _callback_embark(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, void *pParam)
{
	if (pEvent->nEvent != EVENT_TICK) return S_OK;
	((CPassenger*)pParam)->Embark((enum ENUM_ACTION)nParam);
	return S_OK;
}

void CPassenger::BeBorn()
{
	// Get Body!
	if (!m_pBody) m_pBody = GetSim()->GetBody();
	if (!m_pBody) return;
	IKineNode *pNode = m_pBody->BodyNode(BODY_OBJECT);
	pNode->QueryInterface(&m_pObjBody);
	pNode->Release();

	// Plan Actions!
	IAction *pAction = NULL;
	IFWDevice *pDev = m_pActionTick->FWDevice();

	AVULONG stepDuration = 150;
	AVULONG turnDuration = 300;
	AVFLOAT stepLen = 15.0f;

	Embark(ENTER_ARR_FLOOR, false);

	for (AVULONG i = 0; i < GetWaypointCount(); i++)
	{
		WAYPOINT *wp = GetWaypoint(i);
		switch (wp->nAction)
		{
		case MOVE:
			pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Move", m_pActionTick, pAction, 1, (AVSTRING)(wp->wstrStyle.c_str()), m_pBody, BODY_ROOT, wp->vector.y+160.0f, wp->vector.x, 0);
			break;
		case WAIT:
			pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Wait", m_pActionTick, pAction, 0, (AVSTRING)(wp->wstrStyle.c_str()), m_pBody, wp->nTime - GetSim()->GetTimeLowerBound());
			break;
		case WALK:
			pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Walk", m_pActionTick, pAction, stepDuration, (AVSTRING)(wp->wstrStyle.c_str()), m_pBody, wp->vector.x, -wp->vector.y, stepLen, DEG2RAD(45));
			break;
		case TURN:
			pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Turn", m_pActionTick, pAction, turnDuration, (AVSTRING)(wp->wstrStyle.c_str()), m_pBody, DEG2RAD(180), 3);
			break;
		case ENTER_ARR_FLOOR:
		case ENTER_LIFT:
		case ENTER_DEST_FLOOR:
			pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Generic", m_pActionTick, pAction, 0);
			pAction->SetHandleEventHook(_callback_embark, (AVULONG)wp->nAction, (void*)this);
			break;
		}
	}

	// Plan Death
	pAction = (IAction*)::FWCreateObjWeakPtr(pDev, L"Action", L"Generic", m_pActionTick, pAction, 0);
	pAction->SetHandleEventHook(_callback_death, 0, (void*)this);
}

void CPassenger::Die()
{
	if (!m_pBody) return;

	Embark((IKineNode*)NULL);
	GetSim()->ReleaseBody(m_pBody);
	m_pBody = NULL;
	if (m_pObjBody) m_pObjBody->Release();
	m_pObjBody = NULL;
}

void CPassenger::Embark(enum ENUM_ACTION nAction, bool bSwitchCoord)
{
	IKineNode *pNode = NULL;
	switch (nAction)
	{
	case ENTER_ARR_FLOOR:
		pNode = GetSim()->GetBuilding()->GetStoreyNode(GetArrivalFloor()); break;
	case ENTER_LIFT:
		pNode = GetSim()->GetBuilding()->GetLiftDeck(GetLiftId(), GetDeck()); break;
	case ENTER_DEST_FLOOR:
		pNode = GetSim()->GetBuilding()->GetStoreyNode(GetDestFloor()); break;
	}
	Embark(pNode, bSwitchCoord);
}

void CPassenger::Embark(IKineNode *pNode, bool bSwitchCoord)
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
