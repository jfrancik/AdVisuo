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
	m_pObjLifts = NULL;
	m_pBoneLifts = NULL;
	for (AVULONG i = 0; i < DECK_NUM; i++)
		m_pBoneLiftDecks[i] = NULL;
	m_pBoneLiftLDoors = NULL;
	m_pBoneLiftRDoors = NULL;

	memset(m_pMaterials, 0, sizeof(m_pMaterials));
}

CBuilding::~CBuilding(void)
{
	DeconstructMaterials();
	Deconstruct();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
}

CBuilding::STOREY::STOREY() : m_pBone(NULL), m_pObj(NULL), m_pBoneLDoors(NULL), m_pBoneRDoors(NULL)

{
}

CBuilding::STOREY::~STOREY()
{
}

AVFLOAT CBuilding::GetLiftZPos(int nLift)
{
	FWVECTOR vec = { 0, 0, 0 };
	GetLiftNode(nLift)->LtoG(&vec);
	return vec.z;
}

//AVULONG CBuilding::GetLiftStorey(int nLift)
//{
//	return 0;
//}
//
//AVULONG CBuilding::GetLiftDoorPos(int nLift)
//{
//	return 0;
//}

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

void CBuilding::STOREY::ConstructWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, 
					AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, IKineNode **ppBone)
{
	CBlock block;
	block.Open(GetObj(), GetNode(), __name(strName, LOWORD(nIndex), HIWORD(nIndex)), l, h, d, vecPos, vecRot.x, vecRot.y, vecRot.z);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	if (nWallId != MAT_FLOOR_NUMBER_PLATE)
	{
		block.BuildWall();
		block.BuildRearSection();
	}
	block.SetMaterial(GetBuilding()->GetMaterial(nWallId, LOWORD(nIndex)));
	if (ppBone) *ppBone = block.GetBone();
	block.Close();
}

