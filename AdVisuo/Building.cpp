// Building.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "../CommonFiles/Vector.h"
#include "Building.h"
#include "Block.h"

#include "AdVisuo.h"

#include <freewill.h>
#include <fwrender.h>
#include <D3d9.h>
#include <D3dx9tex.h>

#include <vector>

#pragma warning (disable:4995)
#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBone

CBone::CBone(IKineNode *pNode) : CBoneBase(pNode), m_pNode(pNode)
{ 
	m_pNode->AddRef(); 
}

CBone::~CBone()
{ 
	if (m_pNode) m_pNode->Release(); 
}

void CBone::PushState()					{ if (m_pNode) m_pNode->PushState(); }
void CBone::PopState()					{ if (m_pNode) m_pNode->PopState(); }
void CBone::Invalidate()				{ if (m_pNode) m_pNode->Invalidate(); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElement

CElement::~CElement()
{
	if (m_pObj) m_pObj->Release();
	m_pObj = NULL;
	if (m_pBone) delete m_pBone;
	m_pBone = NULL;
}

void CElement::onCreate(AVSTRING name, AVVECTOR &vec)
{
	static OLECHAR buf[257];
	_snwprintf(buf, 256, L"_bld_%d_%ls", GetBuilding()->GetIndex(), name);

	IKineNode *pNode = NULL;
	GetBuilding()->GetScene()->NewObject(buf, &m_pObj);
	m_pObj->CreateChild(name, &pNode);
	m_pBone = new CBone(pNode);
	pNode->Release();

	ITransform *pT = NULL;
	GetNode()->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	GetNode()->PutLocalTransform(pT);
	pT->Release();
}

void CElement::onMove(AVVECTOR &vec)
{
	ITransform *pT = NULL;
	m_pObj->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	m_pObj->TransformLocal(pT);
	pT->Release();
}

CBoneBase *CElement::onAddBone(AVSTRING name, AVVECTOR &vec)
{
	IKineNode *pNode = NULL;
	GetNode()->CreateChild(name, &pNode);
	CBone *pBone = new CBone(pNode);
	pNode->Release();

	ITransform *pT = NULL;
	pBone->GetNode()->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	pBone->GetNode()->PutLocalTransform(pT);
	pT->Release();

	return pBone;
}

void CElement::onAddWall(CBoneBase *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBoneBase **ppNewBone)
{
	//TRACE(L"Building wall: pos = (%f, %f, %f), l = %f, h = %f, d = %f\n", vecPos.x/0.04f, vecPos.y/0.04f, vecPos.z/0.04f, l/0.04f, h/0.04f, d/0.04f);

	CBlock block;
	block.Open(m_pObj, GetNode(pBone), strName, l, h, d, vecPos, vecRot.x, vecRot.y, vecRot.z);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	block.BuildWall();
	block.BuildRearSection();

	block.SetMaterial(GetBuilding()->GetMaterial(nWallId, LOWORD(nIndex), (nWallId == CBuilding::MAT_FLOOR_NUMBER_PLATE || nWallId == CBuilding::MAT_LIFT_NUMBER_PLATE) ? 256 : 8));
	if (ppNewBone)
	{
		IKineNode *pNewNode = block.GetBone();
		*ppNewBone = new CBone(pNewNode);
		pNewNode->Release();
	}
	block.Close();
}

IMesh *CElement::AddMesh(AVSTRING strName)
{
	IMesh *pMesh = NULL;
	m_pObj->NewMesh(strName, &pMesh);
	pMesh->Open(NULL, NULL);
	pMesh->SupportNormal(0);
	return pMesh;
}

void CElement::Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale, AVFLOAT fTexScale)
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
				pChild->PutLabel((FWSTRING)name.c_str());
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

void CElement::PushState()
{
	if (!m_pObj) return;
	m_pObj->PushState();
}

void CElement::PopState()
{
	if (!m_pObj) return;
	m_pObj->PopState(); 
	m_pObj->Invalidate(); 
	m_pObj->PushState();
}

