// AdVisuoRenderer.h - a part of the AdVisuo Client Software

#pragma once

interface IRenderer;
class CBuildingVis;
class CCamera;

class CAdVisuoRenderer
{
	CBuildingVis *m_pBuilding;
	IRenderer *m_pRenderer;
	CCamera *m_pCamera;
public:
	CAdVisuoRenderer(CBuildingVis *pBuilding, IRenderer *pRenderer, CCamera *pCamera = NULL)
		: m_pBuilding(pBuilding), m_pRenderer(pRenderer), m_pCamera(pCamera)	{ }

	void SetBuilding(CBuildingVis *pBuilding)	{ m_pBuilding = pBuilding; }

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

