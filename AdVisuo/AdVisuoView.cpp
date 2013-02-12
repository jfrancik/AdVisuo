// AdVisuoView.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuo.h"
#include "AdVisuoView.h"
#include "MainFrm.h"
#include <math.h>
#include "freewilltools.h"
#include "Dialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )
float round(float f) { return ceil(f * 1000) / 1000; }

// CAdVisuoView

IMPLEMENT_DYNCREATE(CAdVisuoView, CView)

BEGIN_MESSAGE_MAP(CAdVisuoView, CView)
	ON_COMMAND_RANGE(ID_LAYOUT_SINGLE, ID_LAYOUT_QUADRUPLE, &CAdVisuoView::OnLayout)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LAYOUT_SINGLE, ID_LAYOUT_QUADRUPLE, &CAdVisuoView::OnUpdateLayout)

	ON_COMMAND_RANGE(ID_CAMERA, ID_STOREY_ONEDOWN, &CAdVisuoView::OnCamera)
	ON_COMMAND_RANGE(ID_CAMERA_EXT_REAR, ID_CAMERA_EXT_SIDE, &CAdVisuoView::OnCamera)
	ON_COMMAND_RANGE(ID_STOREY_MENU + 1000, ID_STOREY_MENU + 1300, &CAdVisuoView::OnCamera)
	ON_COMMAND_RANGE(ID_CAMERA_LIFT_MENU + 2000, ID_CAMERA_LIFT_MENU + 2200, &CAdVisuoView::OnCamera)

	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA, ID_STOREY_ONEDOWN, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_EXT_REAR, ID_CAMERA_EXT_SIDE, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_STOREY_MENU + 1000, ID_STOREY_MENU + 1300, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_LIFT_MENU + 2000, ID_CAMERA_LIFT_MENU + 2200, &CAdVisuoView::OnUpdateCamera)

	ON_UPDATE_COMMAND_UI(ID_STOREY_MENU, &CAdVisuoView::OnUpdateStoreyMenu)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_LIFT_MENU, &CAdVisuoView::OnUpdateCameraLiftMenu)


	ON_COMMAND_RANGE(ID_SELCAMERA, ID_SELCAMERA_10, &CAdVisuoView::OnSelCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SELCAMERA, ID_SELCAMERA_10, &CAdVisuoView::OnUpdateSelCamera)


	ON_COMMAND_RANGE(ID_VIEW_ASPECT_WIN, ID_VIEW_ASPECT_CUSTOM, &CAdVisuoView::OnViewAspectRatio)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_ASPECT_WIN, ID_VIEW_ASPECT_CUSTOM, &CAdVisuoView::OnUpdateViewAspectRatio)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ASPECT, &CAdVisuoView::OnUpdateViewAspect)

	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_ACTION_RENDER, &CAdVisuoView::OnActionRender)
	ON_COMMAND(ID_ACTION_SAVESTILL, &CAdVisuoView::OnActionSavestill)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_FULLSCREEN, &CAdVisuoView::OnViewFullScreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, &CAdVisuoView::OnUpdateViewFullScreen)
	ON_COMMAND(ID_ACTION_REWIND, &CAdVisuoView::OnActionRewind)
	ON_COMMAND(ID_HUD_PINDOWN, &CAdVisuoView::OnHudPinDown)
	ON_COMMAND(ID_HUD_PANEL, &CAdVisuoView::OnHudPanel)
	ON_UPDATE_COMMAND_UI(ID_HUD_PANEL, &CAdVisuoView::OnUpdateHudPanel)
	ON_COMMAND(ID_HUD_CAPTION, &CAdVisuoView::OnHudCaption)
	ON_UPDATE_COMMAND_UI(ID_HUD_CAPTION, &CAdVisuoView::OnUpdateHudCaption)
	ON_COMMAND(ID_HUD_SELECT, &CAdVisuoView::OnHudSelect)
	ON_UPDATE_COMMAND_UI(ID_HUD_SELECT, &CAdVisuoView::OnUpdateHudSelect)

	ON_COMMAND(ID_ACTION_PLAY, &CAdVisuoView::OnActionPlay)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PLAY, &CAdVisuoView::OnUpdateActionPlay)
	ON_COMMAND(ID_ACTION_PLAYSPECIAL, &CAdVisuoView::OnActionPlayspecial)
	ON_COMMAND(ID_ACTION_PAUSE, &CAdVisuoView::OnActionPause)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PAUSE, &CAdVisuoView::OnUpdateActionPause)
	ON_COMMAND(ID_ACTION_STOP, &CAdVisuoView::OnActionStop)
	ON_UPDATE_COMMAND_UI(ID_ACTION_STOP, &CAdVisuoView::OnUpdateActionStop)
	ON_COMMAND(ID_ACTION_SLOWDOWN, &CAdVisuoView::OnActionSlowdown)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SLOWDOWN, &CAdVisuoView::OnUpdateActionSlowdown)
	ON_COMMAND(ID_ACTION_SPEEDUP, &CAdVisuoView::OnActionSpeedup)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SPEEDUP, &CAdVisuoView::OnUpdateActionSpeedup)
	ON_COMMAND(ID_ACTION_NORMALPACE, &CAdVisuoView::OnActionNormalpace)
	ON_UPDATE_COMMAND_UI(ID_ACTION_NORMALPACE, &CAdVisuoView::OnUpdateActionNormalpace)
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_PANE2, &CAdVisuoView::OnUpdateStatusbarPane2)
	ON_UPDATE_COMMAND_UI(ID_ACTION_RENDER, &CAdVisuoView::OnUpdateActionRender)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_ACTION_SAVESTILL, &CAdVisuoView::OnUpdateActionSavestill)
	ON_COMMAND(ID_VIEW_MATERIALS, &CAdVisuoView::OnViewMaterials)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MATERIALS, &CAdVisuoView::OnUpdateViewMaterials)
	ON_COMMAND(ID_REC_SCRIPT, &CAdVisuoView::OnRecScript)
	ON_UPDATE_COMMAND_UI(ID_REC_SCRIPT, &CAdVisuoView::OnUpdateRecScript)
	ON_COMMAND(ID_REC_RECORD, &CAdVisuoView::OnRecRecord)
	ON_COMMAND(ID_REC_PLAY, &CAdVisuoView::OnRecPlay)
	ON_COMMAND(ID_TMP_GROUP1, &CAdVisuoView::OnTmpGroup1)
	ON_COMMAND(ID_TMP_GROUP2, &CAdVisuoView::OnTmpGroup2)
	ON_COMMAND(ID_TMP_GROUP3, &CAdVisuoView::OnTmpGroup3)
	ON_UPDATE_COMMAND_UI(ID_TMP_GROUP1, &CAdVisuoView::OnUpdateTmpGroup1)
	ON_UPDATE_COMMAND_UI(ID_TMP_GROUP2, &CAdVisuoView::OnUpdateTmpGroup2)
	ON_UPDATE_COMMAND_UI(ID_TMP_GROUP3, &CAdVisuoView::OnUpdateTmpGroup3)
END_MESSAGE_MAP()

// CAdVisuoView construction/destruction

DWORD CAdVisuoView::c_fpsNUM = 21;

CAdVisuoView::CAdVisuoView() : m_screen(NULL, 2), m_plateCam(&m_sprite), m_hud(&m_sprite), m_script(this), m_instRec(this)
{
	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pBody = NULL;
	m_pActionTick = NULL;
	m_pAuxActionTick = NULL;
	m_pLight1 = NULL;
	m_pLight2 = NULL;

	m_pfps = new DWORD[c_fpsNUM];
	memset(m_pfps, 0, sizeof(DWORD) * c_fpsNUM);
	m_nfps = 0;

	m_nKeyScanTime = 0;
	for (int i = 0; i < N_CAMERAS; i++)
		m_pCamera[i] = NULL;
	m_bSelectOnWheel = false;
	m_bSelectOnMove = false;
	m_bHUDPanel = true;
	m_bHUDCaption = true;
	m_bHUDSelection = true;
	m_nAspectImageIndex = 16;
	m_pbutFloor = NULL;
	m_pbutLift = NULL;
	m_nCamExtSideOption = 2;
	m_bLock = false;
}

