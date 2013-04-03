// AdVisuoRenderer.h - a part of the AdVisuo Client Software

#pragma once

class CLiftGroupVis;
class CCamera;
class CEngine;

class CAdVisuoRenderer
{
	CLiftGroupVis *m_pLiftGroup;
	CCamera *m_pCamera;
	CEngine *m_pEngine;
public:
	CAdVisuoRenderer(CEngine *pEngine, CLiftGroupVis *pLiftGroup, CCamera *pCamera = NULL)
		: m_pLiftGroup(pLiftGroup), m_pEngine(pEngine), m_pCamera(pCamera)	{ }

	void SetLiftGroup(CLiftGroupVis *pLiftGroup)	{ m_pLiftGroup = pLiftGroup; }

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

