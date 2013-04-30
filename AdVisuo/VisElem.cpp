// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "VisElem.h"
#include "VisProject.h"
#include "VisLiftGroup.h"
#include "Engine.h"
#include "Block.h"

namespace fw
{
	#include <freewill.h>
};
using namespace std;

#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemVis

CElemVis::CElemVis(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) 
	: CElem(pProject, pLiftGroup, pParent, nElemId, name, i, vec)
{ 
	m_pBone = NULL;

	// create name
	if (GetParent() && GetParent()->GetName().size())
		SetName(GetParent()->GetName() + L" - " + GetName());

	if (!GetLiftGroup()) 
		return;

	switch (nElemId)
	{
	case ELEM_PROJECT:
	case ELEM_SITE:
		break;

	case ELEM_BONE:
	case ELEM_DECK:
		m_pBone = GetEngine()->CreateChild(GetParent()->GetBone(), (AVSTRING)GetName().c_str());
		if (m_pBone)
			Move(vec);
		break;

	default:
		m_pBone = (HBONE)GetEngine()->CreateObject((AVSTRING)GetName().c_str());
		if (m_pBone)
			Move(vec);
		break;
	}
}

CElemVis::~CElemVis()
{
	if (m_pBone) ((IUnknown*)m_pBone)->Release(); 
}

CEngine *CElemVis::GetEngine()
{ 
	return GetProject() ? GetProject()->GetEngine() : NULL; 
}

HOBJECT CElemVis::GetObject()
{
	HBONE pNode = m_pBone;
	if (pNode == NULL) return NULL;
	pNode->AddRef();

	HOBJECT pObj = NULL;
	if (pNode) pNode->QueryInterface(&pObj);
	while (!pObj)
	{
		HBONE pParentNode = NULL;
		pNode->GetParent(&pParentNode);
		pNode->Release();
		pNode = pParentNode;
		if (pNode == NULL) return NULL;
		pNode->QueryInterface(&pObj);
	}
	pNode->Release();
	pObj->Release();	// weak pointer!
	return pObj;
}

void CElemVis::BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData)
{
	OLECHAR _name[257];
	_snwprintf_s(_name, 256, strName, nIndex);

	HBONE pNewBone = GetEngine()->CreateChild(m_pBone, _name);

	CBlock block;
	block.Open(GetObject(), pNewBone, box.Width(), box.Height(), box.Depth(), box.LeftFrontLower(), vecRot.z, vecRot.x, vecRot.y);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	block.BuildWall();
	block.BuildRearSection();

	block.SetMaterial(GetProject()->GetEngine()->GetMat(nWallId, LOWORD(nIndex)));
	
	((IUnknown*)pNewBone)->Release();

	block.Close();
}


	BOX __helper(AVVECTOR base, AVFLOAT w, AVFLOAT d, AVFLOAT h)
	{
		return BOX(base + Vector(-w/2, -d/2, 0), w, d, h);
	}

void CElemVis::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVULONG nParam, AVFLOAT fParam1, AVFLOAT fParam2)
{
	if (!m_pBone) return;
	AVFLOAT fScale = GetLiftGroup()->GetScale();
	float f;
	AVVECTOR centre = box.CentreLower();

	switch (nModelId)
	{
	case MODEL_MACHINE:
		break;
		box.SetDepth(-box.Depth());
		box.Grow(0.9f, 0.9f, 0.9f);
		//BuildWall(WALL_BEAM, strName, nIndex, box);
		{
		OLECHAR _name[257];
		_snwprintf_s(_name, 256, L"Machine %d", nIndex);

		HBONE pNewBone = NULL;
		m_pBone->CreateChild(_name, &pNewBone);




		LPOLESTR pLabel;
		pNewBone->GetLabel(&pLabel);
		Load((LPOLESTR)(_stdPathModels + L"bunny.obj").c_str(), pLabel, 700, 100);
		
		fw::ITransform *pT = NULL;
		pNewBone->CreateCompatibleTransform(&pT);
		pT->FromRotationX(M_PI/2);
		AVVECTOR vec = box.CentreLower();
		pT->MulTranslationVector((fw::FWVECTOR*)(&vec));
		pNewBone->PutLocalTransform(pT);
		pT->Release();
		pNewBone->Release();

	//IMaterial *pMaterial = NULL;
	//ITexture *pTexture = NULL;
	//GetEngine()->GetRenderer()->CreateTexture(&pTexture);
	//pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"dafoldil.jpg").c_str());
	//pTexture->SetUVTile(1, 1);
	//GetEngine()->GetRenderer()->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	//pMaterial->SetTexture(0, pTexture);
	//IKineChild *pChild = NULL;
	//m_pBone->GetChild(L"main", &pChild);
	//HMESH pMesh = NULL;
	//pChild->QueryInterface(&pMesh);
	//pMesh->SetMaterial(pMaterial);
	//pMesh->Release();
	//pChild->Release();
	//pMaterial->Release();
	//pTexture->Release();
		}


		break;
	case MODEL_OVERSPEED:
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 7, f*30, 30));
		break;




		//f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		//box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		//box.Move(0, f*60, 0);
		//centre = box.CentreLower();
		//BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 7, f*30, 30));
		//break;

	case MODEL_CONTROL_PANEL:
//		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
//		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
//		box.Move(0, f*60, 0);
//		centre = box.CentreLower();
//		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 7, f*30, 30));
		break;

	case MODEL_DRIVE_PANEL:
	case MODEL_GROUP_PANEL:
	case MODEL_ISOLATOR:
		break;

	//case MODEL_CONTROL:
	//	i = GetLiftGroup()->GetShaft(nIndex)->GetShaftLine();	// which shaft line we are
	//	centre.x += p->Width() * (nIndex - GetLiftGroup()->GetShaftBegin(i) - (AVFLOAT)(GetLiftGroup()->GetShaftCount(i) - 1) / 2.0f);
	//	if (i == 0)
	//		centre.y -= p->Depth()/2;
	//	else
	//		centre.y += p->Depth()/2;
	//	p->build(this, nModelId, strName, nIndex, centre, fRot);
	//	break;
	//case MODEL_ISOLATOR:
	//	p->build(this, nModelId, strName, nIndex, centre, fRot);
	//	break;
	case MODEL_CWT:
		break;
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth());
		box.Move(0, -f*2, 0);
		BuildWall(WALL_BEAM, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_RAIL:
		break;
		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CAR:
		break;
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CWT:
		break;
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_PULLEY:
		break;
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		box.Move(0, f*60, 0);
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 6, f*20, 30));
		break;
	case MODEL_LADDER:
		break;
	//	box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
	//	centre = box.CentreLower();
	//	p->build(this, nModelId, strName, nIndex, centre, 0.8, 0.8, 1, fRot);
	//	break;
	case MODEL_LIGHT:
		break;
	case MODEL_JAMB:
	case MODEL_JAMB_CAR:
		break;
		//box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_HEADING:
	case MODEL_HEADING_CAR:
		break;
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_SILL:
	case MODEL_SILL_CAR:
		break;
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_HANDRAIL:
		break;
	}
}

HMESH CElemVis::AddMesh(AVSTRING strName)
{
	HMESH pMesh = NULL;
	GetObject()->NewMesh(strName, &pMesh);
	pMesh->Open(NULL, NULL);
	pMesh->InitAdvNormalSupport(0);
	return pMesh;
}

void CElemVis::Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale, AVFLOAT fTexScale)
{
	HMESH pVMesh = NULL;
	HMESH pFMesh = NULL;
	wstring name;
	
	wifstream verfile(strFilename);
	wstring line;
	AVULONG nVertexBase = 1;
	AVULONG nVertex = 0;
	AVULONG nFace = 0;
	while (getline(verfile, line))
	{
		auto i = line.find(L" ", 0);
		if (i == wstring::npos) continue;

		if (line.substr(0, i) == L"g")
		{
			line.replace(0, i, L"");
			wistringstream iss(line);
			if (!(iss >> name))
				break;
		}
		else
		if (line.substr(0, i) == L"v")
		{
			line.replace(0, i, L"");
			wistringstream iss(line);
			AVFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh)
			{
				pFMesh->InitAdvVertexBlending(0.01f, 0);
				for (AVULONG i = 0; i < nVertex; i++)
					pFMesh->AddBlendWeight(i, 1.0f, strBone);
				pFMesh->Close();
				pFMesh = NULL;
				nVertexBase += nVertex;
				nVertex = 0;
			}
			if (pVMesh == NULL)
				pVMesh = AddMesh(L"temporary");

			pVMesh->SetVertexXYZ(nVertex, a * fScale, b * fScale, c * fScale);
			double l = sqrt(a * a + b * b + c * c);
			pVMesh->AddNormal(&nVertex, a/l, b/l, c/l);
			pVMesh->SetVertexTextureUV(nVertex, 0, sqrt(a*a+c*c) * fTexScale, b * fTexScale);
			nVertex++;
		}
		else
		if (line.substr(0, i) == L"f")
		{
			line[0] = ' ';
			wistringstream iss(line);
			AVFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh == NULL)
			{
				if (pVMesh == NULL) continue;
				pFMesh = pVMesh;
				pVMesh = NULL;
				fw::IKineChild *pChild = NULL;
				pFMesh->QueryInterface(&pChild);
				pChild->PutLabel((AVSTRING)name.c_str());
				pChild->Release();
			}

			a -= nVertexBase;
			b -= nVertexBase;
			c -= nVertexBase;
			pFMesh->SetFace(nFace, a, b, c);
			nFace++;
		}
	}
	
	if (pFMesh)
	{
		pFMesh->InitAdvVertexBlending(0.01f, 0);
		for (AVULONG i = 0; i < nVertex; i++)
			pFMesh->AddBlendWeight(i, 1.0f, strBone);
		pFMesh->Close();
	}
}

void CElemVis::Move(AVVECTOR vec)
{
	CEngine::MoveBone(m_pBone, vec.x, vec.y, vec.z);
}

void CElemVis::MoveTo(AVVECTOR vec)
{
	CEngine::MoveBoneTo(m_pBone, vec.x, vec.y, vec.z);
}

AVVECTOR CElemVis::GetPos()
{
	return CEngine::GetBonePos(GetBone());
}

void CElemVis::PushState()
{
	return CEngine::PushState((HBONE)GetObject());
}

void CElemVis::PopState()
{
	return CEngine::PopState((HBONE)GetObject());
}

void CElemVis::Invalidate()
{
	return CEngine::Invalidate((HBONE)GetObject());
}