CAdVisuoView::~CAdVisuoView()
{
	DeleteAllCameras();

	if (m_pActionTick) m_pActionTick->UnSubscribeAll();
	if (m_pScene) m_pScene->DelAll();

	ULONG nRef;
	
	if (m_pLight2) nRef = m_pLight2->Release();
	if (m_pLight1) nRef = m_pLight1->Release();
	if (m_pAuxActionTick) nRef = m_pAuxActionTick->Release();
	if (m_pActionTick) nRef = m_pActionTick->Release();

	if (m_pBody) nRef = m_pBody->Release();
	if (m_pScene) nRef = m_pScene->Release();
	if (m_pRenderer) nRef = m_pRenderer->Release();
	if (m_pFWDevice) nRef = m_pFWDevice->Release();

	delete [] m_pfps;
}

CString CAdVisuoView::GetDiagnosticMessage()
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

///////////////////////////////////////////////////////////////////////////////////////
// Initialisation and Life-Cycle

static void _on_lost_device (IRndrGeneric*, FWULONG, void *pParam)	{ ((CAdVisuoView*)pParam)->OnLostDevice(); 	}
static void _on_reset_device(IRndrGeneric*, FWULONG, void *pParam)	{ ((CAdVisuoView*)pParam)->OnResetDevice(); }

BOOL CAdVisuoView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CAdVisuoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	CWaitCursor wait_cursor;

	// initialise FreeWill
	if (!IsEngineReady())
		if (!CreateFreeWill(m_hWnd))
		{
			::PostQuitMessage(100);
			return;
		}

	// Setup lost device/reset callbacks
	m_pRenderer->SetCallback(FW_CB_LOSTDEVICE, _on_lost_device, 0, this);
	m_pRenderer->SetCallback(FW_CB_RESETDEVICE, _on_reset_device, 0, this);

	// Initialise Screen Manager
	m_screen.SetRenderer(m_pRenderer);

	// Initialise HUD
	FWULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);
	m_sprite.SetRenderer(m_pRenderer);
	m_plateCam.SetParams(_stdPathModels + L"plateNW.bmp", 0xFF0000FF, 0x80FFFFFF, 12, TRUE, FALSE, L"System", 0xFF000000, 16, false, CSize(2, 2));
	m_hud.Initialise();
	m_hud.SetSimulationTime(GetProject()->GetMaxSimulationTime());
	m_hud.SetPos(CPoint(x/2 - 256, y - 64));
	m_hud.SetAltPos(CPoint(x, y));

	// initialise the simulation
	if (GetDocument()->IsSimReady())
	{
		GetProject()->SetRenderer(m_pRenderer);
		GetProject()->Construct();
		GetProject()->StoreConfig();

		CreateAllCameras();	
		AVULONG nStorey = GetProject()->GetLiftGroup(0)->GetBasementStoreyCount();
		GetCamera(0)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(0)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	  - ID_CAMERA - 1);
		GetCamera(1)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(1)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTFRONT   - ID_CAMERA - 1);
		GetCamera(2)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(2)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTREAR     - ID_CAMERA - 1);
		GetCamera(3)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(3)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR    - ID_CAMERA - 1);
		GetCamera(4)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(4)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_CENTRALFRONT - ID_CAMERA - 1);
		GetCamera(5)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(5)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_CENTRALREAR  - ID_CAMERA - 1);
		GetCamera(6)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(6)->MoveTo(CAMLOC_OUTSIDE, 0);
		GetCamera(7)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(7)->MoveTo(CAMLOC_OUTSIDE, 1);
		GetCamera(8)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(8)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	  - ID_CAMERA - 1);
		GetCamera(9)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(9)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	  - ID_CAMERA - 1);

		Debug(L"Building created, ready for rendering.");

		BeginFrame();
		RenderScene();
		EndFrame();
		
		PrepareSim();
	}

	SetTimer(101, 1000 / 50, NULL);
	m_hud.KeepReady();
}

void CAdVisuoView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	if (((CMDIFrameWnd*)(AfxGetApp()->m_pMainWnd))->MDIGetActive() == GetParentFrame())
	{
		OnScanKey();
		InvalidateRect(NULL, FALSE);
	}
}

void CAdVisuoView::OnLostDevice()
{
	m_sprite.OnLostDevice();
	m_plateCam.OnLostDevice();
	m_hud.OnLostDevice();
}

void CAdVisuoView::OnResetDevice()
{
	FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
	m_pRenderer->SetAmbientLight(cAmb);
	m_sprite.OnResetDevice();
	m_plateCam.OnResetDevice();
	m_hud.OnResetDevice();
}

///////////////////////////////////////////////////////////////////////////////////////
// Sim Initialisation

void CAdVisuoView::PrepareSim()
{
	m_pActionTick->UnSubscribeAll();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		GetProject()->GetSim(i)->Play(m_pActionTick, GetProject()->GetMinSimulationTime());
	for (AVLONG t = GetProject()->GetMinSimulationTime(); t <= 0; t += 40)
		Proceed(t);	// loops un-nested on 24/1/13: Proceed was called too often!
}

///////////////////////////////////////////////////////////////////////////////////////
// FreeWill Initialisation


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

bool CAdVisuoView::CreateFreeWill(HWND hWnd)
{
	if (IsEngineReady())
		return true;

	enum eError { ERROR_FREEWILL, ERROR_DIRECTX, ERROR_INTERNAL };
	try
	{
		// #FreeWill: create the FreeWill device
		HRESULT h;
		h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
		if (FAILED(h)) throw ERROR_FREEWILL;
		Debug(L""); Debug(L"FreeWill+ system initialised successfully.");

		// #FreeWill: create & initialise the renderer
		h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
		if (FAILED(h)) throw ERROR_DIRECTX;
		Debug(L"Renderer started successfully.");
		h = m_pRenderer->InitDisplay(m_hWnd, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
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

		// #FreeWill: create & initialise the character body
		h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody);
		if (FAILED(h)) throw ERROR_INTERNAL;

		// #FreeWill: initialise the Tick Actions
		m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);
		m_pAuxActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);

		m_pScene->PutRenderer(m_pRenderer);

		// #Load the Scene
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
		Debug(L"Biped model loaded.");

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

		// Notify the simulation engine about the scene (fully loaded now)
		IMaterial *pMaterial = NULL;
		m_pScene->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
		IKineChild *pBiped = NULL;
		m_pScene->GetChild(L"Bip01", &pBiped);
		GetProject()->SetScene(m_pScene, pMaterial, pBiped);
		pMaterial->Release();
		pBiped->Release();

		// Load Body Object
		pBody = NULL;
		if (SUCCEEDED(m_pScene->GetChild(L"Bip01", (IKineChild**)&pBody)) && pBody)
		{
			m_pBody->LoadBody(pBody, BODY_SCHEMA_DISCREET);
			pBody->Release();
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

		FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
		m_pRenderer->SetAmbientLight(cAmb);
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

///////////////////////////////////////////////////////////////////////////////////////
// Timer, Cursor and Size Handlers

void CAdVisuoView::OnTimer(UINT_PTR nIDEvent)
{
	m_pfps[m_nfps] = GetTickCount();
	m_nfps = (m_nfps + 1) % c_fpsNUM;

	// #FreeWill: Push the time info into the engine
	if (IsPlaying())
	if (IsPlaying())
		if (!Proceed())
			OnActionStop();

	if (GetPlayTime() > GetProject()->GetMaxSimulationTime())
		OnActionStop();

	// Push the time into Aux Ticks
	if (m_pAuxActionTick && m_pAuxActionTick->AnySubscriptionsLeft() == TRUE)
	{
		FWULONG nMSec = GetTickCount() - m_nAuxTimeRef;
		m_pAuxActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, NULL);
	}

	if (GetDocument()->IsSIMDataReady())
		GetDocument()->OnSIMDataLoaded(m_pActionTick);

	GetDocument()->UpdateAllViews(NULL, 0, 0);

	CView::OnTimer(nIDEvent);
}

BOOL CAdVisuoView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pt;
	if (GetCursorPos(&pt))
	{
		CScreen::HIT hit;
		AVULONG nDummy;
		ScreenToClient(&pt);

		if (m_hud.HitTest(pt) != CHUD::HIT_NONE)
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		else
		{
			m_screen.HitTest(pt, hit, nDummy, nDummy);
			switch (hit)
			{
			case CScreen::HIT_VIEW:	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS)); break;
			case CScreen::HIT_XDIV:	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE)); break;
			case CScreen::HIT_YDIV:	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS)); break;
			case CScreen::HIT_XYDIV:SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL)); break;
			default: return CView::OnSetCursor(pWnd, nHitTest, message);
			}
		}
	}
	return TRUE;
}

