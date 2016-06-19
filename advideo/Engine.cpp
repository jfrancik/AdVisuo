// Sprite.cpp

#include "StdAfx.h"
#include "Engine.h"
//#include "DlgHtRepBug.h"

namespace fw
{
	#include <freewill.h>
	#include <fwrender.h>
	#include <fwaction.h>

	#include "freewill.c"			// #FreeWill: Obligatory!
	#include "freewilltools.h"
};
using namespace fw;

#include <D3d9.h>

#pragma warning (disable:4995)
#pragma warning (disable:4996)
 
#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

CEngine::CEngine()
{
	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pBiped = NULL;
	m_pMaterial = NULL;
	m_pBipedBuf = NULL;
	m_nBipedBufCount = 0;
	m_pLight1 = NULL;
	m_pLight2 = NULL;
	m_pActionTick = NULL;
	m_pAuxActionTick = NULL;
	m_nAuxTimeRef = -1;

	m_pfps = new DWORD[c_fpsNUM];
	memset(m_pfps, 0, sizeof(DWORD) * c_fpsNUM);
	m_nfps = 0;
}

CEngine::~CEngine()
{
	ReleaseAllMats();
	ReleaseFloorPlateMats();
	ReleaseLiftPlateMats();
	remove_all();
	delete [] m_pfps;
	if (m_pAuxActionTick) m_pAuxActionTick->UnSubscribeAll();
	if (m_pAuxActionTick) m_pAuxActionTick->Release();
	if (m_pActionTick) m_pActionTick->UnSubscribeAll();
	if (m_pActionTick) m_pActionTick->Release();
	if (m_pLight2) m_pLight2->Release();
	if (m_pLight1) m_pLight1->Release();
	if (m_pBipedBuf) delete [] m_pBipedBuf;
	m_nBipedBufCount = 0;
	if (m_pBiped) m_pBiped->Release();
	if (m_pMaterial) m_pMaterial->Release();
	if (m_pScene) m_pScene->DelAll();
	if (m_pScene) m_pScene->Release();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pFWDevice) m_pFWDevice->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Engine Creation

static void _on_lost_device (IRndrGeneric*, FWULONG, void *pParam)	{ ((ILostDeviceObserver*)pParam)->OnLostDevice(); 	}
static void _on_reset_device(IRndrGeneric*, FWULONG, void *pParam)	{ ((ILostDeviceObserver*)pParam)->OnResetDevice(); }

