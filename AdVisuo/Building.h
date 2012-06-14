// Building.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include "../CommonFiles/XMLTools.h"
#include "ConstrBuilding.h"
#include "Material.h"
#include <functional>

using namespace std;

interface ISceneObject;
interface IKineNode;
interface IMaterial;
interface ISceneCamera;
interface IRenderer;
interface IScene;
interface IMesh;

class CBlock;
class CBuilding;

class CBone : public CBoneBase
{
	IKineNode *m_pNode;
public:
	CBone(IKineNode *pNode);
	virtual ~CBone();
	virtual void *GetHandle()			{ return m_pNode; }

	// implementation specific
	IKineNode *GetNode()				{ return m_pNode; }
	void PushState();
	void PopState();
	void Invalidate();
};

class CElement : public CElemBase
{
protected:
	// Implementation
	ISceneObject *m_pObj;

	virtual void onCreate(AVSTRING name, AVVECTOR &vec);
	virtual void onMove(AVVECTOR &vec);
	virtual CBoneBase *onAddBone(AVSTRING name, AVVECTOR &vec);
	virtual void onAddWall(CBoneBase *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBoneBase **ppNewBone);

public:
	CElement(CBuildingBase *pBuilding) : CElemBase((CBuildingBase*)pBuilding), m_pObj(NULL)	{ }
	virtual ~CElement();
	
	CBuilding *GetBuilding()				{ return (CBuilding*)CElemBase::GetBuilding(); }
	CBone *GetBone()						{ return (CBone*)CElemBase::GetBone(); }

	// implementation specific
	IKineNode *GetNode()					{ return GetBone()->GetNode(); }
	IKineNode *GetNode(CBoneBase *p)		{ return ((CBone*)p)->GetNode(); }

	IMesh *AddMesh(AVSTRING strName);

	void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);

	void PushState();
	void PopState();
	void Render(IRenderer *pRenderer);
};

class CBuilding : public CBuildingConstr
{
public:
	CBuilding(void);
	~CBuilding(void);

	CMaterialManager m_materials;
	static const AVULONG MAT_TEXTURES_PER_ITEM = 8;	// number of positions reserved for

	friend class CCamera;

// Attributes (FreeWill related)
private:

	IScene *m_pScene;

	// Materials
	IRenderer *m_pRenderer;
	IMaterial *m_pMaterials[MAT_RESERVED_LAST * MAT_TEXTURES_PER_ITEM];

	bool bFastLoad;	// press Shift+Esc to stop building most of the building structures

	virtual CElemBase *CreateElement()		{ return new CElement(this); }

public:
	CElement *GetStoreyElement(AVULONG nStorey)											{ return (CElement*)CBuildingConstr::GetStoreyElement(nStorey);}
	CBone *GetStoreyBone(AVULONG nStorey)												{ return (CBone*)GetStoreyElement(nStorey)->GetBone(); }
	
	CElement *GetLiftElement(AVULONG nLift)												{ return (CElement*)CBuildingConstr::GetLiftElement(nLift); }
	CBone *GetLiftBone(AVULONG nLift)													{ return (CBone*)CBuildingConstr::GetLiftBone(nLift); }
	CBone *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CBone*)CBuildingConstr::GetLiftDeck(nLift, nDeck); }
	CBone *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CBone*)CBuildingConstr::GetLiftDoor(nLift, nDoor); }

	CElement *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElement*)CBuildingConstr::GetShaftElement(nStorey, nShaft); }
	CElement *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElement*)CBuildingConstr::GetShaftElementLobbySide(nStorey, nShaft); }
	CElement *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElement*)CBuildingConstr::GetShaftElementLeft(nStorey, nShaft); }
	CElement *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElement*)CBuildingConstr::GetShaftElementRight(nStorey, nShaft); }
	CElement *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElement*)CBuildingConstr::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CBone *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)					{ return (CBone*)CBuildingConstr::GetShaftDoor(nStorey, nShaft, nDoor); }

// Main Implementation
public:
	AVVECTOR GetLiftPos(int nLift);							// returns position of the lift

	IRenderer *GetRenderer()								{ return m_pRenderer; }
	IScene *GetScene()										{ return m_pScene; }
	
	void SetRenderer(IRenderer *pRenderer);
	void SetScene(IScene *pScene);

	IMaterial *GetMaterial(AVULONG nItemId, AVLONG i = 0, AVULONG nMaxI = 8);
	void SetMaterial(AVULONG nItemId, AVULONG i, IMaterial *pMaterial);
	void SetMaterial(AVULONG nItemId, AVULONG i, AVFLOAT r, AVFLOAT g, AVFLOAT b, AVFLOAT fAlpha = 1.0f);
	void SetMaterial(AVULONG nItemId, AVULONG i, LPCOLESTR szFileName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha = 1.0f);
	void SetMaterial(AVULONG nItemId, AVULONG i, BYTE* pData, AVULONG nDataSize, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha = 1.0f);
	void DeconstructMaterials();
	
	void SetMaterialLiftPlate(AVULONG nItemId, AVULONG i, AVULONG nLift);
	void SetMaterialFloorPlate(AVULONG nItemId, AVULONG i, AVULONG nFloor);

	void Construct(AVVECTOR vec);

	void StoreConfig();
	void RestoreConfig();
};
