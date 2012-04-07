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

#define F_PI ((AVFLOAT)M_PI)
#define F_PI_2 ((AVFLOAT)M_PI_2)
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBldObject

static OLECHAR *__name(OLECHAR *name, LONG i)
{
	static OLECHAR buf[257];
	_snwprintf(buf, 256, name, i);
	return buf;
}

static OLECHAR *__name(OLECHAR *name, LONG i, LONG j)
{
	static OLECHAR buf[257];
	_snwprintf(buf, 256, name, i, j);
	return buf;
}

void CBldObject::Create(AVSTRING name)
{
	m_pBuilding->GetScene()->NewObject(name, &m_pObj);
	m_pObj->CreateChild(name, &m_pBone);
}

void CBldObject::Create(AVSTRING name, AVVECTOR v)
{
	Create(name);

	ITransform *pT = NULL;
	m_pBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&v));
	m_pBone->PutLocalTransform(pT);
	pT->Release();
}

void CBldObject::Create(AVSTRING name, AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	AVVECTOR vec = {x, y, z};
	Create(name, vec);
}

BONE CBldObject::AddBone(AVSTRING name)
{
	BONE bone;
	m_pBone->CreateChild(name, &bone);
	return bone;
}

BONE CBldObject::AddBone(AVSTRING name, AVVECTOR v)
{
	BONE pBone = AddBone(name);

	ITransform *pT = NULL;
	pBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&v));
	pBone->PutLocalTransform(pT);
	pT->Release();
	return pBone;
}

BONE CBldObject::AddBone(AVSTRING name, AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	AVVECTOR vec = {x, y, z};
	return AddBone(name, vec);
}

void CBldObject::Deconstruct()
{
	if (m_pObj) m_pObj->Release();
	m_pObj = NULL;
	if (m_pBone) m_pBone->Release();
	m_pBone = NULL;
}

void CBldObject::Wall(BONE pBone, AVULONG nWallId, AVLONG nIndex, AVSTRING strName, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, BONE *ppBone)
{
	//TRACE(L"Building wall: pos = (%f, %f, %f), l = %f, h = %f, d = %f\n", vecPos.x/0.04f, vecPos.y/0.04f, vecPos.z/0.04f, l/0.04f, h/0.04f, d/0.04f);

	CBlock block;
	block.Open(GetFWObject(), pBone, __name(strName, LOWORD(nIndex), HIWORD(nIndex)), l, h, d, vecPos, vecRot.x, vecRot.y, vecRot.z);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	block.BuildWall();
	block.BuildRearSection();

	block.SetMaterial(GetBuilding()->GetMaterial(nWallId, LOWORD(nIndex), (nWallId == CBuilding::MAT_FLOOR_NUMBER_PLATE || nWallId == CBuilding::MAT_LIFT_NUMBER_PLATE) ? 256 : 8));
	if (ppBone) *ppBone = block.GetBone();
	block.Close();
}

void CBldObject::Wall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData, BONE *ppBone)
{
	Wall(GetBone(), nWallId, nIndex, strName, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppBone);
}

void CBldObject::Wall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, BOX box, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData, BONE *ppBone)
{
	Wall(nWallId, nIndex, strName, box.LeftFrontLower(), box.Width(), box.Height(), box.Depth(), vecRot, nDoorNum, pDoorData, ppBone);
}

void CBldObject::PushState()
{
	if (!GetFWObject()) return;
	GetFWObject()->PushState();
}

void CBldObject::PopState()
{
	if (!GetFWObject()) return;
	GetFWObject()->PopState(); 
	GetFWObject()->Invalidate(); 
	GetFWObject()->PushState();
}

