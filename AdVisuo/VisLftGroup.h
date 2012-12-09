// VisLftGroup.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/ConstrLftGroup.h"
#include "../CommonFiles/ConstrProject.h"
#include "Material.h"

using namespace std;

interface IMaterial;
interface IRenderer;
interface IScene;

class CElemVis;
class CSimVis;

class CLftGroupVis : public CLftGroupConstr
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
	CLftGroupVis(CProject *pProject, AVULONG iIndex);
	~CLftGroupVis();

protected:
	virtual CSim *CreateSim();

public:
	CSimVis *AddSim()																	{ return (CSimVis*)CLftGroupConstr::AddSim(); }
	CSimVis *GetSim()																	{ return (CSimVis*)CLftGroupConstr::GetSim(); }

	CElemVis *GetElement()																{ return (CElemVis*)CLftGroupConstr::GetElement(); }

	CElemVis *GetStoreyElement(AVULONG nStorey)											{ return (CElemVis*)CLftGroupConstr::GetStoreyElement(nStorey); }
	CElemVis *GetMachineRoomElement()													{ return (CElemVis*)CLftGroupConstr::GetMachineRoomElement(); }
	CElemVis *GetPitElement()															{ return (CElemVis*)CLftGroupConstr::GetPitElement(); }

	CElemVis *GetLiftElement(AVULONG nLift)												{ return (CElemVis*)CLftGroupConstr::GetLiftElement(nLift); }
	CElemVis *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemVis*)CLftGroupConstr::GetLiftDeck(nLift, nDeck); }
	CElemVis *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemVis*)CLftGroupConstr::GetLiftDoor(nLift, nDoor); }

	CElemVis *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemVis*)CLftGroupConstr::GetShaftElement(nStorey, nShaft); }
	CElemVis *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemVis*)CLftGroupConstr::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemVis *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CLftGroupConstr::GetShaftElementLeft(nStorey, nShaft); }
	CElemVis *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemVis*)CLftGroupConstr::GetShaftElementRight(nStorey, nShaft); }
	CElemVis *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemVis*)CLftGroupConstr::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemVis *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemVis*)CLftGroupConstr::GetShaftDoor(nStorey, nShaft, nDoor); }

	CElemVis *GetMachineElement(AVULONG nShaft)											{ return (CElemVis*)CLftGroupConstr::GetMachineElement(nShaft); }

	CElemVis *GetPitElement(AVULONG nShaft)												{ return (CElemVis*)CLftGroupConstr::GetPitElement(nShaft); }
	CElemVis *GetPitElementLobbySide(AVULONG nShaft)									{ return (CElemVis*)CLftGroupConstr::GetPitElementLobbySide(nShaft); }
	CElemVis *GetPitElementLeft(AVULONG nShaft)											{ return (CElemVis*)CLftGroupConstr::GetPitElementLeft(nShaft); }
	CElemVis *GetPitElementRight(AVULONG nShaft)										{ return (CElemVis*)CLftGroupConstr::GetPitElementRight(nShaft); }
	CElemVis *GetPitElementLeftOrRight(AVULONG nShaft, AVULONG n)						{ return (CElemVis*)CLftGroupConstr::GetPitElementLeftOrRight(nShaft, n); }

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
