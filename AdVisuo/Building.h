// Building.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include "../CommonFiles/XMLTools.h"
#include <functional>

interface ISceneObject;
interface IKineNode;
interface IMaterial;
interface ISceneCamera;
interface IRenderer;
interface IScene;

class CBlock;

#define MAX_DOORS	6

class CBuilding : public CBuildingBase
{
public:
	CBuilding(void);
	~CBuilding(void);

	enum MATERIAL { MAT_FRONT, MAT_REAR, MAT_SIDE, MAT_CEILING, MAT_FLOOR,
		MAT_SHAFT1, MAT_SHAFT2, MAT_OPENING, MAT_DOOR, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, 
		MAT_LIFT_NUMBER_PLATE, MAT_FLOOR_NUMBER_PLATE = MAT_LIFT_NUMBER_PLATE + 32, MAT_RESERVED_LAST = MAT_LIFT_NUMBER_PLATE + 64,
		// note that MAT_LIFT_NUMBER_PLATE, MAT_FLOOR_NUMBER_PLATE and MAT_LIFT_NUMBER_PLATE are sparsely allocated (32 positions, or 256 texture slots)
	};
	static const AVULONG MAT_TEXTURES_PER_ITEM = 8;	// number of positions reserved for 


	friend class CCamera;

// Attributes (FreeWill related)
private:

	IScene *m_pScene;

	// Materials
	IRenderer *m_pRenderer;
	IMaterial *m_pMaterials[MAT_RESERVED_LAST * MAT_TEXTURES_PER_ITEM];

// XML Load / Store --- see xml.cpp for implementation
public:
	// XML-enabled sub-structures
	struct SHAFT : public CBuildingBase::SHAFT
	{
		// lift structure
		ISceneObject *m_pObj;
		IKineNode *m_pBone;
		IKineNode *m_ppDecks[DECK_NUM];
		IKineNode *m_ppDoors[MAX_DOORS];

		// storey structures
		struct FWSTRUCT
		{
			ISceneObject *pObj;
			IKineNode *pBone;
			IKineNode *ppDoors[MAX_DOORS];
		} *m_pStoreyBones;

	public:
		SHAFT() : m_pStoreyBones(NULL), m_pObj(NULL), m_pBone(NULL)
		{
			memset(m_ppDecks, 0, sizeof(m_ppDecks));
			memset(m_ppDoors, 0, sizeof(m_ppDoors));
		}

		~SHAFT()											{ }

		CBuilding *GetBuilding()							{ return (CBuilding *)CBuildingBase::SHAFT::GetBuilding(); }

		ISceneObject *GetObj()								{ return m_pObj; }
		IKineNode *GetBone()								{ return m_pBone; }
		IKineNode *GetDeck(AVULONG nDeck)					{ return m_ppDecks[nDeck]; }
		IKineNode *GetDoor(AVULONG nDoor)					{ return m_ppDoors[nDoor]; }

		ISceneObject *GetObj(AVULONG nStorey)				{ return m_pStoreyBones ? m_pStoreyBones[nStorey].pObj : NULL; }
		IKineNode *GetBone(AVULONG nStorey)					{ return m_pStoreyBones ? m_pStoreyBones[nStorey].pBone : NULL; }
		IKineNode *GetDoor(AVULONG nStorey, AVULONG nDoor)	{ return m_pStoreyBones ? m_pStoreyBones[nStorey].ppDoors[nDoor] : NULL; }

		void Construct(AVLONG iStorey, AVULONG iShaft);
		void Construct(AVULONG iShaft);
		void Deconstruct();
	};

	struct STOREY : public CBuildingBase::STOREY
	{
		ISceneObject *m_pObj;
		IKineNode *m_pBone;

	public:
		STOREY() : m_pObj(NULL), m_pBone(NULL)	{ }
		~STOREY()								{ }

		CBuilding *GetBuilding()				{ return (CBuilding *)CBuildingBase::STOREY::GetBuilding(); }

		ISceneObject *GetObj()					{ return m_pObj; }
		IKineNode *GetBone()					{ return m_pBone; }

		void Construct(AVLONG iStorey);
		void Deconstruct();
	};

// Main Implementation
public:
	virtual SHAFT *CreateShaft()			{ return new SHAFT; }
	virtual STOREY *CreateStorey()			{ return new STOREY; }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }


public:
	ISceneObject *GetStoreyObj(AVULONG nStorey)					{ return GetStorey(nStorey)->GetObj(); }
	IKineNode *GetStoreyBone(AVULONG nStorey)					{ return GetStorey(nStorey)->GetBone(); }
	
	ISceneObject *GetLiftObj(AVULONG nShaft)					{ return GetShaft(nShaft)->GetObj(); }
	IKineNode *GetLiftBone(AVULONG nShaft)						{ return GetShaft(nShaft)->GetBone(); }
	IKineNode *GetLiftDeck(AVULONG nShaft, AVULONG nDeck)		{ return GetShaft(nShaft)->GetDeck(nDeck); }
	IKineNode *GetLiftDoor(AVULONG nShaft, AVULONG nDoor)		{ return GetShaft(nShaft)->GetDoor(nDoor); }

	ISceneObject *GetShaftObj(AVULONG nStorey, AVULONG nShaft)	{ return GetShaft(nShaft)->GetObj(nStorey); }
	IKineNode *GetShaftBone(AVULONG nStorey, AVULONG nShaft)	{ return GetShaft(nShaft)->GetBone(nStorey); }
	IKineNode *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)		{ return GetShaft(nShaft)->GetDoor(nStorey, nDoor); }

	AVFLOAT GetLiftZPos(int nLift);							// returns position of the lift above the ground

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

	void Construct(AVSTRING pLabel, AVVECTOR v);
	void Deconstruct();

	void StoreConfig();
	void RestoreConfig();
};
