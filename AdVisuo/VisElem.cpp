// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "VisElem.h"
#include "VisProject.h"
#include "VisLiftGroup.h"
#include "Engine.h"
#include "Block.h"

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
		GetParent()->GetBone()->CreateChild((AVSTRING)GetName().c_str(), &m_pBone);
		if (m_pBone)
			Move(vec);
		break;

	default:
		if (GetEngine() && GetEngine()->GetScene())
			GetEngine()->GetScene()->NewObject((AVSTRING)GetName().c_str(), (ISceneObject**)&m_pBone);
		if (m_pBone)
			Move(vec);
		break;
	}
}

CElemVis::~CElemVis()
{
	if (m_pBone) m_pBone->Release(); 
}

CEngine *CElemVis::GetEngine()
{ 
	return GetProject() ? GetProject()->GetEngine() : NULL; 
}

ISceneObject *CElemVis::GetObject()
{
	IKineNode *pNode = m_pBone;
	if (pNode == NULL) return NULL;
	pNode->AddRef();

	ISceneObject *pObj = NULL;
	if (pNode) pNode->QueryInterface(&pObj);
	while (!pObj)
	{
		IKineNode *pParentNode = NULL;
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

	IKineNode *pNewBone = NULL;
	m_pBone->CreateChild(_name, &pNewBone);

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

	block.SetMaterial(GetLiftGroup()->GetMaterial(nWallId, LOWORD(nIndex)));
	
	pNewBone->Release();

	block.Close();
}


	BOX __helper(AVVECTOR base, AVFLOAT w, AVFLOAT d, AVFLOAT h)
	{
		return BOX(base + Vector(-w/2, -d/2, 0), w, d, h);
	}

void CElemVis::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVULONG nParam, AVFLOAT fParam1, AVFLOAT fParam2)
{
	return;
//	BuildWall(WALL_BEAM, strName, nIndex, box, Vector(0, 0, fRot));
	
	if (!m_pBone) return;
	AVFLOAT fScale = GetLiftGroup()->GetScale();
	float f;
	AVVECTOR centre = box.CentreLower();

	switch (nModelId)
	{
	case MODEL_MACHINE:
		box.SetDepth(-box.Depth());
		box.Grow(0.9f, 0.9f, 0.9f);
		//BuildWall(WALL_BEAM, strName, nIndex, box);
		{
		OLECHAR _name[257];
		_snwprintf_s(_name, 256, L"Machine %d", nIndex);

		IKineNode *pNewBone = NULL;
		m_pBone->CreateChild(_name, &pNewBone);




		LPOLESTR pLabel;
		pNewBone->GetLabel(&pLabel);
		Load((LPOLESTR)(LPCOLESTR)(_stdPathModels + "bunny.obj"), pLabel, 700, 100);
		
		ITransform *pT = NULL;
		pNewBone->CreateCompatibleTransform(&pT);
		pT->FromRotationX(M_PI/2);
		AVVECTOR vec = box.CentreLower();
		pT->MulTranslationVector((FWVECTOR*)(&vec));
		pNewBone->PutLocalTransform(pT);
		pT->Release();
		pNewBone->Release();

	IMaterial *pMaterial = NULL;
	ITexture *pTexture = NULL;
	GetEngine()->GetRenderer()->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"dafoldil.jpg"));
	pTexture->SetUVTile(1, 1);
	GetEngine()->GetRenderer()->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);
	IKineChild *pChild = NULL;
	m_pBone->GetChild(L"main", &pChild);
	IMesh *pMesh = NULL;
	pChild->QueryInterface(&pMesh);
	pMesh->SetMaterial(pMaterial);
	pMesh->Release();
	pChild->Release();
	pMaterial->Release();
	pTexture->Release();
		}


		break;
	case MODEL_OVERSPEED:
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		box.Move(0, f*60, 0);
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 7, f*30, 30));
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
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth());
		box.Move(0, -f*2, 0);
		BuildWall(WALL_BEAM, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_RAIL:
		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CAR:
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CWT:
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_PULLEY:
		f = (GetLiftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		box.Move(0, f*60, 0);
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 6, f*20, 30));
		break;
	//case MODEL_LADDER:
	//	box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
	//	centre = box.CentreLower();
	//	p->build(this, nModelId, strName, nIndex, centre, 0.8, 0.8, 1, fRot);
	//	break;
	case MODEL_JAMB:
	case MODEL_JAMB_CAR:
		//box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_HEADING:
	case MODEL_HEADING_CAR:
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_SILL:
	case MODEL_SILL_CAR:
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	}
}

void CElemVis::Move(AVVECTOR vec)
{
	ITransform *pT = NULL;
	m_pBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	m_pBone->TransformLocal(pT);
	pT->Release();
}

IMesh *CElemVis::AddMesh(AVSTRING strName)
{
	IMesh *pMesh = NULL;
	GetObject()->NewMesh(strName, &pMesh);
	pMesh->Open(NULL, NULL);
	pMesh->SupportNormal(0);
	return pMesh;
}

void CElemVis::Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale, AVFLOAT fTexScale)
{
	IMesh *pVMesh = NULL;
	IMesh *pFMesh = NULL;
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
			FWFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh)
			{
				pFMesh->SupportBlendWeight(0.01f, 0);
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
			FWFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh == NULL)
			{
				if (pVMesh == NULL) continue;
				pFMesh = pVMesh;
				pVMesh = NULL;
				IKineChild *pChild = NULL;
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
		pFMesh->SupportBlendWeight(0.01f, 0);
		for (AVULONG i = 0; i < nVertex; i++)
			pFMesh->AddBlendWeight(i, 1.0f, strBone);
		pFMesh->Close();
	}
}

void CElemVis::PushState()
{
	if (!GetObject()) return;
	GetObject()->PushState();
}

void CElemVis::PopState()
{
	if (!GetObject()) return;
	GetObject()->PopState(); 
	GetObject()->Invalidate(); 
	GetObject()->PushState();
}

void CElemVis::Invalidate()
{
	if (!GetObject()) return;
	GetObject()->Invalidate(); 
}

void CElemVis::Render(IRenderer *pRenderer)
{
	if (!GetObject()) return;
	GetObject()->Render((IRndrGeneric*)pRenderer);
}

