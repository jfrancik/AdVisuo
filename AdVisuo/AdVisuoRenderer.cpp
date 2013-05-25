// AdVisuoRenderer.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuoRenderer.h"

#include "Camera.h"
#include "VisSim.h"
#include "VisProject.h"
#include "Engine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////////////
// CAdVisuo Renderer

void CAdVisuoRenderer::RenderLifts(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		for (AVULONG j = pLiftGroup->GetShaft(i)->GetLiftBegin(); j < pLiftGroup->GetShaft(i)->GetLiftEnd(); j++)
			m_pEngine->Render(pLiftGroup->GetLiftElement(j)->GetObject());
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		for (AVULONG j = pLiftGroup->GetShaft(i)->GetLiftBegin(); j < pLiftGroup->GetShaft(i)->GetLiftEnd(); j++)
			m_pEngine->Render(pLiftGroup->GetLiftElement(j)->GetObject());
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
		for (AVULONG j = pLiftGroup->GetShaft(iShaft)->GetLiftBegin(); j < pLiftGroup->GetShaft(iShaft)->GetLiftEnd(); j++)
			m_pEngine->Render(pLiftGroup->GetLiftElement(j)->GetObject());
}

// Render Shafts
void CAdVisuoRenderer::RenderShafts(CLiftGroupVis *pLiftGroup, AVULONG nRow, AVULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pEngine->Render(pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, nRow)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElement(iStorey, i)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->GetObject());
	}
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pEngine->Render(pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, 1-nRow)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElement(iStorey, i)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElementLeftOrRight(iStorey, i, nRow)->GetObject());
	}
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
	{
		m_pEngine->Render(pLiftGroup->GetShaftElementLeft(iStorey, iShaft)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElement(iStorey, iShaft)->GetObject());
		m_pEngine->Render(pLiftGroup->GetShaftElementRight(iStorey, iShaft)->GetObject());
	}
}
	
void CAdVisuoRenderer::RenderPits(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
	{
		m_pEngine->Render(pLiftGroup->GetPitElementLeftOrRight(i, nRow)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElement(i)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElementLeftOrRight(i, 1-nRow)->GetObject());
	}
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
	{
		m_pEngine->Render(pLiftGroup->GetPitElementLeftOrRight(i, 1-nRow)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElement(i)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElementLeftOrRight(i, nRow)->GetObject());
	}
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
	{
		m_pEngine->Render(pLiftGroup->GetPitElementLeft(iShaft)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElement(iShaft)->GetObject());
		m_pEngine->Render(pLiftGroup->GetPitElementRight(iShaft)->GetObject());
	}
}

void CAdVisuoRenderer::RenderMachines(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow);
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(pLiftGroup->GetMachineElement(i)->GetObject());
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(pLiftGroup->GetMachineElement(i)->GetObject());
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(pLiftGroup->GetMachineElement(iShaft)->GetObject());
}

void CAdVisuoRenderer::RenderShafts(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)pLiftGroup->GetStoreyCount() - 1));
	RenderPits(pLiftGroup, nRow);
	for (AVLONG i = 0; i < iStorey; i++)
		RenderShafts(pLiftGroup, nRow, i);
	RenderMachines(pLiftGroup, nRow);
	for (AVLONG i = pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShafts(pLiftGroup, nRow, i);
	RenderShafts(pLiftGroup, nRow, iStorey);
}

// Render Shafts Lobby Side
void CAdVisuoRenderer::RenderShaftsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow, AVULONG iStorey)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(pLiftGroup->GetShaftElementLobbySide(iStorey, i)->GetObject());
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(pLiftGroup->GetShaftElementLobbySide(iStorey, i)->GetObject());
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(pLiftGroup->GetShaftElementLobbySide(iStorey, iShaft)->GetObject());
}
	
void CAdVisuoRenderer::RenderPitsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iShaft = m_pCamera->GetShaftPos(nRow); 
	iShaft = max((AVLONG)pLiftGroup->GetShaftBegin(nRow) - 1, min(iShaft, (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow)));
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow); i < iShaft; i++)
		m_pEngine->Render(pLiftGroup->GetPitElementLobbySide(i)->GetObject());
	for (AVLONG i = pLiftGroup->GetShaftBegin(nRow) + pLiftGroup->GetShaftCount(nRow) - 1; i > iShaft; i--)
		m_pEngine->Render(pLiftGroup->GetPitElementLobbySide(i)->GetObject());
	if (iShaft >= (AVLONG)pLiftGroup->GetShaftBegin(nRow) && iShaft < (AVLONG)pLiftGroup->GetShaftBegin(nRow) + (AVLONG)pLiftGroup->GetShaftCount(nRow))
		m_pEngine->Render(pLiftGroup->GetPitElementLobbySide(iShaft)->GetObject());
}
	
void CAdVisuoRenderer::RenderShaftsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)pLiftGroup->GetStoreyCount() - 1));
	RenderPitsLobbySide(pLiftGroup, nRow);
	for (AVLONG i = 0; i < iStorey; i++)
		RenderShaftsLobbySide(pLiftGroup, nRow, i);
	for (AVLONG i = pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		RenderShaftsLobbySide(pLiftGroup, nRow, i);
	RenderShaftsLobbySide(pLiftGroup, nRow, iStorey);
}