void CBldObject::Render(IRenderer *pRenderer)
{
	if (!GetFWObject()) return;
	GetFWObject()->Render(pRenderer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuilding

CBuilding::CBuilding(void) : CBuildingBase(), m_pRenderer(NULL), m_pScene(NULL)
{
	memset(m_pMaterials, 0, sizeof(m_pMaterials));
}

CBuilding::~CBuilding(void)
{
	DeconstructMaterials();
	Deconstruct();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
}

AVFLOAT CBuilding::GetLiftZPos(int nLift)
{
	FWVECTOR vec = { 0, 0, 0 };
	if (GetLiftObject(nLift).GetBone())
		GetLiftObject(nLift).GetBone()->LtoG(&vec);
	return vec.z;
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

void CBuilding::STOREY::Construct(AVLONG iStorey)
{
	iStorey -= GetBuilding()->GetBasementStoreyCount();

	// imposed parameters
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 2.5f;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)

	// create skeletal structure (object & bone)
	m_obj.Create(__name(L"Storey_%d", iStorey), 0, 0, GetLevel());

	// collect door information
	std::vector<FLOAT> doordata;
	for (AVULONG i = 0; i < GetBuilding()->GetShaftCount(); i++)
	{
		SHAFT *pShaft = GetBuilding()->GetShaft(i);
		if (i < GetBuilding()->GetShaftCount(0))
			doordata.push_back(pShaft->GetBoxDoor().Left() - GetBox().LeftExt() - opn);
		else
			doordata.push_back(GetBox().RightExt() - pShaft->GetBoxDoor().Right() - opn);
		doordata.push_back(pShaft->GetBoxDoor().Width() + opn + opn);
		doordata.push_back(pShaft->GetBoxDoor().Height() + opn);
	}

	AVVECTOR v;

	// build walls
	GetObject().Wall(MAT_FLOOR,   iStorey, L"Storey_%d_Floor",   GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), GetBox().LowerThickness(), GetBox().DepthExt());
	GetObject().Wall(MAT_CEILING, iStorey, L"Storey_%d_Ceiling", GetBox().LeftExtRearExtUpper(), GetBox().WidthExt(), GetBox().UpperThickness(), GetBox().DepthExt());
	
	v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+40, 0);
	GetObject().Wall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Left_Nameplate", v, 2, 80, 40, Vector(0, F_PI_2));
	v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-40, 0);
	GetObject().Wall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Right_Nameplate", v, -2, 80, 40, Vector(-F_PI, -F_PI_2));

//	GetObject().Wall(MAT_FRONT, iStorey+1, L"Storey_%d_Cube",   Vector(-160, 20, 25), 40, 40, 40);
//	GetObject().Wall(MAT_FRONT, iStorey+2, L"Storey_%d_ExtraWall",	GetBox().CentreRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(-F_PI/2),
//			GetBuilding()->GetShaftCount(1), GetBuilding()->GetShaftCount(1) ? &doordata[3 * GetBuilding()->GetShaftCount(0)] : NULL);

	if (GetBox().FrontThickness() > 0)
		GetObject().Wall(MAT_REAR, iStorey, L"Storey_%d_RearWall",		GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0),
			GetBuilding()->GetShaftCount(0), &doordata[0]);
	if (GetBox().RearThickness() > 0)
		GetObject().Wall(MAT_FRONT, iStorey, L"Storey_%d_FrontWall",	GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(F_PI),
				GetBuilding()->GetShaftCount(1), GetBuilding()->GetShaftCount(1) ? &doordata[3 * GetBuilding()->GetShaftCount(0)] : NULL);
	if (GetBox().LeftThickness() > 0)
		GetObject().Wall(MAT_SIDE, iStorey, L"Storey_%d_LeftWall", GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		GetObject().Wall(MAT_SIDE, iStorey, L"Storey_%d_RightWall", GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(-F_PI_2));
}

void CBuilding::STOREY::Deconstruct()
{
	m_obj.Deconstruct();
}