bool g_bFullScreen = false;
bool g_bReenter = false;
#define MB_CANCELTRYCONTINUE        0x00000006L
#define IDTRYAGAIN      10
#define IDCONTINUE      11
HRESULT __stdcall HandleErrors(struct FWERROR *p, BOOL bRaised)
{
	if (!bRaised)
	{
		TRACE("Last error recovered\n");
		return S_OK;
	}

	FWSTRING pLabel = NULL;
	if (p->pSender)
	{
		IKineChild *pChild;
		if (SUCCEEDED(p->pSender->QueryInterface(&pChild)) && pChild)
		{
			pChild->GetLabel(&pLabel);
			pChild->Release();
		}
	}

	CString str;
	if (pLabel)
		str.Format(L"%ls(%d): Error 0x%x (class %ls, object %ls), %ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, pLabel, p->pMessage);
	else
		str.Format(L"%ls(%d): Error 0x%x (class %ls), %ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, p->pMessage);
	TRACE(L"******** ERROR ********\n** %s\n", str);
	if (!g_bFullScreen && !g_bReenter)
	{
		g_bReenter = true;

		int nRes = AfxMessageBox(str, MB_ABORTRETRYIGNORE | MB_DEFBUTTON3);

//		CDlgHtRepBug dlg(str);
//		dlg.m_cat = 1;
//		dlg.DoModal();
			
		switch (nRes)
		{
		case IDABORT: FatalAppExit(0, L"Application stopped"); break;
		case IDRETRY: DebugBreak(); break;
		case IDIGNORE: break;
		}
		g_bReenter = false;
	}
	return p->nCode;
}

bool CEngine::Create(HWND hWnd, ILostDeviceObserver *pLDO)
{
	if (IsReady())
		return true;	// nothing to do!

	enum eError { ERROR_FREEWILL, ERROR_DIRECTX, ERROR_INTERNAL_ };
	try
	{
		// #FreeWill: create the FreeWill device
		OutText(L"Initialising FreeWill+ system...");
		HRESULT h;
		h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
		if (FAILED(h)) throw ERROR_FREEWILL;

		// #FreeWill: create & initialise the renderer
		h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
		if (FAILED(h)) throw ERROR_DIRECTX;
		h = m_pRenderer->InitDisplay(hWnd, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		if (FAILED(h)) throw ERROR_INTERNAL_;

	//	FWCOLOR back = { 0.0f, 0.0f, 0.0f };		// black
	// 	FWCOLOR back = { 0.56f, 0.68f, 0.83f };		// blue
	//	FWCOLOR back = { 0.33f, 0.33f, 0.33f };		// gray
		FWCOLOR back = { 0.33f, 0.33f, 0.90f };		// gray
		m_pRenderer->PutBackColor(back);

		// #FreeWill: create & initialise the buffers - determine hardware factors
		IMeshVertexBuffer *pVertexBuffer;
		IMeshFaceBuffer *pFaceBuffer;
		h = m_pRenderer->GetBuffers(&pVertexBuffer, &pFaceBuffer); 
		if (FAILED(h)) throw ERROR_INTERNAL_;
		h = pVertexBuffer->Create(2000000, MESH_VERTEX_XYZ | MESH_VERTEX_NORMAL | MESH_VERTEX_BONEWEIGHT | MESH_VERTEX_TEXTURE, 4, 1);
		if (FAILED(h)) throw ERROR_INTERNAL_;
		h = pFaceBuffer->Create(2000000); if (FAILED(h)) throw ERROR_INTERNAL_;
		pVertexBuffer->Release();
		pFaceBuffer->Release();

		// #FreeWill: create & initialise the animation scene
		h = m_pFWDevice->CreateObject(L"Scene", IID_IScene, (IFWUnknown**)&m_pScene);
		if (FAILED(h)) throw ERROR_INTERNAL_;

		// #FreeWill: initialise the Tick Actions
		m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);
		m_pAuxActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);

		m_pScene->PutRenderer(m_pRenderer);

		// #Load the Scene
		OutText(L"Loading biped model...");
		IFileLoader *pLoader;
		m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);
		ISceneObject *pBip01 = NULL;
		m_pScene->NewObject(L"Bip01", &pBip01);
		pLoader->LoadObject((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"lobby.3D").c_str(), L"Bip01", pBip01);


		IKineChild *pFootsteps = NULL;
		pBip01->GetChild(L"Bip01.Footsteps", &pFootsteps);
		if (pFootsteps)
		{
			ITransform *pT = NULL;
			pFootsteps->GetLocalTransformRef(&pT);
			pT->MulScale(0, 0, 0);
			pT->Release();
			pFootsteps->Release();
		}

		pBip01->Release();
		pLoader->Release();

		// Reset Character Position
		IKineNode *pBody = NULL;
		if (SUCCEEDED(m_pScene->GetChild(L"Bip01.Bip01", (IKineChild**)&pBody)) && pBody)
		{
			ITransform *pT = NULL;
			pBody->GetBaseTransformRef(&pT);
			pT->Reset(FALSE, TRUE);					// reset translation stored in file
			pT->MulTranslationXYZ(0, 160, 37.15f);	// stand on the floor surface
			pT->Release();
			pBody->Invalidate();
			pBody->Release();
		}

		// initialise biped and create material (for spawning)
		m_pScene->GetChild(L"Bip01", &m_pBiped);
		m_pScene->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&m_pMaterial);
		if (m_pBiped)
		{
			m_pBiped->StoreState(0, NULL, &m_nBipedBufCount);
			m_pBipedBuf = new BYTE[m_nBipedBufCount];
			m_pBiped->StoreState(m_nBipedBufCount, m_pBipedBuf, NULL);
		}

		// setup lights
		m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight1);
		m_pScene->AddChild(L"DirLight1", m_pLight1);
		FWCOLOR cWhite1 = { 0.7f, 0.7f, 0.7f };
		m_pLight1->PutDiffuseColor(cWhite1);
		m_pLight1->Create(__FW_Vector(0.1f, -0.3f, -0.4f));

		m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight2);
		m_pScene->AddChild(L"DirLight2", m_pLight2);
		FWCOLOR cWhite2 = { 0.6f, 0.6f, 0.6f };
		m_pLight2->PutDiffuseColor(cWhite2);
		m_pLight2->Create(__FW_Vector(0, 1, 3));

		// Lost Device Callbacks
		if (pLDO)
		{
			m_pRenderer->SetCallback(FW_CB_LOSTDEVICE, _on_lost_device, 0, pLDO);
			m_pRenderer->SetCallback(FW_CB_RESETDEVICE, _on_reset_device, 0, pLDO);
		}
		OutText(L"FreeWill+ initialised successfully...");
	}
	catch (eError e)
	{
		CString str;
		switch (e)
		{
		case ERROR_FREEWILL: str = L"FreeWill Graphics system not found. Please reinstall the application."; break;
		case ERROR_DIRECTX:  str = L"The Direct3D renderer could not be initialised. Please update or re-install DirectX."; break;
		case ERROR_INTERNAL_: str = L"FreeWill Graphics System could not be initialised. Contact the Technical Support";  break;
		default:             str = L"Unidentified internal error occured. Contact the Technical Support";  break;
		}
		str += L" This is a fatal error and the application will now shut down.";
		AfxMessageBox(str, MB_OK | MB_ICONHAND);
//		CDlgHtRepBug dlg(str);
//		dlg.DoModal();
		return false;
	}

	// #FreeWill: set-up the error handler
	m_pFWDevice->SetUserErrorHandler(HandleErrors);
	m_pFWDevice->EnableErrorException(true);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DX Bridge