void CElement::Render(IRenderer *pRenderer)
{
	if (!m_pObj) return;
	m_pObj->Render(pRenderer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuilding

CBuilding::CBuilding(void) : CBuildingConstr(), m_materials(this), m_pRenderer(NULL), m_pScene(NULL)
{
	memset(m_pMaterials, 0, sizeof(m_pMaterials));
	bFastLoad = false;
}

CBuilding::~CBuilding(void)
{
	DeconstructMaterials();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
}

AVVECTOR CBuilding::GetLiftPos(int nLift)
{
	AVVECTOR vec = { 0, 0, 0 };
	if (GetLiftElement(nLift)->GetBone())
		GetLiftElement(nLift)->GetBone()->GetNode()->LtoG((FWVECTOR*)&vec);
	return vec;
}

void CBuilding::SetRenderer(IRenderer *pRenderer)
{
	if (m_pRenderer) m_pRenderer->Release(); 
	m_pRenderer = pRenderer; 
	if (m_pRenderer) m_pRenderer->AddRef(); 
}

void CBuilding::SetScene(IScene *pScene)
{
	if (m_pScene) m_pScene->Release(); 
	m_pScene = pScene; 
	if (m_pScene) m_pScene->AddRef(); 
}

IMaterial *CBuilding::GetMaterial(AVULONG nItemId, AVLONG i, AVULONG nMaxI)
{
	IMaterial *p = m_pMaterials[nItemId * MAT_TEXTURES_PER_ITEM + (i + 100 * nMaxI) % nMaxI];
	if (!p) p = m_pMaterials[nItemId * MAT_TEXTURES_PER_ITEM];
	return p;
}

void CBuilding::SetMaterial(AVULONG nItemId, AVULONG i, IMaterial *pMaterial)
{
	AVULONG ind = nItemId * MAT_TEXTURES_PER_ITEM + i;
	if (ind > sizeof(m_pMaterials) / sizeof(IMaterial*)) return;
	if (m_pMaterials[ind]) m_pMaterials[ind]->Release();
	m_pMaterials[ind] = pMaterial;
	if (m_pMaterials[ind]) m_pMaterials[ind]->AddRef();
}

void CBuilding::SetMaterial(AVULONG nItemId, AVULONG i, AVFLOAT r, AVFLOAT g, AVFLOAT b, AVFLOAT a)
{
	FWCOLOR diff = { r, g, b, a };
	IMaterial *pMaterial = NULL;
	m_pRenderer->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetDiffuseColor(diff);
	pMaterial->SetAmbientColor(diff);
	pMaterial->SetSpecularColor(diff);
	pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(TRUE);
	pMaterial->SetAlphaMode(a < 0.99 ? MAT_ALPHA_MATERIAL : MAT_ALPHA_DISABLE);
	pMaterial->SetCullingMode(a < 0.99 ? MAT_CULLING_CCW_CW : MAT_CULLING_DISABLE);
	SetMaterial(nItemId, i, pMaterial);
	pMaterial->Release();
}

void CBuilding::SetMaterial(AVULONG nItemId, AVULONG i, LPCOLESTR szFileName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha)
{
	ITexture *pTexture = NULL;
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)szFileName);
	pTexture->SetUVTile(fUTile, fVTile);

	IMaterial *pMaterial = NULL;
	m_pRenderer->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);
//	pMaterial->SetTextured(TRUE);

	pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(FALSE);
	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);
	pMaterial->SetCullingMode(fAlpha < 0.99 ? MAT_CULLING_CCW_CW : MAT_CULLING_DISABLE);

	SetMaterial(nItemId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuilding::SetMaterial(AVULONG nItemId, AVULONG i, BYTE* pData, AVULONG nDataSize, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha)
{
	ITexture *pTexture = NULL;
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFileInMemory(pData, nDataSize);
	pTexture->SetUVTile(fUTile, fVTile);

	IMaterial *pMaterial = NULL;
	m_pRenderer->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);

	pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(FALSE);
	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);
	pMaterial->SetCullingMode(fAlpha < 0.99 ? MAT_CULLING_CCW_CW : MAT_CULLING_DISABLE);

	SetMaterial(nItemId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuilding::SetMaterialLiftPlate(AVULONG nItemId, AVULONG i, AVULONG nLift)
{
	ITexture *pTexture = NULL;
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"plate_lift.bmp"));
	pTexture->SetUVTile(10, 10);

	HRESULT h;
	IDirect3DTexture9 *p = NULL;
	IID iid;
	h = pTexture->GetContextObject(0, &iid, (void**)&p);
	IDirect3DSurface9 *pSurface;
	h = p->GetSurfaceLevel(0, &pSurface);
	HDC hDC = NULL;
	h = pSurface->GetDC(&hDC);

	CString str;
	str.Format(L"%c", 'A' + nLift );
	CRect r(0, 0, 128, 128);
	HFONT hFont = ::CreateFont(64, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,OUT_OUTLINE_PRECIS, CLIP_STROKE_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, L"Arial");
	HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
	CRect rect(0, 0, 63, 63);
	::SetBkMode(hDC, TRANSPARENT);
	::DrawText(hDC, str, -1, rect, DT_CENTER);
	::SelectObject(hDC, hOldFont);

	pSurface->ReleaseDC(hDC);
	pSurface->Release();
	p->Release();

	IMaterial *pMaterial = NULL;
	m_pRenderer->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);
//	pMaterial->SetTextured(TRUE);

	pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(FALSE);
