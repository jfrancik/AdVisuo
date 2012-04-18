// Building.h - a part of the AdVisuo Client Software

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include "../CommonFiles/XMLTools.h"
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

#define MAX_DOORS	6
class CBlock;
class CBuilding;

typedef IKineNode *BONE;

class CBldObject
{
	CBuilding *m_pBuilding;
	ISceneObject *m_pObj;
	BONE m_pBone;
public:
	CBldObject(CBuilding *pBuilding) : m_pBuilding(pBuilding), m_pObj(NULL), m_pBone(NULL)	{ }
	CBldObject() : m_pBuilding(NULL), m_pObj(NULL), m_pBone(NULL)	{ }
	CBldObject(CBuilding *pBuilding, ISceneObject *pObj, BONE pBone) : m_pBuilding(pBuilding), m_pObj(pObj), m_pBone(pBone) { }

	CBuilding *GetBuilding()				{ return m_pBuilding; }
	ISceneObject *GetFWObject()				{ return m_pObj; }
	BONE GetBone()							{ return m_pBone; }

	void Create(AVSTRING name);
	void Create(AVSTRING name, AVVECTOR v);
	void Create(AVSTRING name, AVFLOAT x, AVFLOAT y, AVFLOAT z);

	BONE AddBone(AVSTRING name);
	BONE AddBone(AVSTRING name, AVVECTOR v);
	BONE AddBone(AVSTRING name, AVFLOAT x, AVFLOAT y, AVFLOAT z);

	void AddWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, BONE *ppBone = NULL);
	void AddWall(BONE pBone, AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, BONE *ppBone = NULL);
	void AddWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					BOX box, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, BONE *ppBone = NULL);

	IMesh *AddMesh(AVSTRING strName);

	void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);

	void PushState();
	void PopState();
	void Render(IRenderer *pRenderer);

	void Deconstruct();
};

class CBuilding : public CBuildingBase
{
public:
	CBuilding(void);
	~CBuilding(void);

	enum MATERIAL { MAT_FRONT, MAT_REAR, MAT_SIDE, MAT_CEILING, MAT_FLOOR,
		MAT_BEAM, MAT_SHAFT, MAT_OPENING, MAT_DOOR, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, MAT_LIFT_DOOR,
		MAT_LIFT_NUMBER_PLATE, MAT_FLOOR_NUMBER_PLATE = MAT_LIFT_NUMBER_PLATE + 32, MAT_RESERVED_LAST = MAT_LIFT_NUMBER_PLATE + 64,
		// note that MAT_LIFT_NUMBER_PLATE, MAT_FLOOR_NUMBER_PLATE and MAT_LIFT_NUMBER_PLATE are sparsely allocated (32 positions, or 256 texture slots)
	};
	static const AVULONG MAT_TEXTURES_PER_ITEM = 8;	// number of positions reserved for

	CMaterialManager m_materials;


	friend class CCamera;

// Attributes (FreeWill related)
private:

	IScene *m_pScene;

	// Materials
	IRenderer *m_pRenderer;
	IMaterial *m_pMaterials[MAT_RESERVED_LAST * MAT_TEXTURES_PER_ITEM];

	bool bFastLoad;	// press Shift+Esc to stop building most of the building structures

// XML Load / Store --- see xml.cpp for implementation
public:
	// XML-enabled sub-structures
	
	struct LIFT : public CBuildingBase::LIFT
	{
		// lift structure
		CBldObject m_obj;
		BONE m_pDecks[DECK_NUM];
		BONE m_pDoors[MAX_DOORS];

	public:
		LIFT(CBuilding *pBuilding, AVULONG nId) : CBuildingBase::LIFT(pBuilding, nId), m_obj(pBuilding)
		{
			memset(m_pDecks, 0, sizeof(m_pDecks));
			memset(m_pDoors, 0, sizeof(m_pDoors));
		}
		~LIFT()												{ }

		CBuilding *GetBuilding()							{ return (CBuilding *)CBuildingBase::LIFT::GetBuilding(); }

		CBldObject &GetObject()								{ return m_obj; }
		BONE GetDeck(AVULONG nDeck)							{ return m_pDecks[nDeck]; }
		BONE GetDoor(AVULONG nDoor)							{ return m_pDoors[nDoor]; }

		void Construct(AVULONG iShaft);
		void Deconstruct();
	};