void CAdVisuoView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (cx && cy && m_pRenderer)
	{
		FWULONG x, y;
		m_pRenderer->GetViewSize(&x, &y);
		m_hud.SetPos(CPoint(x/2 - 256, y - 64));
		m_hud.SetAltPos(CPoint(x, y));
		m_sprite.OnResize();

		OnAdjustCameras();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Rendering

void CAdVisuoView::OnDraw(CDC *pDC)
{
	ASSERT_VALID(GetDocument());

	if (m_bLock) return;

	if (!GetDocument() || !IsEngineReady() || !GetDocument()->IsSimReady() || !GetCurCamera() || !GetCurCamera()->IsReady())
	{
		// if anything went wrong or is not yet ready...
		CRect rect;
		GetClientRect(rect);
		::FillRect(pDC->m_hDC, rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
	}
	else
	{
		BeginFrame();
		RenderScene(m_bHUDSelection);
		RenderHUD(m_bHUDPanel, false, m_bHUDCaption, m_bHUDSelection, GetPlayTime());
		EndFrame();
	}
}

void CAdVisuoView::BeginFrame()
{
//	if (m_hWnd) m_pRenderer->PutWindow(m_hWnd);
	m_pRenderer->BeginFrame();
}

void CAdVisuoView::EndFrame()
{
	m_pRenderer->EndFrame();
}

void CAdVisuoView::RenderScene(bool bHUDSelection)
{
	CAdVisuoRenderer renderer(GetProject()->GetLiftGroup(GetCurLiftGroupIndex()), m_pRenderer);

	FWCOLOR active = { 1, 0.86f, 0.47f }, inactive = { 1, 1, 1 };
	m_screen.Prepare(inactive, active, bHUDSelection);

	for (AVULONG i = 0; i < m_screen.GetCount(); i++)
	{
		if (!m_screen.IsEnabled(i)) continue;

		m_screen.Prepare(i, bHUDSelection);

		CCamera *pCamera = GetCamera(m_screen.GetCamera(i));
		if (!pCamera) continue;
		renderer.SetupCamera(pCamera);

		m_pLight1->Render(m_pRenderer);
		m_pLight2->Render(m_pRenderer);

		// my own display list goes here... instead of m_pScene->Render(pRenderer);
		for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
			GetProject()->GetLiftGroup(i)->GetSim()->RenderPassengers(m_pRenderer, 0);

		// for multiple lift groups
		for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		{
			if (i == GetCurLiftGroupIndex())
				continue;
			renderer.SetLiftGroup(GetProject()->GetLiftGroup(i));
			renderer.RenderSideOuter(i < GetCurLiftGroupIndex() ? 0 : 1);
		}
		renderer.SetLiftGroup(GetProject()->GetLiftGroup(GetCurLiftGroupIndex()));

		switch (pCamera->GetLoc())
		{
		case CAMLOC_LOBBY:
		case CAMLOC_OVERHEAD:
			renderer.RenderCentre();
			break;
		case CAMLOC_LIFT:
		case CAMLOC_SHAFT: 
			renderer.RenderSide();
			break;
		default:
			switch (pCamera->GetYZone())
			{
			case -1:
				renderer.RenderSideOuter(0);
				break;
			case 0:
			case 1:
			case 2:
				renderer.RenderCentreOuter();
				break;
			case 3:
				renderer.RenderSideOuter(1);
				break;
			}
			break;
		}

		for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
			GetProject()->GetLiftGroup(i)->GetSim()->RenderPassengers(m_pRenderer, 1);
	}
}

void CAdVisuoView::RenderHUD(bool bHUDPanel, bool bHUDClockOnly, bool bHUDCaption, bool bHUDSelection, AVLONG nTime)
{
	m_screen.Prepare();

	FWULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);

	m_sprite.Begin();

	// draw camera plates
	if (bHUDCaption)
		for (AVULONG i = 0; i < m_screen.GetCount(); i++)
		{
			if (!m_screen.IsEnabled(i)) continue;
			CCamera *pCamera = GetCamera(m_screen.GetCamera(i));
			if (!pCamera) continue;

			LPTSTR text = pCamera->GetTextDescription();
			
			AVULONG x0, x1, y0, y1;
			m_screen.Get(i, x0, x1, y0, y1, bHUDSelection);

			CSize size = m_plateCam.Calc(text);
			size.cx = min(size.cx, (LONG)x1 - (LONG)x0);
			m_plateCam.Draw(CPoint(x0, y0), size, text);
		}

	if (bHUDPanel)
	{
		// cursor pos
		CPoint pt(0, 0);
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// draw HUD panel
		m_hud.SetItemStatus(CHUD::HIT_PLAY, IsPlaying() && !IsPaused() ? 2 : 0);
		m_hud.SetItemStatus(CHUD::HIT_FULL_SCREEN, ((CMDIFrameWndEx*)::AfxGetMainWnd())->IsFullScreen() ? 2 : 0);
		m_hud.SetTime(nTime);
		m_hud.SetLoadedTime(GetDocument()->GetLoadedTime());
		if (bHUDClockOnly) m_hud.Hide();
		m_hud.Draw(pt);
	}

	m_sprite.End();
}

///////////////////////////////////////////////////////////////////////////////////////
// Snapshot and Video Rendering

bool CAdVisuoView::RenderToVideo(LPCTSTR lpszFilename, AVULONG nFPS, AVULONG nResX, AVULONG nResY, 
								AVULONG nTimeFrom, AVULONG nTimeTo, bool bShowCaptions, bool bShowClock, CBUpdate fn)
{
	m_bLock = true;
	bool bResult = true;

	// Prepare...
	AVFLOAT fAR = m_screen.GetAspectRatio();
	m_screen.SetAspectRatio((AVFLOAT)nResX / (AVFLOAT)nResY);
	OnAdjustCameras();

	Rewind(nTimeFrom);

	m_pRenderer->Stop();

	m_pRenderer->InitOffScreen(nResX, nResY);
	m_pFWDevice->EnableErrorException(TRUE);
	
	try
	{
		m_pRenderer->OpenMovieFile(lpszFilename, nFPS);

		// first frame - to initialise
		m_script.Play();
		AVULONG t = nTimeFrom;
		Proceed(t);

		m_pRenderer->SetTargetOffScreen();
		m_hud.SetPos(CPoint(nResX/2 - 256, nResY - 64));
		m_hud.SetAltPos(CPoint(nResX, nResY));
		m_sprite.OnResize();
		BeginFrame();
		RenderScene();
		RenderHUD(bShowClock, true, bShowCaptions, false, t);
		EndFrame();
		t += 1000 / nFPS;

		AfxGetMainWnd()->SetActiveWindow();

		// all other frames
		for (t; t <= nTimeTo; t += GetAccel() * 1000 / nFPS)
		{
			MSG msg;
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_PAINT || msg.message == WM_TIMER)
					break;
				if (!IsDialogMessage(&msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}

			// proceed the script - and provide for any pre-programmed fast forward
			m_script.Proceed(t, t);

			Proceed(t);

			// push time to the auxiliary player
			if (m_pAuxActionTick && m_pAuxActionTick->AnySubscriptionsLeft() == TRUE)
			{
				FWULONG nMSec = t - m_nAuxTimeRef;
				m_pAuxActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, NULL);
			}

			m_pRenderer->SetTargetOffScreen();
			BeginFrame();
			RenderScene();
			RenderHUD(bShowClock, true, bShowCaptions, false, t);
			EndFrame();

			bool bStop = false, bPreview = false;
			fn(L"Rendering to file...", t, bStop, bPreview);
			if (bStop) break;

			if (bPreview)
			{
				m_pRenderer->SetTargetToScreen();
				BeginFrame();
				RenderScene();
				EndFrame();
			}
		}

		bool bStop = false, bPreview = false;
		fn(L"Finalising...", t, bStop, bPreview);

		m_pRenderer->SetTargetToScreen();
		BeginFrame();
		RenderScene();
		RenderHUD(false, true, bShowCaptions, false, 0);
		EndFrame();
		for (int i = 0; i < 40; i++)
		{
			m_pRenderer->SetTargetOffScreen();
			BeginFrame();
			RenderScene();
			RenderHUD(false, false, bShowCaptions, false, 0);
			EndFrame();
		}

		m_pRenderer->CloseMovieFile();
		m_pRenderer->DoneOffScreen();
	}
	catch (FWERROR *) 
	{ 
		m_pRenderer->CloseMovieFile();
		m_pRenderer->DoneOffScreen();
		bResult = false;
	}
	m_pFWDevice->EnableErrorException(FALSE);
	m_pRenderer->Stop();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
	{
		GetProject()->GetLiftGroup(i)->GetSim()->Stop();
		GetProject()->GetLiftGroup(i)->RestoreConfig();
	}
	PrepareSim();

	m_screen.SetAspectRatio(fAR);
	FWULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);
	m_hud.SetPos(CPoint(x/2 - 256, y - 64));
	m_hud.SetAltPos(CPoint(x, y));
	m_sprite.OnResize();

	OnAdjustCameras();

	m_bLock = false;
	return bResult;
}

