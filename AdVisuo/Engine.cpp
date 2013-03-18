// Sprite.cpp

#include "StdAfx.h"
#include "Engine.h"
#include "Dialogs.h"

#include "freewill.c"			// #FreeWill: Obligatory!
#include "freewilltools.h"


#pragma warning (disable:4995)
#pragma warning (disable:4996)
 
#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )

DWORD CEngine::c_fpsNUM = 21;

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
	remove_all();
	delete [] m_pfps;
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

		CDlgReportBug::Report(3, str);
		CDlgReportBug dlg(str);
		dlg.m_cat = 1;
		dlg.DoModal();
			
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

	enum eError { ERROR_FREEWILL, ERROR_DIRECTX, ERROR_INTERNAL };
	try
	{
		// #FreeWill: create the FreeWill device
		Debug(L"Initialising FreeWill+ system...");
		HRESULT h;
		h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
		if (FAILED(h)) throw ERROR_FREEWILL;

		// #FreeWill: create & initialise the renderer
		Debug(L"Starting renderer...");
		h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
		if (FAILED(h)) throw ERROR_DIRECTX;
		h = m_pRenderer->InitDisplay(hWnd, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		if (FAILED(h)) throw ERROR_INTERNAL;

	//	FWCOLOR back = { 0.0f, 0.0f, 0.0f };		// black
	// 	FWCOLOR back = { 0.56f, 0.68f, 0.83f };		// blue
	//	FWCOLOR back = { 0.33f, 0.33f, 0.33f };		// gray
		FWCOLOR back = { 0.33f, 0.33f, 0.90f };		// gray
		m_pRenderer->PutBackColor(back);

		// #FreeWill: create & initialise the buffers - determine hardware factors
		IMeshVertexBuffer *pVertexBuffer;
		IMeshFaceBuffer *pFaceBuffer;
		h = m_pRenderer->GetBuffers(&pVertexBuffer, &pFaceBuffer); 
		if (FAILED(h)) throw ERROR_INTERNAL;
		h = pVertexBuffer->Create(2000000, MESH_VERTEX_XYZ | MESH_VERTEX_NORMAL | MESH_VERTEX_BONEWEIGHT | MESH_VERTEX_TEXTURE, 4, 1);
		if (FAILED(h)) throw ERROR_INTERNAL;
		h = pFaceBuffer->Create(2000000); if (FAILED(h)) throw ERROR_INTERNAL;
		pVertexBuffer->Release();
		pFaceBuffer->Release();

		// #FreeWill: create & initialise the animation scene
		h = m_pFWDevice->CreateObject(L"Scene", IID_IScene, (IFWUnknown**)&m_pScene);
		if (FAILED(h)) throw ERROR_INTERNAL;

		// #FreeWill: initialise the Tick Actions
		m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);
		m_pAuxActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);

		m_pScene->PutRenderer(m_pRenderer);

		// #Load the Scene
		Debug(L"Loading biped model...");
		IFileLoader *pLoader;
		m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);
		ISceneObject *pBip01 = NULL;
		m_pScene->NewObject(L"Bip01", &pBip01);
		pLoader->LoadObject((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"lobby.3D"), L"Bip01", pBip01);


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
		Debug(L"FreeWill+ initialised successfully...");
	}
	catch (eError e)
	{
		CString str;
		switch (e)
		{
		case ERROR_FREEWILL: str = L"FreeWill Graphics system not found. Please reinstall the application."; break;
		case ERROR_DIRECTX:  str = L"The Direct3D renderer could not be initialised. Please update or re-install DirectX."; break;
		case ERROR_INTERNAL: str = L"FreeWill Graphics System could not be initialised. Contact the Technical Support";  break;
		default:             str = L"Unidentified internal error occured. Contact the Technical Support";  break;
		}
		str += L" This is a fatal error and the application will now shut down.";
		AfxMessageBox(str, MB_OK | MB_ICONHAND);
		CDlgReportBug::Report(3, str);
		CDlgReportBug dlg(str);
		dlg.DoModal();
		return false;
	}

	// #FreeWill: set-up the error handler
	m_pFWDevice->SetUserErrorHandler(HandleErrors);
	
	return true;
}

void CEngine::RenderLights()
{
	FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
	m_pRenderer->SetAmbientLight(cAmb);
	m_pLight1->Render(m_pRenderer);
	m_pLight2->Render(m_pRenderer);
}

void CEngine::InitOffScreen(CSize sz, LPCTSTR pAviFile, FWULONG nFPS)
{
	m_pFWDevice->EnableErrorException(TRUE);
	m_pRenderer->InitOffScreen(sz.cx, sz.cy);
	m_pRenderer->OpenMovieFile(pAviFile, nFPS);
}

void CEngine::InitOffScreen(CSize sz, LPCTSTR pImgFile, FW_RENDER_BITMAP fmt)
{
	m_pFWDevice->EnableErrorException(TRUE);
	m_pRenderer->InitOffScreen(sz.cx, sz.cy);
	m_pRenderer->OpenStillFile(pImgFile, fmt);
}

