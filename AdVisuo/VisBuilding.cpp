// Building.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "VisBuilding.h"
#include "VisProject.h"

#include <freewill.h>
#include <fwrender.h>
#include <D3d9.h>
//#include <D3dx9tex.h>

#pragma warning (disable:4995)
#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuildingVis

CBuildingVis::CBuildingVis(CProject *pProject) : CBuildingConstr(pProject), m_materials(this), m_pRenderer(NULL), m_pScene(NULL)
{
	memset(m_pMaterials, 0, sizeof(m_pMaterials));
}

CBuildingVis::~CBuildingVis()
{
	DeconstructMaterials();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
}

AVVECTOR CBuildingVis::GetLiftPos(int nLift)
{
	AVVECTOR vec = { 0, 0, 0 };
	if (GetLiftElement(nLift)->GetBone())
		GetLiftElement(nLift)->GetBone()->GetNode()->LtoG((FWVECTOR*)&vec);
	return vec;
}

void CBuildingVis::SetRenderer(IRenderer *pRenderer)
{
	if (m_pRenderer) m_pRenderer->Release(); 
	m_pRenderer = pRenderer; 
	if (m_pRenderer) m_pRenderer->AddRef(); 
}

void CBuildingVis::SetScene(IScene *pScene)
{
	if (m_pScene) m_pScene->Release(); 
	m_pScene = pScene; 
	if (m_pScene) m_pScene->AddRef(); 
}

	static void _helperConvMatInd(AVULONG &nWallId, AVULONG &i)
	{
		if (nWallId < CBuildingVis::WALL_LIFT_NUMBER_PLATE)
		{
			nWallId *= 8;
			i = i % 8;
		}
		else
		{
			nWallId = CBuildingVis::WALL_LIFT_NUMBER_PLATE * 8 + (nWallId - CBuildingVis::WALL_LIFT_NUMBER_PLATE) * 256;
			i = i % 256;
		}
	}

IMaterial *CBuildingVis::GetMaterial(AVULONG nWallId, AVLONG i)
{
	_helperConvMatInd(nWallId, (AVULONG&)i);
	IMaterial *p = m_pMaterials[nWallId + i];
	if (!p) p = m_pMaterials[nWallId];
	return p;
}

void CBuildingVis::SetMaterial(AVULONG nWallId, AVLONG i, IMaterial *pMaterial)
{
	_helperConvMatInd(nWallId, (AVULONG&)i);
	if (nWallId + i > sizeof(m_pMaterials) / sizeof(IMaterial*)) return;
	if (m_pMaterials[nWallId + i]) m_pMaterials[nWallId + i]->Release();
	m_pMaterials[nWallId + i] = pMaterial;
	if (m_pMaterials[nWallId + i]) m_pMaterials[nWallId + i]->AddRef();
}

void CBuildingVis::SetMaterial(AVULONG nWallId, AVLONG i, AVFLOAT r, AVFLOAT g, AVFLOAT b, AVFLOAT a)
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
	SetMaterial(nWallId, i, pMaterial);
	pMaterial->Release();
}

void CBuildingVis::SetMaterial(AVULONG nWallId, AVLONG i, LPCOLESTR szFileName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha)
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

	SetMaterial(nWallId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuildingVis::SetMaterial(AVULONG nWallId, AVLONG i, BYTE* pData, AVULONG nDataSize, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT fAlpha)
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

	SetMaterial(nWallId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuildingVis::SetMaterialLiftPlate(AVULONG nWallId, AVLONG i, AVULONG nLift)
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

	SetMaterial(nWallId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuildingVis::SetMaterialFloorPlate(AVULONG nWallId, AVLONG i, AVULONG nFloor)
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

	SetMaterial(nWallId, i, pMaterial);
	pMaterial->Release();
	pTexture->Release();
}

void CBuildingVis::DeconstructMaterials()
{
	for (AVULONG i = 0; i < sizeof(m_pMaterials) / sizeof(IMaterial*); i++)
	{
		if (m_pMaterials[i]) m_pMaterials[i]->Release();
		m_pMaterials[i] = NULL;
	}
}

void CBuildingVis::Construct(AVVECTOR vec)
{
//	ASSERT(GetRenderer() && GetScene());
//	if (!GetRenderer() || !GetScene()) return;

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
		SetMaterialFloorPlate(CBuildingVis::WALL_FLOOR_NUMBER_PLATE, (i+256)%256, i);
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		SetMaterialLiftPlate(CBuildingVis::WALL_LIFT_NUMBER_PLATE, i, i);

	CBuildingConstr::Construct(vec);
}

void CBuildingVis::StoreConfig()
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

void CBuildingVis::RestoreConfig()
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