bool CAdVisuoView::RenderToBitmap(LPCTSTR pFilename, enum FW_RENDER_BITMAP fmt)
{
	AVULONG x0, x1, y0, y1;
	m_screen.Get(x0, x1, y0, y1);

	m_pRenderer->InitOffScreen(x1 - x0, y1 - y0);
	m_pRenderer->OpenStillFile(pFilename, fmt);

	BeginFrame();
	RenderScene();
//	RenderHUD(true, true, true, false);
	EndFrame();

	m_pRenderer->DoneOffScreen();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// Proceeding the Animation

bool CAdVisuoView::Proceed(FWLONG nMSec)
{
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		GetProject()->GetLiftGroup(i)->GetSim()->SetColouringMode(((CAdVisuoApp*)AfxGetApp())->GetColouringMode());

	m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0);
	return (m_pActionTick->AnySubscriptionsLeft() == TRUE);
}

bool CAdVisuoView::Proceed()
{
	AVLONG nTime = GetPlayTime();
	m_script.Proceed((AVULONG&)nTime);
	return Proceed(nTime);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Play Control

void CAdVisuoView::Play()
{
	if (!m_pRenderer) return;
	PutAccel(1);
	m_pRenderer->Play();
	//m_pRenderer->PutPlayTime(GetProject()->GetMinSimulationTime());
	m_script.Play();
}

void CAdVisuoView::Rewind(FWULONG nMSec)
{
	CWaitCursor wait;

	m_pRenderer->Stop();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
	{
		GetProject()->GetLiftGroup(i)->GetSim()->Stop();
		GetProject()->GetLiftGroup(i)->RestoreConfig();
	}

	m_pActionTick->UnSubscribeAll();

	AVLONG t;
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		t = GetProject()->GetLiftGroup(i)->GetSim()->FastForward(m_pActionTick, nMSec);

	for ( ; t <= (AVLONG)nMSec; t += 40)
		Proceed(t);
	
	PutAccel(1);
	m_pRenderer->Play();
	m_pRenderer->PutPlayTime(t);
}

FWULONG CAdVisuoView::GetFPS()
{
	if (m_pfps[m_nfps] == 0)
		return 0;
	else
		return 1000 * (c_fpsNUM-1) / (m_pfps[(m_nfps+c_fpsNUM-1)%c_fpsNUM] - m_pfps[m_nfps]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Auxiliary Player

void CAdVisuoView::AuxPlay(IAction **pAuxAction, AVULONG nClockValue)
{
	if (!pAuxAction) return;
	*pAuxAction = m_pAuxActionTick;
	m_pAuxActionTick->AddRef();

	m_pAuxActionTick->UnSubscribeAll();
	m_nAuxTimeRef = (nClockValue == 0x7FFFFFFF) ? ::GetTickCount() : nClockValue;
}

bool CAdVisuoView::IsAuxPlaying()
{
	return (m_pAuxActionTick->AnySubscriptionsLeft() == TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////
// Camera Control

void CAdVisuoView::OnAdjustCameras()
{
//	if (::GetKeyState(VK_CONTROL) < 0)
//		return;

	for (AVULONG i = 0; i < m_screen.GetCount(); i++)
		if (m_screen.IsEnabled(i))
		{
			CCamera *pCamera = GetCamera(m_screen.GetCamera(i));
			if (pCamera) pCamera->Adjust(m_screen.GetAspectRatio(i));
		}
}

void CAdVisuoView::OnDrag(int dx, int dy, int dz, bool bShift, bool bCtrl, bool bAlt)
{
	if (!GetCurCamera()) return;

	// if (bCtrl) if (abs(dx) > abs(dy)) dy = 0; else dx = 0;

	switch (GetAdVisuoApp()->GetWalkMode())
	{
	case 0:
	case 2:
		// CCTV & Ghost Mode
		if (dx != 0) !bShift ? GetCurCamera()->Pan(DEG2RAD((FWFLOAT)dx/5.0f)) : GetCurCamera()->Move(-(FWFLOAT)dx, 0, 0);
		if (dy != 0) !bShift ? GetCurCamera()->Tilt(DEG2RAD(-(FWFLOAT)dy/5.0f)) : GetCurCamera()->Move(0, 0, -(FWFLOAT)dy);
		if (dz != 0) !bShift ? GetCurCamera()->Zoom(DEG2RAD((FWFLOAT)dz / 2.0f)) : GetCurCamera()->Zoom(DEG2RAD((FWFLOAT)dz / 2.0f));
		break;
	case 1:
		// Walk Mode
		if (dx != 0 && !bShift) GetCurCamera()->Pan(DEG2RAD((FWFLOAT)dx/5.0f));
		if (dy != 0 && !bShift) GetCurCamera()->Move(0, -(FWFLOAT)dy, 0);
		if (dx != 0 &&  bShift) GetCurCamera()->Move(-(FWFLOAT)dx, 0, 0);
		if (dy != 0 &&  bShift) GetCurCamera()->Move(0, 0, -(FWFLOAT)dy);
		if (dz != 0) GetCurCamera()->Tilt((FWFLOAT)dz/10.0f);
		break;
	}

	GetCurCamera()->CheckLocation();
}

void CAdVisuoView::OnScanKey()
{
	if (g_bFullScreen && ::GetKeyState(VK_ESCAPE) < 0)
		OnViewFullScreen();

	if (GetFocus() != this) return;
	if (!GetCurCamera()) return;

	DWORD nKeyScanTime = GetTickCount();
	DWORD nSpan = m_nKeyScanTime ? nKeyScanTime - m_nKeyScanTime : 0;
	m_nKeyScanTime = nKeyScanTime;
	
	bool bShift = (::GetKeyState(VK_SHIFT) < 0);
	bool bCtrl = (::GetKeyState(VK_CONTROL) < 0);

	if (bCtrl && ::GetKeyState(VK_RETURN) < 0)
		OnViewFullScreen();

	AVFLOAT fDist = nSpan * 0.1f;
	AVFLOAT fAngle = nSpan * (FWFLOAT)M_PI/180/30;
	AVFLOAT fZoom = nSpan * 0.00016f;
	if (bCtrl) { fDist *= 4; fAngle *= 4; fZoom *= 4; }

	if (::GetKeyState('W') < 0) bShift ? GetCurCamera()->Move(0, 0, fDist) : GetCurCamera()->Move(0, fDist, 0);
	if (::GetKeyState('A') < 0) bShift ? GetCurCamera()->Move(fDist, 0, 0) : GetCurCamera()->Pan(-fAngle);
	if (::GetKeyState('S') < 0) bShift ? GetCurCamera()->Move(0, 0, -fDist) : GetCurCamera()->Move(0, -fDist, 0);
	if (::GetKeyState('D') < 0) bShift ? GetCurCamera()->Move(-fDist, 0, 0) : GetCurCamera()->Pan(fAngle);
	if (::GetKeyState(VK_UP) < 0) bShift ? GetCurCamera()->Move(0, 0, fDist) : GetCurCamera()->Move(0, fDist, 0);
	if (::GetKeyState(VK_LEFT) < 0) bShift ? GetCurCamera()->Move(fDist, 0, 0) : GetCurCamera()->Pan(-fAngle);
	if (::GetKeyState(VK_DOWN) < 0) bShift ? GetCurCamera()->Move(0, 0, -fDist) : GetCurCamera()->Move(0, -fDist, 0);
	if (::GetKeyState(VK_RIGHT) < 0) bShift ? GetCurCamera()->Move(-fDist, 0, 0) : GetCurCamera()->Pan(fAngle);
	if (::GetKeyState('T') < 0) bShift ? GetCurCamera()->Zoom(-fZoom) : GetCurCamera()->Zoom(fZoom);

	if (::GetKeyState(VK_PRIOR) < 0) GetCurCamera()->Tilt(fAngle);
	if (::GetKeyState(VK_NEXT) < 0) GetCurCamera()->Tilt(-fAngle);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Generated message map functions


///////////////////////////////////////////////////////////////////////////////////////
// Mouse

void CAdVisuoView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_hud.OnDragBegin(point))
		m_hit = CScreen::HIT_NONE;
	else
		m_screen.HitTest(point, m_hit, m_nHitX, m_nHitY);

	if (m_hit == CScreen::HIT_VIEW)
		m_screen.SetActive(m_nHitX);
	else if (m_hit == CScreen::HIT_XDIV || m_hit == CScreen::HIT_YDIV || m_hit == CScreen::HIT_XYDIV)
		OnAdjustCameras();

	SetCapture();
	m_ptHit = point;

	CView::OnLButtonDown(nFlags, point);
}

void CAdVisuoView::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint delta(point.x - m_ptHit.x, point.y - m_ptHit.y);
	
	if (GetCapture() == this)
		if (m_hud.IsDragging())
			m_hud.OnDrag(point);
		else
			switch (m_hit)
			{
			case CScreen::HIT_NONE:
				break;
			case CScreen::HIT_VIEW:
				OnDrag(delta.x, delta.y, 0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
				m_ptHit = point;
				break;
			default:
				m_screen.OnDrag(delta, m_hit, m_nHitX, m_nHitY);
				OnAdjustCameras();
				break;
			}
	else if (m_bSelectOnMove)
	{
		m_screen.HitTest(point, m_hit, m_nHitX, m_nHitY);
		if (m_hit == CScreen::HIT_VIEW)
			m_screen.SetActive(m_nHitX);
	}

	CView::OnMouseMove(nFlags, point);
}

void CAdVisuoView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPoint delta(point.x - m_ptHit.x, point.y - m_ptHit.y);
	if (GetCapture() == this)
	{
		if (m_hud.IsDragging())
			m_hud.OnDragCommit(point, m_hWnd);
		else
			switch (m_hit)
			{
			case CScreen::HIT_VIEW:
				OnDrag(delta.x, delta.y, 0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
				break;
			default:
				m_screen.OnDrag(delta, m_hit, m_nHitX, m_nHitY);
				m_screen.OnDragCommit(m_hit, m_nHitX, m_nHitY);
				OnAdjustCameras();
				break;
			}

		ReleaseCapture();
		m_hit = CScreen::HIT_NONE;
	}

	CView::OnLButtonUp(nFlags, point);
}

BOOL CAdVisuoView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_bSelectOnWheel)
	{
		ScreenToClient(&pt);
		m_screen.HitTest(pt, m_hit, m_nHitX, m_nHitY);
		if (m_hit == CScreen::HIT_VIEW)
			m_screen.SetActive(m_nHitX);
	}

	OnDrag(0, 0, zDelta/120, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CAdVisuoView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAdVisuoView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


///////////////////////////////////////////////////////////////////////////////////////
// Camera


void CAdVisuoView::OnSelCamera(UINT nCmd)
{
	AVLONG nCamera = nCmd - ID_SELCAMERA_1;
	if (nCamera >= 0 && nCamera < N_CAMERAS)
	{
		m_screen.SetCurCamera(nCamera);
		GetCurCamera()->Adjust(m_screen.GetCurAspectRatio());
	}
}

void CAdVisuoView::OnUpdateSelCamera(CCmdUI *pCmdUI)
{
	AVLONG nCamera = pCmdUI->m_nID - ID_SELCAMERA_1;
	pCmdUI->SetCheck(m_screen.GetCurCamera() == nCamera);
	if (nCamera >=0 && nCamera < N_CAMERAS && GetCamera(nCamera))
	{
		GetCamera(nCamera)->CheckLocation();
		pCmdUI->SetText(GetCamera(nCamera)->GetTextDescription());
	}
}

void CAdVisuoView::OnLayout(UINT nCmd)
{
	m_screen.SetConfig(nCmd - ID_LAYOUT_SINGLE);
	OnAdjustCameras();
}

void CAdVisuoView::OnUpdateLayout(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_screen.GetConfig() == pCmdUI->m_nID - ID_LAYOUT_SINGLE);
}

void CAdVisuoView::OnCamera(UINT nCmd)
{
	if (!GetCurCamera()) return;
	
	CAMDESC desc;
	GetCurCamera()->GetDescription(&desc);

	if (::GetKeyState(VK_CONTROL) >= 0)
	{
		// No Control Key
		IAction *pAction = NULL;
		AuxPlay(&pAction); 
		if (pAction)
		{
			if (nCmd >= ID_STOREY_MENU + 1000 && nCmd < ID_STOREY_MENU + 1300)
				GetCurCamera()->AnimateTo(CAMLOC_STOREY, pAction, nCmd - ID_STOREY_MENU - 1000, GetViewAspectRatio());
			else if (nCmd >= ID_CAMERA_LIFT_MENU + 2000 && nCmd < ID_CAMERA_LIFT_MENU + 2200)
				GetCurCamera()->AnimateTo(CAMLOC_LIFT, pAction, nCmd - ID_CAMERA_LIFT_MENU - 2000, GetViewAspectRatio());
			else switch (nCmd)
			{
				case ID_CAMERA_OVERHEAD:		GetCurCamera()->AnimateTo(CAMLOC_OVERHEAD, pAction, 0, GetViewAspectRatio()); break;
				case ID_CAMERA_LEFTREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 0, GetViewAspectRatio()); break;
				case ID_CAMERA_CENTRALREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 1, GetViewAspectRatio()); break;
				case ID_CAMERA_RIGHTREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 2, GetViewAspectRatio()); break;
				case ID_CAMERA_RIGHTSIDE:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 3, GetViewAspectRatio()); break;
				case ID_CAMERA_RIGHTFRONT:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 4, GetViewAspectRatio()); break;
				case ID_CAMERA_CENTRALFRONT:	GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 5, GetViewAspectRatio()); break;
				case ID_CAMERA_LEFTFRONT:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 6, GetViewAspectRatio()); break;
				case ID_CAMERA_LEFTSIDE:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 7, GetViewAspectRatio()); break;
				case ID_CAMERA_LIFT_MENU:		break;
				case ID_CAMERA_LIFTRIGHT:		GetCurCamera()->AnimateTo(CAMLOC_LIFT, pAction, GetCurCamera()->GetLift()-1, GetViewAspectRatio()); break;
				case ID_CAMERA_LIFTLEFT:		GetCurCamera()->AnimateTo(CAMLOC_LIFT, pAction, GetCurCamera()->GetLift()+1, GetViewAspectRatio()); break;
				case ID_STOREY_MENU:			break;
				case ID_STOREY_ONEUP:			GetCurCamera()->AnimateTo(CAMLOC_STOREY, pAction, GetCurCamera()->GetStorey()+1, GetViewAspectRatio()); break;
				case ID_STOREY_ONEDOWN:			GetCurCamera()->AnimateTo(CAMLOC_STOREY, pAction, GetCurCamera()->GetStorey()-1, GetViewAspectRatio()); break;
				case ID_CAMERA_EXT_FRONT:		GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, pAction, 0, GetViewAspectRatio()); break;
				case ID_CAMERA_EXT_REAR:		GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, pAction, 1, GetViewAspectRatio()); break;
				case ID_CAMERA_EXT_SIDE:		if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 2) 
													GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, pAction, m_nCamExtSideOption = 3, GetViewAspectRatio());
												else if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 3)
													GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, pAction, m_nCamExtSideOption = 2, GetViewAspectRatio());
												else 
													GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, pAction, m_nCamExtSideOption, GetViewAspectRatio()); 
												break;
			}

			pAction->Release();
		}
	}
	else
	{
		// With Control Key - fast motion
		if (nCmd >= ID_STOREY + 1000 && nCmd < ID_STOREY + 1200)
			GetCurCamera()->MoveTo(CAMLOC_STOREY, nCmd - ID_STOREY - 1000, GetViewAspectRatio());
		else if (nCmd >= ID_CAMERA_LIFT_MENU + 2000 && nCmd < ID_CAMERA_LIFT_MENU + 2200)
			GetCurCamera()->MoveTo(CAMLOC_LIFT, nCmd - ID_CAMERA_LIFT_MENU - 2000, GetViewAspectRatio());
		else switch (nCmd)
		{
			case ID_CAMERA_OVERHEAD:		GetCurCamera()->MoveTo(CAMLOC_OVERHEAD, 0, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTREAR:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 0, GetViewAspectRatio()); break;
			case ID_CAMERA_CENTRALREAR:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 1, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTREAR:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 2, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTSIDE:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 3, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTFRONT:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 4, GetViewAspectRatio()); break;
			case ID_CAMERA_CENTRALFRONT:	GetCurCamera()->MoveTo(CAMLOC_LOBBY, 5, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTFRONT:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 6, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTSIDE:		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 7, GetViewAspectRatio()); break;
			case ID_CAMERA_LIFTRIGHT:		GetCurCamera()->MoveTo(CAMLOC_LIFT, GetCurCamera()->GetLift()-1, GetViewAspectRatio()); break;
			case ID_CAMERA_LIFTLEFT:		GetCurCamera()->MoveTo(CAMLOC_LIFT, GetCurCamera()->GetLift()+1, GetViewAspectRatio()); break;
			case ID_STOREY_ONEUP:			GetCurCamera()->MoveTo(CAMLOC_STOREY, GetCurCamera()->GetStorey()+1, GetViewAspectRatio()); break;
			case ID_STOREY_ONEDOWN:			GetCurCamera()->MoveTo(CAMLOC_STOREY, GetCurCamera()->GetStorey()-1, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_FRONT:		GetCurCamera()->MoveTo(CAMLOC_OUTSIDE, 0, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_REAR:		GetCurCamera()->MoveTo(CAMLOC_OUTSIDE, 1, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_SIDE:		if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 2) 
												GetCurCamera()->MoveTo(CAMLOC_OUTSIDE, m_nCamExtSideOption = 3, GetViewAspectRatio());
											else if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 3)
												GetCurCamera()->MoveTo(CAMLOC_OUTSIDE, m_nCamExtSideOption = 2, GetViewAspectRatio());
											else 
												GetCurCamera()->MoveTo(CAMLOC_OUTSIDE, m_nCamExtSideOption, GetViewAspectRatio()); 
											break;
		}
	}
}

