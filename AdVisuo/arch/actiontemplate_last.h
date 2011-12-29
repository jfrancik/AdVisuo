// actiontemplate.h
//
// Everything You Would Ever Need to Implement Action+ Based Objects:
// template class CTAction - fully implements IAction
/////////////////////////////////////////////////////////////////////////////

#if !defined(_ACTIONTEMPL__)
#define _ACTIONTEMPL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "actionplus.h"
#include <algorithm>
#include <functional>
#include <map>
#include <list>
#include <set>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;

#include "LiftKinematicsCalculator.h"

#pragma warning (disable:4503)	// decorated name length exceeded

using namespace std;

#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)
#define for_each_1(Z, TYPE, COLL, VAR)	\
	bool Z;\
	for (TYPE::iterator i = COLL.begin(); i != COLL.end(); i++)		\
		for (TYPE::value_type &VAR = (Z = true , *i); Z; Z = false)
#define for_each(TYPE, COLL, VAR)	for_each_1(TOKENPASTE(x_var_unique_, __COUNTER__), TYPE, COLL, VAR)

///////////////////////////////////////////////////////////
//
// IAction implementation template

class CTAction : public FWUNKNOWN<IAction, IID_IAction, IAction>
{
protected:

	// Activity Flag - used by Suspend and Resume
	FWULONG m_nSuspended;

	// Time Data
	FWULONG m_nStartTime;			// start time for the action (considering suspend times)
	FWULONG m_nPeriod;				// the period (total time) of action
	FWULONG m_nSuspendTime;			// time when action suspended or zero
	FWFLOAT m_fTimePrev;			// time stamp for the GetProgPhase function

	// Style info (optional)
	FWSTRING m_pStyle;
	set<std::wstring> m_styles;

	// Envelope Data
	struct ACTION_ENVELOPE_DATA m_env;

	// Current Event
	ACTION_EVENT *m_pCurEvent;

	// Handle Event Hook Function
	HANDLE_EVENT_HOOK_FUNC m_pfHook;
	FWULONG m_nParam;
	void *m_pParam;

	// Typedefs
	struct ACTION_SUBS_EX;
	struct COLL;
	typedef list<ACTION_SUBS_EX>		LIST_OF_SUBS;
	typedef map<INT, LIST_OF_SUBS>		MAP_INT_LIST_OF_SUBS;
	typedef map<FWULONG, COLL>			MAP_OF_COLL;

	// Extended version of ACTION_SUBS
	// Contains C++ specific info: unsubscribtion iterator and sorting operators
	struct ACTION_SUBS_EX : public ACTION_SUBS
	{
		ACTION_SUBS_EX(IAction *pOriginator, IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWULONG nTrigger = 0, FWULONG nId = 0)
		{ 
			this->pOriginator = pOriginator;
			this->pSubscriber = pSubscriber;
			this->nEvent = nEvent;
			this->nFlags = nFlags;
			this->nTrigger = nTrigger;
			this->nId = nId;
			this->pColl = NULL;
		}
		LIST_OF_SUBS *pColl;
		LIST_OF_SUBS::iterator iter;
		friend bool operator <(const ACTION_SUBS_EX &i1, const ACTION_SUBS_EX &i2) { return i1.nTrigger < i2.nTrigger; }
		friend bool operator >(const ACTION_SUBS_EX &i1, const ACTION_SUBS_EX &i2) { return i1.nTrigger > i2.nTrigger; }
	};

	// Subscription Lists
	struct COLL			// contains collections of all subscribers of a given event code
	{
		LIST_OF_SUBS lstAny;			// subscriptions with the ACTION_ANY flag
		MAP_INT_LIST_OF_SUBS mapEq;		// subscriptions with the ACTION_EQ flag
		LIST_OF_SUBS lstLte;			// subscriptions with the ACTION_LTE flag
		LIST_OF_SUBS lstGte;			// subscriptions with the ACTION_GTE flag
	};
	
	MAP_OF_COLL m_Subsribers;	// the Subscription List: maps event code to COLL structure


