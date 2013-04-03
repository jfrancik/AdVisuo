// Block.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Block.h"
#include "../CommonFiles/Vector.h"

#include <freewill.h>

#pragma warning (disable: 4996)

CBlock::CBlock()
{
	m_pBone = NULL;
	m_pMesh = NULL;

	m_l = m_h = m_d = 0.0f;

	m_x = 0.0f;
}

CBlock::~CBlock(void)
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pBone) m_pBone->Release();
}

void CBlock::Open(ISceneObject *pObject, IKineNode *pBone, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR v, AVFLOAT fRotZ, AVFLOAT fRotX, AVFLOAT fRotY)
{
	Close();

	m_l = l; m_h = h; m_d = d;
	m_x = 0.0f;

	// create a bone
	m_pBone = pBone;
	if (!m_pBone) return;
	m_pBone->AddRef();

	// place & rotate the bone
	ITransform *pT = NULL;
	m_pBone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(fRotZ);
	pT->MulRotationX(fRotX);
	pT->MulRotationY(fRotY);
	pT->MulTranslationXYZ(v.x, v.y, v.z);
	m_pBone->PutLocalTransform(pT);

	// create a new mesh
	LPOLESTR pLabel;
	m_pBone->GetLabel(&pLabel);
	pObject->NewMesh(pLabel, &m_pMesh);

	pT->FromRotationX((AVFLOAT)(M_PI/2));
	m_pMesh->PutTransform(pT);

	m_pMesh->Open(NULL, NULL);
//	m_pMesh->SupportNormal(0);
	pT->Release();
}

void CBlock::Close()
{
	if (!m_pMesh) return;

	bool bReverseNormals = false;
	if (m_l < 0) bReverseNormals = !bReverseNormals;
	if (m_h < 0) bReverseNormals = !bReverseNormals;
	if (m_d < 0) bReverseNormals = !bReverseNormals;

	AVULONG nVertex = 0;
	AVULONG nFace = 0;
	for each (PLANE plane in m_planes)
	{
		if (bReverseNormals)
		{
			if (plane.norm == LT) plane.norm = RT;
			else if (plane.norm == RT) plane.norm = LT;
		}

		AVVECTOR vNormal;
		switch (plane.norm)
		{
			case RT: vNormal = VectorNormalisedCross(plane.vI, plane.vJ); break;
			case LT: vNormal = VectorNormalisedCross(plane.vJ, plane.vI); break;
		}
		AVFLOAT UVI = VectorLen(plane.vI) / plane.nI / 100.0f;	//UV coordinates per 100 units 
		AVFLOAT UVJ = VectorLen(plane.vJ) / plane.nJ / 100.0f;	
		plane.uBase /= 100.0f; plane.vBase /= 100.0f; 

		AVULONG nVertexBase = nVertex;
		for (AVULONG i = 0; i <= plane.nI; i++)
			for (AVULONG j = 0; j <= plane.nJ; j++)
			{
				AVFLOAT fi = (AVFLOAT)i / (AVFLOAT)plane.nI;
				AVFLOAT fj = (AVFLOAT)j / (AVFLOAT)plane.nJ;
				AVFLOAT x = plane.v.x + plane.vI.x * fi + plane.vJ.x * fj;
				AVFLOAT y = plane.v.y + plane.vI.y * fi + plane.vJ.y * fj;
				AVFLOAT z = plane.v.z + plane.vI.z * fi + plane.vJ.z * fj;
				m_pMesh->SetVertexXYZ(nVertex, x, y, z);
				switch (plane.norm)
				{
					case LT: m_pMesh->SetVertexTextureUV(nVertex, 0, plane.uBase + (AVFLOAT)j * UVJ, plane.vBase + (AVFLOAT)i * UVI); break;
					case RT: m_pMesh->SetVertexTextureUV(nVertex, 0, plane.uBase + (AVFLOAT)(plane.nJ - j) * UVJ, plane.vBase + (AVFLOAT)i * UVI); break;
				}
				m_pMesh->SetNormalVector(nVertex, __FW(vNormal)); 
				nVertex++;
			}

		for (AVULONG i = 0; i < plane.nI; i++)
			for (AVULONG j = 0; j < plane.nJ; j++)
			{
				AVULONG n1 = nVertexBase + j + i * (plane.nJ+1);
				AVULONG n2 = n1 + plane.nJ + 1;
				AVULONG n3 = n1 + 1;
				AVULONG n4 = n2 + 1;
				switch (plane.norm)
				{
					case RT: 
						m_pMesh->SetFace(nFace, n1, n3, n2); nFace++;
						m_pMesh->SetFace(nFace, n2, n3, n4); nFace++;
						break;
					case LT: 
						m_pMesh->SetFace(nFace, n1, n2, n3); nFace++;
						m_pMesh->SetFace(nFace, n2, n4, n3); nFace++;
						break;
				}
			}
	}
	m_planes.clear();

	LPOLESTR pLabel;
	m_pBone->GetLabel(&pLabel);
	for (AVULONG i = 0; i < nVertex; i++)
	{
		m_pMesh->SetBoneName(i, 0, pLabel); 
		m_pMesh->SetBoneWeight(i, 0, 1.0f); 
	}

	//// old version (with adv vertex blending)
	//m_pMesh->InitAdvVertexBlending(0.01f, 0);
	//LPOLESTR pLabel;
	//m_pBone->GetLabel(&pLabel);
	//for (AVULONG i = 0; i < nVertex; i++)
	//	m_pMesh->AddBlendWeight(i, 1.0f, pLabel); 

	m_pMesh->Close();

	m_pMesh->Release(); m_pMesh = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;
}