void CAdVisuoView::OnUpdateCamera(CCmdUI *pCmdUI)
{
	if (GetCurCamera() == NULL)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	CAMDESC desc;
	GetCurCamera()->GetDescription(&desc);

	if (pCmdUI->m_nID >= ID_STOREY + 1000 && pCmdUI->m_nID < ID_STOREY + 1050)
	{
		pCmdUI->Enable(desc.camloc != CAMLOC_LIFT);
		pCmdUI->SetCheck(pCmdUI->m_nID - ID_STOREY - 1000 == GetCurCamera()->GetStorey());
	}
	else if (pCmdUI->m_nID >= ID_CAMERA_LIFT_MENU + 2000 && pCmdUI->m_nID < ID_CAMERA_LIFT_MENU + 2200)
		pCmdUI->SetCheck(pCmdUI->m_nID - ID_CAMERA_LIFT_MENU - 2000 == GetCurCamera()->GetLift());
	else switch (pCmdUI->m_nID)
	{
	case ID_CAMERA_OVERHEAD:		pCmdUI->SetCheck(desc.camloc == CAMLOC_OVERHEAD); break;
	case ID_CAMERA_LEFTREAR:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 0); break;
	case ID_CAMERA_CENTRALREAR:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 1); break;
	case ID_CAMERA_RIGHTREAR:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 2); break;
	case ID_CAMERA_RIGHTSIDE:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 3); break;
	case ID_CAMERA_RIGHTFRONT:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 4); break;
	case ID_CAMERA_CENTRALFRONT:	pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 5); break;
	case ID_CAMERA_LEFTFRONT:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 6); break;
	case ID_CAMERA_LEFTSIDE:		pCmdUI->SetCheck(desc.camloc == CAMLOC_LOBBY && desc.index == 7); break;
	case ID_CAMERA_LIFTRIGHT:		pCmdUI->Enable(desc.camloc == CAMLOC_LIFT && GetCurCamera()->GetLift() > 0);  break;
	case ID_CAMERA_LIFTLEFT:		pCmdUI->Enable(desc.camloc == CAMLOC_LIFT && GetCurCamera()->GetLift() < (AVLONG)GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->GetLiftCount() - 1); break;
	case ID_STOREY_ONEUP:			pCmdUI->Enable(desc.camloc != CAMLOC_LIFT && GetCurCamera()->GetStorey() < (AVLONG)GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->GetStoreyCount() - 1); break;
	case ID_STOREY_ONEDOWN:			pCmdUI->Enable(desc.camloc != CAMLOC_LIFT && GetCurCamera()->GetStorey() > 0);break;

	case ID_CAMERA_EXT_FRONT:		pCmdUI->SetCheck(desc.camloc == CAMLOC_OUTSIDE && desc.index == 0); break;
	case ID_CAMERA_EXT_REAR:		pCmdUI->SetCheck(desc.camloc == CAMLOC_OUTSIDE && desc.index == 1); break;
	case ID_CAMERA_EXT_SIDE:		pCmdUI->SetCheck(desc.camloc == CAMLOC_OUTSIDE && desc.index >= 2);
									if (pCmdUI->m_pOther)
									{
										AVULONG nImageIndex;
										if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 2) nImageIndex = 48;
										else if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 3) nImageIndex = 49;
										else nImageIndex = (m_nCamExtSideOption == 2) ? 48 : 49;
										CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
										if ((ULONG)(pButton) < 1000)
											MessageBeep(-1);
										else
										if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
											if (pButton->GetImageIndex(FALSE) != nImageIndex)
											{
												pButton->SetImageIndex(nImageIndex, FALSE);
												if (pButton) pButton->Redraw();
											}
									}
									break;
	}
}

