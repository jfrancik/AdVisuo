// AdVisuoRenderer.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuoRenderer.h"

#include "Camera.h"
#include "VisSim.h"
#include "VisProject.h"
#include "Engine.h"

#include <freewill.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////////////
// CAdVisuo Renderer

bool CAdVisuoRenderer::SetupCamera(CCamera *pCamera)
{
	if (!pCamera || !pCamera->GetCamera()) return false;

	m_pCamera = pCamera;

	pCamera->CheckLocation();

	m_pLiftGroup->GetProject()->GetEngine()->PutCamera(pCamera->GetCamera());
	m_pEngine->Render(pCamera->GetCamera());
	return true;
}

void CAdVisuoRenderer::RenderLifts(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		for (AVULONG j = m_pLiftGroup->GetShaft(i)->GetLiftBegin(); j < m_pLiftGroup->GetShaft(i)->GetLiftEnd(); j++)
			m_pEngine->Render(m_pLiftGroup->GetLiftElement(j)->GetObject());
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		for (AVULONG j = m_pLiftGroup->GetShaft(i)->GetLiftBegin(); j < m_pLiftGroup->GetShaft(i)->GetLiftEnd(); j++)
			m_pEngine->Render(m_pLiftGroup->GetLiftElement(j)->GetObject());
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
		for (AVULONG j = m_pLiftGroup->GetShaft(iShaft)->GetLiftBegin(); j < m_pLiftGroup->GetShaft(iShaft)->GetLiftEnd(); j++)
			m_pEngine->Render(m_pLiftGroup->GetLiftElement(j)->GetObject());
}

// Render Shafts
void CAdVisuoRenderer::RenderShafts(FWULONG nRow, FWULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, nRow)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElement(iStorey, i)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->GetObject());
	}
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElement(iStorey, i)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, nRow)->GetObject());
	}
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
	{
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLeft(iStorey, iShaft)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElement(iStorey, iShaft)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetShaftElementRight(iStorey, iShaft)->GetObject());
	}
}
	
void CAdVisuoRenderer::RenderPits(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pEngine->Render(m_pLiftGroup->GetPitElementLeftOrRight(i, nRow)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElement(i)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElementLeftOrRight(i, 1-nRow)->GetObject());
	}
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pEngine->Render(m_pLiftGroup->GetPitElementLeftOrRight(i, 1-nRow)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElement(i)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElementLeftOrRight(i, nRow)->GetObject());
	}
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
	{
		m_pEngine->Render(m_pLiftGroup->GetPitElementLeft(iShaft)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElement(iShaft)->GetObject());
		m_pEngine->Render(m_pLiftGroup->GetPitElementRight(iShaft)->GetObject());
	}
}

void CAdVisuoRenderer::RenderMachines(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(m_pLiftGroup->GetMachineElement(i)->GetObject());
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(m_pLiftGroup->GetMachineElement(i)->GetObject());
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(m_pLiftGroup->GetMachineElement(iShaft)->GetObject());
}

void CAdVisuoRenderer::RenderShafts(FWULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pLiftGroup->GetStoreyCount() - 1));
	RenderPits(nRow);
	for (FWLONG i = 0; i < iStorey; i++)
		RenderShafts(nRow, i);
	RenderMachines(nRow);
	for (FWLONG i = m_pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShafts(nRow, i);
	RenderShafts(nRow, iStorey);
}

// Render Shafts Lobby Side
void CAdVisuoRenderer::RenderShaftsLobbySide(FWULONG nRow, FWULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLobbySide(iStorey, i)->GetObject());
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLobbySide(iStorey, i)->GetObject());
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(m_pLiftGroup->GetShaftElementLobbySide(iStorey, iShaft)->GetObject());
}
	
void CAdVisuoRenderer::RenderPitsLobbySide(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)m_pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow)));
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(m_pLiftGroup->GetPitElementLobbySide(i)->GetObject());
	for (FWLONG i = m_pLiftGroup->GetShaftBegin(nRow) + m_pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(m_pLiftGroup->GetPitElementLobbySide(i)->GetObject());
	if (iShaft >= (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pLiftGroup->GetShaftBegin(nRow) + (AVLONG)m_pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(m_pLiftGroup->GetPitElementLobbySide(iShaft)->GetObject());
}
	
void CAdVisuoRenderer::RenderShaftsLobbySide(FWULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pLiftGroup->GetStoreyCount() - 1));
	RenderPitsLobbySide(nRow);
	for (FWLONG i = 0; i < iStorey; i++)
		RenderShaftsLobbySide(nRow, i);
	for (FWLONG i = m_pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShaftsLobbySide(nRow, i);
	RenderShaftsLobbySide(nRow, iStorey);
}

// Render Storeys (Lobbies)
void CAdVisuoRenderer::RenderStoreys()
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pLiftGroup->GetStoreyCount() - 1));
	if (m_pLiftGroup->GetPit())
		m_pEngine->Render(m_pLiftGroup->GetPitElement()->GetObject());
	for (FWLONG i = 0; i < iStorey; i++)
		m_pEngine->Render(m_pLiftGroup->GetStoreyElement(i)->GetObject());
	if (m_pLiftGroup->GetMachineRoom())
		m_pEngine->Render(m_pLiftGroup->GetMachineRoomElement()->GetObject());
	for (FWLONG i = m_pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		m_pEngine->Render(m_pLiftGroup->GetStoreyElement(i)->GetObject());
	m_pEngine->Render(m_pLiftGroup->GetStoreyElement(iStorey)->GetObject());
}

void CAdVisuoRenderer::RenderCentre()
{
	RenderShafts(0);
	RenderLifts(0);
	RenderShaftsLobbySide(0);
	RenderShafts(1);
	RenderLifts(1);
	RenderShaftsLobbySide(1);
	RenderStoreys();
}

void CAdVisuoRenderer::RenderSide(AVLONG nLiftRow)
{
	if (nLiftRow < 0)
		nLiftRow = m_pLiftGroup->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
	RenderShafts(1-nLiftRow);
	RenderLifts(1-nLiftRow);
	RenderShaftsLobbySide(1-nLiftRow);
	RenderStoreys();
	RenderShafts(nLiftRow);
	RenderShaftsLobbySide(nLiftRow);
	RenderLifts(nLiftRow);
}

void CAdVisuoRenderer::RenderCentreOuter()
{
	RenderLifts(0);
	RenderShafts(0);
	RenderShaftsLobbySide(0);
	RenderLifts(1);
	RenderShafts(1);
	RenderShaftsLobbySide(1);
	RenderStoreys();
}

void CAdVisuoRenderer::RenderSideOuter(AVLONG nLiftRow)
{
	if (nLiftRow < 0)
		nLiftRow = m_pLiftGroup->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
	RenderLifts(1-nLiftRow);
	RenderShafts(1-nLiftRow);
	RenderShaftsLobbySide(1-nLiftRow);
	RenderStoreys();
	RenderShaftsLobbySide(nLiftRow);
	RenderLifts(nLiftRow);
	RenderShafts(nLiftRow);
}