IMesh *CBlock::GetMesh()
{ 
	m_pMesh->AddRef(); 
	return m_pMesh; 
}

IKineNode *CBlock::GetBone()
{ 
	m_pBone->AddRef(); 
	return m_pBone; 
}

void CBlock::SetMaterial(IMaterial *pMaterial)
{
	if (pMaterial) m_pMesh->SetMaterial(pMaterial);
}
	
void CBlock::BuildPlane(PLANE &plane)
{
	m_planes.push_back(plane);
}

void CBlock::BuildPlane(NORMAL_DIR norm, AVVECTOR v, AVULONG nI, AVVECTOR vI, AVULONG nJ, AVVECTOR vJ, AVFLOAT uBase, AVFLOAT vBase)
{
	PLANE plane = { norm, v, nI, vI, nJ, vJ, uBase, vBase };
	BuildPlane(plane);
}

void CBlock::BuildPlane(NORMAL_DIR norm, AVFLOAT vx, AVFLOAT vy, AVFLOAT vz, AVULONG nI, AVFLOAT vix, AVFLOAT viy, AVFLOAT viz, AVULONG nJ, AVFLOAT vjx, AVFLOAT vjy, AVFLOAT vjz, AVFLOAT uBase, AVFLOAT vBase)
{
	AVVECTOR v = { vx, vy, vz }, vI = { vix, viy, viz }, vJ = { vjx, vjy, vjz };
	BuildPlane(norm, v, nI, vI, nJ, vJ, uBase, vBase);
}

const AVULONG nSection = 1;

void CBlock::BuildSimpleBlock()
{
	BuildPlane(RT, 0, 0, 0,		nSection,	0, m_h, 0,	nSection,  m_l, 0, 0);	// side
	BuildPlane(LT, 0, 0, m_d,	nSection,	0, m_h, 0,	nSection,  m_l, 0, 0);	// side

	BuildPlane(RT, 0, 0, 0,		nSection,	m_l, 0, 0,  nSection,  0, 0, m_d);	// bottom
	BuildPlane(LT, 0, m_h, 0,	nSection,	m_l, 0, 0,  nSection,  0, 0, m_d);	// upper

	BuildPlane(LT, 0, 0, 0,		nSection,	0, m_h, 0,  nSection,  0, 0, m_d);	// front section
	BuildPlane(RT, m_l, 0, 0,	nSection,	0, m_h, 0,  nSection,  0, 0, m_d);	// rear section
}

void CBlock::BuildFrontSection(AVFLOAT h0, AVFLOAT h1, AVFLOAT d0, AVFLOAT d1)
{
	if (h1 == 0) h1 = m_h;
	if (d1 == 0) d1 = m_d;
	BuildPlane(RT, m_x, h0, d0,  nSection,  0, 0, d1-d0,  nSection,  0, h1-h0, 0);	// front section
}

void CBlock::BuildRearSection(AVFLOAT h0, AVFLOAT h1, AVFLOAT d0, AVFLOAT d1)
{
	if (h1 == 0) h1 = m_h;
	if (d1 == 0) d1 = m_d;
	BuildPlane(LT, m_x, h0, d0,  nSection,  0, 0, d1-d0,  nSection,  0, h1-h0, 0);	// rear section
}

void CBlock::BuildWall(AVFLOAT l, AVFLOAT h0, AVFLOAT h1, AVFLOAT d0, AVFLOAT d1)
{
	if (l  == 0) l = m_l - m_x;
	if (h1 == 0) h1 = m_h;
	if (d1 == 0) d1 = m_d;
	BuildPlane(RT, m_x, h0, d0,		nSection,  l, 0, 0,			nSection,  0, 0, d1 - d0,	m_x, d0);	// bottom
	BuildPlane(LT, m_x, h1, d0,		nSection,  l, 0, 0,			nSection,  0, 0, d1 - d0,	m_x, d0);	// top
	BuildPlane(RT, m_x, h0, d0,		nSection,  0, h1 - h0, 0,	nSection,  l, 0, 0,			m_x, h0);	// side
	BuildPlane(LT, m_x, h0, d1,		nSection,  0, h1 - h0, 0,	nSection,  l, 0, 0,			m_x, h0);	// side
	m_x += l;
}

void CBlock::BuildWallTo(AVFLOAT l, AVFLOAT h0, AVFLOAT h1, AVFLOAT d0, AVFLOAT d1)
{
	BuildWall(l - m_x, h0, h1, d0, d1);
}

void CBlock::BuildDoor(AVFLOAT w, AVFLOAT h)
{
	BuildRearSection(0, h);
	BuildWall(w, h);
	BuildFrontSection(0, h);
}

void CBlock::BuildWindow(AVFLOAT w, AVFLOAT h0, AVFLOAT h1)
{
	BuildRearSection(h0, h1);
	BuildWall(w, 0, h0);
	ProgressCurX(-w);
	BuildWall(w, h1);
	BuildFrontSection(h0, h1);
}
