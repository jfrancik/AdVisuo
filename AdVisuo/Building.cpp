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

void CBldObject::AddWall(BONE pBone, AVULONG nWallId, AVLONG nIndex, AVSTRING strName, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
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

void CBldObject::AddWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData, BONE *ppBone)
{
	AddWall(GetBone(), nWallId, nIndex, strName, vecPos, l, h, d, vecRot, nDoorNum, pDoorData, ppBone);
}

void CBldObject::AddWall(AVULONG nWallId, AVLONG nIndex, AVSTRING strName, BOX box, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData, BONE *ppBone)
{
	AddWall(nWallId, nIndex, strName, box.LeftFrontLower(), box.Width(), box.Height(), box.Depth(), vecRot, nDoorNum, pDoorData, ppBone);
}

IMesh *CBldObject::AddMesh(AVSTRING strName)
{
	IMesh *pMesh = NULL;
	GetFWObject()->NewMesh(strName, &pMesh);
	pMesh->Open(NULL, NULL);
	pMesh->SupportNormal(0);
	return pMesh;
}

void CBldObject::Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale, AVFLOAT fTexScale)
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

CBuilding::CBuilding(void) : CBuildingBase(), m_materials(this), m_pRenderer(NULL), m_pScene(NULL)
{
	memset(m_pMaterials, 0, sizeof(m_pMaterials));
	bFastLoad = true;
}

CBuilding::~CBuilding(void)
{
	DeconstructMaterials();
	Deconstruct();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
}

