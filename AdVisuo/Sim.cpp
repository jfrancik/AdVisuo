// Sim.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Sim.h"
#include "Lift.h"
#include "Passenger.h"
#include "vector"

#include <freewill.h>
#include "freewilltools.h"

#pragma warning(disable:4018)

using namespace std;

std::wstring _sim_error::ErrorMessage()
{
	switch (_error)
	{
		case E_SIM_NOT_FOUND:	return L"project not found";
		case E_SIM_NO_BUILDING:	return L"corrupt or missing building structure";
		case E_SIM_PASSENGERS:	return L"no hall calls found";
		case E_SIM_LIFTS:		return L"inconsistent building structure: too many or too few lifts";
		case E_SIM_FLOORS:		return L"inconsistent building structure: too many or too few floors";
		case E_SIM_LIFT_DECKS:	return L"inconsistent building structure: wrong number of lift decks";
		case E_SIM_FILE_STRUCT:	return L"data appear in wrong sequence within the simulation file";
		case E_SIM_INTERNAL:	return L"internal error";
		default:				return L"unidentified error";
	}
}

CSim::CSim(CBuildingBase *pBuilding)
	: CSimBase(pBuilding), 
	  m_phase(PHASE_NONE),
	  m_pScene(NULL), m_pBiped(NULL), m_pMaterial(NULL), m_pBipedBuf(NULL), m_nBipedBufCount(0), m_nColouringMode(0), m_nTime(0), m_nTimeLowerBound(0)
{
}

CSim::~CSim(void)
{
	SetScene(NULL);
	remove_all();
}

void CSim::SetScene(IScene *pScene)
{
	if (m_pScene) m_pScene->Release();
	if (m_pMaterial) m_pMaterial->Release();
	if (m_pBiped) m_pBiped->Release();
	if (m_pBipedBuf) delete [] m_pBipedBuf;
	m_nBipedBufCount = 0;

	m_pScene = pScene;

	if (m_pScene) 
	{
		m_pScene->AddRef();
		m_pScene->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&m_pMaterial);
		m_pScene->GetChild(L"Bip01", &m_pBiped);
		if (m_pBiped)
		{
			m_pBiped->StoreState(0, NULL, &m_nBipedBufCount);
			m_pBipedBuf = new BYTE[m_nBipedBufCount];
			m_pBiped->StoreState(m_nBipedBufCount, m_pBipedBuf, NULL);
		}
	}
}

IBody *CSim::GetBody()
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

void CSim::ReleaseBody(IBody *pBody)
{
	release(pBody);
}

void CSim::PrePlay()
{
	// Determine Time Offset
	AVLONG nTimeLowerBound = 0;
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		nTimeLowerBound = min(nTimeLowerBound, GetPassenger(i)->GetBornTime());
	nTimeLowerBound = min(nTimeLowerBound, -100);
	SetTimeLowerBound(nTimeLowerBound);
}

void CSim::Play(IAction *pActionTick, AVLONG nTime)
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

AVLONG CSim::FastForward(IAction *pActionTick, AVLONG nTime)
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

void CSim::RenderPassengers(IRenderer *pRenderer, AVLONG nPhase)
{
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Render(pRenderer, nPhase);
}

void CSim::Stop()
{
	for (FWULONG i = 0; i < GetPassengerCount(); i++)
		GetPassenger(i)->Die();
	remove_all();
}

IBody *CSim::create()
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

void CSim::destroy(IBody *p)
{
	static int nCount = 0;

	p->Release();
}