	//////////////////////////////////////////////
	// Helpers for Implementation of Derivated Classes
protected:
	HRESULT QueryStdParams(IFWEnumParams *pEnum, struct ACTION_SUBS **ppSubs = NULL)
	{
		HRESULT h = S_OK;
		IAction *pTickSource = NULL;
		IAction *pPrevAction = NULL;
		FWSTRING pStyle = NULL;
		m_nStartTime = m_nPeriod = 0;

		// 1st: pTickSource, obligatory, may be NULL
		h = pEnum->QueryPUNKNOWN(IID_IAction, (FWPUNKNOWN*)&pTickSource);
		if FAILED(h) return ERROR(h);	// This parameter is obligatory

		// 2nd, option 1: Start Time
		h = pEnum->QueryULONG(&m_nStartTime);

		// 2nd, option 2: Previous Action
		if FAILED(h) h = pEnum->QueryPUNKNOWN(IID_IAction, (FWPUNKNOWN*)&pPrevAction);

		// 3rd, Period, optional, only taken if 2nd successfully read
		if SUCCEEDED(h) pEnum->QueryULONG(&m_nPeriod);

		// 4th, Style, optional
		pEnum->QuerySTRING(&pStyle);

		// Standard operations now...
		// Subscribe, Suspend and SetStyleString never fails in this implementation so no checks are done...
		if (pTickSource) 
		{
			pTickSource->Subscribe(this, EVENT_TICK, ACTION_GTE, m_nStartTime, 0, ppSubs);
			pTickSource->Release();
		}
		if (pPrevAction) 
		{
			pPrevAction->Subscribe(this, EVENT_END, ACTION_ANY | ACTION_ONCE | ACTION_RESUME, 0, 0, NULL);
			pPrevAction->Release();
			h = Suspend(0);
		}
		SetStyleString(pStyle);

		return S_OK;
	}

	HRESULT ErrorStdParams(IFWEnumParams *pEnum, HRESULT h)
	{
		m_nPeriod = 0;	// prevents running the action... (shall not always run...)
		pEnum->ErrorEx(__WFILE__, __LINE__, h);
		pEnum->Release();
		return h;
	}

public:

	//////////////////////////////////////////////
	// Construction & Deconstruction

	CTAction() : m_nSuspended(0), m_pStyle(NULL),
		m_nStartTime(0), m_nSuspendTime(0), m_nPeriod(0), m_fTimePrev(0.0f),
		m_pfHook(NULL), m_nParam(0), m_pParam(NULL)
	{ 
		m_env.type = ACTION_ENV_REWRITE_FROM_SENDER;
		m_pCurEvent = NULL;
	}

	~CTAction()
	{
		if (m_pStyle) free(m_pStyle);
		UnSubscribeAll();
	}

