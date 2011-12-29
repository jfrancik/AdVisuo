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

class _sim_error
{
public:
	enum ERROR_CODES { 
		E_SIM_NOT_FOUND = 0x80000100, 
		E_SIM_NO_BUILDING,
		E_SIM_PASSENGERS, 
		E_SIM_LIFTS, 
		E_SIM_FLOORS, 
		E_SIM_LIFT_DECKS,
		E_SIM_FILE_STRUCT,
		E_SIM_INTERNAL };
	_sim_error(enum ERROR_CODES err_code)	{ _error = err_code; }
	enum ERROR_CODES Error()				{ return _error; }
	std::wstring ErrorMessage();
private:
	enum ERROR_CODES _error;
};

class CSim : public CSimBase, protected CRepos<IBody>
{
	AVULONG m_nColouringMode;	// the Colouring Mode
	AVULONG m_nTime;			// the Current Time
	AVLONG m_nTimeLowerBound;	// Time Lower Bound (not greater than zero)

	IScene *m_pScene;
	IMaterial *m_pMaterial;
	IKineChild *m_pBiped;
	BYTE *m_pBipedBuf;			// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

	// Loading Phase
	enum PHASE { PHASE_NONE, PHASE_PRJ, PHASE_BLD, PHASE_STRUCT, PHASE_SIM } m_phase;

public:
	CSim(CBuildingBase *pBuilding);
	virtual ~CSim();

	CLift *GetLift(int i)			{ return (CLift*)CSimBase::GetLift(i); }
	CPassenger *GetPassenger(int i)	{ return (CPassenger*)CSimBase::GetPassenger(i); }
	CBuilding *GetBuilding()		{ return (CBuilding*)CSimBase::GetBuilding(); }

	// access & initialisation
	IScene *GetScene()				{ return m_pScene; }
	void SetScene(IScene *pScene);

	AVULONG GetColouringMode()		{ return m_nColouringMode; }
	void SetColouringMode(AVULONG n){ m_nColouringMode = n; }

	AVULONG GetTime()				{ return m_nTime; }
	void SetTime(AVULONG n)			{ m_nTime = n; }

	AVLONG GetTimeLowerBound()		{ return m_nTimeLowerBound; }
	void SetTimeLowerBound(AVLONG n){ m_nTimeLowerBound = n; }

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
protected:
	static CComPtr<IXmlReader> GetReaderFromBuf(LPCOLESTR pBuf);
	static CComPtr<IXmlWriter> GetWriterToBuf(LPCOLESTR pBuf, size_t nSize);
	static CComPtr<IXmlReader> GetReaderFromFile(LPCOLESTR pFileName);
	static CComPtr<IXmlWriter> GetWriterToFile(LPCOLESTR pFileName);
public:






	void LoadFromBuf(LPCOLESTR pBuf)										{ Load(GetReaderFromBuf(pBuf)); }
	void LoadFromFile(LPCOLESTR pFileName)									{ Load(GetReaderFromFile(pFileName)); }
	void Load(CComPtr<IXmlReader> pReader);

	void StoreToFile(LPCOLESTR pFileName)									{ Store(GetWriterToFile(pFileName)); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(CComPtr<IXmlWriter> pWriter);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, vector<CSim*> &sims)		{ LoadIndex(GetReaderFromBuf(pBuf), sims); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, vector<CSim*> &sims)	{ LoadIndex(GetReaderFromFile(pFileName), sims); }
	static void LoadIndex(CComPtr<IXmlReader> pReader, vector<CSim*>&);

	void XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName);
	void XFeed(CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName);

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
};
