// Script.h

#include "Camera.h"

#pragma once

class CAdVisuoView;
using namespace std;

#define N_CAMERAS	10

class CScriptEvent
{
	CAdVisuoView *m_pView;

	AVULONG m_nTime;
	AVULONG m_nTimeAnim;
	AVULONG m_nTimeFF;
	CString m_desc;

	struct
	{
		CAMPARAMS params;
		CAMLOC camloc;
		AVULONG nStorey;
		AVULONG nLift;
	} m_camera[N_CAMERAS];

public:
	CScriptEvent(CAdVisuoView *pView) : m_pView(pView), m_nTime(0), m_nTimeAnim(0), m_nTimeFF(0) { } 

	CString CScriptEvent::GetDesc();
	AVULONG GetTime()			{ return m_nTime; }
	AVULONG GetAnimTime()		{ return m_nTimeAnim; }
	AVULONG GetFFTime()			{ return m_nTimeFF; }

	void SetTime(AVULONG n)		{ m_nTime = n; }
	void SetAnimTime(AVULONG n)	{ m_nTime += m_nTimeAnim; m_nTimeAnim = n; if (m_nTime > m_nTimeAnim) m_nTime -= m_nTimeAnim; else m_nTime = 0; }
	void SetFFTime(AVULONG n)	{ m_nTimeFF = n; }

	void Record();
	void Play();
};

class CScript
{
	CAdVisuoView *m_pView;
	vector<CScriptEvent*> m_events;
	AVULONG m_nPos;


public:
	CScript(CAdVisuoView *pView) : m_pView(pView) { } 

	AVULONG size()								{ return m_events.size(); }
	CScriptEvent *operator[](AVULONG i)			{ return m_events[i]; }

	void Record();
	void Play(AVULONG i);
	void Delete(AVULONG i);

	void Play();
	void Proceed(AVULONG nTime);
};
