// Sim.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "Lift.h"
#include "Passenger.h"
#include "Building.h"
#include "repos.h"
using namespace std;

interface IAction;
interface IScene;
interface IMaterial;
interface IKineChild;
interface IBody;
interface IRenderer;

class CProject;

class CSim : public CSimBase, protected CRepos<IBody>
{
	AVULONG m_nColouringMode;	// the Colouring Mode
	AVULONG m_nTime;			// the Current Time
	AVLONG m_nTimeLowerBound;	// Time Lower Bound (not greater than zero)

	AVVECTOR m_vecOffset;

	IScene *m_pScene;
	IMaterial *m_pMaterial;
	IKineChild *m_pBiped;
	BYTE *m_pBipedBuf;			// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

public:
	CSim(CBuildingBase *pBuilding);
	virtual ~CSim();

	CLift *GetLift(int i)			{ return (CLift*)CSimBase::GetLift(i); }
	CPassenger *GetPassenger(int i)	{ return (CPassenger*)CSimBase::GetPassenger(i); }
	CBuilding *GetBuilding()		{ return (CBuilding*)CSimBase::GetBuilding(); }

	// access & initialisation
	IScene *GetScene()				{ return m_pScene; }
	void SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped);
	void SetScene()					{ SetScene(NULL, NULL, NULL); }

	AVULONG GetColouringMode()		{ return m_nColouringMode; }
	void SetColouringMode(AVULONG n){ m_nColouringMode = n; }

	AVULONG GetTime()				{ return m_nTime; }
	void SetTime(AVULONG n)			{ m_nTime = n; }

	AVLONG GetTimeLowerBound()		{ return m_nTimeLowerBound; }
	void SetTimeLowerBound(AVLONG n){ m_nTimeLowerBound = n; }

	AVVECTOR GetOffsetVector()		{ return m_vecOffset; }
	void SetOffsetVector(AVVECTOR v){ m_vecOffset = v; }

	// repository
	IBody *GetBody();
	void ReleaseBody(IBody*);

	void PrePlay();
	void Play(IAction *pActionTick, AVLONG nTime = 0);
	AVLONG FastForward(IAction *pActionTick, AVLONG nTime);					// returns the earliest time that must be scanned before FF (usually < nTime)
	void RenderPassengers(IRenderer *pRenderer, AVLONG nPhase = 0);
	void Stop();

protected:
	// CRepos<IBody> functions
	virtual IBody *create();
	virtual void destroy(IBody*);

	virtual CPassengerBase *CreatePassenger(AVULONG nId)	{ return new CPassenger(this, nId); }
	virtual CLiftBase *CreateLift(AVULONG nId)				{ return new CLift(this, nId); }

	friend class CProject;
};
