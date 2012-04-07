// Block.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Block.h"
#include "../CommonFiles/Vector.h"
#include <algorithm>

#pragma warning (disable: 4996)

CBlock::CBlock()
{
	m_pBoneLabel = NULL;
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

void CBlock::Open(ISceneObject *pObject, IKineNode *pParentNode, FWSTRING strBoneLabel, FWFLOAT l, FWFLOAT h, FWFLOAT d, AVVECTOR v, FWFLOAT fRotZ, FWFLOAT fRotX, FWFLOAT fRotY)
{
	Close();

	m_pBoneLabel = strBoneLabel;
	m_l = l; m_h = h; m_d = d;
	m_x = 0.0f;

	// create a bone
	pParentNode->CreateChild(m_pBoneLabel, &m_pBone);

	// place & rotate the bone
	ITransform *pT = NULL;
	m_pBone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(fRotZ);
	pT->MulRotationX(fRotX);
	pT->MulRotationY(fRotY);
	pT->MulTranslationXYZ(v.x, v.y, v.z);
	m_pBone->PutLocalTransform(pT);

	// create a new mesh
	OLECHAR buf[256];
	_snwprintf(buf, 255, L"%s_mesh", strBoneLabel);
	pObject->NewMesh(buf, &m_pMesh);

	pT->FromRotationX((FWFLOAT)(M_PI/2));
	m_pMesh->PutTransform(pT);

	m_pMesh->Open(NULL, NULL);
	m_pMesh->SupportNormal(0);
	pT->Release();
}

void CBlock::Close()
{
	if (!m_pMesh) return;

	bool bReverseNormals = false;
	if (m_l < 0) bReverseNormals = !bReverseNormals;
	if (m_h < 0) bReverseNormals = !bReverseNormals;
	if (m_d < 0) bReverseNormals = !bReverseNormals;

	FWULONG nVertex = 0;
	FWULONG nFace = 0;
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
		FWFLOAT UVI = VectorLen(plane.vI) / plane.nI / 100.0f;	//UV coordinates per 100 units 
		FWFLOAT UVJ = VectorLen(plane.vJ) / plane.nJ / 100.0f;	
		plane.uBase /= 100.0f; plane.vBase /= 100.0f; 

		FWULONG nVertexBase = nVertex;
		for (FWULONG i = 0; i <= plane.nI; i++)
			for (FWULONG j = 0; j <= plane.nJ; j++)
			{
				FWFLOAT fi = (FWFLOAT)i / (FWFLOAT)plane.nI;
				FWFLOAT fj = (FWFLOAT)j / (FWFLOAT)plane.nJ;
				FWFLOAT x = plane.v.x + plane.vI.x * fi + plane.vJ.x * fj;
				FWFLOAT y = plane.v.y + plane.vI.y * fi + plane.vJ.y * fj;
				FWFLOAT z = plane.v.z + plane.vI.z * fi + plane.vJ.z * fj;
				m_pMesh->SetVertexXYZ(nVertex, x, y, z);
				switch (plane.norm)
				{
					case LT: m_pMesh->SetVertexTextureUV(nVertex, 0, plane.uBase + (FWFLOAT)j * UVJ, plane.vBase + (FWFLOAT)i * UVI); break;
					case RT: m_pMesh->SetVertexTextureUV(nVertex, 0, plane.uBase + (FWFLOAT)(plane.nJ - j) * UVJ, plane.vBase + (FWFLOAT)i * UVI); break;
				}
				m_pMesh->AddNormalVector(&nVertex, __FW(vNormal)); 
				nVertex++;
			}

		for (FWULONG i = 0; i < plane.nI; i++)
			for (FWULONG j = 0; j < plane.nJ; j++)
			{
				FWULONG n1 = nVertexBase + j + i * (plane.nJ+1);
				FWULONG n2 = n1 + plane.nJ + 1;
				FWULONG n3 = n1 + 1;
				FWULONG n4 = n2 + 1;
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

	m_pMesh->SupportBlendWeight(0.01f, 0);
	for (FWULONG i = 0; i < nVertex; i++)
		m_pMesh->AddBlendWeight(i, 1.0f, m_pBoneLabel); 

	m_pMesh->Close();

	m_pMesh->Release(); m_pMesh = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;
}

void CBlock::SetMaterial(IMaterial *pMaterial)
{
	if (pMaterial) m_pMesh->SetMaterial(pMaterial);
}
	
void CBlock::BuildPlane(PLANE &plane)
{
	m_planes.push_back(plane);
}

void CBlock::BuildPlane(NORMAL_DIR norm, AVVECTOR v, FWULONG nI, AVVECTOR vI, FWULONG nJ, AVVECTOR vJ, FWFLOAT uBase, FWFLOAT vBase)
{
	PLANE plane = { norm, v, nI, vI, nJ, vJ, uBase, vBase };
	BuildPlane(plane);
}

void CBlock::BuildPlane(NORMAL_DIR norm, FWFLOAT vx, FWFLOAT vy, FWFLOAT vz, FWULONG nI, FWFLOAT vix, FWFLOAT viy, FWFLOAT viz, FWULONG nJ, FWFLOAT vjx, FWFLOAT vjy, FWFLOAT vjz, FWFLOAT uBase, FWFLOAT vBase)
{
	AVVECTOR v = { vx, vy, vz }, vI = { vix, viy, viz }, vJ = { vjx, vjy, vjz };
	BuildPlane(norm, v, nI, vI, nJ, vJ, uBase, vBase);
}

const FWULONG nSection = 1;

void CBlock::BuildSimpleBlock()
{
	BuildPlane(RT, 0, 0, 0,		nSection,	0, m_h, 0,	nSection,  m_l, 0, 0);	// side
	BuildPlane(LT, 0, 0, m_d,	nSection,	0, m_h, 0,	nSection,  m_l, 0, 0);	// side

	BuildPlane(RT, 0, 0, 0,		nSection,	m_l, 0, 0,  nSection,  0, 0, m_d);	// bottom
	BuildPlane(LT, 0, m_h, 0,	nSection,	m_l, 0, 0,  nSection,  0, 0, m_d);	// upper

	BuildPlane(LT, 0, 0, 0,		nSection,	0, m_h, 0,  nSection,  0, 0, m_d);	// front section
	BuildPlane(RT, m_l, 0, 0,	nSection,	0, m_h, 0,  nSection,  0, 0, m_d);	// rear section
}

void CBlock::BuildFrontSection(FWFLOAT h0, FWFLOAT h1, FWFLOAT d0, FWFLOAT d1)
{
	if (h1 == 0) h1 = m_h;
	if (d1 == 0) d1 = m_d;
	BuildPlane(RT, m_x, h0, d0,  nSection,  0, 0, d1-d0,  nSection,  0, h1-h0, 0);	// front section
}

void CBlock::BuildRearSection(FWFLOAT h0, FWFLOAT h1, FWFLOAT d0, FWFLOAT d1)
{
	if (h1 == 0) h1 = m_h;
	if (d1 == 0) d1 = m_d;
	BuildPlane(LT, m_x, h0, d0,  nSection,  0, 0, d1-d0,  nSection,  0, h1-h0, 0);	// rear section
}

void CBlock::BuildWall(FWFLOAT l, FWFLOAT h0, FWFLOAT h1, FWFLOAT d0, FWFLOAT d1)
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

void CBlock::BuildWallTo(FWFLOAT l, FWFLOAT h0, FWFLOAT h1, FWFLOAT d0, FWFLOAT d1)
{
	BuildWall(l - m_x, h0, h1, d0, d1);
}

void CBlock::BuildDoor(FWFLOAT w, FWFLOAT h)
{
	BuildRearSection(0, h);
	BuildWall(w, h);
	BuildFrontSection(0, h);
}

void CBlock::BuildWindow(FWFLOAT w, FWFLOAT h0, FWFLOAT h1)
{
	BuildRearSection(h0, h1);
	BuildWall(w, 0, h0);
	ProgressCurX(-w);
	BuildWall(w, h1);
	BuildFrontSection(h0, h1);
}
