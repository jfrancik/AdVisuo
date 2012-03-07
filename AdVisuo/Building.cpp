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
	if (GetLiftBone(nLift))
		GetLiftBone(nLift)->LtoG(&vec);
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
	pMaterial->SetTwoSided(FALSE);
//	pMaterial->SetTextured(FALSE);
	pMaterial->SetAlphaMode(a < 0.9 ? MAT_ALPHA_MATERIAL : MAT_ALPHA_DISABLE);
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
	pTexture->SetUVTile(2.5f, 2.5f);

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

void ConstructWall(CBuilding *pBuilding, IKineNode *pBone, ISceneObject *pObj, AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, IKineNode **ppBone = NULL)
{
	TRACE(L"Building wall: pos = (%f, %f, %f), l = %f, h = %f, d = %f\n", vecPos.x/0.04f, vecPos.y/0.04f, vecPos.z/0.04f, l/0.04f, h/0.04f, d/0.04f);
	CBlock block;
	block.Open(pObj, pBone, __name(strName, LOWORD(nIndex), HIWORD(nIndex)), l, h, d, vecPos, vecRot.x, vecRot.y, vecRot.z);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	if (nWallId != CBuilding::MAT_FLOOR_NUMBER_PLATE)
	{
		block.BuildWall();
		block.BuildRearSection();
	}

	block.SetMaterial(pBuilding->GetMaterial(nWallId, LOWORD(nIndex), (nWallId == CBuilding::MAT_FLOOR_NUMBER_PLATE || nWallId == CBuilding::MAT_LIFT_NUMBER_PLATE) ? 256 : 8));
	if (ppBone) *ppBone = block.GetBone();
	block.Close();
}

void ConstructWall(CBuilding *pBuilding, IKineNode *pBone, ISceneObject *pObj, AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					BOX box, AVVECTOR vecRot = Vector(0),
					AVULONG nDoorNum = 0, FLOAT *pDoorData = NULL, IKineNode **ppBone = NULL)
{
	ConstructWall(pBuilding, pBone, pObj, nWallId, nIndex, strName, 
					box.LeftFrontLower(), box.Width(), box.Height(), box.Depth(), vecRot,
					nDoorNum, pDoorData, ppBone);
}					
					
void CBuilding::STOREY::Construct(AVLONG iStorey)
{
	iStorey -= GetBuilding()->GetBasementStoreyCount();

	// imposed parameters
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 2.5f;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)

	// create skeletal structure (object & bone)
	ITransform *pT = NULL;
	GetBuilding()->GetScene()->NewObject(__name(L"Storey_%d", iStorey), &m_pObj);
	m_pObj->CreateChild(L"Storey", &m_pBone);
	m_pBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(0, 0, GetLevel());
	m_pBone->PutLocalTransform(pT);
	pT->Release();

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
	ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_FLOOR,   iStorey, L"Storey_%d_Floor",   GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), GetBox().LowerThickness(), GetBox().DepthExt());
	ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_CEILING, iStorey, L"Storey_%d_Ceiling", GetBox().LeftExtRearExtUpper(), GetBox().WidthExt(), GetBox().UpperThickness(), GetBox().DepthExt());
	
	if (GetBox().FrontThickness() > 0)
		ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_REAR, iStorey, L"Storey_%d_RearWall",		GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0),
			GetBuilding()->GetShaftCount(0), &doordata[0]);
	if (GetBox().RearThickness() > 0)
		ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_FRONT, iStorey, L"Storey_%d_FrontWall",	GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(F_PI),
				GetBuilding()->GetShaftCount(1), &doordata[3 * GetBuilding()->GetShaftCount(0)]);
	if (GetBox().LeftThickness() > 0)
		ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_SIDE, iStorey, L"Storey_%d_LeftWall", GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_SIDE, iStorey, L"Storey_%d_RightWall", GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(-F_PI_2));

	v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+20, 0);
	ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Left_Nameplate", v, 2, 40, 40, Vector(0, F_PI_2));
	v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-20, 0);
	ConstructWall(GetBuilding(), GetBone(), GetObj(), MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Right_Nameplate", v, -2, 40, 40, Vector(-F_PI, -F_PI_2));


	GetObj()->Invalidate();
}

