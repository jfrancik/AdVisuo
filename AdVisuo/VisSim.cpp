// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisSim.h"
#include "VisLift.h"
#include "VisPassenger.h"
#include "vector"

#include <freewill.h>
#include "freewilltools.h"

#pragma warning(disable:4018)

using namespace std;

CSimVis::CSimVis() : CSim(), 
	  m_pScene(NULL), m_pBiped(NULL), m_pMaterial(NULL), m_pBipedBuf(NULL), m_nBipedBufCount(0), m_nColouringMode(0), m_nTime(0), m_nTimeLowerBound(0)
{
}

CSimVis::~CSimVis(void)
{
	SetScene();
	remove_all();
}

void CSimVis::SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped)
{
	if (m_pScene) m_pScene->Release();
	if (m_pMaterial) m_pMaterial->Release();
	if (m_pBiped) m_pBiped->Release();
	if (m_pBipedBuf) delete [] m_pBipedBuf;
	m_nBipedBufCount = 0;

	m_pScene = pScene; if (m_pScene) m_pScene->AddRef();
	m_pMaterial = pMaterial; if (m_pMaterial) m_pMaterial->AddRef();
	m_pBiped = pBiped; if (m_pBiped) m_pBiped->AddRef();

	if (m_pBiped) 
	{
		m_pBiped->StoreState(0, NULL, &m_nBipedBufCount);
		m_pBipedBuf = new BYTE[m_nBipedBufCount];
		m_pBiped->StoreState(m_nBipedBufCount, m_pBipedBuf, NULL);
	}

	if (GetLiftGroup())
		GetLiftGroup()->SetScene(m_pScene);
}

IBody *CSimVis::GetBody()
{
	IBody *pBody = get();
	if (pBody && m_pBipedBuf)
	{
		IKineNode *pNode = pBody->BodyNode(BODY_OBJECT);
		pNode->RetrieveState(m_nBipedBufCount, m_pBipedBuf, NULL);
		pNode->Invalidate();
		pNode->Release();
	}
	return pBody;
}

void CSimVis::ReleaseBody(IBody *pBody)
{
	release(pBody);
}

void CSimVis::PrePlay()
{
	// Determine Time Offset
	AVLONG nTimeLowerBound = 0;
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		nTimeLowerBound = min(nTimeLowerBound, GetPassenger(i)->GetBornTime());
	nTimeLowerBound = min(nTimeLowerBound, -100);
	SetTimeLowerBound(nTimeLowerBound);
}

void CSimVis::Play(IAction *pActionTick, AVLONG nTime)
{
	if (nTime == 0) nTime = GetTimeLowerBound();

	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLift(i)->Play(pActionTick, nTime);
	}

	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetBornTime() >= nTime)
			GetPassenger(i)->Play(pActionTick);
}

AVLONG CSimVis::FastForward(IAction *pActionTick, AVLONG nTime)
{
	AVLONG nEarliestTime = 0x7FFFFFFF;

	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		AVLONG t = GetLift(i)->FastForward(pActionTick, nTime);
		nEarliestTime = min(nEarliestTime, t);
	}

	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		if (GetPassenger(i)->GetUnloadTime() >= nTime)
		{
			GetPassenger(i)->Play(pActionTick);

			AVLONG t = GetPassenger(i)->GetBornTime();
			nEarliestTime = min(nEarliestTime, t);
		}
	return min(nEarliestTime, GetSimulationTime());
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
	remove_all();
}

IBody *CSimVis::create()
{
	static int nCount = 0;

	// Reproduction
	if (!m_pBiped) return NULL;
	IKineNode *pNode = NULL;
	if FAILED(m_pBiped->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pNode))
		return NULL;

	// scene object operatons...
	ISceneObject *pSceneObj = NULL;
	pNode->QueryInterface(&pSceneObj);
	pSceneObj->PutVisible(TRUE);
	pSceneObj->PutMaterial(m_pMaterial, TRUE);
	pSceneObj->Release();

	// load body
	IBody *pBody = NULL;
	m_pScene->FWDevice()->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&pBody);
	pBody->LoadBody(pNode, BODY_SCHEMA_DISCREET);

	pNode->Release();

	return pBody;
}

void CSimVis::destroy(IBody *p)
{
	static int nCount = 0;

	p->Release();
}