	//////////////////////////////////////////////
	// Error Definitions

	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(ACTION_E_CANNOTSUBSCRIBE,	L"Action cannot subscribe for the given event", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(ACTION_E_CANNOTSETUP,		L"Action cannot set-up for the given verb", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(ACTION_E_INVALIDPERIOD,		L"Action timing function called with an invalid value of action period", FW_SEV_CRITICAL)
	FW_ERROR_END


	//////////////////////////////////////////////
	// Subscription

	virtual HRESULT __stdcall Subscribe(IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWULONG nTrigger, FWULONG nId, struct ACTION_SUBS **pHandle)
	{
		if (pSubscriber == this)
			nFlags |= ACTION_WEAKPTR;	// auto-references are always weak - to avoid looped refs
		
		ACTION_SUBS_EX subs(this, pSubscriber, nEvent, nFlags, nTrigger, nId);

		if ((nFlags & ACTION_WEAKPTR) == 0)
			pSubscriber->AddRef();	

		LIST_OF_SUBS *pColl;
		LIST_OF_SUBS::iterator iter;

		COLL &coll = m_Subsribers[nEvent];

		switch (nFlags & ACTION_MASK_TRIGGER)
		{
		case ACTION_ANY:
			pColl = &coll.lstAny;
			iter = coll.lstAny.insert(coll.lstAny.end(), subs);
			break;
		case ACTION_EQ:
			pColl = &coll.mapEq[nTrigger];
			iter = pColl->insert(pColl->end(), subs);
			break;
		case ACTION_LTE:
			{
				pColl = &coll.lstLte;
				LIST_OF_SUBS::iterator pos = lower_bound(coll.lstLte.begin(), coll.lstLte.end(), subs, std::greater<ACTION_SUBS_EX>());
				iter = coll.lstLte.insert(pos, subs);
				break;
			}
		case ACTION_GTE:
			{
				pColl = &coll.lstGte;
				LIST_OF_SUBS::iterator pos = lower_bound(coll.lstGte.begin(), coll.lstGte.end(), subs);
				iter = coll.lstGte.insert(pos, subs);
				break;
			}
		}
		(*iter).pColl = pColl;
		(*iter).iter = iter;

		if (pHandle) *pHandle = &(*iter);
		
		return S_OK;
	}

	virtual HRESULT __stdcall UnSubscribe(FWULONG nTimeStamp, ACTION_SUBS *pSubs)
	{
		if (pSubs->pOriginator != this)
			return pSubs->pOriginator->UnSubscribe(nTimeStamp, pSubs);
		else
		{
			ACTION_EVENT event = { NULL, nTimeStamp, EVENT_UNSUBSCRIBE, 0, NULL, pSubs };
			pSubs->pSubscriber->SendEventEx(&event);
			if ((pSubs->nFlags & ACTION_WEAKPTR) == 0)
				pSubs->pSubscriber->Release();

			((ACTION_SUBS_EX*)pSubs)->pColl->erase(((ACTION_SUBS_EX*)pSubs)->iter);

			return S_OK;
		}
	}

	virtual HRESULT __stdcall UnSubscribeAll()
	{
		for_each(MAP_OF_COLL, m_Subsribers, v)
		{
			for_each(LIST_OF_SUBS, v.second.lstAny, v)
				if (v.pSubscriber) v.pSubscriber->Release();
			for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
			{
				for_each(LIST_OF_SUBS, v.second, v)
					if (v.pSubscriber) v.pSubscriber->Release();
			}
			for_each(LIST_OF_SUBS, v.second.lstLte, v)
				if (v.pSubscriber) v.pSubscriber->Release();
			for_each(LIST_OF_SUBS, v.second.lstGte, v)
				if (v.pSubscriber) v.pSubscriber->Release();
		}
		m_Subsribers.clear();
		return S_OK;
	}

	virtual HRESULT __stdcall GetSubscriptionCount(/*[out, retval]*/ FWULONG *p)
	{
		if (p) *p = SubscriptionCount();
		return S_OK;
	}

	virtual FWULONG __stdcall SubscriptionCount()
	{
		FWULONG nCount = 0;
		for_each(MAP_OF_COLL, m_Subsribers, v)
		{
			nCount += v.second.lstAny.size();
			for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
				nCount += v.second.size();
			nCount += v.second.lstLte.size();
			nCount += v.second.lstGte.size();
		}
		return nCount;
	}

	virtual BOOL __stdcall AnySubscriptionsLeft()
	{
		for_each(MAP_OF_COLL, m_Subsribers, v)
		{
			if (v.second.lstGte.size() || v.second.lstAny.size() || v.second.lstLte.size()) return true;
			for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
				if (v.second.size()) return true;
		}
		return false;
	}

	virtual HRESULT __stdcall IsSubscriptionCount()
	{
		return AnySubscriptionsLeft() ? S_OK : S_FALSE;
	}

	//////////////////////////////////////////////
	// Style Functions

	virtual HRESULT __stdcall SetStyleString(FWSTRING p)
	{
		if (m_pStyle) free(m_pStyle);
		m_styles.clear();
		m_pStyle = wcsdup(p);

		FWSTRING pSep = L"; ";
		while (p && *p)
		{
			p += wcsspn(p, pSep);
			ULONG i = wcscspn(p, pSep);
			wstring token(p, i);
			if (i) m_styles.insert(token);
			p += i;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall GetStyleString(FWSTRING *p)
	{
		if (p) *p = m_pStyle;
		return S_OK;
	}

	virtual HRESULT __stdcall IsStyle(FWSTRING strStyle)
	{
		if (m_styles.find(strStyle) != m_styles.end()) return S_OK;
		else return S_FALSE;
	}
	
	//////////////////////////////////////////////
	// Life Cycle

	virtual HRESULT __stdcall Suspend(FWULONG nTimeStamp)
	{
		m_nSuspended++;
		if (m_nSuspended == 1)
		{
			// just suspended
			ACTION_EVENT event = { NULL, nTimeStamp, EVENT_SUSPENDED, 0, NULL, NULL };
			SendEventEx(&event);
			m_nSuspendTime = nTimeStamp;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall Resume(FWULONG nTimeStamp)
	{
		if (m_nSuspended == 0) return S_OK;		// already resumed
		m_nSuspended--;
		if (m_nSuspended == 0)
		{
			// just resumed
			//if (m_nSuspendTime > m_nStartTime)
			//	m_nStartTime += nTimeStamp - m_nSuspendTime;
			m_nStartTime = nTimeStamp;

			ACTION_EVENT event = { NULL, nTimeStamp, EVENT_RESUMED, 0, NULL, NULL };
			SendEventEx(&event);
		}
		return S_OK;
	}

	virtual HRESULT __stdcall IsSuspended()
	{
		if (m_nSuspended != 0) return S_OK;
		else return S_FALSE;
	}

	//////////////////////////////////////////////
	// Time and Phase Functions

	virtual HRESULT __stdcall SetStartTime(FWULONG nStartTime)	{ m_nStartTime = nStartTime; return S_OK; }
	virtual HRESULT __stdcall GetStartTime(FWULONG *p)			{ if (p) *p = m_nStartTime; return S_OK; }
	virtual FWULONG __stdcall StartTime()						{ return m_nStartTime; }
	virtual HRESULT __stdcall GetPeriod(FWULONG *p)				{ if (p) *p = m_nPeriod; return S_OK; }
	virtual FWULONG __stdcall Period()							{ return m_nPeriod; }
	virtual HRESULT __stdcall GetCompleteTime(FWULONG *p)		{ if (p) *p = StartTime() + Period(); return S_OK; }
	virtual FWULONG __stdcall CompleteTime()					{ return StartTime() + Period(); }

	virtual HRESULT __stdcall IsStarted(struct ACTION_EVENT *pEvent)
	{
		return (pEvent->nTimeStamp >= StartTime()) ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall IsOverdue(struct ACTION_EVENT *pEvent)
	{
		return (pEvent->nTimeStamp >= CompleteTime()) ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall IsMorituri(struct ACTION_EVENT *pEvent)
	{
		switch (pEvent->pSubs->nFlags & ACTION_MASK_MODE)
		{
		case ACTION_AUTO:		return IsOverdue(pEvent);
		case ACTION_ONCE:		return S_OK;
		case ACTION_MANUAL:		return S_FALSE;
		case ACTION_MORITURI:	return S_OK;
		}
		return S_FALSE;
	}

	virtual HRESULT __stdcall Die(struct ACTION_EVENT *pEvent)
	{
		if ((pEvent->pSubs->nFlags & ACTION_MASK_MODE) == ACTION_ONCE) 
			return S_OK;	// will soon die anyway... change to MORITURI would trigger EVENT_END - not desired with ACTION_ONCE events.
		pEvent->pSubs->nFlags &= ~ACTION_MASK_MODE; 
		pEvent->pSubs->nFlags |= ACTION_MORITURI; return S_OK;
	}

	virtual HRESULT __stdcall GetTime(struct ACTION_EVENT *pEvent, FWULONG *p)
	{
		if (!p) return S_OK;
		*p = pEvent->nTimeStamp - StartTime();
		return S_OK;
	}

	virtual HRESULT __stdcall GetPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
	{
		if (!p) return S_OK;
		if (m_nPeriod == 0)
		{
			*p = 1;
			return S_OK;
		}
		return ApplyEnvelope(pEvent, Time(pEvent), Period(), p);
	}

	virtual FWULONG __stdcall Time(struct ACTION_EVENT *pEvent)		{ FWULONG t; GetTime(pEvent, &t); return t; }
	virtual FWFLOAT __stdcall Phase(struct ACTION_EVENT *pEvent)	{ FWFLOAT ph; GetPhase(pEvent, &ph); return ph; }

	virtual HRESULT __stdcall GetDeltaPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
	{
		FWFLOAT fT;
		GetPhase(pEvent, &fT);
		if (p) *p = fT - m_fTimePrev;
		m_fTimePrev = fT;
		return S_OK;
	}

	virtual HRESULT __stdcall GetProgPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
	{
		FWFLOAT fT;
		GetPhase(pEvent, &fT);
		if (p)
			if (m_fTimePrev == 1.0)
				*p = 1.0;
			else
				*p = (FWFLOAT)(fT - m_fTimePrev) / (FWFLOAT)(1.0 - m_fTimePrev);
		m_fTimePrev = fT;
		return S_OK;
	}

	//////////////////////////////////////////////
	// Time Envelope Functions

	virtual HRESULT __stdcall SetSinusoidalEnvelope(FWFLOAT fEaseIn, FWFLOAT fEaseOut)			{ m_env.type = ACTION_ENV_SIN; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetParabolicEnvelope(FWFLOAT fEaseIn, FWFLOAT fEaseOut)			{ m_env.type = ACTION_ENV_PARA; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetSinusoidalEnvelopeT(FWULONG timeEaseIn, FWULONG timeEaseOut)	{ m_env.type = ACTION_ENV_SIN_TIME; m_env.timeEaseIn = timeEaseIn; m_env.timeEaseOut = timeEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetParabolicEnvelopeT(FWULONG timeEaseIn, FWULONG timeEaseOut)	{ m_env.type = ACTION_ENV_PARA_TIME; m_env.timeEaseIn = timeEaseIn; m_env.timeEaseOut = timeEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetPhysicalEnvelopeT(FWFLOAT s, FWFLOAT v, FWFLOAT a, FWFLOAT j)	{ m_env.type = ACTION_ENV_PHYSICAL; m_env.fS = s; m_env.fV = v; m_env.fA = a; m_env.fJ = j; return S_OK; }
	virtual HRESULT __stdcall SetNoneEnvelope()													{ m_env.type = ACTION_ENV_NONE; return S_OK; }
	virtual HRESULT __stdcall SetEnvelope(enum ACTION_ENVELOPE t, FWFLOAT fEaseIn, FWFLOAT fEaseOut)	{ m_env.type = t; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }

	virtual HRESULT __stdcall SetEnvelopeEx(struct ACTION_ENVELOPE_DATA *p)						{ m_env = *p; return S_OK; }

	virtual HRESULT __stdcall GetEnvelope(ACTION_EVENT *pEvent, /*[out]*/ struct ACTION_ENVELOPE_DATA *p)
	{
//		if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pSender)
//			pEvent->pSender->GetEnvelope(NULL, &m_env);
//		if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pOriginator)
//			pEvent->pOriginator->GetEnvelope(NULL, &m_env);
		*p = m_env;
		return S_OK;
	}

	virtual HRESULT __stdcall ApplyEnvelope(ACTION_EVENT *pEvent, FWULONG timeIn, FWULONG timeRef, FWFLOAT *pfOut)
	{
		if (!pfOut) return S_OK;

//		if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER) 
//			if (m_pCurEvent && m_pCurEvent->pSender && m_pCurEvent->pSender != this)
//				return m_pCurEvent->pSender->ApplyEnvelope(pEvent, timeIn, timeRef, pfOut);

		//struct ACTION_ENVELOPE_DATA qq;
		//GetEnvelope(pEvent, &qq);

		//if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pOriginator)
		//	return pEvent->pOriginator->ApplyEnvelope(NULL, timeIn, pfOut);
		//if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pSender)
		//	return pEvent->pSender->ApplyEnvelope(NULL, timeIn, pfOut) == S_OK;

		if (m_env.type == ACTION_ENV_PHYSICAL)
		{
			LiftKinematicsCalculator calc(0, 0, m_env.fS, m_env.fV, m_env.fA, m_env.fJ);
			*pfOut = (FWFLOAT)calc.getPosition(calc.getStopTime() * timeIn / Period()) / m_env.fS;
			return S_OK;
		}

		double t = timeIn;
		double T = timeRef;
		double f = 0;
		double T1, T2;

		switch (m_env.type)
		{
		case ACTION_ENV_PARA:
		case ACTION_ENV_SIN:
			T1 = m_env.fEaseIn * T;
			T2 = m_env.fEaseOut * T;
			break;
		case ACTION_ENV_PARA_TIME:
		case ACTION_ENV_SIN_TIME:
			T1 = m_env.timeEaseIn * T;
			T2 = m_env.timeEaseOut * T;
			break;
		}
			
		double q, s, v0;
		switch (m_env.type)
		{
		case ACTION_ENV_PARA:
		case ACTION_ENV_PARA_TIME:
			TRACE(L"P");
			v0 = 2.0 / (T2 - T1 + T);
			if (t < 0)			f = 0;
			else if (t < T1)	f = v0 * t * t / 2.0 / T1;
			else if (t <= T2)	f = v0 * T1 / 2.0 + v0 * (t - T1);
			else if (t < T)		f = v0 * T1 / 2.0 + v0 * (T2 - T1) + (v0 - v0 * (t - T2) / (T - T2) / 2.0) * (t - T2);
			else				f = 1;
			break;
		case ACTION_ENV_SIN:
		case ACTION_ENV_SIN_TIME:
			TRACE(L"S");
			q = T1 * 2.0 / M_PI + T2 - T1 + (T - T2) * 2.0 / M_PI;
			if (t < 0)			s = 0;
			else if (t < T1)	s = T1 * 2.0 / M_PI * (sin((t / T1) * M_PI / 2.0 - M_PI / 2.0) + 1.0);
			else if (t < T2)	s = 2.0 * T1 / M_PI + t - T1;
			else if (t < T)		s = 2.0 * T1 / M_PI + T2 - T1 + ((T - T2) * (2.0 / M_PI)) * sin(((t - T2) / (T - T2)) * M_PI / 2.0);
			else				s = q;
			f = s / q;
			break;
		default:
			TRACE(L"N");
			f = t / T;
			break;
		}

		if (f < 0) f = 0; if (f > 1) f = 1;
		*pfOut = (FWFLOAT)f;
		return S_OK;
	}

	//////////////////////////////////////////////
	// Events

	virtual HRESULT __stdcall SendEvent(FWULONG nTimeStamp, FWULONG nEvent, FWULONG nSubCode, IFWUnknown *pPtr)
	{
		ACTION_EVENT event = { NULL, nTimeStamp, nEvent, nSubCode, pPtr, NULL };
		return SendEventEx(&event);
	}

	virtual HRESULT __stdcall SendEventEx(struct ACTION_EVENT *pEvent)
	{
		HRESULT h = S_OK;

		// store the current event info
		ACTION_EVENT *pPrevEvent = m_pCurEvent;
		m_pCurEvent = pEvent;

		if (m_pfHook)
			h = m_pfHook(pEvent, this, m_nParam, m_pParam);
		if (h == S_OK)
			h = HandleEvent(pEvent);	// the only direct call to HandleEvent!
		m_pCurEvent = pPrevEvent;
		return h;
	}

	virtual HRESULT __stdcall RaiseEvent(FWULONG nTimeStamp, FWULONG nEvent, FWULONG nSubCode, IFWUnknown *pPtr)
	{
		ACTION_EVENT event = { this, nTimeStamp, nEvent, nSubCode, pPtr, NULL, NULL };
		return RaiseEventEx(&event);
	}

private:
	FWULONG _GetTimeSetting(ACTION_SUBS *pSubs, ACTION_EVENT *pEvent)
	{
		if (pSubs->nFlags & ACTION_NOTIME) return pEvent->nTimeStamp;
		if ((pSubs->nFlags & ACTION_MASK_LTE_GTE) && (pSubs->nFlags & ACTION_ONCE))
			return pSubs->nTrigger;
		else
			return pEvent->nSubCode;
	}

public:

	virtual HRESULT __stdcall RaiseEventEx(struct ACTION_EVENT *pEvent)
	{
		// first send it to myself
		if (SendEventEx(pEvent) != S_OK) 
			return S_OK;	// immediate return if no further porcessing required/possible

		// now, create lists of subscribers
		// no events will be sent or raised in this section

		// locate the COLLection of collections of subscribed actions
		MAP_OF_COLL::iterator pos = m_Subsribers.find(pEvent->nEvent);
		if (pos == m_Subsribers.end()) return S_OK;		// no subscribers
		COLL &coll = (*pos).second;

		// all the subscriptions will be scanned and classified as ACTION_RESUME, ACTION_SUSPEND, ACTION_UNSUBSCRIBE and regular (HandleEvent)
		list<ACTION_SUBS_EX*> lstSuspend;
		list<ACTION_SUBS_EX*> lstResume;
		list<ACTION_SUBS_EX*> lstCalls;

		int nAux;

		// scan ACTION_ANY subscriptions
		if (coll.lstAny.size())
			for (LIST_OF_SUBS::iterator i = coll.lstAny.begin(); i != coll.lstAny.end(); i++ )
				switch ((*i).nFlags & ACTION_MASK_USE)
				{
					case ACTION_CALL: lstCalls.push_back(&(*i)); break;
					case ACTION_RESUME: lstResume.push_back(&(*i)); break;
					case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
				}

		// scan ACTION_EQ subscriptions
		if (coll.mapEq.size())
		{
			MAP_INT_LIST_OF_SUBS::iterator pos = coll.mapEq.find(pEvent->nSubCode);
			if (pos != coll.mapEq.end())
				for (LIST_OF_SUBS::iterator i = (*pos).second.begin(); i != (*pos).second.end(); i++ )
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}
		}

		// scan ACTION_LTE subscriptions
		if (coll.lstLte.size())
			for (LIST_OF_SUBS::iterator i = coll.lstLte.begin(); i != coll.lstLte.end(); i++ )
				if (pEvent->nSubCode <= (*i).nTrigger)
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}

		// scan ACTION_GTE subscriptions
		if (coll.lstGte.size())
			for (LIST_OF_SUBS::iterator i = coll.lstGte.begin(); i != coll.lstGte.end(); i++ )
			{
				nAux = (*i).nTrigger;
				if (pEvent->nSubCode >= (*i).nTrigger)
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}
				else
					break;
			}

		// store the current event info
		ACTION_EVENT *pPrevEvent = m_pCurEvent;
		m_pCurEvent = pEvent;

		// Execute all ACTION_RESUME operations
		for (list<ACTION_SUBS_EX*>::iterator i = lstResume.begin(); i != lstResume.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;
			pSubs->pSubscriber->Resume(_GetTimeSetting(pSubs, pEvent));
			UnSubscribe(pEvent->nTimeStamp, pSubs);
		}

		// Call the actual HandleEvent
		for (list<ACTION_SUBS_EX*>::iterator i = lstCalls.begin(); i != lstCalls.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;

			// ignore suspended actions
			if (pSubs->pSubscriber->IsSuspended() == S_OK)
				continue;
			// if ((pSubs->nFlags & ACTION_MASK_MODE) == ACTION_MORITURI) continue;	// another discontinued protection
			// if (pSubs->pSubscriber == this) continue;	// removed protection against self subscription

			// prepare EVENT's local copy - the subscriber will only access this...
			ACTION_EVENT myEvent = *pEvent;
			myEvent.pSender = this;
			myEvent.pPrev = pEvent;
			myEvent.pSubs = pSubs;
			if (!(pSubs->nFlags & ACTION_NOTIME))
				myEvent.nSubCode = _GetTimeSetting(pSubs, pEvent);

			// Raise EVENT_BEGIN - when event handled for the first time
			if ((pSubs->nFlags & ACTION_MASK_MODE) != ACTION_ONCE && (pSubs->nFlags & ACTION_RESERVED_1) == 0)
			{
				pSubs->nFlags |= ACTION_RESERVED_1;
				pSubs->pSubscriber->RaiseEvent(myEvent.nTimeStamp, EVENT_BEGIN, pSubs->pSubscriber->StartTime(), (IFWUnknown*)&myEvent);
			}
			
			// Recursive Call: Pass onto Subscriber's subscribers
			pSubs->pSubscriber->RaiseEventEx(&myEvent);

			// Send EVENT_END and unsubscribe if action IsMorituri
			if (pSubs->pSubscriber && pSubs->pSubscriber->IsMorituri(&myEvent) == S_OK)
			{
				if ((pSubs->nFlags & ACTION_MASK_MODE) != ACTION_ONCE)		// no EVENT_END for ACTION_ONCE actions
					pSubs->pSubscriber->RaiseEvent(myEvent.nTimeStamp, EVENT_END, pSubs->pSubscriber->CompleteTime(), (IFWUnknown*)&myEvent);
				UnSubscribe(myEvent.nTimeStamp, pSubs);
			}
		}
		
		// Execute all ACTION_SUSPEND operations
		for (list<ACTION_SUBS_EX*>::iterator i = lstSuspend.begin(); i != lstSuspend.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;
			pSubs->pSubscriber->Suspend(_GetTimeSetting(pSubs, pEvent));
			UnSubscribe(pEvent->nTimeStamp, pSubs);
		}
		
		m_pCurEvent = pPrevEvent;
		return S_OK;
	}

	virtual HRESULT __stdcall SetHandleEventHook(HANDLE_EVENT_HOOK_FUNC pfHook, FWULONG nParam, void *pParam)
	{
		m_pfHook = pfHook;
		m_nParam = nParam;
		m_pParam = pParam;
		return S_OK;
	}
};

	//////////////////////////////////////////////
	// Action Setup
	// to be implemented in the derived classes...

typedef CTAction ACTION;

#endif  // _ACTIONTEMPL_