void CBuilding::STOREY::Deconstruct()
{
	if (m_pObj) m_pObj->Release();
	m_pObj = NULL;
	if (m_pBone) m_pBone->Release();
	m_pBone = NULL;
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
		memset(m_pStoreyBones, 0, sizeof(FWSTRUCT) * GetBuilding()->GetStoreyCount());
	}
	ITransform *pT = NULL;
	GetBuilding()->GetScene()->NewObject(__name(L"Storey_%d_Shaft_%d", iStorey, iShaft), &m_pStoreyBones[iStorey].pObj);
	ISceneObject *pObj = m_pStoreyBones[iStorey].pObj;
	pObj->CreateChild(L"StoreyShaft", &m_pStoreyBones[iStorey].pBone);
	IKineNode *pBone = m_pStoreyBones[iStorey].pBone;
	pBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(0, 0, GetBuilding()->GetStorey(iStorey)->GetLevel());
	pBone->PutLocalTransform(pT);
	pT->Release();

	GetBox().SetHeight(GetBuilding()->GetStorey(iStorey)->GetBox().HeightExt());
	ULONG nIndex = MAKELONG(iStorey, iShaft);

	if (GetBoxBeam().Width() > 0)
	{
		ConstructWall(GetBuilding(), pBone, pObj, MAT_SHAFT1, nIndex, L"Storey_%d_Shaft_%d_Beam", GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), -GetBoxBeam().Height(), -GetBoxBeam().Depth());
		ConstructWall(GetBuilding(), pBone, pObj, MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_BmRr", GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
	}

	if (GetBox().LeftThickness() > 0)
		ConstructWall(GetBuilding(), pBone, pObj, MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Lt", GetLeftWallBox(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		ConstructWall(GetBuilding(), pBone, pObj, MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Rt", GetRightWallBox(), Vector(F_PI_2));
	if (GetBox().RearThickness() != 0)
		ConstructWall(GetBuilding(), pBone, pObj, MAT_SHAFT2, nIndex, L"Storey_%d_Shaft_%d_Rr", GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());

	// The Opening
	AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
	ConstructWall(GetBuilding(), pBone, pObj, MAT_OPENING, nIndex, L"Storey_%d_Shaft_%d_Opening", 
		GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), 
		GetBoxDoor().Width() + opn + opn, GetBoxDoor().Height() + opn, fOpeningThickness,
		Vector(0), 1, door);

	// Plates
	if (GetShaftLine() == 0)
		ConstructWall(GetBuilding(), pBone, pObj, MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 10, 0.5, Vector(F_PI, 0, F_PI));
	else
		ConstructWall(GetBuilding(), pBone, pObj, MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 10, 0.5, Vector(0, 0, F_PI));

	// Door
	ConstructWall(GetBuilding(), pBone, pObj, MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Left" , GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(0), 0, NULL, &m_pStoreyBones[iStorey].ppDoors[0]);
	ConstructWall(GetBuilding(), pBone, pObj, MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Right", GetBoxDoor().RightRearUpper(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(F_PI, F_PI), 0, NULL, &m_pStoreyBones[iStorey].ppDoors[1]);
}

void CBuilding::SHAFT::Construct(AVULONG iShaft)
{
	// The Lifts

	// Create skeletal elements (entire lift)
	GetBuilding()->GetScene()->NewObject(__name(L"Lift_%d", iShaft), &m_pObj);
	m_pObj->CreateChild(L"Lift", &m_pBone);

	// move to the shaft
	ITransform *pT = NULL;
	//m_pBone->CreateCompatibleTransform(&pT);
	//pT->FromTranslationVector((FWVECTOR*)(&GetLiftPos(0)));
	//m_pBone->PutLocalTransform(pT);
	//pT->Release();

	for (AVULONG iDeck = 0; iDeck < GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_pBone->CreateChild(__name(L"Deck_%d", iDeck), &m_ppDecks[iDeck]);

		// elevate the deck (if applicable)
		m_ppDecks[iDeck]->CreateCompatibleTransform(&pT);
		pT->FromTranslationXYZ(0, 0, GetBuilding()->GetStorey(iDeck + 2)->GetLevel());
		m_ppDecks[iDeck]->PutLocalTransform(pT);
		pT->Release();

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		ConstructWall(GetBuilding(), m_ppDecks[iDeck], m_pObj, MAT_LIFT_FLOOR, iShaft, L"Lift_%d_Deck_%d_Floor", GetBoxCar().LeftExtRearExtLowerExt(), GetBoxCar().WidthExt(), GetBoxCar().LowerThickness(), GetBoxCar().DepthExt());
			
		ConstructWall(GetBuilding(), m_ppDecks[iDeck], m_pObj, MAT_LIFT_CEILING, iShaft, L"Lift_%d_Deck_%d_Ceiling", GetBoxCar().LeftExtRearExtUpper(), GetBoxCar().WidthExt(), GetBoxCar().LowerThickness(), GetBoxCar().DepthExt());



		ConstructWall(GetBuilding(), m_ppDecks[iDeck], m_pObj, MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_RearWall", GetBoxCar().LeftExtRearLower(), GetBoxCar().WidthExt(), GetBoxCar().Height(), -GetBoxCar().RearThickness());


		//// find out the height of the walls
		//AVFLOAT fHeight = (iDeck == GetDeckCount()-1) ? pShaft->GetBoxCar().Height() : GetStorey(iDeck)->GetHeight();

		//// Front Wall
		//_snwprintf(buf, 256, L"Lift_%d_Deck_%d_FrontWall", i, iDeck);
		//block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), fHeight, pShaft->GetBoxCar().FrontThickness(), pShaft->GetBoxCar().LeftExtFrontLower() - pShaft->GetBoxCar(), 0);
		//block.BuildFrontSection();
		//block.BuildWall((pShaft->GetBoxCar().Width() - pShaft->GetBoxDoor().Width()) / 2 + pShaft->GetBoxCar().LeftThickness());
		//block.BuildDoor(pShaft->GetBoxDoor().Width(), pShaft->GetBoxDoor().Height());
		//block.BuildWall();
		//block.BuildRearSection();
		//block.SetMaterial(GetMaterial(MAT_LIFT));
		//block.Close();

		//// Rear Wall
		//_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RearWall", i, iDeck);
		//block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), fHeight, pShaft->GetBoxCar().FrontThickness(), pShaft->GetBoxCar().LeftExtRearExtLower() - pShaft->GetBoxCar() + Vector(0, 1.0f, 0), 0);
		//block.BuildSimpleBlock();
		//block.SetMaterial(GetMaterial(MAT_LIFT));
		//block.Close();

		//// Left Wall
		//_snwprintf(buf, 256, L"Lift_%d_Deck_%d_LeftWall", i, iDeck);
		//block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().Depth(), fHeight, pShaft->GetBoxCar().LeftThickness(), pShaft->GetBoxCar().LeftExtFrontLower() - pShaft->GetBoxCar(), F_PI_2);
		//block.BuildSimpleBlock();
		//block.SetMaterial(GetMaterial(MAT_LIFT));
		//block.Close();

		//// Right Wall
		//_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RightWall", i, iDeck);
		//block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().Depth(), fHeight, pShaft->GetBoxCar().LeftThickness(), pShaft->GetBoxCar().RightFrontLower() - pShaft->GetBoxCar(), F_PI_2);
		//block.BuildSimpleBlock();
		//block.SetMaterial(GetMaterial(MAT_LIFT));
		//block.Close();

		//// Doors - need to be built yet...
		//m_pBoneLiftLDoors[i] = NULL;
		//m_pBoneLiftRDoors[i] = NULL;

		//m_pObjLifts[i]->Invalidate();
	}




	
//	// The Lifts
//	for (AVULONG i = 0; i < nLifts; i++)
//	{
////		SHAFT *pShaft = GetShaft(i < 5 ? 0 : 1);
//		SHAFT *pShaft = GetShaft(i);
//
//		AVULONG nDecks = (pShaft->GetDeck() == DECK_DOUBLE) ? DECK_NUM : 1;
//
//		// Create skeletal elements (entire lift)
//		_snwprintf(buf, 256, L"Lift_%d", i);
//		GetScene()->NewObject(buf, &m_pObjLifts[i]);
//		m_pObjLifts[i]->CreateChild(L"Lift", &m_pBoneLifts[i]);
//
//		// move to the shaft
//		m_pBoneLifts[i]->CreateCompatibleTransform(&pT);
////		pT->FromTranslationVector((FWVECTOR*)(&GetLiftPos(0, 1)));
//		pT->FromTranslationVector((FWVECTOR*)(&GetLiftPos(i, 0)));
//		m_pBoneLifts[i]->PutLocalTransform(pT);
//		pT->Release();
//
//
//		for (AVULONG iDeck = 0; iDeck < nDecks; iDeck++)
//		{
//			// Create skeletal elements (the deck)
//			_snwprintf(buf, 256, L"Deck_%d", iDeck);
//			m_pBoneLifts[i]->CreateChild(buf, &m_pBoneLiftDecks[iDeck][i]);
//
//			// elevate the deck (if applicable)
//			m_pBoneLiftDecks[iDeck][i]->CreateCompatibleTransform(&pT);
//			pT->FromTranslationXYZ(0, 0, GetStorey(iDeck)->GetLevel());
//			m_pBoneLiftDecks[iDeck][i]->PutLocalTransform(pT);
//			pT->Release();
//
//			CBlock block;
//			
//			// Floor
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_Floor", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), pShaft->GetBoxCar().LowerThickness(), pShaft->GetBoxCar().DepthExt(), pShaft->GetBoxCar().LeftExtRearExtLowerExt() - pShaft->GetBoxCar(), 0);
//			block.BuildSimpleBlock();
//			block.SetMaterial(GetMaterial(MAT_LIFT_FLOOR));
//			block.Close();
//
//			// Ceiling
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_Ceiling", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), pShaft->GetBoxCar().LowerThickness(), pShaft->GetBoxCar().DepthExt(), pShaft->GetBoxCar().LeftExtRearExtUpper() - pShaft->GetBoxCar(), 0);
//			block.BuildSimpleBlock();
//			block.SetMaterial(GetMaterial(MAT_LIFT_CEILING));
//			block.Close();
//
//			// find out the height of the walls
//			AVFLOAT fHeight = (iDeck == nDecks-1) ? pShaft->GetBoxCar().Height() : GetStorey(iDeck)->GetHeight();
//
//			// Front Wall
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_FrontWall", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), fHeight, pShaft->GetBoxCar().FrontThickness(), pShaft->GetBoxCar().LeftExtFrontLower() - pShaft->GetBoxCar(), 0);
//			block.BuildFrontSection();
//			block.BuildWall((pShaft->GetBoxCar().Width() - pShaft->GetBoxDoor().Width()) / 2 + pShaft->GetBoxCar().LeftThickness());
//			block.BuildDoor(pShaft->GetBoxDoor().Width(), pShaft->GetBoxDoor().Height());
//			block.BuildWall();
//			block.BuildRearSection();
//			block.SetMaterial(GetMaterial(MAT_LIFT));
//			block.Close();
//
//			// Rear Wall
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RearWall", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().WidthExt(), fHeight, pShaft->GetBoxCar().FrontThickness(), pShaft->GetBoxCar().LeftExtRearExtLower() - pShaft->GetBoxCar() + Vector(0, 1.0f, 0), 0);
//			block.BuildSimpleBlock();
//			block.SetMaterial(GetMaterial(MAT_LIFT));
//			block.Close();
//
//			// Left Wall
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_LeftWall", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().Depth(), fHeight, pShaft->GetBoxCar().LeftThickness(), pShaft->GetBoxCar().LeftExtFrontLower() - pShaft->GetBoxCar(), F_PI_2);
//			block.BuildSimpleBlock();
//			block.SetMaterial(GetMaterial(MAT_LIFT));
//			block.Close();
//
//			// Right Wall
//			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RightWall", i, iDeck);
//			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->GetBoxCar().Depth(), fHeight, pShaft->GetBoxCar().LeftThickness(), pShaft->GetBoxCar().RightFrontLower() - pShaft->GetBoxCar(), F_PI_2);
//			block.BuildSimpleBlock();
//			block.SetMaterial(GetMaterial(MAT_LIFT));
//			block.Close();
//
//			// Doors - need to be built yet...
//			m_pBoneLiftLDoors[i] = NULL;
//			m_pBoneLiftRDoors[i] = NULL;
//
//			m_pObjLifts[i]->Invalidate();
//		}
//	}
}

void CBuilding::SHAFT::Deconstruct()
{
	if (m_pObj) m_pObj->Release();
	m_pObj = NULL;
	if (m_pBone) m_pBone->Release();
	m_pBone = NULL;

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (m_ppDecks[i]) m_ppDecks[i]->Release();
	memset(m_ppDecks, 0, sizeof(m_ppDecks));
	for (AVULONG i = 0; i < MAX_DOORS; i++)
		if (m_ppDoors[i]) m_ppDoors[i]->Release();
	memset(m_ppDoors, 0, sizeof(m_ppDoors));

	if (m_pStoreyBones == NULL) return;
	for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
	{
		if (m_pStoreyBones[i].pBone) m_pStoreyBones[i].pBone->Release();
		if (m_pStoreyBones[i].pObj)  m_pStoreyBones[i].pObj->Release();
		for (AVULONG j = 0; j < MAX_DOORS; j++)
			if (m_pStoreyBones[i].ppDoors[j]) m_pStoreyBones[i].ppDoors[j]->Release();
	}
	delete [] m_pStoreyBones;
}

void CBuilding::Construct(AVSTRING pLabel, AVVECTOR v)
{
	ASSERT(GetRenderer() && GetScene());
	if (!GetRenderer() || !GetScene()) return;

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
		if (GetLiftObj(i))	GetLiftObj(i)->PushState();
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
		if (GetLiftObj(i))	{ GetLiftObj(i)->PopState(); GetLiftObj(i)->Invalidate(); GetLiftObj(i)->PushState(); }
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

