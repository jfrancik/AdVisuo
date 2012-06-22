// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"
#include "../CommonFiles/XMLTools.h"

interface IMesh;
interface IKineNode;
interface ISceneObject;
interface IRenderer;

interface IScene;
interface IMaterial;
interface IKineChild;

class CBuildingVis;
class CSimVis;

using namespace std;


class _prj_error
{
public:
	enum ERROR_CODES { 
		E_PRJ_NOT_FOUND = 0x80000100, 
		E_PRJ_NO_BUILDING,
		E_PRJ_PASSENGERS, 
		E_PRJ_LIFTS, 
		E_PRJ_FLOORS, 
		E_PRJ_LIFT_DECKS,
		E_PRJ_FILE_STRUCT,
		E_PRJ_INTERNAL };
	_prj_error(enum ERROR_CODES err_code)	{ _error = err_code; }
	enum ERROR_CODES Error()				{ return _error; }
	std::wstring ErrorMessage();
private:
	enum ERROR_CODES _error;
};

class CBoneVis : public CBone
{
	IKineNode *m_pNode;
public:
	CBoneVis(IKineNode *pNode);
	virtual ~CBoneVis();
	virtual void *GetHandle()			{ return m_pNode; }

	// implementation specific
	IKineNode *GetNode()				{ return m_pNode; }
	void PushState();
	void PopState();
	void Invalidate();
};

class CElemVis : public CElem
{
protected:
	// Implementation
	ISceneObject *m_pObj;

	virtual void onCreate(CElem *pParent, AVULONG nElemId, AVSTRING name, AVVECTOR &vec);
	virtual void onMove(AVVECTOR &vec);
	virtual CBone *onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec);
	virtual void onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone);

public:
	CElemVis(CProject *pProject, CBuilding *pBuilding) : CElem(pProject, (CBuilding*)pBuilding), m_pObj(NULL)	{ }
	virtual ~CElemVis();
	
	CBuildingVis *GetBuilding()				{ return (CBuildingVis*)CElem::GetBuilding(); }
	CBoneVis *GetBone()						{ return (CBoneVis*)CElem::GetBone(); }

	// implementation specific
	IKineNode *GetNode()					{ return GetBone()->GetNode(); }
	IKineNode *GetNode(CBone *p)			{ return ((CBoneVis*)p)->GetNode(); }

	IMesh *AddMesh(AVSTRING strName);

	void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);

	void PushState();
	void PopState();
	void Render(IRenderer *pRenderer);
};

class CProjectVis : public CProjectConstr
{
	// Loading Phase
	enum PHASE { PHASE_NONE, PHASE_PRJ, PHASE_SIM, PHASE_BLD, PHASE_STRUCT, PHASE_SIMDATA };

	std::vector<PHASE> m_phases;

public:
	CProjectVis()							{ }
	virtual ~CProjectVis()					{ }

	virtual CBuilding *CreateBuilding();
	virtual CSim *CreateSim(CBuilding *pBuilding);

	CSimVis *GetSim()						{ return (CSimVis*)CProjectConstr::GetSim(); }
	CBuildingVis *GetBuilding()				{ return (CBuildingVis*)CProjectConstr::GetBuilding(); }
	CSimVis *GetSim(int i)					{ return (CSimVis*)CProjectConstr::GetSim(i); }
	CBuildingVis *GetBuilding(int i)		{ return (CBuildingVis*)CProjectConstr::GetBuilding(i); }
	
	virtual CElem *CreateElement(CBuilding *pBuilding)			{ return new CElemVis(this, pBuilding); }

	// FreeWill specific
	void SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped);
	void SetRenderer(IRenderer *pRenderer);
	void StoreConfig();

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
	void LoadFromBuf(LPCOLESTR pBuf, AVLONG nLiftGroup)						{ Load(pBuf, nLiftGroup); }
	void LoadFromFile(LPCOLESTR pFileName, AVLONG nLiftGroup)				{ Load((std::wstring)pFileName, nLiftGroup); }
	void Load(xmltools::CXmlReader reader, AVLONG nLiftGroup);

	void StoreToFile(LPCOLESTR pFileName)									{ Store((std::wstring)pFileName); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(xmltools::CXmlWriter writer);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, vector<CProjectVis*> &prjs)		{ LoadIndex(pBuf, prjs); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, vector<CProjectVis*> &prjs)	{ LoadIndex((std::wstring)pFileName, prjs); }
	static void LoadIndex(xmltools::CXmlReader reader, vector<CProjectVis*>&);
};