void CEngine::DoneOffScreen()
{
	m_pRenderer->CloseMovieFile();	// will only close the file if previously opened
	m_pRenderer->DoneOffScreen();
	m_pFWDevice->EnableErrorException(FALSE);
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






void CEngine::OnTimer()
{
	m_pfps[m_nfps] = GetTickCount();
	m_nfps = (m_nfps + 1) % c_fpsNUM;
}


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




CEngine::ANIMATOR::ANIMATOR(CEngine *pEngine, LONG nTime) : m_pEngine(pEngine), m_pBody(NULL), m_pBone(NULL)
{
	SetAt(nTime);
}

CEngine::ANIMATOR::ANIMATOR(CEngine *pEngine, IBody *pBody, LONG nTime) : m_pEngine(pEngine), m_pBody(pBody), m_pBone(NULL)
{
	if (m_pBody) m_pBody->AddRef();
	SetAt(nTime);
}

CEngine::ANIMATOR::ANIMATOR(CEngine *pEngine, IKineNode *pBone, LONG nTime) : m_pEngine(pEngine), m_pBody(NULL), m_pBone(pBone)
{
	if (m_pBone) m_pBone->AddRef();
	SetAt(nTime);
}

CEngine::ANIMATOR::~ANIMATOR()
{
	if (m_pBody) m_pBody->Release();
	if (m_pBone) m_pBone->Release();
}

void CEngine::ANIMATOR::SetAt(LONG nTime)
{
	m_pAction = (IAction*)FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Generic", m_pEngine->m_pActionTick, nTime, 0);
}

void CEngine::ANIMATOR::SetCB(CB_HANDLE fn, AVULONG nParam, void *pParam)
{
	m_pAction = (IAction*)FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Generic", m_pEngine->m_pActionTick, m_pAction, 0);
	m_pAction->SetHandleEventHook(fn, nParam, pParam);
}

void CEngine::ANIMATOR::Generic()
{
	m_pAction = (IAction*)FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Generic", m_pEngine->m_pActionTick, m_pAction, 0);
}

void CEngine::ANIMATOR::Move(AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	if (m_pBody)
		m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Move", m_pEngine->m_pActionTick, m_pAction, nDuration, (AVSTRING)(wstrStyle.c_str()), m_pBody, BODY_ROOT, x, y, z);
	else if (m_pBone)
		m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Move", m_pEngine->m_pActionTick, m_pAction, nDuration, (AVSTRING)(wstrStyle.c_str()), m_pBone, x, y, z);
}

void CEngine::ANIMATOR::MoveTo(AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle)
{
	if (m_pBody)
		m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"MoveTo", m_pEngine->m_pActionTick, m_pAction, nDuration, (AVSTRING)(wstrStyle.c_str()), m_pBody, BODY_ROOT, x, y, z);
	else if (m_pBone)
		m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"MoveTo", m_pEngine->m_pActionTick, m_pAction, nDuration, (AVSTRING)(wstrStyle.c_str()), m_pBone, x, y, z);
}

void CEngine::ANIMATOR::Wait(LONG nTimeUntil, std::wstring wstrStyle)
{
	if (!m_pBody) return;
	m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Wait", m_pEngine->m_pActionTick, m_pAction, 0, (AVSTRING)(wstrStyle.c_str()), m_pBody, nTimeUntil);
}

void CEngine::ANIMATOR::Walk(AVFLOAT x, AVFLOAT y, std::wstring wstrStyle)
{
	if (!m_pBody) return;
	AVFLOAT stepLen = 15.0f;
	AVULONG stepDuration = 150;
	m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Walk", m_pEngine->m_pActionTick, m_pAction, stepDuration, (AVSTRING)(wstrStyle.c_str()), m_pBody, x, y, stepLen, DEG2RAD(90));
}

void CEngine::ANIMATOR::Turn(std::wstring wstrStyle)
{
	if (!m_pBody) return;
	AVULONG turnDuration = 300;
	m_pAction = (IAction*)::FWCreateObjWeakPtr(m_pEngine->m_pFWDevice, L"Action", L"Turn", m_pEngine->m_pActionTick, m_pAction, turnDuration, (AVSTRING)(wstrStyle.c_str()), m_pBody, DEG2RAD(180), 3);
}

void CEngine::ANIMATOR::SetParabolicEnvelopeT(AVFLOAT fEaseIn, AVFLOAT fEaseOut)
{
	m_pAction->SetParabolicEnvelopeT(fEaseIn, fEaseOut);
}








CString CEngine::GetDiagnosticMessage()
{
	CString str;
	if (IsPlaying() && !IsPaused())
		str.Format(L"Playing now at %d", GetPlayTime());
	else if (IsPlaying() && IsPaused())
		str.Format(L"Paused at %d", GetPlayTime());
	else
		str.Format(L"Not playing");
	return str;
}

FWULONG CEngine::GetFPS()
{
	if (m_pfps[m_nfps] == 0)
		return 0;
	else
		return 1000 * (c_fpsNUM-1) / (m_pfps[(m_nfps+c_fpsNUM-1)%c_fpsNUM] - m_pfps[m_nfps]);
}

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