void CAdVisuoView::OnUpdateStoreyMenu(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCurCamera() && GetCurCamera()->GetDescription() != CAMLOC_LIFT);

	CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
	if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
	{
		auto pSubItems = &pButton->GetSubItems();
		INT_PTR n = pSubItems->GetSize();
		if (n && (*pSubItems)[n - 1] == m_pbutFloor)
			return;		// we already have this menu in place

		// create the floors menu
		pButton->RemoveAllSubItems();
		CLiftGroupVis *pLiftGroup = GetProject()->GetLiftGroup(GetCurLiftGroupIndex());
		for (AVULONG i = 0; i < pLiftGroup->GetStoreyCount(); i++)
		{
			m_pbutFloor = new CMFCRibbonButton(ID_STOREY_MENU + 1000 + i, pLiftGroup->GetStorey(i)->GetName().c_str());
			m_pbutFloor->SetDefaultMenuLook();
			pButton->AddSubItem(m_pbutFloor);

			if ((AVLONG)i < (AVLONG)pLiftGroup->GetStoreyCount() - 10)
			{
				if (pLiftGroup->GetStoreyCount() > 60 && i > 30) i += 4;
				if (pLiftGroup->GetStoreyCount() > 120 && i > 100) i += 5;
			}
		}
	}
}

