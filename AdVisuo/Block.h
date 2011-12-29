#pragma once

#include <freewill.h>
#include <vector>

class CBlock
{
	FWSTRING m_pBoneLabel;			// bone label
	IKineNode *m_pBone;				// bone created
	IMesh *m_pMesh;					// mesh created

	FWFLOAT m_l, m_h, m_d;			// length (x); height (y); depth (z)

	FWFLOAT m_x;					// current position along the wall

	enum NORMAL_DIR { LT, RT };
	enum FACE_DIR   { FC_FR, FC_RR, FC_BT, FC_TP, FC_LT, FC_RT, };
	struct PLANE	{ NORMAL_DIR norm; FACE_DIR fd; AVVECTOR v; FWULONG nI; AVVECTOR vI; FWULONG nJ; AVVECTOR vJ; FWFLOAT uBase; FWFLOAT vBase; };

	std::vector<PLANE> m_planes;

public:
	CBlock();
	~CBlock();

	void Open(ISceneObject *pObject, IKineNode *pParentNode, FWSTRING strBoneLabel, FWFLOAT l, FWFLOAT h, FWFLOAT d, AVVECTOR v, FWFLOAT fRotZ = 0, FWFLOAT fRotX = 0, FWFLOAT fRotY = 0);
	void Close();

	IMesh *GetMesh()				{ m_pMesh->AddRef(); return m_pMesh; }
	IKineNode *GetBone()			{ m_pBone->AddRef(); return m_pBone; }

	void SetMaterial(IMaterial *pMaterial);

	FWFLOAT GetCurX()				{ return m_x; }
	void SetCurX(FWFLOAT x)			{ m_x = x; }
	void ProgressCurX(FWFLOAT dx)	{ m_x += dx; }

	void BuildPlane(PLANE&);
	void BuildPlane(NORMAL_DIR norm, FACE_DIR fd, AVVECTOR v, FWULONG nI, AVVECTOR vI, FWULONG nJ, AVVECTOR vJ, FWFLOAT uBase = 0.0f, FWFLOAT vBase = 0.0f);
	void BuildPlane(NORMAL_DIR norm, FACE_DIR fd, FWFLOAT vx, FWFLOAT vy, FWFLOAT vz, FWULONG nI, FWFLOAT vix, FWFLOAT viy, FWFLOAT viz, FWULONG nJ, FWFLOAT vjx, FWFLOAT vjy, FWFLOAT vjz, FWFLOAT uBase = 0.0f, FWFLOAT vBase = 0.0f);

	void BuildSimpleBlock();

	void BuildFrontSection(FWFLOAT h0 = 0, FWFLOAT h1 = 0, FWFLOAT d0 = 0, FWFLOAT d1 = 0);
	void BuildRearSection(FWFLOAT h0 = 0, FWFLOAT h1 = 0, FWFLOAT d0 = 0, FWFLOAT d1 = 0);
	
	void BuildWall(FWFLOAT l = 0, FWFLOAT h0 = 0, FWFLOAT h1 = 0, FWFLOAT d0 = 0, FWFLOAT d1 = 0);
	void BuildWallTo(FWFLOAT l, FWFLOAT h0 = 0, FWFLOAT h1 = 0, FWFLOAT d0 = 0, FWFLOAT d1 = 0);

	void BuildDoor(FWFLOAT w, FWFLOAT h);
	void BuildWindow(FWFLOAT w, FWFLOAT h0, FWFLOAT h1);

};