void CBuilding::STOREY::Construct(AVLONG iStorey)
{
	iStorey -= GetBuilding()->GetBasementStoreyCount();

	// create skeletal structure (object & bone)
	ITransform *pT = NULL;
	GetBuilding()->GetScene()->NewObject(__name(L"Storey_%d", iStorey), &m_pObj);
	GetObj()->CreateChild(L"Storey", &m_pBone);
	GetNode()->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(0, 0, SL);
	GetNode()->PutLocalTransform(pT);
	pT->Release();

	// create arrays of lobby doors
	m_pBoneLDoors = new IKineNode*[GetBuilding()->GetShaftCount()];
	memset(m_pBoneLDoors, 0, sizeof(IKineNode*) * GetBuilding()->GetShaftCount());
	m_pBoneRDoors = new IKineNode*[GetBuilding()->GetShaftCount()];
	memset(m_pBoneRDoors, 0, sizeof(IKineNode*) * GetBuilding()->GetShaftCount());

	// collect door information
	std::vector<FLOAT> doordata;
	for (AVULONG i = 0; i < GetBuilding()->GetShaftCount(); i++)
	{
		AVFLOAT opn = 10;		// width of the opening around the door
		SHAFT *pShaft = GetBuilding()->GetShaft(i);
		if (i < GetBuilding()->GetShaftCount(0))
			doordata.push_back(pShaft->m_boxDoor.Left() - m_box.LeftExt() - opn);
		else
			doordata.push_back(m_box.RightExt() - pShaft->m_boxDoor.Right() - opn);
		doordata.push_back(pShaft->m_boxDoor.Width() + opn + opn);
		doordata.push_back(pShaft->m_boxDoor.Height() + opn);
	}

	AVVECTOR v;

	// build walls
	ConstructWall(MAT_FLOOR,   iStorey, L"Storey_%d_Floor",   m_box.LeftExtRearExtLower(), m_box.WidthExt(), m_box.LowerThickness(), m_box.DepthExt());
	ConstructWall(MAT_CEILING, iStorey, L"Storey_%d_Ceiling", m_box.LeftExtRearExtUpper(), m_box.WidthExt(), m_box.UpperThickness(), m_box.DepthExt());
	
	//if (m_box.FrontThickness() > 0)
	//	ConstructWall(MAT_REAR, iStorey, L"Storey_%d_RearWall",   m_box.LeftExtFrontLower(), m_box.WidthExt(), m_box.Height(), m_box.FrontThickness(), Vector(0),
	//		GetBuilding()->GetShaftCount(0), &doordata[0]);
	//if (m_box.RearThickness() > 0)
	//	ConstructWall(MAT_FRONT, iStorey, L"Storey_%d_FrontWall", m_box.RightExtRearLower(), m_box.WidthExt(), m_box.Height(), m_box.RearThickness(), Vector(F_PI),
	//			GetBuilding()->GetShaftCount(1), &doordata[3 * GetBuilding()->GetShaftCount(0)]);
	if (m_box.LeftThickness() > 0)
		ConstructWall(MAT_SIDE, iStorey, L"Storey_%d_LeftWall", m_box.LeftExtFrontLower(), m_box.Depth(), m_box.Height(), m_box.LeftThickness(), Vector(F_PI_2));
	if (m_box.RightThickness() > 0)
		ConstructWall(MAT_SIDE, iStorey, L"Storey_%d_RightWall", m_box.RightExtRearLower(), m_box.Depth(), m_box.Height(), m_box.RightThickness(), Vector(-F_PI_2));

	v = m_box.LeftFrontUpper() + Vector(1, m_box.Depth()/2+20, 0);
	ConstructWall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Left_Nameplate", v, 2, 40, 40, Vector(0, F_PI_2));
	v = m_box.RightRearUpper() + Vector(-1, -m_box.Depth()/2-20, 0);
	ConstructWall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Right_Nameplate", v, -2, 40, 40, Vector(-F_PI, -F_PI_2));


	// The Shafts
	CBlock block;
	// imposed parameters
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 10;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)

	for (AVULONG j = 0; j < GetBuilding()->GetShaftCount(); j++)
	{
		SHAFT *pShaft = GetBuilding()->GetShaft(j);
		SHAFT *pPrevShaft = (j != 0 && j != GetBuilding()->GetShaftCount(0)) ? GetBuilding()->GetShaft(j-1) : GetBuilding()->GetShaft(j);
		AVFLOAT fBeamLen;
				
		switch (pShaft->ShaftLine)
		{
		case 0:
			{
			fBeamLen = max(-pShaft->m_box.DepthRWall(), -pPrevShaft->m_box.DepthRWall());
			ConstructWall(MAT_SHAFT1, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Beam", pShaft->m_box.LeftFrontLower(), fBeamLen, SH, pShaft->m_box.LeftThickness(), Vector(-F_PI_2));
			ConstructWall(MAT_SHAFT2, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Rear", pShaft->m_box.LeftRearLower(), pShaft->m_box.Width(), SH, -pShaft->m_box.RearThickness());
			if (j == GetBuilding()->GetShaftCount(0)-1)
				ConstructWall(MAT_SHAFT1, MAKELONG(iStorey, j), L"Storey_%d_Shaft_Right", pShaft->m_box.RightExtFrontLower(), -pShaft->m_box.DepthRWall(), SH, pShaft->m_box.RightThickness(), Vector(-F_PI_2));

			// The Opening
			AVFLOAT door[] = { opn, pShaft->m_boxDoor.Width(), pShaft->m_boxDoor.Height() };
			ConstructWall(MAT_OPENING, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Opening", 
				pShaft->m_boxDoor.LeftFrontLower() + Vector(-opn, GetBuilding()->m_box.FrontThickness()+bulge, 0),
				pShaft->m_boxDoor.Width()+opn+opn, pShaft->m_boxDoor.Height()+opn, GetBuilding()->m_box.FrontThickness()+bulge, 
				Vector(0), 1, door);

			ConstructWall(MAT_LIFT_NUMBER_PLATE, MAKELONG(j, iStorey), L"Shaft_%d_Storey_%d_Nameplate", pShaft->m_boxDoor.LeftFrontUpper() + Vector(pShaft->m_boxDoor.Width()/2-5, GetBuilding()->m_box.FrontThickness()+bulge+1, 10), 10, 10, 1, Vector(F_PI, 0, F_PI));

			ConstructWall(MAT_DOOR, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Door_Left" , pShaft->m_boxDoor.LeftRearLower(), pShaft->m_boxDoor.Width()/2, pShaft->m_boxDoor.Height(), pShaft->m_boxDoor.Depth(), Vector(0), 0, NULL, &m_pBoneLDoors[j]);
			ConstructWall(MAT_DOOR, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Door_Right", pShaft->m_boxDoor.RightRearUpper(), pShaft->m_boxDoor.Width()/2, pShaft->m_boxDoor.Height(), pShaft->m_boxDoor.Depth(), Vector(F_PI, F_PI), 0, NULL, &m_pBoneRDoors[j]);
			
			break;
			}

		case 1:
			{
			// Left Beam
			fBeamLen = max(pShaft->m_box.DepthRWall(), pPrevShaft->m_box.DepthRWall());
			ConstructWall(MAT_SHAFT1, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Beam", pShaft->m_box.LeftExtFrontLower(), fBeamLen, SH, pShaft->m_box.LeftThickness(), Vector(F_PI_2));
			ConstructWall(MAT_SHAFT2, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Rear", pShaft->m_box.LeftRearExtLower(), pShaft->m_box.Width(), SH, pShaft->m_box.RearThickness());
			if (j == GetBuilding()->GetShaftCount(0))
				ConstructWall(MAT_SHAFT1, MAKELONG(iStorey, j), L"Storey_%d_Shaft_Opp_Right", pShaft->m_box.RightFrontLower(), pShaft->m_box.DepthRWall(), SH, pShaft->m_box.RightThickness(), Vector(F_PI_2));

			// The Opening
			AVFLOAT door[] = { opn, pShaft->m_boxDoor.Width(), pShaft->m_boxDoor.Height() };
			ConstructWall(MAT_OPENING, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Opening", 
				pShaft->m_boxDoor.LeftFrontLower() + Vector(-opn, 0, 0), 
				pShaft->m_boxDoor.Width()+opn+opn, pShaft->m_boxDoor.Height()+opn, GetBuilding()->m_box.FrontThickness()+bulge, 
				Vector(0), 1, door);

			ConstructWall(MAT_LIFT_NUMBER_PLATE, MAKELONG(j, iStorey), L"Shaft_%d_Storey_%d_Nameplate", pShaft->m_boxDoor.LeftFrontUpper() + Vector(pShaft->m_boxDoor.Width()/2+5, -GetBuilding()->m_box.FrontThickness()-bulge-1, 10), 10, 10, 1, Vector(0, 0, F_PI));

			ConstructWall(MAT_DOOR, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Door_Left" , pShaft->m_boxDoor.LeftFrontLower(), pShaft->m_boxDoor.Width()/2, pShaft->m_boxDoor.Height(), pShaft->m_boxDoor.Depth(), Vector(0), 0, NULL, &m_pBoneLDoors[j]);
			ConstructWall(MAT_DOOR, MAKELONG(iStorey, j), L"Storey_%d_Shaft_%d_Door_Right", pShaft->m_boxDoor.RightFrontUpper(), pShaft->m_boxDoor.Width()/2, pShaft->m_boxDoor.Height(), pShaft->m_boxDoor.Depth(), Vector(F_PI, F_PI), 0, NULL, &m_pBoneRDoors[j]);

			break;
			}
		}
	}

	GetObj()->Invalidate();
}

void CBuilding::STOREY::Deconstruct()
{
	if (m_pBone) m_pBone->Release();
	m_pBone = NULL;
	if (m_pObj) 
	{
		m_pObj->Release();
		m_pObj = NULL;
	}

	for (AVULONG i = 0; i < GetBuilding()->GetShaftCount(); i++)
	{
		if (m_pBoneLDoors && m_pBoneLDoors[i]) m_pBoneLDoors[i]->Release();
		if (m_pBoneRDoors && m_pBoneRDoors[i]) m_pBoneRDoors[i]->Release();
	}
	if (m_pBoneLDoors) delete [] m_pBoneLDoors; m_pBoneLDoors = NULL;
	if (m_pBoneRDoors) delete [] m_pBoneRDoors; m_pBoneRDoors = NULL;

}

void CBuilding::Construct(AVSTRING pLabel, AVVECTOR v)
{
	ASSERT(GetRenderer() && GetScene());
	if (!GetRenderer() || !GetScene()) return;


	if (GetStoreyCount() == 0) return;

	// imposed parameters
	AVFLOAT gap = 1.0f;			// gap between lift doors
	AVFLOAT opn = 10;			// width of the opening around the door
	AVFLOAT bulge = 2;			// bulge of the opening (above the wall)




	OLECHAR buf[257];
	ITransform *pT = NULL;

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Construct(i);

	// Create the Lifts
	ULONG nLifts = GetLiftCount();
	if (nLifts == 0)
		return;		// this should never happen
	// create arrays for: lift objects, bones, deck bones, doors
	m_pObjLifts = new ISceneObject*[nLifts];
	m_pBoneLifts = new IKineNode*[nLifts];
	for (AVULONG i = 0; i < DECK_NUM; i++)
	{
		m_pBoneLiftDecks[i] = new IKineNode*[nLifts];
		memset(m_pBoneLiftDecks[i], 0, nLifts*sizeof(IKineNode*));
	}
	m_pBoneLiftLDoors = new IKineNode*[nLifts];
	m_pBoneLiftRDoors = new IKineNode*[nLifts];

	// The Lifts
	for (AVULONG i = 0; i < nLifts; i++)
	{
//		SHAFT *pShaft = GetShaft(i < 5 ? 0 : 1);
		SHAFT *pShaft = GetShaft(i);

		AVULONG nDecks = (pShaft->TypeOfLift == LIFT_DOUBLE_DECK) ? DECK_NUM : 1;

		// Create skeletal elements (entire lift)
		_snwprintf(buf, 256, L"Lift_%d", i);
		GetScene()->NewObject(buf, &m_pObjLifts[i]);
		m_pObjLifts[i]->CreateChild(L"Lift", &m_pBoneLifts[i]);

		// move to the shaft
		m_pBoneLifts[i]->CreateCompatibleTransform(&pT);
//		pT->FromTranslationVector((FWVECTOR*)(&GetLiftPos(0, 1)));
		pT->FromTranslationVector((FWVECTOR*)(&GetLiftPos(i, 0)));
		m_pBoneLifts[i]->PutLocalTransform(pT);
		pT->Release();


		for (AVULONG iDeck = 0; iDeck < nDecks; iDeck++)
		{
			// Create skeletal elements (the deck)
			_snwprintf(buf, 256, L"Deck_%d", iDeck);
			m_pBoneLifts[i]->CreateChild(buf, &m_pBoneLiftDecks[iDeck][i]);

			// elevate the deck (if applicable)
			m_pBoneLiftDecks[iDeck][i]->CreateCompatibleTransform(&pT);
			pT->FromTranslationXYZ(0, 0, GetStorey(iDeck)->SL);
			m_pBoneLiftDecks[iDeck][i]->PutLocalTransform(pT);
			pT->Release();

			CBlock block;
			
			// Floor
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_Floor", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.WidthExt(), pShaft->m_boxCar.LowerThickness(), pShaft->m_boxCar.DepthExt(), pShaft->m_boxCar.LeftExtRearExtLowerExt() - pShaft->m_boxCar, 0);
			block.BuildSimpleBlock();
			block.SetMaterial(GetMaterial(MAT_LIFT_FLOOR));
			block.Close();

			// Ceiling
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_Ceiling", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.WidthExt(), pShaft->m_boxCar.LowerThickness(), pShaft->m_boxCar.DepthExt(), pShaft->m_boxCar.LeftExtRearExtUpper() - pShaft->m_boxCar, 0);
			block.BuildSimpleBlock();
			block.SetMaterial(GetMaterial(MAT_LIFT_CEILING));
			block.Close();

			// find out the height of the walls
			AVFLOAT fHeight = (iDeck == nDecks-1) ? pShaft->m_boxCar.Height() : GetStorey(iDeck)->SH;

			// Front Wall
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_FrontWall", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.WidthExt(), fHeight, pShaft->m_boxCar.FrontThickness(), pShaft->m_boxCar.LeftExtFrontLower() - pShaft->m_boxCar, 0);
			block.BuildFrontSection();
			block.BuildWall((pShaft->m_boxCar.Width() - pShaft->m_boxDoor.Width()) / 2 + pShaft->m_boxCar.LeftThickness());
			block.BuildDoor(pShaft->m_boxDoor.Width(), pShaft->m_boxDoor.Height());
			block.BuildWall();
			block.BuildRearSection();
			block.SetMaterial(GetMaterial(MAT_LIFT));
			block.Close();

			// Rear Wall
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RearWall", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.WidthExt(), fHeight, pShaft->m_boxCar.FrontThickness(), pShaft->m_boxCar.LeftExtRearExtLower() - pShaft->m_boxCar + Vector(0, 1.0f, 0), 0);
			block.BuildSimpleBlock();
			block.SetMaterial(GetMaterial(MAT_LIFT));
			block.Close();

			// Left Wall
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_LeftWall", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.Depth(), fHeight, pShaft->m_boxCar.LeftThickness(), pShaft->m_boxCar.LeftExtFrontLower() - pShaft->m_boxCar, F_PI_2);
			block.BuildSimpleBlock();
			block.SetMaterial(GetMaterial(MAT_LIFT));
			block.Close();

			// Right Wall
			_snwprintf(buf, 256, L"Lift_%d_Deck_%d_RightWall", i, iDeck);
			block.Open(m_pObjLifts[i], m_pBoneLiftDecks[iDeck][i], buf, pShaft->m_boxCar.Depth(), fHeight, pShaft->m_boxCar.LeftThickness(), pShaft->m_boxCar.RightFrontLower() - pShaft->m_boxCar, F_PI_2);
			block.BuildSimpleBlock();
			block.SetMaterial(GetMaterial(MAT_LIFT));
			block.Close();

			// Doors - need to be built yet...
			m_pBoneLiftLDoors[i] = NULL;
			m_pBoneLiftRDoors[i] = NULL;

			m_pObjLifts[i]->Invalidate();
		}
	}
}

void CBuilding::Deconstruct()
{
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();

	for (AVULONG i = 0; i < GetLiftCount(); i++)
	{
		if (m_pBoneLifts && m_pBoneLifts[i]) m_pBoneLifts[i]->Release();
		for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
			if (m_pBoneLiftDecks[iDeck] && m_pBoneLiftDecks[iDeck][i]) m_pBoneLiftDecks[iDeck][i]->Release();
		if (m_pObjLifts  && m_pObjLifts[i]) 
			m_pObjLifts[i]->Release();
		if (m_pBoneLiftLDoors && m_pBoneLiftLDoors[i]) m_pBoneLiftLDoors[i]->Release();
		if (m_pBoneLiftRDoors && m_pBoneLiftRDoors[i]) m_pBoneLiftRDoors[i]->Release();
	}
	if (m_pBoneLifts) delete [] m_pBoneLifts; m_pBoneLifts = NULL;
	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
		if (m_pBoneLiftDecks[iDeck]) { delete [] m_pBoneLiftDecks[iDeck]; m_pBoneLiftDecks[iDeck] = NULL; }
	if (m_pObjLifts) delete [] m_pObjLifts; m_pObjLifts = NULL;
	if (m_pBoneLiftLDoors) delete [] m_pBoneLiftLDoors; m_pBoneLiftLDoors = NULL;
	if (m_pBoneLiftRDoors) delete [] m_pBoneLiftRDoors; m_pBoneLiftRDoors = NULL;
}

void CBuilding::StoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		if (GetLiftObj(i))	GetLiftObj(i)->PushState();
		if (GetLDoor(i))	GetLDoor(i)->PushState();
		if (GetRDoor(i))	GetRDoor(i)->PushState();
	}
	for (FWULONG i = 0; i < GetShaftCount(); i++)
		for (FWULONG j = 0; j < GetStoreyCount(); j++)
		{
			if (GetExtLDoor(i, j)) GetExtLDoor(i, j)->PushState();
			if (GetExtRDoor(i, j)) GetExtRDoor(i, j)->PushState();
		}
}

void CBuilding::RestoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
	{
		if (GetLiftObj(i))	{ GetLiftObj(i)->PopState(); GetLiftObj(i)->Invalidate(); GetLiftObj(i)->PushState(); }
		if (GetLDoor(i))	{ GetLDoor(i)->PopState(); GetLDoor(i)->Invalidate(); GetLDoor(i)->PushState(); }
		if (GetRDoor(i))	{ GetRDoor(i)->PopState(); GetRDoor(i)->Invalidate(); GetRDoor(i)->PushState(); }
	}
	for (FWULONG i = 0; i < GetShaftCount(); i++)
		for (FWULONG j = 0; j < GetStoreyCount(); j++)
		{
			if (GetExtLDoor(i, j)) { GetExtLDoor(i, j)->PopState(); GetExtLDoor(i, j)->Invalidate(); GetExtLDoor(i, j)->PushState(); }
			if (GetExtRDoor(i, j)) { GetExtRDoor(i, j)->PopState(); GetExtRDoor(i, j)->Invalidate(); GetExtRDoor(i, j)->PushState(); }
		}
}