void CAdVisuoView::OnUpdateCameraLiftMenu(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCurCamera() != NULL);

	CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
	if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
	{
		auto pSubItems = &pButton->GetSubItems();
		INT_PTR n = pSubItems->GetSize();
		if (n && (*pSubItems)[n - 1] == m_pbutLift)
			return;		// we already have this menu in place

		// create the lifts menu
		pButton->RemoveAllSubItems();
		CLiftGroupVis *pLiftGroup = GetProject()->GetLiftGroup(GetCurLiftGroupIndex());
		for (AVULONG i = 0; i < pLiftGroup->GetLiftCount(); i++)
		{
			if (i == pLiftGroup->GetShaftCount(0))
			{
				m_pbutLift = new CMFCRibbonSeparator(TRUE);
				pButton->AddSubItem(m_pbutLift);
			}
			m_pbutLift = new CMFCRibbonButton(ID_CAMERA_LIFT_MENU + 2000 + i, pLiftGroup->GetLift(i)->GetName().c_str());
			m_pbutLift->SetDefaultMenuLook();
			pButton->AddSubItem(m_pbutLift);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Action

void CAdVisuoView::OnActionPlay()
{
	if (GetDocument()->IsSimReady() && !IsPlaying())
	{
		Play();
		Debug(L"Visualisation started...");
	}
}

void CAdVisuoView::OnActionPause()
{
	Pause();
}

void CAdVisuoView::OnActionStop()
{
	m_pRenderer->Stop();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
	{
		GetProject()->GetLiftGroup(i)->GetSim()->Stop();
		GetProject()->GetLiftGroup(i)->RestoreConfig();
	}
	PrepareSim();
}

void CAdVisuoView::OnActionSlowdown()		{ PutAccel(GetAccel() / 2); }
void CAdVisuoView::OnActionSpeedup()		{ PutAccel(GetAccel() * 2); }
void CAdVisuoView::OnActionNormalpace()		{ PutAccel(1); }

void CAdVisuoView::OnUpdateActionPlay(CCmdUI *pCmdUI)		{ pCmdUI->Enable(IsEngineReady() && GetDocument()->IsSimReady() && !IsPlaying()); pCmdUI->SetCheck(IsPlaying()); }
void CAdVisuoView::OnUpdateActionPause(CCmdUI *pCmdUI)		{ pCmdUI->Enable(IsPlaying()); pCmdUI->SetCheck(IsPaused()); }
void CAdVisuoView::OnUpdateActionStop(CCmdUI *pCmdUI)		{ pCmdUI->Enable(IsPlaying()); }
void CAdVisuoView::OnUpdateActionSlowdown(CCmdUI *pCmdUI)	{ pCmdUI->Enable(IsPlaying()); }
void CAdVisuoView::OnUpdateActionSpeedup(CCmdUI *pCmdUI)	{ pCmdUI->Enable(IsPlaying()); }
void CAdVisuoView::OnUpdateActionNormalpace(CCmdUI *pCmdUI)	{ pCmdUI->Enable(IsPlaying()); }

///////////////////////////////////////////////////////////////////////////////////////
// View

void CAdVisuoView::OnHudPanel()			{ m_bHUDPanel = !m_bHUDPanel; m_hud.KeepReady();}
void CAdVisuoView::OnHudCaption()		{ m_bHUDCaption = !m_bHUDCaption; }
void CAdVisuoView::OnHudSelect()		{ m_bHUDSelection = !m_bHUDSelection; }

void CAdVisuoView::OnUpdateHudPanel(CCmdUI *pCmdUI)		{ pCmdUI->SetCheck(m_bHUDPanel); }
void CAdVisuoView::OnUpdateHudCaption(CCmdUI *pCmdUI)	{ pCmdUI->SetCheck(m_bHUDCaption); }
void CAdVisuoView::OnUpdateHudSelect(CCmdUI *pCmdUI)	{ pCmdUI->SetCheck(m_bHUDSelection); }

void CAdVisuoView::OnViewAspectRatio(UINT nCmd)
{
	switch (nCmd)
	{
	case ID_VIEW_ASPECT_WIN:
		m_screen.SetAspectRatio(0);
		break;
	case ID_VIEW_ASPECT_SCREEN:
		m_screen.SetAspectRatio(GetScreenAspectRatio());
		break;
	case ID_VIEW_ASPECT_16_9:
		m_screen.SetAspectRatio(RATIO_16_9);
		break;
	case ID_VIEW_ASPECT_16_10:
		m_screen.SetAspectRatio(RATIO_16_10);
		break;
	case ID_VIEW_ASPECT_4_3:
		m_screen.SetAspectRatio(RATIO_4_3);
		break;
	case ID_VIEW_ASPECT_CUSTOM:
		{
			CDlgAspectRatio dlg(GetWindowAspectRatio(), GetScreenAspectRatio());
			dlg.SetAspectRatio(m_screen.GetAspectRatio());
			if (dlg.DoModal() == IDOK)
				m_screen.SetAspectRatio(dlg.GetAspectRatio());
		}
		break;
	}

	// determine the aspect ratio - for UI update
	AVFLOAT f = round(m_screen.GetAspectRatio());
	if (f == 0) m_nAspectImageIndex = 16;
	else if (f == round(RATIO_16_9)) m_nAspectImageIndex = 17;
	else if (f == round(RATIO_16_10)) m_nAspectImageIndex = 18;
	else if (f == round(RATIO_4_3)) m_nAspectImageIndex = 19;

	OnAdjustCameras();
}

void CAdVisuoView::OnUpdateViewAspectRatio(CCmdUI *pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_ASPECT_WIN:
		pCmdUI->SetRadio(round(m_screen.GetAspectRatio()) == 0);
		break;
	case ID_VIEW_ASPECT_SCREEN:
		pCmdUI->SetCheck(round(m_screen.GetAspectRatio()) == round(GetScreenAspectRatio()));
		break;
	case ID_VIEW_ASPECT_16_9:
		pCmdUI->SetRadio(round(m_screen.GetAspectRatio()) == round(RATIO_16_9));
		break;
	case ID_VIEW_ASPECT_16_10:
		pCmdUI->SetRadio(round(m_screen.GetAspectRatio()) == round(RATIO_16_10));
		break;
	case ID_VIEW_ASPECT_4_3:
		pCmdUI->SetRadio(round(m_screen.GetAspectRatio()) == round(RATIO_4_3));
		break;
	case ID_VIEW_ASPECT_CUSTOM:
		break;
	}
}

void CAdVisuoView::OnUpdateViewAspect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
	if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
		if (pButton->GetImageIndex(TRUE) != m_nAspectImageIndex)
		{
			pButton->SetImageIndex(m_nAspectImageIndex, TRUE);
			if (pButton) pButton->Redraw();
		}
}

void CAdVisuoView::OnViewFullScreen()
{
	CMDIFrameWndEx *pMainWnd = (CMDIFrameWndEx*)::AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)));

	if (!pMainWnd->IsFullScreen())
	{
		m_bMaximised = pMainWnd->IsZoomed();
		if (m_bMaximised)
			pMainWnd->ShowWindow(SW_RESTORE);
		pMainWnd->ShowFullScreen();

		CWnd *pTB = FindWindow(NULL, L"Full Screen");
		CString str;
		if (pTB) pTB->GetWindowText(str);
		if (pTB && str == L"Full Screen")
			pTB->ShowWindow(SW_HIDE);
//		CWnd *pTB = WindowFromPoint(CPoint(110, 110));
//		if (pTB) pTB->ShowWindow(SW_HIDE);

		CRect rect;
		GetDesktopWindow()->GetWindowRect(rect);

		pMainWnd->SetWindowPos(&wndTopMost, -13, -54, rect.Width() + 26, rect.Height() + 54 + 13, SWP_SHOWWINDOW);

		g_bFullScreen = true;
		m_pRenderer->ResetDeviceEx(NULL, 0, 0);
	}
	else
	{
		pMainWnd->ShowFullScreen();
		pMainWnd->ShowWindow(SW_MAXIMIZE);
		if (!m_bMaximised) pMainWnd->ShowWindow(SW_RESTORE);

		pMainWnd->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		m_pRenderer->ResetDeviceEx(m_hWnd, 0, 0);
		g_bFullScreen = false;
	}

	FWULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);
	m_hud.SetPos(CPoint(x/2 - 256, y - 64));
	m_hud.SetAltPos(CPoint(x, y));
	m_sprite.OnResize();
	OnAdjustCameras();
	
	if (!pMainWnd->IsFullScreen()) 
		Sleep(1500);
}

