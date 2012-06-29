// AdVisuoRenderer.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuoRenderer.h"

#include "Camera.h"

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer

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

	m_pBuilding->GetScene()->PutCamera(pCamera->GetCamera());
	pCamera->GetCamera()->Render(m_pRenderer);
	return true;
}

void CAdVisuoRenderer::RenderLifts(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
		for (AVULONG j = m_pBuilding->GetShaft(i)->GetLiftBegin(); j < m_pBuilding->GetShaft(i)->GetLiftEnd(); j++)
			m_pBuilding->GetLiftElement(j)->Render(m_pRenderer);
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
		for (AVULONG j = m_pBuilding->GetShaft(i)->GetLiftBegin(); j < m_pBuilding->GetShaft(i)->GetLiftEnd(); j++)
			m_pBuilding->GetLiftElement(j)->Render(m_pRenderer);
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
		for (AVULONG j = m_pBuilding->GetShaft(iShaft)->GetLiftBegin(); j < m_pBuilding->GetShaft(iShaft)->GetLiftEnd(); j++)
			m_pBuilding->GetLiftElement(j)->Render(m_pRenderer);
}

// Render Shafts
void CAdVisuoRenderer::RenderShafts(FWULONG nRow, FWULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pBuilding->GetShaftElementLeftOrRight(iStorey, i, nRow)->Render(m_pRenderer);
		m_pBuilding->GetShaftElement(iStorey, i)->Render(m_pRenderer);
		m_pBuilding->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->Render(m_pRenderer);
	}
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pBuilding->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->Render(m_pRenderer);
		m_pBuilding->GetShaftElement(iStorey, i)->Render(m_pRenderer);
		m_pBuilding->GetShaftElementLeftOrRight(iStorey, i, nRow)->Render(m_pRenderer);
	}
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
	{
		m_pBuilding->GetShaftElementLeft(iStorey, iShaft)->Render(m_pRenderer);
		m_pBuilding->GetShaftElement(iStorey, iShaft)->Render(m_pRenderer);
		m_pBuilding->GetShaftElementRight(iStorey, iShaft)->Render(m_pRenderer);
	}
}
	
void CAdVisuoRenderer::RenderPits(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pBuilding->GetPitElementLeftOrRight(i, nRow)->Render(m_pRenderer);
		m_pBuilding->GetPitElement(i)->Render(m_pRenderer);
		m_pBuilding->GetPitElementLeftOrRight(i, 1-nRow)->Render(m_pRenderer);
	}
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pBuilding->GetPitElementLeftOrRight(i, 1-nRow)->Render(m_pRenderer);
		m_pBuilding->GetPitElement(i)->Render(m_pRenderer);
		m_pBuilding->GetPitElementLeftOrRight(i, nRow)->Render(m_pRenderer);
	}
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
	{
		m_pBuilding->GetPitElementLeft(iShaft)->Render(m_pRenderer);
		m_pBuilding->GetPitElement(iShaft)->Render(m_pRenderer);
		m_pBuilding->GetPitElementRight(iShaft)->Render(m_pRenderer);
	}
}

void CAdVisuoRenderer::RenderMachines(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
		m_pBuilding->GetMachineElement(i)->Render(m_pRenderer);
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pBuilding->GetMachineElement(i)->Render(m_pRenderer);
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
		m_pBuilding->GetMachineElement(iShaft)->Render(m_pRenderer);
}

void CAdVisuoRenderer::RenderShafts(FWULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pBuilding->GetStoreyCount() - 1));
	RenderPits(nRow);
	for (FWLONG i = 0; i < iStorey; i++)
		RenderShafts(nRow, i);
	RenderMachines(nRow);
	for (FWLONG i = m_pBuilding->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShafts(nRow, i);
	RenderShafts(nRow, iStorey);
}

// Render Shafts Lobby Side
void CAdVisuoRenderer::RenderShaftsLobbySide(FWULONG nRow, FWULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
		m_pBuilding->GetShaftElementLobbySide(iStorey, i)->Render(m_pRenderer);
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pBuilding->GetShaftElementLobbySide(iStorey, i)->Render(m_pRenderer);
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
		m_pBuilding->GetShaftElementLobbySide(iStorey, iShaft)->Render(m_pRenderer);
}
	
void CAdVisuoRenderer::RenderPitsLobbySide(FWULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)m_pBuilding->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow)));
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow); i < iShaft; i++)
		m_pBuilding->GetPitElementLobbySide(i)->Render(m_pRenderer);
	for (FWLONG i = m_pBuilding->GetShaftBegin(nRow) + m_pBuilding->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pBuilding->GetPitElementLobbySide(i)->Render(m_pRenderer);
	if (iShaft >= (AVLONG)m_pBuilding->GetShaftBegin(nRow) && iShaft < (AVLONG)m_pBuilding->GetShaftBegin(nRow) + (AVLONG)m_pBuilding->GetShaftCount(nRow))
		m_pBuilding->GetPitElementLobbySide(iShaft)->Render(m_pRenderer);
}
	
void CAdVisuoRenderer::RenderShaftsLobbySide(FWULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pBuilding->GetStoreyCount() - 1));
	RenderPitsLobbySide(nRow);
	for (FWLONG i = 0; i < iStorey; i++)
		RenderShaftsLobbySide(nRow, i);
	for (FWLONG i = m_pBuilding->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShaftsLobbySide(nRow, i);
	RenderShaftsLobbySide(nRow, iStorey);
}

// Render Storeys (Lobbies)
void CAdVisuoRenderer::RenderStoreys()
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)m_pBuilding->GetStoreyCount() - 1));
	if (m_pBuilding->GetPit())
		m_pBuilding->GetPitElement()->Render(m_pRenderer);
	for (FWLONG i = 0; i < iStorey; i++)
		m_pBuilding->GetStoreyElement(i)->Render(m_pRenderer);
	if (m_pBuilding->GetMachineRoom())
		m_pBuilding->GetMachineRoomElement()->Render(m_pRenderer);
	for (FWLONG i = m_pBuilding->GetStoreyCount() - 1; i > iStorey; i--)
		m_pBuilding->GetStoreyElement(i)->Render(m_pRenderer);
	m_pBuilding->GetStoreyElement(iStorey)->Render(m_pRenderer);
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
		nLiftRow = m_pBuilding->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
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
		nLiftRow = m_pBuilding->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
	RenderLifts(1-nLiftRow);
	RenderShafts(1-nLiftRow);
	RenderShaftsLobbySide(1-nLiftRow);
	RenderStoreys();
	RenderShaftsLobbySide(nLiftRow);
	RenderLifts(nLiftRow);
	RenderShafts(nLiftRow);
}