AVVECTOR CBuilding::GetLiftPos(int nLift)
{
	AVVECTOR vec = { 0, 0, 0 };
	if (GetLiftObject(nLift).GetBone())
		GetLiftObject(nLift).GetBone()->LtoG((FWVECTOR*)&vec);
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

void CBuilding::STOREY::Construct(AVLONG iStorey)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

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
	GetObject().AddWall(MAT_FLOOR,   iStorey, L"Storey_%d_Floor",   GetBox().LeftExtRearExtLower(), GetBox().WidthExt(), GetBox().LowerThickness(), GetBox().DepthExt());
	GetObject().AddWall(MAT_CEILING, iStorey, L"Storey_%d_Ceiling", GetBox().LeftExtRearExtUpper(), GetBox().WidthExt(), GetBox().UpperThickness(), GetBox().DepthExt());

	if (GetBuilding()->bFastLoad) return;
	
	v = GetBox().LeftFrontUpper() + Vector(1, GetBox().Depth()/2+40, 0);
	GetObject().AddWall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Left_Nameplate", v, 2, 80, 40, Vector(0, F_PI_2));
	v = GetBox().RightRearUpper() + Vector(-1, -GetBox().Depth()/2-40, 0);
	GetObject().AddWall(MAT_FLOOR_NUMBER_PLATE, iStorey, L"Storey_%d_Right_Nameplate", v, -2, 80, 40, Vector(-F_PI, -F_PI_2));

//	GetObject().AddWall(MAT_FRONT, iStorey+1, L"Storey_%d_Cube",   Vector(-160, 20, 25), 40, 40, 40);
//	GetObject().AddWall(MAT_FRONT, iStorey+2, L"Storey_%d_ExtraWall",	GetBox().CentreRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(-F_PI/2),
//			GetBuilding()->GetShaftCount(1), GetBuilding()->GetShaftCount(1) ? &doordata[3 * GetBuilding()->GetShaftCount(0)] : NULL);

	if (GetBox().FrontThickness() > 0)
		GetObject().AddWall(MAT_REAR, iStorey, L"Storey_%d_RearWall",		GetBox().LeftExtFrontLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().FrontThickness(), Vector(0),
			GetBuilding()->GetShaftCount(0), &doordata[0]);
	if (GetBox().RearThickness() > 0)
		GetObject().AddWall(MAT_FRONT, iStorey, L"Storey_%d_FrontWall",	GetBox().RightExtRearLower(), GetBox().WidthExt(), GetBox().Height(), GetBox().RearThickness(), Vector(F_PI),
				GetBuilding()->GetShaftCount(1), GetBuilding()->GetShaftCount(1) ? &doordata[3 * GetBuilding()->GetShaftCount(0)] : NULL);
	if (GetBox().LeftThickness() > 0)
		GetObject().AddWall(MAT_SIDE, iStorey, L"Storey_%d_LeftWall", GetBox().LeftExtFrontLower(), GetBox().Depth(), GetBox().Height(), GetBox().LeftThickness(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		GetObject().AddWall(MAT_SIDE, iStorey, L"Storey_%d_RightWall", GetBox().RightExtRearLower(), GetBox().Depth(), GetBox().Height(), GetBox().RightThickness(), Vector(-F_PI_2));
}

void CBuilding::STOREY::Deconstruct()
{
	m_obj.Deconstruct();
}

void CBuilding::SHAFT::Construct(AVLONG iStorey, AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

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

	if (GetBuilding()->bFastLoad) return;

	if (GetBoxBeam().Width() > 0)
	{
		GetObject(iStorey).AddWall(MAT_BEAM, nIndex, L"Storey_%d_Shaft_%d_Beam", GetBoxBeam().LeftFrontLower(), GetBoxBeam().Width(), -GetBoxBeam().Height(), -GetBoxBeam().Depth());
		GetObject(iStorey).AddWall(MAT_SHAFT, nIndex, L"Storey_%d_Shaft_%d_BmRr", GetBoxBeam().LeftRearLower(), GetBoxBeam().Width(), GetBox().Height(), -GetBox().RearThickness());
	}

	if (GetBox().LeftThickness() > 0)
		GetObjectLeft(iStorey).AddWall(MAT_SHAFT, nIndex, L"Storey_%d_Shaft_%d_Lt", GetLeftWallBox(), Vector(F_PI_2));
	if (GetBox().RightThickness() > 0)
		GetObjectRight(iStorey).AddWall(MAT_SHAFT, nIndex, L"Storey_%d_Shaft_%d_Rt", GetRightWallBox(), Vector(F_PI_2));
	if (GetBox().RearThickness() != 0)
		GetObject(iStorey).AddWall(MAT_SHAFT, nIndex, L"Storey_%d_Shaft_%d_Rr", GetBox().LeftExtRearLower(), GetBox().WidthExt(), GetBox().Height(), -GetBox().RearThickness());

	// The Opening
	AVFLOAT door[] = { opn, GetBoxDoor().Width(), GetBoxDoor().Height() };
	GetObjectLobbySide(iStorey).AddWall(MAT_OPENING, nIndex, L"Storey_%d_Shaft_%d_Opening", 
		GetBoxDoor().LeftRearLower() + Vector(-opn, 0, 0), 
		GetBoxDoor().Width() + opn + opn, GetBoxDoor().Height() + opn, fOpeningThickness,
		Vector(0), 1, door);

	// Plates
	if (GetShaftLine() == 0)
		GetObjectLobbySide(iStorey).AddWall(MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(-5, 0, 15), 10, 10, 0.5, Vector(F_PI, 0, F_PI));
	else
		GetObjectLobbySide(iStorey).AddWall(MAT_LIFT_NUMBER_PLATE, MAKELONG(iShaft, iStorey), L"Shaft_%d_Storey_%d_Nameplate", 
			GetBoxDoor().CentreFrontUpper() + Vector(5, 0, 15), 10, 10, 0.5, Vector(0, 0, F_PI));

	// Door
	GetObjectLobbySide(iStorey).AddWall(MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Left" , GetBoxDoor().LeftRearLower(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(0), 0, NULL, &m_pStoreyBones[iStorey].pDoors[0]);
	GetObjectLobbySide(iStorey).AddWall(MAT_DOOR, MAKELONG(iStorey, iShaft), L"Storey_%d_Shaft_%d_Door_Right", GetBoxDoor().RightRearUpper(), GetBoxDoor().Width()/2, GetBoxDoor().Height(), fDoorThickness, Vector(F_PI, F_PI), 0, NULL, &m_pStoreyBones[iStorey].pDoors[1]);
}

void CBuilding::SHAFT::Deconstruct()
{
	if (m_pStoreyBones == NULL) return;
	for (AVULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
	{
		m_pStoreyBones[i].m_obj.Deconstruct();
		m_pStoreyBones[i].m_objLobbySide.Deconstruct();
		m_pStoreyBones[i].m_objLeft.Deconstruct();
		m_pStoreyBones[i].m_objRight.Deconstruct();
		for (AVULONG j = 0; j < MAX_DOORS; j++)
			if (m_pStoreyBones[i].pDoors[j]) m_pStoreyBones[i].pDoors[j]->Release();
	}
	delete [] m_pStoreyBones;
	m_pStoreyBones = NULL;
}

void CBuilding::LIFT::Construct(AVULONG iShaft)
{
	if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) < 0 && !GetBuilding()->bFastLoad) { GetBuilding()->bFastLoad = true; MessageBeep(MB_OK); }

	// Create skeletal elements (entire lift)
	m_obj.Create(__name(L"Lift_%d", iShaft), GetShaft()->GetLiftPos(0 /*+ iShaft % 4*/));

	for (AVULONG iDeck = 0; iDeck < GetShaft()->GetDeckCount(); iDeck++)
	{
		// Create skeletal elements (the deck)
		m_pDecks[iDeck] = m_obj.AddBone(__name(L"Deck_%d", iDeck), 0, 0, GetBuilding()->GetGroundStorey(iDeck)->GetLevel() - GetBuilding()->GetGroundStorey()->GetLevel()); 

		AVULONG nIndex = MAKELONG(iShaft, iDeck);
		BOX box = GetShaft()->GetBoxCar() - GetShaft()->GetLiftPos(0);
		BOX boxDoor0 = GetShaft()->GetBoxCarDoor(0) - GetShaft()->GetLiftPos(0);
		AVFLOAT door[] = { boxDoor0.Left() - box.LeftExt(), boxDoor0.Width(), boxDoor0.Height() };
		AVFLOAT fDoorThickness0 = boxDoor0.Depth() * 0.4f;

		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT_FLOOR, iShaft, L"Lift_%d_Deck_%d_Floor", box.LeftExtRearExtLowerExt(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT_CEILING, iShaft, L"Lift_%d_Deck_%d_Ceiling", box.LeftExtRearExtUpper(), box.WidthExt(), box.LowerThickness(), box.DepthExt());
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT_DOOR, iShaft, L"Lift_%d_Deck_%d_Door1", boxDoor0.LeftFrontLower(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(0), 0, NULL, &m_pDoors[0]);
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT_DOOR, iShaft, L"Lift_%d_Deck_%d_Door2", boxDoor0.RightFrontUpper(), boxDoor0.Width()/2, boxDoor0.Height(), -fDoorThickness0, Vector(F_PI, F_PI), 0, NULL, &m_pDoors[1]);
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_FrontWall", box.LeftExtFrontLower(), box.WidthExt(), box.Height(), box.FrontThickness(), Vector(0), 1, door);
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_RearWall", box.RightExtRearLower(), box.WidthExt(), box.Height(), box.RearThickness(), Vector(F_PI));
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_LeftWall", box.LeftRearLower(), box.Depth(), box.Height(), box.LeftThickness(), Vector(-F_PI_2));
		m_obj.AddWall(m_pDecks[iDeck], MAT_LIFT, iShaft, L"Lift_%d_Deck_%d_RightWall", box.RightFrontLower(), box.Depth(), box.Height(), box.RightThickness(), Vector(F_PI_2));
	}
}

void CBuilding::LIFT::Deconstruct()
{
	m_obj.Deconstruct();

	for (AVULONG i = 0; i < DECK_NUM; i++)
		if (m_pDecks[i]) m_pDecks[i]->Release();
	memset(m_pDecks, 0, sizeof(m_pDecks));
	for (AVULONG i = 0; i < MAX_DOORS; i++)
		if (m_pDoors[i]) m_pDoors[i]->Release();
	memset(m_pDoors, 0, sizeof(m_pDoors));
}

void CBuilding::Construct()
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

	for (AVULONG iLift = 0; iLift < GetLiftCount(); iLift++)
		GetLift(iLift)->Construct(iLift);

	for (AVULONG iStorey = 0; iStorey < GetStoreyCount(); iStorey++)
	{
		GetStorey(iStorey)->Construct(iStorey);
		for (AVULONG iShaft = 0; iShaft < GetShaftCount(); iShaft++)
			GetShaft(iShaft)->Construct(iStorey, iShaft);
	}
}

void CBuilding::Deconstruct()
{
	for (AVULONG i = 0; i < GetStoreyCount(); i++)
		GetStorey(i)->Deconstruct();
	for (AVULONG i = 0; i < GetShaftCount(); i++)
		GetShaft(i)->Deconstruct();
	for (AVULONG i = 0; i < GetLiftCount(); i++)
		GetLift(i)->Deconstruct();
}

void CBuilding::StoreConfig()
{
	for (FWULONG i = 0; i < GetLiftCount(); i++)
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
	for (FWULONG i = 0; i < GetLiftCount(); i++)
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