//	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);

	SetMaterial(nItemId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuilding::SetMaterialFloorPlate(AVULONG nItemId, AVULONG i, AVULONG nFloor)
{
	ITexture *pTexture = NULL;
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"plate_floor.bmp"));
	pTexture->SetUVTile(1.25f, 1.25f);

	HRESULT h;
	IDirect3DTexture9 *p = NULL;
	IID iid;
	h = pTexture->GetContextObject(0, &iid, (void**)&p);
	IDirect3DSurface9 *pSurface;
	h = p->GetSurfaceLevel(0, &pSurface);
	HDC hDC = NULL;
	h = pSurface->GetDC(&hDC);

	CString str;
	str.Format(L"%d", nFloor);
	CRect r(0, 0, 128, 128);
	HFONT hFont = ::CreateFont(64, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,OUT_OUTLINE_PRECIS, CLIP_STROKE_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, L"Arial");
	HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
	CRect rect(0, 0, 127, 63);
	::SetBkMode(hDC, TRANSPARENT);
	::DrawText(hDC, str, -1, rect, DT_CENTER);
	::SelectObject(hDC, hOldFont);

	pSurface->ReleaseDC(hDC);
	pSurface->Release();
	p->Release();

	IMaterial *pMaterial = NULL;
	m_pRenderer->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);
//	pMaterial->SetTextured(TRUE);

	pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(FALSE);
//	pMaterial->SetAlphaMode(MAT_ALPHA_TEXTURE);
//	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);

	SetMaterial(nItemId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuilding::DeconstructMaterials()
{
	for (AVULONG i = 0; i < sizeof(m_pMaterials) / sizeof(IMaterial*); i++)
	{
		if (m_pMaterials[i]) m_pMaterials[i]->Release();
		m_pMaterials[i] = NULL;
	}
}

void CBuilding::Construct(AVVECTOR vec)
{
	ASSERT(GetRenderer() && GetScene());
	if (!GetRenderer() || !GetScene()) return;

	m_materials.Set(MAT_BACKGROUND, RGB(80, 80, 80));
	m_materials.Set(MAT_LOBBY_1, RGB(255, 0, 0), 0.3f);
	m_materials.Set(MAT_LOBBY_2, RGB(255, 255, 0), 0.3f);
	m_materials.Set(MAT_LOBBY_3, RGB(0, 255, 255), 0.3f);
	m_materials.Set(MAT_LOBBY_4, RGB(255, 180, 0), 0.3f);
	m_materials.Set(MAT_LOBBY_5, RGB(203, 0, 255), 0.3f);
	m_materials.Set(MAT_LOBBY_6, RGB(255, 255, 154), 0.3f);
	m_materials.Set(MAT_LOBBY_7, RGB(0, 76, 255), 0.3f);
	m_materials.Set(MAT_LOBBY_0, RGB(0, 255, 0), 0.3f);

	m_materials.Set(::MAT_FLOOR,		_stdPathModels + L"floor3.jpg",  1.0f, 1.0f, 1.0f);
	m_materials.Set(::MAT_CEILING,		_stdPathModels + L"ceiling.jpg", 2.0f, 2.0f, 1.0f);
	m_materials.Set(::MAT_DOOR,			RGB(102, 76, 76), 0.5f);
	m_materials.Set(::MAT_LIFT_DOOR,	RGB(102, 76, 76), 0.5f);
	m_materials.Set(::MAT_OPENING,		RGB(154, 154, 154), 0.3f);
	m_materials.Set(::MAT_SHAFT,		RGB(102, 51, 0), 0.3f);
	m_materials.Set(::MAT_BEAM,			RGB(128, 128, 76));
	m_materials.Set(::MAT_LIFT,			RGB(76, 80, 90), 0.3f);
	m_materials.Set(::MAT_LIFT_FLOOR,	_stdPathModels + L"marble.jpg", 3.0f, 3.0f);
	m_materials.Set(::MAT_LIFT_CEILING,	_stdPathModels + L"metal3.jpg", 2.0f, 2.0f);

	for (AVLONG i = -(AVLONG)GetBasementStoreyCount(); i < (AVLONG)(GetStoreyCount() - GetBasementStoreyCount()); i++)
		SetMaterialFloorPlate(CBuilding::MAT_FLOOR_NUMBER_PLATE, (i+256)%256, i);
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		SetMaterialLiftPlate(CBuilding::MAT_LIFT_NUMBER_PLATE, i, i);

	CBuildingConstr::Construct(vec);
}

void CBuilding::StoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PushState();
		for (FWULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	GetLiftDoor(i, k)->PushState();
	}
	for (FWULONG i = 0; i < GetStoreyCount(); i++)
		for (FWULONG j = 0; j < GetShaftCount(); j++)
			for (FWULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
					GetShaftDoor(i, j, k)->PushState();
}

void CBuilding::RestoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		GetLiftElement(i)->PopState();
		for (FWULONG k = 0; k < MAX_DOORS; k++)
			if (GetLiftDoor(i, k))	{ GetLiftDoor(i, k)->PopState(); GetLiftDoor(i, k)->Invalidate(); GetLiftDoor(i, k)->PushState(); }
	}
	for (FWULONG i = 0; i < GetStoreyCount(); i++)
		for (FWULONG j = 0; j < GetShaftCount(); j++)
			for (FWULONG k = 0; k < MAX_DOORS; k++)
				if (GetShaftDoor(i, j, k))
				{
					GetShaftDoor(i, j, k)->PopState();
					GetShaftDoor(i, j, k)->Invalidate();
					GetShaftDoor(i, j, k)->PushState();
				}
}