void CAdVisuoView::OnUpdateViewFullScreen(CCmdUI *pCmdUI)
{
	CMDIFrameWndEx *pMainWnd = (CMDIFrameWndEx*)::AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)));
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(pMainWnd->IsFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////
// HUD specific

void CAdVisuoView::OnActionPlayspecial()
{
	if (!IsEngineReady() || !GetDocument()->IsSimReady())
		return;

	if (!IsPlaying())
		OnActionPlay();
	else
		OnActionPause();
}

void CAdVisuoView::OnActionRewind()		{ Rewind(m_hud.GetSliderTime()); }
void CAdVisuoView::OnHudPinDown()		{ m_hud.OnPinDown(); }
	
///////////////////////////////////////////////////////////////////////////////////////
// File	

void CAdVisuoView::OnActionRender()
{
	// display Video Params dlg
	AVFLOAT fAR = m_screen.GetAspectRatio();

	typedef void (WINAPI *PROC)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
	PROC _SHGetKnownFolderPath = (PROC) GetProcAddress(GetModuleHandle(L"Shell32.dll"), "SHGetKnownFolderPath");

	CString path;
	if (_SHGetKnownFolderPath)
	{
		// Windows Vista / 7
		PWSTR buf;
		_SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, &buf);
		path = buf; path += L"\\" + GetDocument()->GetTitle() + L".avi";
	}
	else
	{
		// Windows XP
		wchar_t buf[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, 0, buf);
		path = buf; 
		path.Replace(L"Pictures", L"Videos");
		path += L"\\" + GetDocument()->GetTitle() + L".avi";
	}

	CDlgVideo dlg(GetWindowAspectRatio(), 25, path, 
					0, GetDocument()->GetLoadedTime(),
					[this](AVFLOAT fAspectRatio)
					{
						m_screen.SetAspectRatio(fAspectRatio);
						AVFLOAT f = round(fAspectRatio);
						if (f == 0) m_nAspectImageIndex = 16;
						else if (f == round(RATIO_16_9)) m_nAspectImageIndex = 17;
						else if (f == round(RATIO_16_10)) m_nAspectImageIndex = 18;
						else if (f == round(RATIO_4_3)) m_nAspectImageIndex = 19;
						OnAdjustCameras();
					},
					[this, fAR]()
					{
						m_screen.SetAspectRatio(fAR);
						AVFLOAT f = round(fAR);
						if (f == 0) m_nAspectImageIndex = 16;
						else if (f == round(RATIO_16_9)) m_nAspectImageIndex = 17;
						else if (f == round(RATIO_16_10)) m_nAspectImageIndex = 18;
						else if (f == round(RATIO_4_3)) m_nAspectImageIndex = 19;
						OnAdjustCameras();
					});
	if (dlg.DoModal() != IDOK)
		return;
//	if (dlg.m_timeTo == 0) dlg.m_timeTo = 0x7fffffff;

	// Stop GUI, display VideoCtrl window
	AfxGetMainWnd()->EnableWindow(FALSE);
	CDlgVideoCtrl ctrl(this);
	ctrl.Create(IDD_ADV_VIDEOCTRL);
	ctrl.SetWindowPos(&CWnd::wndTopMost, 15, 120, 0, 0, SWP_NOSIZE);
	ctrl.ShowWindow(SW_SHOW);
	ctrl.SetForegroundWindow();
	ctrl.SetStatus(L"Initialising...", FALSE);
	ctrl.m_wndPreview.SetCheck(BST_CHECKED);

	if (RenderToVideo(dlg.GetPath(), dlg.GetFPS(), dlg.GetX(), dlg.GetY(), dlg.GetTFrom(), dlg.GetTTo(), dlg.ShowCaptions(), dlg.ShowClock(),
		[&ctrl](LPCTSTR strStatus, AVULONG nTime, bool &bStop, bool &bPreview)
		{
			ctrl.SetStatus(strStatus, TRUE);
			ctrl.SetTime(nTime);
			bStop = ctrl.m_bStopping;
			bPreview = ctrl.m_wndPreview.GetCheck() == BST_CHECKED;
		}))
		ShellExecute(m_hWnd, L"open", dlg.GetPath(), NULL, NULL, SW_SHOW);

	ctrl.DestroyWindow();
	AfxGetMainWnd()->EnableWindow(TRUE);
	AfxGetMainWnd()->SetActiveWindow();
}

void CAdVisuoView::OnActionSavestill()
{
	CFileDialog dlg(FALSE, L"jpg", L"*.jpg", OFN_OVERWRITEPROMPT, L"JPG Files (*.jpg)|*.jpg|BMP Files (*.bmp)|*.bmp|Targa Files (*.tga)|*.tga|PNG Files (*.png)|*.png|All Files|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		enum FW_RENDER_BITMAP fmt = RENDER_JPG;
		CString ext = dlg.GetFileExt();
		if (ext.CompareNoCase(L"bmp") == 0) fmt = RENDER_BMP;
		if (ext.CompareNoCase(L"jpg") == 0) fmt = RENDER_JPG;
		if (ext.CompareNoCase(L"tga") == 0) fmt = RENDER_TGA;
		if (ext.CompareNoCase(L"png") == 0) fmt = RENDER_PNG;

		RenderToBitmap(dlg.GetPathName(), fmt);
	}
}

void CAdVisuoView::OnUpdateActionRender(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->IsSimReady() && !IsPlaying() && !((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
	pCmdUI->SetCheck(IsPlaying());
}

void CAdVisuoView::OnUpdateActionSavestill(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->IsSimReady() && !((CMDIFrameWndEx*)AfxGetMainWnd())->IsFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////
// Status Bar

void CAdVisuoView::OnUpdateStatusbarPane2(CCmdUI *pCmdUI)
{
	FWULONG nTime = GetPlayTime();
	CString str;
	FWULONG fps = GetFPS();
	if (fps)
		str.Format(L"%d fps", fps);
	else
		str = L"??? fps";
	pCmdUI->SetText(str);
}

void CAdVisuoView::OnViewMaterials()
{
	if (CDlgMaterials::c_dlg == NULL)
	{
		CDlgMaterials *pDlg = new CDlgMaterials(&GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->m_materials);
		pDlg->Create(CDlgMaterials::IDD);
		pDlg->CenterWindow();
		pDlg->ShowWindow(SW_SHOW);
	}
	else
		CDlgMaterials::c_dlg->DestroyWindow();
}


void CAdVisuoView::OnUpdateViewMaterials(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(CDlgMaterials::c_dlg != NULL);
}


void CAdVisuoView::OnRecScript()
{
	if (CDlgScript::c_dlg == NULL)
	{
		CDlgScript *pDlg = new CDlgScript(&m_script);
		pDlg->Create(CDlgScript::IDD);
		pDlg->CenterWindow();
		pDlg->ShowWindow(SW_SHOW);
	}
	else if (CDlgScript::c_dlg->IsWindowVisible())
		CDlgScript::c_dlg->ShowWindow(SW_HIDE);
	else
		CDlgScript::c_dlg->ShowWindow(SW_SHOW);
}


void CAdVisuoView::OnUpdateRecScript(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(CDlgScript::c_dlg != NULL && CDlgScript::c_dlg->IsWindowVisible());
}


void CAdVisuoView::OnRecRecord()
{
	m_instRec.Record();
}


void CAdVisuoView::OnRecPlay()
{
	AVULONG a = 0;
	m_instRec.Play(a);
}


void CAdVisuoView::OnTmpGroup1()
{
	GetCurCamera()->SetLiftGroup(GetProject()->GetLiftGroup(0));
	if (::GetKeyState(VK_CONTROL) >= 0)
	{
		IAction *pAction = NULL;
		AuxPlay(&pAction); 
		if (pAction)
		{
			GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 6, GetViewAspectRatio());
			pAction->Release();
		}
	}
	else
		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 6, GetViewAspectRatio());
}


void CAdVisuoView::OnTmpGroup2()
{
	GetCurCamera()->SetLiftGroup(GetProject()->GetLiftGroup(1));
	if (::GetKeyState(VK_CONTROL) >= 0)
	{
		IAction *pAction = NULL;
		AuxPlay(&pAction); 
		if (pAction)
		{
			GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 6, GetViewAspectRatio());
			pAction->Release();
		}
	}
	else
		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 6, GetViewAspectRatio());
}


void CAdVisuoView::OnTmpGroup3()
{
	GetCurCamera()->SetLiftGroup(GetProject()->GetLiftGroup(2));
	if (::GetKeyState(VK_CONTROL) >= 0)
	{
		IAction *pAction = NULL;
		AuxPlay(&pAction); 
		if (pAction)
		{
			GetCurCamera()->AnimateTo(CAMLOC_LOBBY, pAction, 6, GetViewAspectRatio());
			pAction->Release();
		}
	}
	else
		GetCurCamera()->MoveTo(CAMLOC_LOBBY, 6, GetViewAspectRatio());
}


void CAdVisuoView::OnUpdateTmpGroup1(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(GetCurLiftGroupIndex() == 0);
}


void CAdVisuoView::OnUpdateTmpGroup2(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetProject()->GetLiftGroupsCount() >= 2);
	pCmdUI->SetRadio(GetCurLiftGroupIndex() == 1);
}


void CAdVisuoView::OnUpdateTmpGroup3(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetProject()->GetLiftGroupsCount() >= 3);
	pCmdUI->SetRadio(GetCurLiftGroupIndex() == 2);
}
