// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"
#include "../CommonFiles/XMLTools.h"
#include <vector>

interface IMesh;
interface IKineNode;
interface ISceneObject;
interface IRenderer;

class CProjectVis;
class CLiftGroupVis;
class CEngine;

using namespace std;

class CElemVis : public CElem
{
	IKineNode *m_pBone;
public:
	CElemVis(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);
	virtual ~CElemVis();

	CProjectVis *GetProject()				{ return (CProjectVis*)CElem::GetProject(); }
	CLiftGroupVis *GetLiftGroup()			{ return (CLiftGroupVis*)CElem::GetLiftGroup(); }
	CElemVis *GetParent()					{ return (CElemVis*)CElem::GetParent(); }
	CEngine *GetEngine();

	// FreeWill+ specific
	IKineNode *GetBone()					{ return m_pBone; }
	ISceneObject *GetObject();

	// Implemenmtation
	virtual void BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot = Vector(0), AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL);
	virtual void BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot = 0, AVULONG nParam = 0, AVFLOAT fParam1 = 0, AVFLOAT fParam2 = 0);

	// implementation specific

	virtual void Move(AVVECTOR vec);
	virtual void MoveTo(AVVECTOR vec);
	AVVECTOR GetPos();
	void PushState();
	void PopState();
	void Invalidate();

	IMesh *AddMesh(AVSTRING strName);
	void Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale = 1.0f, AVFLOAT fTexScale = 1.0f);
	void Render(IRenderer *pRenderer);
};