	struct SHAFT : public CBuildingBase::SHAFT
	{
		// storey structures
		struct FWSTRUCT
		{
			FWSTRUCT() 	{ memset(pDoors, 0, sizeof(pDoors)); }
			CBldObject m_obj;
			CBldObject m_objLobbySide;
			CBldObject m_objLeft;
			CBldObject m_objRight;
			BONE pDoors[MAX_DOORS];
		} *m_pStoreyBones;

	public:
		SHAFT(CBuilding *pBuilding, AVULONG nId) : CBuildingBase::SHAFT(pBuilding, nId), m_pStoreyBones(NULL)		{ }
		~SHAFT()											{ }

		CBuilding *GetBuilding()							{ return (CBuilding *)CBuildingBase::SHAFT::GetBuilding(); }

		CBldObject &GetObject(AVULONG nStorey)				{ return m_pStoreyBones[nStorey].m_obj; }
		CBldObject &GetObjectLobbySide(AVULONG nStorey)		{ return m_pStoreyBones[nStorey].m_objLobbySide; }
		CBldObject &GetObjectLeft(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_objLeft; }
		CBldObject &GetObjectRight(AVULONG nStorey)			{ return m_pStoreyBones[nStorey].m_objRight; }
		CBldObject &GetObjectLeftOrRight(AVULONG nStorey, AVULONG n)	{ return n == 0 ? GetObjectLeft(nStorey) : GetObjectRight(nStorey); }
		BONE GetDoor(AVULONG nStorey, AVULONG nDoor)		{ return m_pStoreyBones ? m_pStoreyBones[nStorey].pDoors[nDoor] : NULL; }

		void Construct(AVLONG iStorey, AVULONG iShaft);
		void Deconstruct();
	};

	struct STOREY : public CBuildingBase::STOREY
	{
		CBldObject m_obj;

	public:
		STOREY(CBuilding *pBuilding, AVULONG nId) : 	CBuildingBase::STOREY(pBuilding, nId), m_obj(pBuilding)		{ }
		~STOREY()								{ }

		CBuilding *GetBuilding()				{ return (CBuilding *)CBuildingBase::STOREY::GetBuilding(); }

		CBldObject &GetObject()					{ return m_obj; }

		void Construct(AVLONG iStorey);
		void Deconstruct();
	};

// Main Implementation
public:
	virtual STOREY *CreateStorey(AVULONG nId){ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)	{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)	{ return new LIFT(this, nId); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return (STOREY*)CBuildingBase::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	LIFT *GetLift(AVULONG i)				{ return (LIFT*)CBuildingBase::GetLift(i); }

public:
	CBldObject &GetStoreyObject(AVULONG nStorey)				{ return GetStorey(nStorey)->GetObject(); }
	BONE GetStoreyBone(AVULONG nStorey)							{ return GetStoreyObject(nStorey).GetBone(); }
	
	CBldObject &GetLiftObject(AVULONG nLift)					{ return GetLift(nLift)->GetObject(); }
	BONE GetLiftBone(AVULONG nLift)								{ return GetLiftObject(nLift).GetBone(); }
	BONE GetLiftDeck(AVULONG nLift, AVULONG nDeck)				{ return GetLift(nLift)->GetDeck(nDeck); }
	BONE GetLiftDoor(AVULONG nLift, AVULONG nDoor)				{ return GetLift(nLift)->GetDoor(nDoor); }

	CBldObject &GetShaftObject(AVULONG nStorey, AVULONG nShaft)	{ return GetShaft(nShaft)->GetObject(nStorey); }
	CBldObject &GetShaftObjectLobbySide(AVULONG nStorey, AVULONG nShaft){ return GetShaft(nShaft)->GetObjectLobbySide(nStorey); }
	CBldObject &GetShaftObjectLeft(AVULONG nStorey, AVULONG nShaft){ return GetShaft(nShaft)->GetObjectLeft(nStorey); }
	CBldObject &GetShaftObjectRight(AVULONG nStorey, AVULONG nShaft){ return GetShaft(nShaft)->GetObjectRight(nStorey); }
	CBldObject &GetShaftObjectLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n){ return GetShaft(nShaft)->GetObjectLeftOrRight(nStorey, n); }
	BONE GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)		{ return GetShaft(nShaft)->GetDoor(nStorey, nDoor); }

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

	void Construct();
	void Deconstruct();

	void StoreConfig();
	void RestoreConfig();
};