IDirect3DDevice9 *CEngine::GetDXDevice()
{
	IDirect3DDevice9 *pDevice = NULL;
	m_pRenderer->GetDeviceHandle(1, (FWHANDLE*)&pDevice);
	return pDevice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Status

bool CEngine::IsReady()					{ return m_pScene != NULL; }
bool CEngine::IsRunning()				{ return (m_pActionTick->AnySubscriptionsLeft() == TRUE); }
CSize CEngine::GetViewSize()			{ CSize size; m_pRenderer->GetViewSize((AVULONG*)&size.cx, (AVULONG*)&size.cy); return size; }
CSize CEngine::GetBackBufferSize()		{ CSize size; m_pRenderer->GetBackBufferSize((AVULONG*)&size.cx, (AVULONG*)&size.cy); return size; }
AVULONG CEngine::GeViewtWidth()			{ AVULONG x, y; m_pRenderer->GetViewSize(&x, &y); return x; }
AVULONG CEngine::GetViewHeight()		{ AVULONG x, y; m_pRenderer->GetViewSize(&x, &y); return y; }

CString CEngine::GetDiagnosticMessage()
{
	CString str;
	if (IsPlaying() && !IsPaused())
		str.Format(L"Visualisation playing now at %d.", GetPlayTime());
	else if (IsPlaying() && IsPaused())
		str.Format(L"Visualisation paused at %d.", GetPlayTime());
	else
		str.Format(L"Visualisation not started.");
	return str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Material manager

void CEngine::SetSolidMat(AVULONG i, AVCOLOR color, AVSTRING pLabel)
{
	if (pLabel) m_materials[i].m_pLabel = wcsdup(pLabel);
	m_materials[i].m_bSolid = true;
	m_materials[i].m_color = color;
}

void CEngine::SetSolidMat(AVULONG i, FWULONG r, FWULONG g, FWULONG b, FWULONG a, AVSTRING pLabel)
{
	AVCOLOR color = { (FWFLOAT)r / 256.0f, (FWFLOAT)g / 256.0f, (FWFLOAT)b / 256.0f, (FWFLOAT)a / 256.0f };
	SetSolidMat(i, color, pLabel);
}

void CEngine::SetTexturedMat(AVULONG i, AVSTRING pFName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha, AVSTRING pLabel)
{
	if (pLabel) m_materials[i].m_pLabel = wcsdup(pLabel);
	m_materials[i].m_bSolid = false;
	m_materials[i].m_pFName = wcsdup(pFName);
	m_materials[i].m_fUTile = fUTile;
	m_materials[i].m_fVTile = fVTile;
}

void CEngine::CreateMat(AVULONG i)
{
	if (i == MAT_BACKGROUND)
		return;		// reserved for the background, no actual material needed

	IMaterial *pMaterial = NULL;
	if (m_materials[i].m_pMaterial)
		pMaterial = m_materials[i].m_pMaterial;
	else
		m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);

	if (m_materials[i].m_bSolid)
	{
		pMaterial->SetDiffuseColor(_FWCOLOR(m_materials[i].m_color));
		pMaterial->SetAmbientColor(_FWCOLOR(m_materials[i].m_color));
		pMaterial->SetSpecularColor(_FWCOLOR(m_materials[i].m_color));
		pMaterial->SetSelfIlluminationOff();
		pMaterial->SetTwoSided(TRUE);
		pMaterial->SetAlphaMode(m_materials[i].m_color.a < 0.99 ? MAT_ALPHA_MATERIAL : MAT_ALPHA_DISABLE);
		pMaterial->SetCullingMode(m_materials[i].m_color.a < 0.99 ? MAT_CULLING_CCW_CW : MAT_CULLING_DISABLE);
	}
	else
	{
		if (!m_materials[i].m_pFName) return;
		ITexture *pTexture = NULL;
		m_pRenderer->CreateTexture(&pTexture);
		pTexture->LoadFromFile(m_materials[i].m_pFName);
		pTexture->SetUVTile(m_materials[i].m_fUTile, m_materials[i].m_fVTile);
		pMaterial->SetTexture(0, pTexture);
		pMaterial->SetSelfIlluminationOff();
		pMaterial->SetTwoSided(FALSE);
		if (m_materials[i].m_color.a) pMaterial->SetAlpha(m_materials[i].m_color.a);
		pMaterial->SetCullingMode(m_materials[i].m_color.a < 0.99 ? MAT_CULLING_CCW_CW : MAT_CULLING_DISABLE);
		pTexture->Release();
	}
	m_materials[i].m_pMaterial = pMaterial;
}

void CEngine::ReleaseMat(AVULONG i)
{
	if (m_materials[i].m_pMaterial) 
		m_materials[i].m_pMaterial->Release();
	m_materials[i].m_pMaterial = NULL;
}

void CEngine::CreateFloorPlateMats(AVULONG nStoreys, AVULONG nBasementStoreys)
{
	for (AVULONG i = 0; i < nStoreys; i++)
	{
		ITexture *pTexture = NULL;
		m_pRenderer->CreateTexture(&pTexture);
		pTexture->LoadFromFile(STD_PATH(L"plate_floor.bmp"));
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
		str.Format(L"%d", i - nBasementStoreys);
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
		m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
		pMaterial->SetTexture(0, pTexture);
		pTexture->Release();

		pMaterial->SetSelfIlluminationOff();
		pMaterial->SetTwoSided(FALSE);
	//	pMaterial->SetAlphaMode(MAT_ALPHA_TEXTURE);
	//	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);

		m_matFloorPlates.push_back(pMaterial);
	}
}

void CEngine::CreateLiftPlateMats(AVULONG nShafts)
{
	for (AVULONG i = 0; i < nShafts; i++)
	{
		ITexture *pTexture = NULL;
		m_pRenderer->CreateTexture(&pTexture);
		pTexture->LoadFromFile(STD_PATH(L"plate_lift.bmp"));
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
		str.Format(L"%c", 'A' + i);
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
		m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
		pMaterial->SetTexture(0, pTexture);
		pTexture->Release();

		pMaterial->SetSelfIlluminationOff();
		pMaterial->SetTwoSided(FALSE);
	//	if (fAlpha < 0.99) pMaterial->SetAlpha(fAlpha);

		m_matLiftPlates.push_back(pMaterial);
	}
}

void CEngine::ReleaseFloorPlateMats()
{ 
	for each (IMaterial *pMaterial in m_matFloorPlates) 
		pMaterial->Release(); 
}

void CEngine::ReleaseLiftPlateMats()
{ 
	for each (IMaterial *pMaterial in m_matLiftPlates) 
		pMaterial->Release(); 
}

IMaterial *CEngine::GetMat(AVULONG nWallId, AVLONG i)
{
	AVULONG LookUp[] = 
	{ MAT_LOBBY_0, MAT_LOBBY_0, MAT_LOBBY_0, MAT_CEILING, MAT_FLOOR,
		MAT_BEAM, MAT_SHAFT, MAT_OPENING, MAT_DOOR, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, MAT_LIFT_DOOR,
		MAT_PLATE_LIFT, MAT_PLATE_FLOOR, MAT_BEAM
	};

	AVULONG iMat = LookUp[nWallId];
	if (iMat == MAT_LOBBY_0)
		return m_materials[iMat + i % 8].m_pMaterial;
	else if (iMat == MAT_PLATE_LIFT)
		return i < (AVLONG)m_matLiftPlates.size() ? m_matLiftPlates[i] : m_materials[MAT_BACKGROUND].m_pMaterial;
	else if (iMat == MAT_PLATE_FLOOR)
		return i < (AVLONG)m_matFloorPlates.size() ? m_matFloorPlates[i] : m_materials[MAT_BACKGROUND].m_pMaterial;
	else
		return m_materials[iMat].m_pMaterial;
}

void CEngine::InitMats(AVULONG nStoreys, AVULONG nBasementStoreys, AVULONG nShafts)
{
	SetSolidMat(MAT_BACKGROUND,			80, 80, 80,	100,							L"Background");
	SetSolidMat(MAT_LOBBY_0,			0, 255, 0, 76,								L"Lobby walls 0");
	SetSolidMat(MAT_LOBBY_1,			255, 0, 0, 76,								L"Lobby walls 1");
	SetSolidMat(MAT_LOBBY_2,			255, 255, 0, 76,							L"Lobby walls 2");
	SetSolidMat(MAT_LOBBY_3,			0, 255, 255, 76,							L"Lobby walls 3");
	SetSolidMat(MAT_LOBBY_4,			255, 180, 0, 76,							L"Lobby walls 4");
	SetSolidMat(MAT_LOBBY_5,			203, 0, 255, 76,							L"Lobby walls 5");
	SetSolidMat(MAT_LOBBY_6,			255, 255, 154, 76,							L"Lobby walls 6");
	SetSolidMat(MAT_LOBBY_7,			0, 76, 255, 76,								L"Lobby walls 7");
	SetTexturedMat(MAT_FLOOR,			STD_PATH(L"floor3.jpg"),  1.0f, 1.0f, 1.0f,	L"Lobby floors");
	SetTexturedMat(MAT_CEILING,			STD_PATH(L"ceiling.jpg"), 2.0f, 2.0f, 1.0f,	L"Lobby ceilings");
	SetSolidMat(MAT_DOOR,				102, 76, 76, 128,							L"Shaft doors");
	SetSolidMat(MAT_LIFT_DOOR,			102, 76, 76, 128,							L"Lift doors");
	SetSolidMat(MAT_OPENING,			128, 128, 128, 255,							L"Openings");
	SetSolidMat(MAT_LIFT,				76, 80, 90, 76,								L"Lift walls");
	SetTexturedMat(MAT_LIFT_FLOOR,		STD_PATH(L"marble.jpg"), 3.0f, 3.0f, 1.0f,	L"Lift floors");
	SetTexturedMat(MAT_LIFT_CEILING,	STD_PATH(L"metal3.jpg"), 2.0f, 2.0f, 1.0f,	L"Lift ceilings");
	SetSolidMat(MAT_SHAFT,				102, 51, 0, 76,								L"Shaft walls");
	SetSolidMat(MAT_BEAM,				128, 128, 76, 100,							L"Division beams");

	CreateAllMats();
	CreateFloorPlateMats(nStoreys, nBasementStoreys);
	CreateLiftPlateMats(nShafts);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering cycle

void CEngine::BeginFrame()
{ 
	m_pRenderer->PutBackColor(_FWCOLOR(GetMatColor(MAT_BACKGROUND)));
	m_pRenderer->BeginFrame(); 
}

void CEngine::EndFrame()
{ 
	m_pRenderer->EndFrame(); 
	m_pfps[m_nfps] = GetTickCount();
	m_nfps = (m_nfps + 1) % c_fpsNUM;
}

void CEngine::Proceed(AVLONG nMSec)
{
	m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Auxiliary Player

void CEngine::AuxPlay(IAction **pAuxAction, AVULONG nClockValue)
{
	if (!pAuxAction) return;
	*pAuxAction = m_pAuxActionTick;
	m_pAuxActionTick->AddRef();

	m_pAuxActionTick->UnSubscribeAll();
	m_nAuxTimeRef = (nClockValue == 0x7FFFFFFF) ? ::GetTickCount() : nClockValue;
}

void CEngine::ProceedAux(FWLONG nMSec)
{
	if (m_pAuxActionTick && m_pAuxActionTick->AnySubscriptionsLeft() == TRUE)
		m_pAuxActionTick->RaiseEvent(nMSec - m_nAuxTimeRef, EVENT_TICK, nMSec - m_nAuxTimeRef, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Tools

bool CEngine::Play()						{ if (!IsReady()) return false; m_pRenderer->Play(); return true; }
bool CEngine::Play(AVLONG nMSec)			{ if (!IsReady()) return false; m_pRenderer->Play(); m_pRenderer->PutPlayTime(nMSec); return true; }
bool CEngine::Pause()						{ if (!IsReady()) return false; m_pRenderer->Pause(); return true; }
bool CEngine::Stop()						{ if (!IsReady()) return false; m_pRenderer->Stop(); m_pActionTick->UnSubscribeAll(); return true; }
bool CEngine::IsPlaying()					{ return IsReady() && (m_pRenderer->IsPlaying() == S_OK); }
bool CEngine::IsPaused()					{ return IsReady() && (m_pRenderer->IsPaused() == S_OK); }
AVLONG CEngine::GetPlayTime()				{ AVLONG nTime; if (!IsPlaying()) return 0; m_pRenderer->GetPlayTime(&nTime); return nTime; }
void CEngine::SetPlayTime(AVULONG nTime)	{ m_pRenderer->PutPlayTime(nTime); } 
AVULONG CEngine::GetFPS()					{ return m_pfps[m_nfps] ? 1000 * (c_fpsNUM-1) / (m_pfps[(m_nfps+c_fpsNUM-1)%c_fpsNUM] - m_pfps[m_nfps]) : 0; }
AVFLOAT CEngine::GetAccel()					{ AVFLOAT accel; m_pRenderer->GetAccel(&accel); return accel; }
void CEngine::PutAccel(AVFLOAT accel)		{ m_pRenderer->PutAccel(accel); }
void CEngine::SlowDown(AVFLOAT f)			{ PutAccel(GetAccel() / f); }
void CEngine::SpeedUp(AVFLOAT f)			{ PutAccel(GetAccel() * f); }
void CEngine::ResetAccel()					{ m_pRenderer->PutAccel(1); }

/////////////////////////////////////////////////////////////////////////////////////////////////
// Device Reset and off screen modes

void CEngine::ResetDevice(HWND hWnd)		{ m_pRenderer->ResetDeviceEx(hWnd, 0, 0); }

void CEngine::StartTargetToImage(CSize size, LPCTSTR pImgFile, enum BMP_FORMAT fmt)
{
	FW_RENDER_BITMAP conv[] = { RENDER_BMP, RENDER_JPG, RENDER_TGA, RENDER_PNG };
	m_pFWDevice->EnableErrorException(TRUE);
	m_pRenderer->InitOffScreen(size.cx, size.cy);
	m_pRenderer->OpenStillFile(pImgFile, conv[fmt]);
}

void CEngine::StartTargetToImage(CSize size, LPCTSTR pImgFile)
{
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	_wsplitpath(pImgFile, drive, dir, fname, ext);

	enum BMP_FORMAT fmt;
	if (_wcsicmp(ext, L".bmp") == 0) fmt = FORMAT_BMP;
	else if (_wcsicmp(ext, L".jpg") == 0) fmt = FORMAT_JPG;
	else if (_wcsicmp(ext, L".tga") == 0) fmt = FORMAT_TGA;
	else if (_wcsicmp(ext, L".png") == 0) fmt = FORMAT_PNG;
	else fmt = FORMAT_BMP;

	StartTargetToImage(size, pImgFile, fmt);
}

void CEngine::StartTargetToVideo(CSize sz, LPCTSTR pFilename, FWULONG nFPS, FWULONG nBitrate)
{
	m_pFWDevice->EnableErrorException(TRUE);
	m_pRenderer->InitOffScreen(sz.cx, sz.cy);
	m_pRenderer->OpenMovieFile(pFilename, nFPS, nBitrate);
}

void CEngine::SetTargetToScreen()			{ m_pRenderer->SetTargetToScreen(); }
void CEngine::SetTargetOffScreen()			{ m_pRenderer->SetTargetOffScreen(); }

void CEngine::DoneTargetOffScreen()
{
	m_pRenderer->CloseMovieFile();	// will only close the file if previously opened
	m_pRenderer->DoneOffScreen();
	m_pFWDevice->EnableErrorException(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport preparation

void CEngine::PrepareViewport(AVFLOAT fX, AVFLOAT fY, AVFLOAT fWidth, AVFLOAT fHeight, AVLONG nX, AVLONG nY, AVLONG nWidth, AVLONG nHeight, bool bClear)
{
	m_pRenderer->PutViewport(fX, fY, fWidth, fHeight, nX, nY, nWidth, nHeight);
	if (bClear)
		m_pRenderer->Clear();
}

void CEngine::PrepareViewport(AVFLOAT fX, AVFLOAT fY, AVFLOAT fWidth, AVFLOAT fHeight, AVLONG nX, AVLONG nY, AVLONG nWidth, AVLONG nHeight, AVCOLOR backColor, bool bClear)
{
	FWCOLOR color;
	m_pRenderer->GetBackColor(&color);
	m_pRenderer->PutBackColor(_FWCOLOR(backColor));
	PrepareViewport(fX, fY, fWidth, fHeight, nX, nY, nWidth, nHeight, bClear);
	m_pRenderer->PutBackColor(color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Other rendering functions

void CEngine::RenderLights()
{
	FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
	m_pRenderer->SetAmbientLight(cAmb);
	m_pLight1->Render(m_pRenderer);
	m_pLight2->Render(m_pRenderer);
}

void CEngine::Render(ISceneCamera *p)
{
	p->Render(m_pRenderer);
}

void CEngine::Render(ISceneObject *p)
{
	p->Render(m_pRenderer);
}

void CEngine::PutCamera(ISceneCamera *p)
{
	m_pScene->PutCamera(p); 
}

	static AVCOLOR HSV_to_RGB(AVFLOAT h, AVFLOAT s, AVFLOAT v)
	{
		// H is given on [0, 6]. S and V are given on [0, 1].
		float f = h - (int)h;
		if (((int)h & 1) == 0) f = 1 - f; // if i is even
		float m = v * (1 - s);
		float n = v * (1 - s * f);

		AVCOLOR RGBA;
		switch ((int)h) 
		{
			case 6:
			case 0: RGBA.r = v; RGBA.g = n; RGBA.b = m; RGBA.a = 1; return RGBA;
			case 1: RGBA.r = n; RGBA.g = v; RGBA.b = m; RGBA.a = 1; return RGBA;
			case 2: RGBA.r = m; RGBA.g = v; RGBA.b = n; RGBA.a = 1; return RGBA;
			case 3: RGBA.r = m; RGBA.g = n; RGBA.b = v; RGBA.a = 1; return RGBA;
			case 4: RGBA.r = n; RGBA.g = m; RGBA.b = v; RGBA.a = 1; return RGBA;
			case 5: RGBA.r = v; RGBA.g = m; RGBA.b = n; RGBA.a = 1; return RGBA;
			default: RGBA.r = 0; RGBA.g = 0; RGBA.b = 0; RGBA.a = 1; return RGBA;
		}
	}

void CEngine::RenderPassenger(IBody *pBody, AVULONG nPhase, AVLONG timeSpawn, AVLONG timeLoad, AVLONG spanWait)
{
	// get the current playing time...
	AVLONG nTime = GetPlayTime();
	AVLONG nAge = nTime - timeSpawn;
	if (nAge < 550 && nPhase == 0 || nAge >= 550 && nPhase == 1)
		return;

	// get the body object
	if (!pBody) return;
	ISceneObject *pObjBody = NULL;
	IKineNode *pNode = pBody->BodyNode(BODY_OBJECT);
	pNode->QueryInterface(&pObjBody);
	pNode->Release();
	if (!pObjBody)
		return;

	// Determine Temperature
	AVLONG time;
	switch (SETTINGS::nColouringMode)
	{
	case 0: 
		time = 0;
		break;
	case 1: 
		if (nTime < timeLoad - spanWait)
			time = 0;
		else if (nTime >= timeLoad)
			time = spanWait;
		else
			time = nTime + spanWait - timeLoad;
		break;
	case 2: 
		time = spanWait;
		break;
	}

	// convert the useful range into 0..2
	AVFLOAT fTime;
	AVFLOAT r1 = 1000.f * SETTINGS::nThresholdGreen;
	AVFLOAT r2 = 1000.f * SETTINGS::nThresholdRed;
	if (time <= r1)
		fTime = 0;
	else if (time >= r2)
		fTime = 2.f;
	else
		fTime = 2.f * ((AVFLOAT)time - r1) / (r2 - r1);

	AVCOLOR color = HSV_to_RGB(2 - fTime, 1, 1);

	// Determine transparency (where we are very young...)
	AVFLOAT fAlpha = 1.0f;
	if (nAge < 50) fAlpha = 0.0;
	else if (nAge < 550) fAlpha = (nAge - 50) / 500.0f;

	// Set Material colour & alpha
	IMaterial *pMaterial = NULL;
	pObjBody->GetMaterial(&pMaterial);
	if (pMaterial) 
	{
		pMaterial->SetDiffuseColor(_FWCOLOR(color));
		pMaterial->SetAlpha(fAlpha);
		pMaterial->Release();
	}

	// Render & Bye
	pObjBody->Render((IRndrGeneric*)m_pRenderer);
	pObjBody->Release();
}

void CEngine::Embark(HBODY pBody, HBONE pNode, bool bSwitchCoord)
{
	if (!pBody) return;
	HBONE pMe = pBody->BodyNode(BODY_OBJECT);

	if (bSwitchCoord && pNode)
	{
		FWVECTOR v1 = { 0, 0, 0 };
		pMe->LtoG(&v1);

		ITransform *pT;
		pMe->CreateCompatibleTransform(&pT);
		pNode->GetGlobalTransform(pT);
		pMe->Transform(pT, KINE_LOCAL | KINE_INVERTED);
		pMe->GetParentTransform(pT);
		pMe->Transform(pT, KINE_LOCAL | KINE_REGULAR);

		// compensate for Z coordinate
		FWVECTOR v2 = { 0, 0, 0 };
		pNode->LtoG(&v2);
		pT->FromTranslationXYZ(0, 0, v2.z - v1.z);
		pMe->Transform(pT, KINE_LOCAL | KINE_REGULAR);

		pT->Release();
	}
		
	HBONE pParent;
	pMe->GetParent(&pParent);
	if (pParent)
	{
		pParent->DelChildPtr(pMe);
		pParent->Release();
	}

	if (pNode)
	{
		OLECHAR buf[257];
		pNode->CreateUniqueLabel(L"Passenger", 256, buf);
		pNode->AddChild(buf, pMe);
	}

	pMe->Release();
}

HBONE CEngine::CreateChild(HBONE pParent, AVSTRING strLabel)
{
	HBONE h = NULL;
	OLECHAR buf[257];
	pParent->CreateUniqueLabel(strLabel, 256, buf);
	pParent->CreateChild(buf, &h);
	return h;
}

HOBJECT CEngine::CreateObject(AVSTRING strLabel)
{
	HOBJECT h = NULL;
	OLECHAR buf[257];
	m_pScene->CreateUniqueLabel(strLabel, 256, buf);
	m_pScene->NewObject(buf, &h);
	return h;
}

void CEngine::DeleteChild(HBONE pParent, HBONE hBone)
{
	if (pParent && hBone) pParent->DelChildPtr(hBone);
}

HCAMERA CEngine::CreateCamera(HBONE pParent, AVSTRING strLabel)
{
	HCAMERA h = NULL;

	m_pFWDevice->CreateObject(L"Camera", IID_ISceneCamera, (IFWUnknown**)&h);
	pParent->AddChild(L"Camera", h);

	h->Create(__FW_Vector(0, 0, 0), __FW_Vector(0, 1, 0), __FW_Vector(0, 0, 1.0f));
	h->PutPerspective((AVFLOAT)M_PI / 4, 20.0f, 10000.0f, 0.0f);
	h->PutVisible(TRUE);

	IKineTargetedObj *pTO = NULL;
	h->QueryInterface(&pTO);
	pTO->PutConfig(KINE_TARGET_ORBITING, NULL);
	pTO->Release();

	return h;
}

AVVECTOR CEngine::GetBonePos(HBONE bone)
{
	AVVECTOR pos = { 0, 0, 0 };
	bone->LtoG((FWVECTOR*)&pos);
	return pos;
}

AVVECTOR CEngine::GetBonePosLocal(HBONE bone, HBONE ref)
{
	AVVECTOR pos = { 0, 0, 0 };
	bone->LtoG((FWVECTOR*)&pos);
	ref->GtoL((FWVECTOR*)&pos);
	return pos;
}


void CEngine::ResetBone(HBONE bone)
{
	bone->Reset();
}

void CEngine::MoveBone(HBONE bone, AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	ITransform *pT = NULL;
	bone->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(x, y, z);
	bone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
}

void CEngine::MoveBoneTo(HBONE bone, AVFLOAT x, AVFLOAT y, AVFLOAT z)
{
	ITransform *pT = NULL;
	bone->GetLocalTransformRef(&pT);
	pT->FromTranslationXYZ(x, y, z);
	pT->Release();
	bone->Invalidate();
}

void CEngine::RotateBoneX(HBONE bone, AVFLOAT f)
{
	ITransform *pT = NULL;
	bone->CreateCompatibleTransform(&pT);
	pT->FromRotationX(f);
	bone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
}

void CEngine::RotateBoneY(HBONE bone, AVFLOAT f)
{
	ITransform *pT = NULL;
	bone->CreateCompatibleTransform(&pT);
	pT->FromRotationY(f);
	bone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
}

void CEngine::RotateBoneZ(HBONE bone, AVFLOAT f)
{
	ITransform *pT = NULL;
	bone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(f);
	bone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
}

void CEngine::PushState(HBONE bone)
{
	if (!bone) return;
	bone->PushState();
}

void CEngine::PopState(HBONE bone)
{
	if (!bone) return;
	bone->PopState(); 
	bone->Invalidate(); 
	bone->PushState();
}

void CEngine::Invalidate(HBONE bone)
{
	if (!bone) return;
	bone->Invalidate(); 
}

void CEngine::SetCameraPerspective(HCAMERA hCamera, AVFLOAT fFOV, AVFLOAT fClipNear, AVFLOAT fClipFar, AVFLOAT fAspect)
{
	hCamera->PutPerspective(fFOV, fClipNear, fClipFar, fAspect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Repository Utilities

IBody *CEngine::SpawnBiped()
{
	IBody *pBody = get();
	if (pBody && m_pBipedBuf)
	{
		IKineNode *pNode = pBody->BodyNode(BODY_OBJECT);
		pNode->RetrieveState(m_nBipedBufCount, m_pBipedBuf, NULL);
		pNode->Invalidate();
		pNode->Release();
	}
	return pBody;
}

void CEngine::KillBiped(IBody *pBody)
{
	release(pBody);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CRepos<IBody> implementation

IBody *CEngine::create()
{
	if (!m_pBiped) return NULL;
	static int nCount = 0;

	// Reproduction
	IKineNode *pNode = NULL;
	if FAILED(m_pBiped->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pNode))
		return NULL;

	// scene object operatons...
	ISceneObject *pSceneObj = NULL;
	pNode->QueryInterface(&pSceneObj);
	pSceneObj->PutVisible(TRUE);
	pSceneObj->PutMaterial(m_pMaterial, TRUE);
	pSceneObj->Release();

	// load body
	IBody *pBody = NULL;
	m_pScene->FWDevice()->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&pBody);
	pBody->LoadBody(pNode, BODY_SCHEMA_DISCREET);

	pNode->Release();

	return pBody;
}

void CEngine::destroy(IBody *p)
{
	static int nCount = 0;

	p->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Animators

HACTION CEngine::StartAnimation(LONG nTime)
{
	return (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, nTime, 0);
}

	int _callback(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, IAnimationListener *p)
	{
		switch (pEvent->nEvent)
		{
		case EVENT_BEGIN:	return p->OnAnimationBegin(nParam);
		case EVENT_TICK:	return p->OnAnimationTick(nParam);
		case EVENT_END:		return p->OnAnimationEnd(nParam);
		default:			return 0;
		}
	}

HACTION CEngine::SetAnimationListener(HACTION aHandle, IAnimationListener *pListener, AVULONG nParam)
{
	aHandle = (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, aHandle, 0);
	aHandle->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback, nParam, (void*)pListener);
	return aHandle;
}

HACTION CEngine::DoNothing(HACTION aHandle)
{
	return (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, aHandle, 0);
}

HACTION CEngine::Move(HACTION aHandle, IBody *pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, aHandle, nDuration, (AVSTRING)(wstrStyle.c_str()), pBody, BODY_ROOT, x, y, z);
}

HACTION CEngine::Move(HACTION aHandle, IKineNode *pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, aHandle, nDuration, (AVSTRING)(wstrStyle.c_str()), pBone, x, y, z);
}

HACTION CEngine::MoveTo(HACTION aHandle, IBody *pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"MoveTo", m_pActionTick, aHandle, nDuration, (AVSTRING)(wstrStyle.c_str()), pBody, BODY_ROOT, x, y, z);
}

HACTION CEngine::MoveTo(HACTION aHandle, IKineNode *pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"MoveTo", m_pActionTick, aHandle, nDuration, (AVSTRING)(wstrStyle.c_str()), pBone, x, y, z);
}

HACTION CEngine::Wait(HACTION aHandle, IBody *pBody, LONG nTimeUntil, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Wait", m_pActionTick, aHandle, 0, (AVSTRING)(wstrStyle.c_str()), pBody, nTimeUntil);
}

HACTION CEngine::Walk(HACTION aHandle, IBody *pBody, AVFLOAT x, AVFLOAT y, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, aHandle, c_nStepDuration, (AVSTRING)(wstrStyle.c_str()), pBody, x, y, (AVFLOAT)c_nStepLen, DEG2RAD(90));
}

HACTION CEngine::Turn(HACTION aHandle, IBody *pBody, std::wstring wstrStyle)
{
	return (IAction*)::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, aHandle, c_nTurnDuration, (AVSTRING)(wstrStyle.c_str()), pBody, DEG2RAD(180), 3);
}

HACTION CEngine::SetEnvelope(HACTION aHandle, AVFLOAT fEaseInMsec, AVFLOAT fEaseOutMsec)
{
	aHandle->SetParabolicEnvelopeT(fEaseInMsec, fEaseOutMsec);
	return aHandle;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Camera Animators

void CEngine::StartCameraAnimation(AVULONG nClockValue)
{
	m_pAuxActionTick->UnSubscribeAll();
	m_nAuxTimeRef = (nClockValue == 0x7FFFFFFF) ? ::GetTickCount() : nClockValue;
}

HACTION CEngine::MoveCameraTo(IKineNode *pNode, AVVECTOR v, AVULONG nTime, AVULONG nDuration, IKineNode *pRef)
{
	IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"MoveTo", m_pAuxActionTick, nTime, nDuration, pNode, *(FWVECTOR*)&v, pRef);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	return pAction;
}

HACTION CEngine::PanCamera(IKineNode *pNode, AVFLOAT alpha, AVULONG nTime, AVULONG nDuration)
{
	ITransform *pT = NULL;
	m_pScene->CreateCompatibleTransform(&pT);

	pT->FromRotationZ(alpha);
	IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"RotateTo", m_pAuxActionTick, nTime, nDuration, pNode, pT, ((IKineNode*)NULL));
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	pT->Release();
	return pAction;
}

HACTION CEngine::TiltCamera(IKineNode *pNode, AVFLOAT alpha, AVULONG nTime, AVULONG nDuration)
{
	ITransform *pT = NULL;
	m_pScene->CreateCompatibleTransform(&pT);

	pT->FromRotationX(alpha);
	IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"RotateTo", m_pAuxActionTick, nTime, nDuration, pNode, pT, ((IKineNode*)NULL));
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	pT->Release();
	return pAction;
}
	
	struct ZOOMINFO
	{
		ISceneCamera *pCamera;
		AVFLOAT fFOV0, fFOV1;
		AVFLOAT fClipNear0, fClipNear1;
		AVFLOAT fClipFar0, fClipFar1;
	};

	int _callback_zoom(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, ZOOMINFO *p)
	{
		switch (pEvent->nEvent)
		{
		case EVENT_BEGIN:
			p->pCamera->GetPerspective(&p->fFOV0, &p->fClipNear0, &p->fClipFar0, NULL);
			if (p->fFOV1 < 0) p->fFOV1 = p->fFOV0;
			if (p->fClipNear1 < 0) p->fClipNear1 = p->fClipNear0;
			if (p->fClipFar1 < 0) p->fClipFar1 = p->fClipFar0;
			break;
		case EVENT_END:
			p->pCamera->Release();
			delete p;
			break;
		case EVENT_TICK:
			FWFLOAT t;
			pAction->GetPhase(pEvent, &t);
			AVFLOAT fFOV = p->fFOV0 + t * (p->fFOV1 - p->fFOV0);
//			t = min(1.0f, t * 20);
			AVFLOAT fClipNear = p->fClipNear0 + t * (p->fClipNear1 - p->fClipNear0);
			AVFLOAT fClipFar = p->fClipFar0 + t * (p->fClipFar1 - p->fClipFar0);
			p->pCamera->PutPerspective(fFOV, fClipNear, fClipFar, 0);
		}

		return S_OK;
	}

HACTION CEngine::ZoomCamera(ISceneCamera *pCamera, AVFLOAT fFOV, AVFLOAT fClipNear, AVFLOAT fClipFar, AVULONG nTime, AVULONG nDuration)
{
	ZOOMINFO *p = new ZOOMINFO;
	p->pCamera = pCamera;
	p->pCamera->AddRef();
	p->fFOV1 = fFOV;
	p->fClipNear1 = fClipNear;
	p->fClipFar1 = fClipFar;

	IAction *pAction = (IAction*)FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pAuxActionTick, nTime, nDuration);
	pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_zoom, 0, p);

	return pAction;
}

HACTION CEngine::SetCameraEnvelope(HACTION aHandle, AVFLOAT fEaseInMsec, AVFLOAT fEaseOutMsec)
{
	aHandle->SetParabolicEnvelopeT(fEaseInMsec,fEaseOutMsec);
	return aHandle;
}