void CBuilding::SHAFT::Construct(AVLONG iStorey, AVULONG iShaft)
{
	// imposed parameters
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 2.5f;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)
	AVFLOAT fDoorThickness = GetBoxDoor().Depth() * 0.2f;
	AVFLOAT fOpeningThickness = GetBoxDoor().Depth() * 0.4f;

	// create skeletal structure (object & bone)
	if (m_pStoreyBones == NULL)
	{
		m_pStoreyBones = new FWSTRUCT[GetBuilding()->GetStoreyCount()];
		for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
		{
			m_pStoreyBones[i].m_obj = CBldObject(GetBuilding());
			m_pStoreyBones[i].m_objLobbySide = CBldObject(GetBuilding());
			m_pStoreyBones[i].m_objLeft = CBldObject(GetBuilding());
			m_pStoreyBones[i].m_objRight = CBldObject(GetBuilding());
		}
	}
	GetObject(iStorey).Create(__name(L"Storey_%d_Shaft_%d", iStorey, iShaft), 0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel());
	GetObjectLobbySide(iStorey).Create(__name(L"Storey_%d_Shaft_%d_LobbySide", iStorey, iShaft), 0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel());
	GetObjectLeft(iStorey).Create(__name(L"Storey_%d_Shaft_%d_LeftSide", iStorey, iShaft), 0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel());
	GetObjectRight(iStorey).Create(__name(L"Storey_%d_Shaft_%d_RightSide", iStorey, iShaft), 0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel());
	
	GetBox().SetHeight(GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey, iShaft);

	if (GetBoxBeam().Width() > 0)
	{
		GetObject(iStorey).Wall(MAT_SHAFT1, nIndex, L"Storey_%d_Shaft_%d_Beam", GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), -GetBoxBeam().Height(), -GetBoxBeam().Depth());
		GetObject(iStorey).Wall(MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_BmRr", GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
	}

	if (GetBox().LeftThickness() > 0)
		GetObjectLeft(iStorey).Wall(MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Lt", GetLeftWallBox(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		GetObjectRight(iStorey).Wall(MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Rt", GetRightWallBox(), Vector(F_PI_2));
	if (GetBox().RearThickness() != 0)
		GetObject(iStorey).Wall(MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Rr", GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());

	// The Opening
	AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
	GetObjectLobbySide(iStorey).Wall(MAT_OPENING, nIndex, L"Storey_%d_Shaft_%d_Opening", 
		GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), 
		GetBoxDoor().Width() + opn + opn, GetBoxDoor().Height() + opn, fOpeningThickness,
		Vector(0), 1, door);

	// Plates
	if (GetShaftLine() == 0)
		GetObjectLobbySide(iStorey).Wall(MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 10, 0.5, Vector(F_PI, 0, F_PI));
	else
		GetObjectLobbySide(iStorey).Wall(MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 10, 0.5, Vector(0, 0, F_PI));

	// Door
	GetObjectLobbySide(iStorey).Wall(MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Left" , GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(0), 0, NULL, &m_pStoreyBones[iStorey].pDoors[0]);
	GetObjectLobbySide(iStorey).Wall(MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Right", GetBoxDoor().RightRearUpper(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(F_PI, F_PI), 0, NULL, &m_pStoreyBones[iStorey].pDoors[1]);
}

void CBuilding::SHAFT::Construct(AVULONG iShaft)
{
	// The Lifts

	// Create skeletal elements (entire lift)
	m_obj.Create(__name(L"Lift_%d", iShaft), GetLiftPos(0 /*+ iShaft % 4*/));

	for (AVULONG iDeck = 0; iDeck < GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_pDecks[iDeck] = m_obj.AddBone(__name(L"Deck_%d", iDeck), 0, 0, GetBuilding()->GetGroundStorey(iDeck)->GetLevel() - GetBuilding()->GetGroundStorey()->GetLevel()); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		BOX box = GetBoxCar() - GetLiftPos(0);
		BOX boxDoor0 = GetBoxCarDoor(0) - GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };
		AVFLOAT fDoorThickness0 = boxDoor0.Depth() * 0.4f;

		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT_FLOOR, iShaft, L"Lift_%d_Deck_%d_Floor", box.LeftExtRearExtLowerExt(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT_CEILING, iShaft, L"Lift_%d_Deck_%d_Ceiling", box.LeftExtRearExtUpper(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_RearWall", box.RightExtRearLower(), box.WidthExt(), box.Height(), box.RearThickness(), Vector(F_PI));
		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_LeftWall", box.LeftRearLower(), box.Depth(), box.Height(), box.LeftThickness(), Vector(-F_PI_2));
		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_RightWall", box.RightFrontLower(), box.Depth(), box.Height(), box.RightThickness(), Vector(F_PI_2));
		m_obj.Wall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_FrontWall", box.LeftExtFrontLower(), box.WidthExt(), box.Height(), box.FrontThickness(), Vector(0), 1, door);
		m_obj.Wall(m_pDecks[iDeck], MAT_DOOR, iShaft, L"Lift_%d_Deck_%d_Door1", boxDoor0.LeftFrontLower(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(0), 0, NULL, &m_pDoors[0]);
		m_obj.Wall(m_pDecks[iDeck], MAT_DOOR, iShaft, L"Lift_%d_Deck_%d_Door2", boxDoor0.RightFrontUpper(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(F_PI, F_PI), 0, NULL, &m_pDoors[1]);
	}
}

void CBuilding::SHAFT::Deconstruct()
{
	m_obj.Deconstruct();

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (m_pDecks[i]) m_pDecks[i]->Release();
	memset(m_pDecks, 0, sizeof(m_pDecks));
	for (AVULONG i = 0; i < MAX_DOORS; i++)
		if (m_pDoors[i]) m_pDoors[i]->Release();
	memset(m_pDoors, 0, sizeof(m_pDoors));

	if (m_pStoreyBones)
		for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
		{
			m_pStoreyBones[i].m_obj.Deconstruct();
			m_pStoreyBones[i].m_objLobbySide.Deconstruct();
			for (AVULONG j = 0; j < MAX_DOORS; j++)
				if (m_pStoreyBones[i].pDoors[j]) m_pStoreyBones[i].pDoors[j]->Release();
		}
	delete [] m_pStoreyBones;
}

void CBuilding::Construct(AVSTRING pLabel, AVVECTOR v)
{
	ASSERT(GetRenderer() && GetScene());
	if (!GetRenderer() || !GetScene()) return;

	SetMaterial(CBuilding::MAT_FRONT, 0, 0.0f, 1.0f, 0.0f, 0.3f);	// transparency set
	SetMaterial(CBuilding::MAT_REAR, 0,	0.0f, 1.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 0,	0.0f, 1.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 1, 1.0f, 0.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 1,  1.0f, 0.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 1,  1.0f, 0.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 2, 1.0f, 1.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 2,  1.0f, 1.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 2,  1.0f, 1.0f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 3, 0.0f, 1.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 3,  0.0f, 1.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 3,  0.0f, 1.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 4, 1.0f, 0.7f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 4,  1.0f, 0.7f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 4,  1.0f, 0.7f, 0.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 5, 0.8f, 0.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 5,  0.8f, 0.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 5,  0.8f, 0.0f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 6, 1.0f, 1.0f, 0.6f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 6,  1.0f, 1.0f, 0.6f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 6,  1.0f, 1.0f, 0.6f, 0.3f);
	SetMaterial(CBuilding::MAT_FRONT, 7, 0.0f, 0.3f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_REAR, 7,  0.0f, 0.3f, 1.0f, 0.3f);
	SetMaterial(CBuilding::MAT_SIDE, 7,  0.0f, 0.3f, 1.0f, 0.3f);
//	SetMaterial(CBuilding::MAT_SIDE, _stdPathModels + L"yellobrk.jpg", 1.0f, 1.0f);
	SetMaterial(CBuilding::MAT_SHAFT1, 0, 0.5f, 0.5f, 0.3f);
	SetMaterial(CBuilding::MAT_SHAFT2, 0, 0.4f, 0.2f, 0.0f, 0.3f);	// 0.4f, 0.2f, 0.0f
	SetMaterial(CBuilding::MAT_CEILING, 0, _stdPathModels + L"ceiling.jpg", 2.0f, 2.0f, 1.0f);
	SetMaterial(CBuilding::MAT_FLOOR, 0, _stdPathModels + L"floor3.jpg", 1.0f, 1.0f, 1.0f);
//	SetMaterial(CBuilding::MAT_DOOR, 0, _stdPathModels + L"metal1.jpg", 1.0f, 1.0f, 0.75f);
//	SetMaterial, CBuilding::MAT_OPENING, 0, _stdPathModels + L"metal2.jpg", 1.0f, 1.0f, 0.8f);
	SetMaterial(CBuilding::MAT_DOOR, 0,	0.4f, 0.3f, 0.3f, 0.5f);
	SetMaterial(CBuilding::MAT_OPENING, 0, 0.6f, 0.6f, 0.6f, 0.3f);

	//SetMaterial(CBuilding::MAT_LIFT, 0, _stdPathModels + L"oak.jpg", 1.0f, 0.75f);
	SetMaterial(CBuilding::MAT_LIFT, 0,	0.3f, 0.31f, 0.35f, 0.3f);
	SetMaterial(CBuilding::MAT_LIFT_FLOOR, 0, _stdPathModels + L"marble.jpg", 3.0f, 3.0f);
	SetMaterial(CBuilding::MAT_LIFT_CEILING, 0, _stdPathModels + L"metal3.jpg", 2.0f, 2.0f);

	for (AVLONG i = -(AVLONG)GetBasementStoreyCount(); i < (AVLONG)(GetStoreyCount() - GetBasementStoreyCount()); i++)
		SetMaterialFloorPlate(CBuilding::MAT_FLOOR_NUMBER_PLATE, (i+256)%256, i);
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		SetMaterialLiftPlate(CBuilding::MAT_LIFT_NUMBER_PLATE, i, i);	
	
	for (AVULONG iStorey = 0; iStorey < GetStoreyCount(); iStorey++)
	{
		GetStorey(iStorey)->Construct(iStorey);
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->Construct(iStorey, iShaft);
	}

	for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
		GetShaft(iShaft)->Construct(iShaft);
}

void CBuilding::Deconstruct()
{
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Deconstruct();
}

void CBuilding::StoreConfig()
{
	for (FWULONG i = 0; i < GetShaftCount(); i++)
	{
		GetLiftObject(i).PushState();
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
	for (FWULONG i = 0; i < GetShaftCount(); i++)
	{
		GetLiftObject(i).PopState();
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

