// AdVisuoRenderer.h - a part of the AdVisuo Client Software

#pragma once

interface IRenderer;
class CLftGroupVis;
class CCamera;

class CAdVisuoRenderer
{
	CLftGroupVis *m_pLftGroup;
	IRenderer *m_pRenderer;
	CCamera *m_pCamera;
public:
	CAdVisuoRenderer(CLftGroupVis *pLftGroup, IRenderer *pRenderer, CCamera *pCamera = NULL)
		: m_pLftGroup(pLftGroup), m_pRenderer(pRenderer), m_pCamera(pCamera)	{ }

	void SetLftGroup(CLftGroupVis *pLftGroup)	{ m_pLftGroup = pLftGroup; }

	bool SetupCamera(CCamera *pCamera);

	void RenderLifts(AVULONG nRow);
	void RenderShafts(AVULONG nRow, AVULONG iStorey);
	void RenderPits(AVULONG nRow);
	void RenderMachines(AVULONG nRow);
	void RenderShafts(AVULONG nRow);
	void RenderShaftsLobbySide(AVULONG nRow, AVULONG iStorey);
	void RenderPitsLobbySide(AVULONG nRow);
	void RenderShaftsLobbySide(AVULONG nRow);
	void RenderStoreys();

	void RenderCentre();
	void RenderSide(AVLONG nLiftRow = -1);
	void RenderCentreOuter();
	void RenderSideOuter(AVLONG nLiftRow = -1);
};

