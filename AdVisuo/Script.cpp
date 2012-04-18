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

	for (AVULONG i = 0; i < N_CAMERAS; i++)
	{
		CCamera *pCamera = m_pView->GetCamera(i);
		pCamera->CheckLocation();
		m_camera[i] = m_pView->GetCamera(i)->GetCameraParams();
	}
}

void CScriptEvent::Play()
{
	IAction *pAction = NULL;
	m_pView->AuxPlay(&pAction); 
	for (AVULONG i = 0; i < N_CAMERAS; i++)
		if (GetAnimTime() == 0)
			m_pView->GetCamera(i)->MoveTo(m_camera[i]);
		else
			m_pView->GetCamera(i)->AnimateTo(pAction, m_camera[i], GetAnimTime());
	pAction->Release();
}

void CScript::Record()
{
	CScriptEvent *p = new CScriptEvent(m_pView);
	p->Record();
	m_events.push_back(p);
	sort(m_events.begin(), m_events.end(), [](CScriptEvent *p1, CScriptEvent *p2) -> bool { return p1->GetTime() < p2->GetTime(); } );
}

void CScript::Play(AVULONG i)
{
	m_events[i]->Play();
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

void CScript::Proceed(AVULONG nTime)
{
	while (m_nPos < size() && (*this)[m_nPos]->GetTime() < nTime)
	{
		(*this)[m_nPos]->Play();
		m_nPos++;
	}
}