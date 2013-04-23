// Script.h

#include "Camera.h"

#pragma once

class CAdVisuoView;

#define N_CAMERAS	10

class CScriptEvent
{
	CAdVisuoView *m_pView;

	AVLONG m_nTime;
	AVLONG m_nTimeAnim;
	AVLONG m_nTimeFF;
	AVFLOAT m_fAccel;
	CString m_desc;

	CAMPARAMS m_camera[N_CAMERAS];

public:
	CScriptEvent(CAdVisuoView *pView) : m_pView(pView), m_nTime(0), m_nTimeAnim(0), m_nTimeFF(0), m_fAccel(0) { } 

	CString CScriptEvent::GetDesc();
	AVLONG GetTime()			{ return m_nTime; }
	AVLONG GetAnimTime()		{ return m_nTimeAnim; }
	AVLONG GetFFTime()			{ return m_nTimeFF; }
	AVFLOAT GetAccel()			{ return m_fAccel; }

	void SetTime(AVLONG n)		{ m_nTime = n; }
	void SetAnimTime(AVLONG n)	{ m_nTime += m_nTimeAnim; m_nTimeAnim = n; if (m_nTime > m_nTimeAnim) m_nTime -= m_nTimeAnim; else m_nTime = 0; }
	void SetFFTime(AVLONG n)	{ m_nTimeFF = n; }
	void SetAccel(AVFLOAT f)	{ m_fAccel = f; }

	void Record();
	void Play(AVLONG &nTime, AVLONG nAuxClockValue = 0x7FFFFFFF);

	void Serialize(CArchive& ar);
};

class CScript
{
	CAdVisuoView *m_pView;
	std::vector<CScriptEvent*> m_events;
	AVULONG m_nPos;


public:
	CScript(CAdVisuoView *pView) : m_pView(pView) { } 
	~CScript()									{ Reset(); }

	AVULONG size()								{ return m_events.size(); }
	CScriptEvent *operator[](AVULONG i)			{ return m_events[i]; }

	void Record();
	void Play(AVULONG i, AVLONG nAuxClockValue = 0x7FFFFFFF);
	void Delete(AVULONG i);

	void Play();
	void Proceed(AVLONG &nTime, AVLONG nAuxClockValue = 0x7FFFFFFF);

	void Sort();
	void Reset();
	void Serialize(CArchive& ar);
};
