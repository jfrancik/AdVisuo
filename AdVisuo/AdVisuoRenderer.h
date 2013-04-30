// AdVisuoRenderer.h - a part of the AdVisuo Client Software

#pragma once

class CLiftGroupVis;
class CCamera;
class CEngine;
class CProjectVis;

class CAdVisuoRenderer
{
	CCamera *m_pCamera;
	CEngine *m_pEngine;
public:
	CAdVisuoRenderer(CEngine *pEngine) : m_pEngine(pEngine)	{ }
	void Render(CProjectVis *pProject, CCamera *pCamera);

private:
	void RenderLifts(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderShafts(CLiftGroupVis *pLiftGroup, AVULONG nRow, AVULONG iStorey);
	void RenderPits(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderMachines(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderShafts(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderShaftsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow, AVULONG iStorey);
	void RenderPitsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderShaftsLobbySide(CLiftGroupVis *pLiftGroup, AVULONG nRow);
	void RenderStoreys(CLiftGroupVis *pLiftGroup);

	void RenderCentre(CLiftGroupVis *pLiftGroup);
	void RenderSide(CLiftGroupVis *pLiftGroup, AVLONG nLiftRow = -1);
	void RenderCentreOuter(CLiftGroupVis *pLiftGroup);
	void RenderSideOuter(CLiftGroupVis *pLiftGroup, AVLONG nLiftRow = -1);
};

