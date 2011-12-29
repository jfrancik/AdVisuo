// Building.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include <xmllite.h>
#include <functional>

interface ISceneObject;
interface IKineNode;
interface IMaterial;
interface ISceneCamera;
interface IRenderer;
interface IScene;

class CBlock;

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

	// Construction Objects & Bones
	ISceneObject **m_pObjLifts;
	IKineNode **m_pBoneLifts;
	IKineNode **m_pBoneLiftDecks[DECK_NUM];
	IKineNode **m_pBoneLiftLDoors;
	IKineNode **m_pBoneLiftRDoors;

	// Materials
	IRenderer *m_pRenderer;
	IMaterial *m_pMaterials[MAT_RESERVED_LAST * MAT_TEXTURES_PER_ITEM];

// XML Load / Store --- see xml.cpp for implementation
public:
	// XML-enabled sub-structures
	struct SHAFT : public CBuildingBase::SHAFT
	{
	public:
		CBuilding *GetBuilding()				{ return (CBuilding *)CBuildingBase::SHAFT::GetBuilding(); }

		// XML Parse/Feed
		void XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName, CBuilding *pBuilding);
		void XFeed(CComPtr<IXmlWriter> pWriter, AVULONG nIdBuilding, LPCWSTR pTagName);
	};
	struct STOREY : public CBuildingBase::STOREY
	{
		IKineNode *m_pBone;
		ISceneObject *m_pObj;
		IKineNode **m_pBoneLDoors;
		IKineNode **m_pBoneRDoors;

	public:
		STOREY();
		~STOREY();

		CBuilding *GetBuilding()				{ return (CBuilding *)CBuildingBase::STOREY::GetBuilding(); }

		IKineNode *GetNode()					{ return m_pBone; }
		ISceneObject *GetObj()					{ return m_pObj; }
		IKineNode *GetLDoor(int nLift)			{ return m_pBoneLDoors ? m_pBoneLDoors[nLift] : NULL; }
		IKineNode *GetRDoor(int nLift)			{ return m_pBoneRDoors ? m_pBoneRDoors[nLift] : NULL; }

		void ConstructWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
							AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
							AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, IKineNode **ppBone = NULL);

		void Construct(AVLONG iStorey);

		void Deconstruct();
	
		// XML Parse/Feed
		void XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName);
		void XFeed(CComPtr<IXmlWriter> pWriter, AVULONG nIdBuilding, LPCWSTR pTagName);
	};

public:
	// XML Parse/Feed
	void XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName);
	void XFeed(CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName);

// Main Implementation
public:
	virtual SHAFT *CreateShaft()			{ return new SHAFT; }
	virtual STOREY *CreateStorey()			{ return new STOREY; }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }


public:
	AVVECTOR GetLiftPos(AVULONG nShaft, AVULONG nStorey)	{ return GetShaft(nShaft)->m_boxCar + Vector(0, 0, GetStorey(nStorey)->SL); }
	
	IKineNode *GetStoreyNode(int nStorey)					{ return GetStorey(nStorey)->m_pBone; }
	ISceneObject *GetStoreyObj(int nStorey)					{ return GetStorey(nStorey)->m_pObj; }

	IKineNode *GetLiftNode(int nLift)						{ return m_pBoneLifts[nLift]; }
	ISceneObject *GetLiftObj(int nLift)						{ return m_pObjLifts[nLift]; }
	IKineNode *GetLiftDeck(int nLift, int n)				{ return m_pBoneLiftDecks[n][nLift]; }

	IKineNode *GetLDoor(int nLift)							{ return m_pBoneLiftLDoors[nLift]; }
	IKineNode *GetRDoor(int nLift)							{ return m_pBoneLiftRDoors[nLift]; }
	IKineNode *GetExtLDoor(int nLift, AVULONG nStorey)		{ return GetStorey(nStorey)->GetLDoor(nLift); }
	IKineNode *GetExtRDoor(int nLift, AVULONG nStorey)		{ return GetStorey(nStorey)->GetRDoor(nLift); }

	AVFLOAT GetLiftZPos(int nLift);							// returns position of the lift above the ground
//	AVULONG GetLiftStorey(int nLift);						// returns the floor the lift is on (based on GetLiftZPos)
//	AVULONG GetLiftDoorPos(int nLift);						// returns the door position (0 = closed, 1 = fully open)

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
