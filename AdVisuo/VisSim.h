// Sim.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseSimClasses.h"
#include "repos.h"
using namespace std;

class CProjectVis;
class CLiftGroupVis;
class CLiftVis;
class CPassengerVis;

interface IAction;
interface IScene;
interface IMaterial;
interface IKineChild;
interface IBody;
interface IRenderer;

class CSimVis : public CSim, protected CRepos<IBody>
{
	IScene *m_pScene;
	IMaterial *m_pMaterial;
	IKineChild *m_pBiped;
	BYTE *m_pBipedBuf;			// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

	AVULONG m_nColouringMode;	// the Colouring Mode

public:
	CSimVis();
	virtual ~CSimVis();

	CLiftVis *GetLift(int i)			{ return (CLiftVis*)CSim::GetLift(i); }
	CPassengerVis *GetPassenger(int i)	{ return (CPassengerVis*)CSim::GetPassenger(i); }
	CLiftGroupVis *GetLiftGroup()		{ return (CLiftGroupVis*)CSim::GetLiftGroup(); }

	// access & initialisation
	IScene *GetScene()					{ return m_pScene; }
	void SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped);

	AVULONG GetColouringMode()			{ return m_nColouringMode; }
	void SetColouringMode(AVULONG n)	{ m_nColouringMode = n; }

	// repository
	IBody *GetBody();
	void ReleaseBody(IBody*);

	void Play(IAction *pActionTick, AVLONG nTime);
	AVLONG FastForward(IAction *pActionTick, AVLONG nTime);					// returns the earliest time that must be scanned before FF (usually < nTime)
	void RenderPassengers(IRenderer *pRenderer, AVLONG nPhase = 0);
	void Stop();

protected:
	// CRepos<IBody> functions
	virtual IBody *create();
	virtual void destroy(IBody*);

	virtual CPassenger *CreatePassenger(AVULONG nId);
	virtual CLift *CreateLift(AVULONG nId);

	friend class CProjectVis;
};