// Render Storeys (Lobbies)
void CAdVisuoRenderer::RenderStoreys(CLiftGroupVis *pLiftGroup)
{
	AVLONG iStorey = m_pCamera->GetStorey();
	iStorey = max(0, min(iStorey, (AVLONG)pLiftGroup->GetStoreyCount() - 1));
	if (pLiftGroup->GetPit())
		m_pEngine->Render(pLiftGroup->GetPitElement()->GetObject());
	for (AVLONG i = 0; i < iStorey; i++)
		m_pEngine->Render(pLiftGroup->GetStoreyElement(i)->GetObject());
	if (pLiftGroup->GetMR())
		m_pEngine->Render(pLiftGroup->GetMRElement()->GetObject());
	for (AVLONG i = pLiftGroup->GetStoreyCount() - 1; i > iStorey; i--)
		m_pEngine->Render(pLiftGroup->GetStoreyElement(i)->GetObject());
	m_pEngine->Render(pLiftGroup->GetStoreyElement(iStorey)->GetObject());
}

void CAdVisuoRenderer::RenderCentre(CLiftGroupVis *pLiftGroup)
{
	RenderShafts(pLiftGroup, 0);
	RenderLifts(pLiftGroup, 0);
	RenderShaftsLobbySide(pLiftGroup, 0);
	RenderShafts(pLiftGroup, 1);
	RenderLifts(pLiftGroup, 1);
	RenderShaftsLobbySide(pLiftGroup, 1);
	RenderStoreys(pLiftGroup);
}

void CAdVisuoRenderer::RenderSide(CLiftGroupVis *pLiftGroup, AVLONG nLiftRow)
{
	if (nLiftRow < 0)
		nLiftRow = pLiftGroup->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
	RenderShafts(pLiftGroup, 1-nLiftRow);
	RenderLifts(pLiftGroup, 1-nLiftRow);
	RenderShaftsLobbySide(pLiftGroup, 1-nLiftRow);
	RenderStoreys(pLiftGroup);
	RenderShafts(pLiftGroup, nLiftRow);
	RenderShaftsLobbySide(pLiftGroup, nLiftRow);
	RenderLifts(pLiftGroup, nLiftRow);
}

void CAdVisuoRenderer::RenderCentreOuter(CLiftGroupVis *pLiftGroup)
{
	RenderLifts(pLiftGroup, 0);
	RenderShafts(pLiftGroup, 0);
	RenderShaftsLobbySide(pLiftGroup, 0);
	RenderLifts(pLiftGroup, 1);
	RenderShafts(pLiftGroup, 1);
	RenderShaftsLobbySide(pLiftGroup, 1);
	RenderStoreys(pLiftGroup);
}

void CAdVisuoRenderer::RenderSideOuter(CLiftGroupVis *pLiftGroup, AVLONG nLiftRow)
{
	if (nLiftRow < 0)
		nLiftRow = pLiftGroup->GetShaft(m_pCamera->GetShaft())->GetShaftLine();
	RenderLifts(pLiftGroup, 1-nLiftRow);
	RenderShafts(pLiftGroup, 1-nLiftRow);
	RenderShaftsLobbySide(pLiftGroup, 1-nLiftRow);
	RenderStoreys(pLiftGroup);
	RenderShaftsLobbySide(pLiftGroup, nLiftRow);
	RenderLifts(pLiftGroup, nLiftRow);
	RenderShafts(pLiftGroup, nLiftRow);
}


void CAdVisuoRenderer::Render(CProjectVis *pProject, CCamera *pCamera)
{
	if (!m_pEngine || !pProject || !pCamera || !pCamera->GetCamera()) return;

	m_pCamera = pCamera;
	m_pCamera->CheckLocation();
	m_pEngine->PutCamera(pCamera->GetCamera());
	m_pEngine->Render(pCamera->GetCamera());
	
	AVULONG nLiftGroup = pCamera->GetLiftGroup();

	m_pEngine->RenderLights();

	// my own display list goes here... instead of m_pScene->Render(pRenderer);
	for (AVULONG i = 0; i < pProject->GetLiftGroupsCount(); i++)
		pProject->GetLiftGroup(i)->GetSim()->RenderPassengers(0);

	// for multiple lift groups
	for (AVULONG i = 0; i < pProject->GetLiftGroupsCount(); i++)
		if (i != nLiftGroup)
		{
			RenderSideOuter(pProject->GetLiftGroup(i), i < nLiftGroup ? 0 : 1);
	}

	switch (pCamera->GetLoc())
	{
	case CAMLOC_LOBBY:
	case CAMLOC_OVERHEAD:
		RenderCentre(pProject->GetLiftGroup(nLiftGroup));
		break;
	case CAMLOC_LIFT:
	case CAMLOC_SHAFT: 
		RenderSide(pProject->GetLiftGroup(nLiftGroup));
		break;
	default:
		switch (pCamera->GetYZone())
		{
		case -1:
			RenderSideOuter(pProject->GetLiftGroup(nLiftGroup), 0);
			break;
		case 0:
		case 1:
		case 2:
			RenderCentreOuter(pProject->GetLiftGroup(nLiftGroup));
			break;
		case 3:
			RenderSideOuter(pProject->GetLiftGroup(nLiftGroup), 1);
			break;
		}
		break;
	}

	for (AVULONG i = 0; i < pProject->GetLiftGroupsCount(); i++)
		pProject->GetLiftGroup(i)->GetSim()->RenderPassengers(1);
}

