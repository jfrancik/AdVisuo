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

class CProjectVis;
class CBuildingVis;
class CSimVis;

using namespace std;

class CElemVis : public CElem
{
	IKineNode *m_pBone;
public:
	CElemVis(CProject *pProject, CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElemVis();

	CProjectVis *GetProject()				{ return (CProjectVis*)CElem::GetProject(); }
	CBuildingVis *GetBuilding()				{ return (CBuildingVis*)CElem::GetBuilding(); }
	CElemVis *GetParent()					{ return (CElemVis*)CElem::GetParent(); }
	IKineNode *GetBone()					{ return m_pBone; }
	ISceneObject *GetObject();

	// Implemenmtation
	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL);
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0);
	virtual void Move(AVVECTOR vec);

	// implementation specific

	void PushState();
	void PopState();
	void Invalidate();

	IMesh *AddMesh(AVSTRING strName);
	void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);
	void Render(IRenderer *pRenderer);
};

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

class CProjectVis : public CProjectConstr
{
	// Loading Phase
	enum PHASE { PHASE_NONE, PHASE_PRJ, PHASE_SIM, PHASE_BLD, PHASE_STRUCT, PHASE_SIMDATA };

	std::vector<PHASE> m_phases;

public:
	CProjectVis()							{ }
	virtual ~CProjectVis()					{ }

	virtual CBuilding *CreateBuilding(AVULONG iIndex);
	virtual CSim *CreateSim(CBuilding *pBuilding, AVULONG iIndex);

	CSimVis *GetSim()						{ return (CSimVis*)CProjectConstr::GetSim(); }
	CBuildingVis *GetBuilding()				{ return (CBuildingVis*)CProjectConstr::GetBuilding(); }
	CSimVis *GetSim(int i)					{ return (CSimVis*)CProjectConstr::GetSim(i); }
	CBuildingVis *GetBuilding(int i)		{ return (CBuildingVis*)CProjectConstr::GetBuilding(i); }
	
	virtual CElem *CreateElement(CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)			
											{ return new CElemVis(this, pBuilding, pParent, nElemId, name, i, vec); }

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
