// Building.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/ConstrBuilding.h"
#include "Material.h"

using namespace std;

interface IMaterial;
interface IRenderer;
interface IScene;

class CElemVis;

class CBuildingVis : public CBuildingConstr
{
public:
	CMaterialManager m_materials;
	friend class CCamera;

// Attributes (FreeWill related)
private:

	// FreeWill+ elements
	IScene *m_pScene;
	IRenderer *m_pRenderer;
	IMaterial *m_pMaterials[CElem::WALL_LIFT_NUMBER_PLATE * 8 + 2 * 256];

public:
	CBuildingVis(CProject *pProject, AVULONG iIndex);
	~CBuildingVis();

	CElemVis *GetElement()																{ return (CElemVis*)CBuildingConstr::GetElement(); }

	CElemVis *GetStoreyElement(AVULONG nStorey)											{ return (CElemVis*)CBuildingConstr::GetStoreyElement(nStorey); }
	CElemVis *GetMachineRoomElement()													{ return (CElemVis*)CBuildingConstr::GetMachineRoomElement(); }
	CElemVis *GetPitElement()															{ return (CElemVis*)CBuildingConstr::GetPitElement(); }

	CElemVis *GetLiftElement(AVULONG nLift)												{ return (CElemVis*)CBuildingConstr::GetLiftElement(nLift); }
	CElemVis *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemVis*)CBuildingConstr::GetLiftDeck(nLift, nDeck); }
	CElemVis *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemVis*)CBuildingConstr::GetLiftDoor(nLift, nDoor); }

	CElemVis *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemVis*)CBuildingConstr::GetShaftElement(nStorey, nShaft); }
	CElemVis *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemVis*)CBuildingConstr::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemVis *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CBuildingConstr::GetShaftElementLeft(nStorey, nShaft); }
	CElemVis *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CBuildingConstr::GetShaftElementRight(nStorey, nShaft); }
	CElemVis *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemVis*)CBuildingConstr::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemVis *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemVis*)CBuildingConstr::GetShaftDoor(nStorey, nShaft, nDoor); }

	CElemVis *GetMachineElement(AVULONG nShaft)											{ return (CElemVis*)CBuildingConstr::GetMachineElement(nShaft); }

	CElemVis *GetPitElement(AVULONG nShaft)												{ return (CElemVis*)CBuildingConstr::GetPitElement(nShaft); }
	CElemVis *GetPitElementLobbySide(AVULONG nShaft)									{ return (CElemVis*)CBuildingConstr::GetPitElementLobbySide(nShaft); }
	CElemVis *GetPitElementLeft(AVULONG nShaft)											{ return (CElemVis*)CBuildingConstr::GetPitElementLeft(nShaft); }
	CElemVis *GetPitElementRight(AVULONG nShaft)										{ return (CElemVis*)CBuildingConstr::GetPitElementRight(nShaft); }
	CElemVis *GetPitElementLeftOrRight(AVULONG nShaft, AVULONG n)						{ return (CElemVis*)CBuildingConstr::GetPitElementLeftOrRight(nShaft, n); }

// Main Implementation
public:
	AVVECTOR GetLiftPos(int nLift);							// returns position of the lift

	IRenderer *GetRenderer()								{ return m_pRenderer; }
	IScene *GetScene()										{ return m_pScene; }
	
	void SetRenderer(IRenderer *pRenderer);
	void SetScene(IScene *pScene);

	IMaterial *GetMaterial(AVULONG nWallId, AVLONG i = 0);
	void SetMaterial(AVULONG nWallId, AVLONG i, IMaterial *pMaterial);
	void SetMaterial(AVULONG nWallId, AVLONG i, AVFLOAT r, AVFLOAT g, AVFLOAT b, AVFLOAT fAlpha = 1.0f);
	void SetMaterial(AVULONG nWallId, AVLONG i, LPCOLESTR szFileName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha = 1.0f);
	void SetMaterial(AVULONG nWallId, AVLONG i, BYTE* pData, AVULONG nDataSize, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha = 1.0f);
	void DeconstructMaterials();
	
	void SetMaterialLiftPlate(AVULONG nWallId, AVLONG i, AVULONG nLift);
	void SetMaterialFloorPlate(AVULONG nWallId, AVLONG i, AVULONG nFloor);

	void Construct(AVVECTOR vec);

	void StoreConfig();
	void RestoreConfig();
};
