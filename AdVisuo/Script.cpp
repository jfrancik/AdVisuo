// Script.cpp

#include "StdAfx.h"
#include "Script.h"
#include "AdVisuoView.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>

#include "freewilltools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)

CString CScriptEvent::GetDesc()
{
	wstringstream s;

	CString str;
	str.Format(L"%d:%02d:%02d", (m_nTime/3600000), (m_nTime/60000)%60, (m_nTime/1000)%60);		//, (-t/10)%100);

	s << (LPCTSTR)str << L": " << (LPCTSTR)m_desc;
	return s.str().c_str();
}


void CScriptEvent::Record()
{
	m_desc = m_pView->GetCamera(0)->GetShortTextDescription();
	SetTime(m_pView->GetPlayTime());
	SetAnimTime(1000);
	SetFFTime(0);
	SetAccel(m_pView->GetAccel());

	for (AVULONG i = 0; i < N_CAMERAS; i++)
	{
		CCamera *pCamera = m_pView->GetCamera(i);
		pCamera->CheckLocation();
		m_camera[i] = m_pView->GetCamera(i)->GetCameraParams();
	}
}

void CScriptEvent::Play(AVULONG &nTime, AVULONG nAuxClockValue)
{
	// EXCEPTIONAL!!!
	if (m_nTime == 968000)
		m_pView->OnTmpGroup2();
	if (m_nTime == 0)
		m_pView->OnTmpGroup1();

	if (GetFFTime() > 0.01f)
	{
		nTime = GetFFTime();
		m_pView->Rewind(nTime);
	}

	IAction *pAction = NULL;
	m_pView->AuxPlay(&pAction, nAuxClockValue); 
	m_pView->PutAccel(GetAccel());
	for (AVULONG i = 0; i < N_CAMERAS; i++)
		if (GetAnimTime() == 0)
			m_pView->GetCamera(i)->MoveTo(m_camera[i]);
		else
			m_pView->GetCamera(i)->AnimateTo(pAction, m_camera[i], GetAnimTime());
	pAction->Release();
}

void CScriptEvent::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_nTime;
		ar << m_nTimeAnim;
		ar << m_nTimeFF;
		ar << m_fAccel;
		ar << m_desc;
		for (int i = 0; i < N_CAMERAS; i++)
			ar.Write(m_camera + i, sizeof(CAMPARAMS));
	}
	else
	{
		ar >> m_nTime;
		ar >> m_nTimeAnim;
		ar >> m_nTimeFF;
		ar >> m_fAccel; 
		ar >> m_desc;
		for (int i = 0; i < N_CAMERAS; i++)
			ar.Read(m_camera + i, sizeof(CAMPARAMS));
	}
}

void CScript::Record()
{
	CScriptEvent *p = new CScriptEvent(m_pView);
	p->Record();
	m_events.push_back(p);
	Sort();
}

void CScript::Play(AVULONG i, AVULONG nAuxClockValue)
{
	m_events[i]->Play(nAuxClockValue);
}

void CScript::Delete(AVULONG i)
{
	delete m_events[i];
	m_events.erase(m_events.begin() + i);
}

void CScript::Play()
{
	m_nPos = 0;
}

void CScript::Proceed(AVULONG &nTime, AVULONG nAuxClockValue)
{
	while (m_nPos < size() && (*this)[m_nPos]->GetTime() < nTime)
	{
		(*this)[m_nPos]->Play(nTime, nAuxClockValue);
		m_nPos++;
	}
}

void CScript::Sort()
{
	sort(m_events.begin(), m_events.end(), [](CScriptEvent *p1, CScriptEvent *p2) -> bool { return p1->GetTime() < p2->GetTime(); } );
}

void CScript::Reset()
{
	while (!m_events.empty())
		Delete(0);
}

void CScript::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		int n = m_events.size();
		ar << n;
		for (int i = 0; i < n; i++)
			m_events[i]->Serialize(ar);
	}
	else
	{
		int n;
		ar >> n;
		Reset();
		for (int i = 0; i < n; i++)
		{
			CScriptEvent *p = new CScriptEvent(m_pView);
			p->Serialize(ar);
			m_events.push_back(p);
		}
		Sort();
	}
}
