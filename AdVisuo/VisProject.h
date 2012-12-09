// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"
#include "../CommonFiles/XMLTools.h"
#include <vector>

interface IMesh;
interface IKineNode;
interface ISceneObject;
interface IRenderer;

interface IScene;
interface IMaterial;
interface IKineChild;

class CProjectVis;
class CLftGroupVis;
class CSimVis;

using namespace std;

class CElemVis : public CElem
{
	IKineNode *m_pBone;
public:
	CElemVis(CProject *pProject, CLftGroup *pLftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElemVis();

	CProjectVis *GetProject()				{ return (CProjectVis*)CElem::GetProject(); }
	CLftGroupVis *GetLftGroup()				{ return (CLftGroupVis*)CElem::GetLftGroup(); }
	CElemVis *GetParent()					{ return (CElemVis*)CElem::GetParent(); }
	IKineNode *GetBone()					{ return m_pBone; }
	ISceneObject *GetObject();

	// Implemenmtation
	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL);
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVULONG nParam = 0, AVFLOAT fParam1 = 0, AVFLOAT fParam2 = 0);
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
public:
	CProjectVis()							{ }
	virtual ~CProjectVis()					{ }

	virtual CLftGroup *CreateLftGroup(AVULONG iIndex);

	CLftGroupVis *GetLftGroup(int i)		{ return (CLftGroupVis*)CProjectConstr::GetLftGroup(i); }
	CLftGroupVis *FindLftGroup(int id)		{ return (CLftGroupVis*)CProjectConstr::FindLftGroup(id); }
	CLftGroupVis *AddLftGroup()				{ return (CLftGroupVis*)CProjectConstr::AddLftGroup(); }

	CSimVis *GetSim(int i)					{ return (CSimVis*)CProjectConstr::GetSim(i); }
	CSimVis *FindSim(int id)				{ return (CSimVis*)CProjectConstr::FindSim(id); }
	
	virtual CElem *CreateElement(CLftGroup *pLftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)			
											{ return new CElemVis(this, pLftGroup, pParent, nElemId, name, i, vec); }

	// FreeWill specific
	void SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped);
	void SetRenderer(IRenderer *pRenderer);
	void StoreConfig();

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
	void LoadFromBuf(LPCOLESTR pBuf)										{ Load(pBuf); }
	void LoadFromFile(LPCOLESTR pFileName)									{ Load((std::wstring)pFileName); }
	void Load(xmltools::CXmlReader reader);

	void StoreToFile(LPCOLESTR pFileName)									{ Store((std::wstring)pFileName); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(xmltools::CXmlWriter writer);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, vector<CProjectVis*> &prjs)		{ LoadIndex(pBuf, prjs); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, vector<CProjectVis*> &prjs)	{ LoadIndex((std::wstring)pFileName, prjs); }
	static void LoadIndex(xmltools::CXmlReader reader, vector<CProjectVis*>&);
};
