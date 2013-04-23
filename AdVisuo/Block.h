#pragma once

namespace fw
{
	interface ISceneObject;
	interface IKineNode;
	interface IMesh;
	interface IMaterial;
};

#include <vector>

class CBlock
{
	fw::IKineNode *m_pBone;			// bone
	fw::IMesh *m_pMesh;				// mesh - created on the bone

	AVFLOAT m_l, m_h, m_d;			// length (x); height (y); depth (z)

	AVFLOAT m_x;					// current position along the wall

	enum NORMAL_DIR { LT, RT };
	struct PLANE	{ NORMAL_DIR norm; AVVECTOR v; AVULONG nI; AVVECTOR vI; AVULONG nJ; AVVECTOR vJ; AVFLOAT uBase; AVFLOAT vBase; };

	std::vector<PLANE> m_planes;

public:
	CBlock();
	~CBlock();

	void Open(fw::ISceneObject *pObject, fw::IKineNode *pBone, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR v, AVFLOAT fRotZ = 0, AVFLOAT fRotX = 0, AVFLOAT fRotY = 0);
	void Close();

	fw::IMesh *GetMesh();
	fw::IKineNode *GetBone();

	void SetMaterial(fw::IMaterial *pMaterial);

	AVFLOAT GetCurX()				{ return m_x; }
	void SetCurX(AVFLOAT x)			{ m_x = x; }
	void ProgressCurX(AVFLOAT dx)	{ m_x += dx; }

	void BuildPlane(PLANE&);
	void BuildPlane(NORMAL_DIR norm, AVVECTOR v, AVULONG nI, AVVECTOR vI, AVULONG nJ, AVVECTOR vJ, AVFLOAT uBase = 0.0f, AVFLOAT vBase = 0.0f);
	void BuildPlane(NORMAL_DIR norm, AVFLOAT vx, AVFLOAT vy, AVFLOAT vz, AVULONG nI, AVFLOAT vix, AVFLOAT viy, AVFLOAT viz, AVULONG nJ, AVFLOAT vjx, AVFLOAT vjy, AVFLOAT vjz, AVFLOAT uBase = 0.0f, AVFLOAT vBase = 0.0f);

	void BuildSimpleBlock();

	void BuildFrontSection(AVFLOAT h0 = 0, AVFLOAT h1 = 0, AVFLOAT d0 = 0, AVFLOAT d1 = 0);
	void BuildRearSection(AVFLOAT h0 = 0, AVFLOAT h1 = 0, AVFLOAT d0 = 0, AVFLOAT d1 = 0);
	
	void BuildWall(AVFLOAT l = 0, AVFLOAT h0 = 0, AVFLOAT h1 = 0, AVFLOAT d0 = 0, AVFLOAT d1 = 0);
	void BuildWallTo(AVFLOAT l, AVFLOAT h0 = 0, AVFLOAT h1 = 0, AVFLOAT d0 = 0, AVFLOAT d1 = 0);

	void BuildDoor(AVFLOAT w, AVFLOAT h);
	void BuildWindow(AVFLOAT w, AVFLOAT h0, AVFLOAT h1);

};
