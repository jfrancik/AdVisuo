﻿// AdVisuoView.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include "AdVisuo.h"
#include "AdVisuoView.h"
#include "MainFrm.h"

#include "VisProject.h"
#include "VisSim.h"

#include "DlgVideo.h"
#include "DlgMat.h"
#include "DlgScript.h"

#include "RibbonScenarioButton.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEG2RAD(d)	( (d) * (AVFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (AVFLOAT)M_PI )
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
	ON_COMMAND_RANGE(ID_CAMERA_GROUP_MENU + 3000, ID_CAMERA_GROUP_MENU + 3200, &CAdVisuoView::OnCamera)
	ON_COMMAND_RANGE(ID_CAMERA_GROUPRIGHT, ID_CAMERA_GROUP_LEFT, &CAdVisuoView::OnCamera)

	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA, ID_STOREY_ONEDOWN, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_EXT_REAR, ID_CAMERA_EXT_SIDE, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_STOREY_MENU + 1000, ID_STOREY_MENU + 1300, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_LIFT_MENU + 2000, ID_CAMERA_LIFT_MENU + 2200, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_GROUP_MENU + 3000, ID_CAMERA_GROUP_MENU + 3200, &CAdVisuoView::OnUpdateCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA_GROUPRIGHT, ID_CAMERA_GROUP_LEFT, &CAdVisuoView::OnUpdateCamera)

	ON_UPDATE_COMMAND_UI(ID_STOREY_MENU, &CAdVisuoView::OnUpdateStoreyMenu)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_LIFT_MENU, &CAdVisuoView::OnUpdateCameraLiftMenu)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_GROUP_MENU, &CAdVisuoView::OnUpdateCameraGroupMenu)

	ON_COMMAND(ID_TENANCY_MENU, &CAdVisuoView::OnTenancyMenu)
	ON_UPDATE_COMMAND_UI(ID_TENANCY_MENU, &CAdVisuoView::OnUpdateTenancyMenu)
	
	ON_COMMAND(ID_SCENARIO_MENU, &CAdVisuoView::OnScenarioMenu)
	ON_UPDATE_COMMAND_UI(ID_SCENARIO_MENU, &CAdVisuoView::OnUpdateScenarioMenu)


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
	ON_UPDATE_COMMAND_UI(ID_FPS, &CAdVisuoView::OnUpdateFps)
	ON_UPDATE_COMMAND_UI(ID_ACTION_RENDER, &CAdVisuoView::OnUpdateActionRender)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_ACTION_SAVESTILL, &CAdVisuoView::OnUpdateActionSavestill)
	ON_COMMAND(ID_VIEW_MATERIALS, &CAdVisuoView::OnViewMaterials)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MATERIALS, &CAdVisuoView::OnUpdateViewMaterials)
	ON_COMMAND(ID_REC_SCRIPT, &CAdVisuoView::OnRecScript)
	ON_UPDATE_COMMAND_UI(ID_REC_SCRIPT, &CAdVisuoView::OnUpdateRecScript)
	ON_COMMAND(ID_REC_RECORD, &CAdVisuoView::OnRecRecord)
	ON_COMMAND(ID_REC_PLAY, &CAdVisuoView::OnRecPlay)
	ON_COMMAND(ID_NAVIGATION_CCTV, &CAdVisuoView::OnNavigationCctv)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATION_CCTV, &CAdVisuoView::OnUpdateNavigationCctv)
	ON_COMMAND(ID_NAVIGATION_WALK, &CAdVisuoView::OnNavigationWalk)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATION_WALK, &CAdVisuoView::OnUpdateNavigationWalk)
	ON_COMMAND(ID_NAVIGATION_GHOST, &CAdVisuoView::OnNavigationGhost)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATION_GHOST, &CAdVisuoView::OnUpdateNavigationGhost)
	ON_COMMAND(ID_CHARACTER_NOCOLOURCODING, &CAdVisuoView::OnCharacterNocolourcoding)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_NOCOLOURCODING, &CAdVisuoView::OnUpdateCharacterNocolourcoding)
	ON_COMMAND(ID_CHARACTER_CURRENTWAITINGTIME, &CAdVisuoView::OnCharacterCurrentwaitingtime)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_CURRENTWAITINGTIME, &CAdVisuoView::OnUpdateCharacterCurrentwaitingtime)
	ON_COMMAND(ID_CHARACTER_EXPECTEDWAITINGTIME, &CAdVisuoView::OnCharacterExpectedwaitingtime)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_EXPECTEDWAITINGTIME, &CAdVisuoView::OnUpdateCharacterExpectedwaitingtime)
	ON_COMMAND(ID_FILE_SIMULATION, &CAdVisuoView::OnFileSimulation)
	ON_COMMAND(ID_FILE_DOWNLOAD, &CAdVisuoView::OnFileDownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_DOWNLOAD, &CAdVisuoView::OnUpdateFileDownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_SIMULATION, &CAdVisuoView::OnUpdateFileSimulation)
END_MESSAGE_MAP()

// CAdVisuoView construction/destruction

CAdVisuoView::CAdVisuoView() : m_screen(&m_engine, 2), m_sprite(&m_engine), m_plateCam(&m_sprite), m_hud(&m_sprite), m_script(this), m_instRec(this)
{
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
	m_nLGButFloor = 0x7fffffff;
	m_pbutLift = NULL;
	m_pbutGroup = NULL;
	m_nCamExtSideOption = 2;
	m_bLock = false;
}

CAdVisuoView::~CAdVisuoView()
{
	for (int i = 0; i < N_CAMERAS; i++) 
		if (m_pCamera[i]) delete m_pCamera[i];
}

///////////////////////////////////////////////////////////////////////////////////////
// Initialisation and Life-Cycle

BOOL CAdVisuoView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CAdVisuoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	CWaitCursor wait_cursor;

	// initialise FreeWill
	if (!m_engine.Create(m_hWnd, this) || !m_engine.IsReady())
	{
		::PostQuitMessage(100);
		return;
	}
	OutText(L"Initialising video subsystems.");

	GetProject()->SetEngine(&m_engine);

	// Initialise HUD
	m_sprite.Initialise();
	m_plateCam.SetParams((_stdPathModels + L"plateNW.bmp").c_str(), 0xFF0000FF, 0x80FFFFFF, 12, TRUE, FALSE, L"System", 0xFF000000, 16, false, CSize(2, 2));
	m_hud.Initialise();
	
	// adjust to the screen size
	OnAdjustViewSize();

	AVULONG nStatus = GetDocument()->GetLoaderStatus();
	if (nStatus <= CAdVisuoLoader::LOADING_STRUCTURE)
	{
		OutText(L"Waiting for the project...");
		Sleep(250);
		nStatus = GetDocument()->GetLoaderStatus();
		while (nStatus <= CAdVisuoLoader::LOADING_STRUCTURE)
		{
			if (!OutWaitMessage(GetDocument()->GetLoaderPosInQueue(), 250))
			{
				OutText(L"Stopping download...");
				GetDocument()->StopDownload();
				OutText(L"Download stopped...");
				SetTimer(101, 1000 / 100, NULL);
				return;
			}
			nStatus = GetDocument()->GetLoaderStatus();
		}
		OutWaitMessage(-1, 100);
	}
	if (nStatus == CAdVisuoLoader::TERMINATED && GetDocument()->GetLoaderReasonForTermination() == CAdVisuoLoader::FAILED)
	{
		OutText(L"Project loading failed...");
		SetTimer(101, 1000 / 100, NULL);
		return;
	}

	ASSERT(nStatus == CAdVisuoLoader::LOADING_DATA || nStatus == CAdVisuoLoader::TERMINATED && GetDocument()->GetLoaderReasonForTermination() == CAdVisuoLoader::COMPLETE);

/*	// test & wait for the project to build
	AVULONG nStat;
	if (!GetDocument()->GetDownloadStatus(nStat))
	{
		OutText(L"Waiting for the project...");
		Sleep(250);		// give it a second chance...
		while (!GetDocument()->GetDownloadStatus(nStat))
		{
			if (nStat >= 0x80000000)
			{
				// error...
				OutText(L"Project loading failed...");
				GetDocument()->StopDownload();
				OutText(L"Download stopped XXX...");
				return;
			}

			if (!OutWaitMessage(nStat, 250))
			{
				// user-requested stop
				OutText(L"Stopping download...");
				GetDocument()->StopDownload();
				OutText(L"Download stopped...");
				return;
			}
		}	
	}*/


	// initialise the simulation
	GetDocument()->ResetTitle();
	LONG maxtime = GetProject()->GetMaxTime();
	m_hud.SetSimulationTime(maxtime);
	OutText(L"Creating building structure...");
	m_engine.InitMats(GetProject()->GetMaxStoreyCount(), GetProject()->GetMaxBasementStoreyCount(), GetProject()->GetMaxShaftCount());
	GetProject()->Construct();
	GetProject()->StoreConfig();

	if (GetProject()->GetLiftGroupsCount() == 0)
		return;

	OutText(L"Initialising cameras...");
	// initialise cameras
	for (int i = 0; i < N_CAMERAS; i++) 
	{
		m_pCamera[i] = new CCamera(); 
		m_pCamera[i]->Create(&m_engine, GetProject(), i, 0, 0); 
	}
	AVULONG nStorey = GetProject()->GetLiftGroup(0)->GetBasementStoreyCount();

	switch (GetProject()->GetLiftGroupsCount())
	{
	case 1:
		GetCamera(0)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(0)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);
		GetCamera(1)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(1)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR    - ID_CAMERA - 1);
		GetCamera(2)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(2)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTFRONT   - ID_CAMERA - 1);
		GetCamera(3)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(3)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTREAR     - ID_CAMERA - 1);
		GetCamera(4)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(4)->MoveTo(CAMLOC_OUTSIDE, 0);
		GetCamera(5)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(5)->MoveTo(CAMLOC_OUTSIDE, 1);
		break;
	case 2:
		GetCamera(0)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(0)->MoveTo(CAMLOC_LIFTGROUP, 0); GetCamera(0)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);
		GetCamera(1)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(1)->MoveTo(CAMLOC_LIFTGROUP, 0); GetCamera(1)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR    - ID_CAMERA - 1);
		GetCamera(2)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(2)->MoveTo(CAMLOC_LIFTGROUP, 1); GetCamera(2)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(3)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(3)->MoveTo(CAMLOC_LIFTGROUP, 1); GetCamera(3)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(4)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(4)->MoveTo(CAMLOC_OUTSIDE, 0);
		GetCamera(5)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(5)->MoveTo(CAMLOC_OUTSIDE, 1);
		break;
	case 3:
		GetCamera(0)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(0)->MoveTo(CAMLOC_LIFTGROUP, 0); GetCamera(0)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);
		GetCamera(1)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(1)->MoveTo(CAMLOC_LIFTGROUP, 1); GetCamera(1)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(2)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(2)->MoveTo(CAMLOC_LIFTGROUP, 2); GetCamera(2)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(3)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(3)->MoveTo(CAMLOC_LIFTGROUP, 0); GetCamera(3)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR    - ID_CAMERA - 1);
		GetCamera(4)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(4)->MoveTo(CAMLOC_LIFTGROUP, 2); GetCamera(4)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(5)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(5)->MoveTo(CAMLOC_LIFTGROUP, 2); GetCamera(5)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		break;
	default:
		GetCamera(0)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(0)->MoveTo(CAMLOC_LIFTGROUP, 0); GetCamera(0)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);
		GetCamera(1)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(1)->MoveTo(CAMLOC_LIFTGROUP, 1); GetCamera(1)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR    - ID_CAMERA - 1);
		GetCamera(2)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(2)->MoveTo(CAMLOC_LIFTGROUP, 2); GetCamera(2)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTFRONT   - ID_CAMERA - 1);
		GetCamera(3)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(3)->MoveTo(CAMLOC_LIFTGROUP, 3); GetCamera(3)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTREAR     - ID_CAMERA - 1);
		GetCamera(4)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(4)->MoveTo(CAMLOC_LIFTGROUP, 2); GetCamera(4)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		GetCamera(5)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(5)->MoveTo(CAMLOC_LIFTGROUP, 3); GetCamera(5)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT    - ID_CAMERA - 1);
		break;
	}
	GetCamera(6)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(6)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);
	GetCamera(7)->MoveTo(CAMLOC_STOREY, nStorey); GetCamera(7)->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT	- ID_CAMERA - 1);

	OutText(L"Building created, ready for rendering.");

	// first render cycle
	m_engine.BeginFrame();
	RenderScene();
	m_engine.EndFrame();

	// necessary to initialise
	Stop();
	
	SetTimer(101, 1000 / 100, NULL);
	m_hud.KeepReady();
}

void CAdVisuoView::OnAdjustViewSize()
{
	m_hud.SetPos(CPoint(m_engine.GeViewtWidth() / 2 - 256, m_engine.GetViewHeight() - 64));
	m_hud.SetAltPos(CPoint(m_engine.GetViewSize()));
	m_sprite.OnResize();
	OnAdjustCameras();
}

void CAdVisuoView::OnAdjustCameras()
{
	for (AVULONG i = 0; i < m_screen.GetCount(); i++)
		if (m_screen.IsEnabled(i))
		{
			CCamera *pCamera = GetCamera(m_screen.GetCamera(i));
			if (pCamera) pCamera->Adjust(m_screen.GetAspectRatio(i));
		}
}

void CAdVisuoView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	if (AVGetMainWnd()->MDIGetActive() == GetParentFrame())
	{
		OnScanKey();
		InvalidateRect(NULL, FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// ILostDeviceObserver implementation

void CAdVisuoView::OnLostDevice()
{
	m_sprite.OnLostDevice();
	m_plateCam.OnLostDevice();
	m_hud.OnLostDevice();
}

void CAdVisuoView::OnResetDevice()
{
	m_sprite.OnResetDevice();
	m_plateCam.OnResetDevice();
	m_hud.OnResetDevice();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Play Control

void CAdVisuoView::Play()
{
	if (CRibbonScenarioButton::IsDirty())
	{
		Stop();
		CRibbonScenarioButton::ClearDirtyFlag();
	}

	m_engine.ResetAccel();
	m_engine.Play();
	m_script.Play();
}

void CAdVisuoView::Stop()
{
	CWaitCursor wait;

	m_engine.Stop();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
	{
		GetProject()->GetLiftGroup(i)->GetCurSim()->Stop();
		GetProject()->GetLiftGroup(i)->RestoreConfig();
	}

	// prepare sim...
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		GetProject()->GetLiftGroup(i)->GetCurSim()->Play(&m_engine);
	for (AVLONG t = GetProject()->GetSimulationStartTime(); t <= 0; t += 40)
		m_engine.Proceed(t);	// loops un-nested on 24/1/13: Proceed was called too often!
}

void CAdVisuoView::Pause()
{
	m_engine.Pause();
}

void CAdVisuoView::Rewind(AVULONG nMSec)
{
	CWaitCursor wait;

	m_engine.Stop();
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
	{
		GetProject()->GetLiftGroup(i)->GetCurSim()->Stop();
		GetProject()->GetLiftGroup(i)->RestoreConfig();
	}

	AVLONG t;
	for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		t = GetProject()->GetLiftGroup(i)->GetCurSim()->FastForward(&m_engine, nMSec);

	for ( ; t <= (AVLONG)nMSec; t += 40)
		m_engine.Proceed(t);
	
	m_engine.ResetAccel();
	m_engine.Play(t);
}

///////////////////////////////////////////////////////////////////////////////////////
// Timer, Cursor and Size Handlers

void CAdVisuoView::OnTimer(UINT_PTR nIDEvent)
{
	// keep download process alive
	AVULONG nStatus = GetDocument()->UpdateDownload(&m_engine);
	if (nStatus == CAdVisuoLoader::TERMINATED && GetDocument()->GetLoaderReasonForTermination() != CAdVisuoLoader::COMPLETE)
	{
		KillTimer(nIDEvent);
		GetDocument()->OnCloseDocument();
		return;
	}

	// #FreeWill: Push the time info into the engine
	if (m_engine.IsPlaying() && !m_bLock)
	{
		AVLONG nTime = m_engine.GetPlayTime();
		m_script.Proceed(nTime);
		m_engine.Proceed(nTime);
		if (!m_engine.IsRunning())
			OnActionStop();
	}
	m_engine.ProceedAux(GetTickCount());
	
	if (m_engine.GetPlayTime() > GetProject()->GetMaxTime())
		OnActionStop();

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
			SetCursor(AVGetApp()->LoadStandardCursor(IDC_ARROW));
		else
		{
			m_screen.HitTest(pt, hit, nDummy, nDummy);
			switch (hit)
			{
			case CScreen::HIT_VIEW:	SetCursor(AVGetApp()->LoadStandardCursor(IDC_CROSS)); break;
			case CScreen::HIT_XDIV:	SetCursor(AVGetApp()->LoadStandardCursor(IDC_SIZEWE)); break;
			case CScreen::HIT_YDIV:	SetCursor(AVGetApp()->LoadStandardCursor(IDC_SIZENS)); break;
			case CScreen::HIT_XYDIV:SetCursor(AVGetApp()->LoadStandardCursor(IDC_SIZEALL)); break;
			default: return CView::OnSetCursor(pWnd, nHitTest, message);
			}
		}
	}
	return TRUE;
}

void CAdVisuoView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (cx && cy && m_engine.IsReady())
		OnAdjustViewSize();
}

///////////////////////////////////////////////////////////////////////////////////////
// Rendering

void CAdVisuoView::OnDraw(CDC *pDC)
{
	ASSERT_VALID(GetDocument());

	if (m_bLock) return;

	if (!GetDocument() || !m_engine.IsReady() || !GetCurCamera() || !GetCurCamera()->IsReady())
	{
		// if anything went wrong or is not yet ready...
		CRect rect;
		GetClientRect(rect);
		::FillRect(pDC->m_hDC, rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
	}
	else
	{
		m_engine.BeginFrame();
		RenderScene(m_bHUDSelection);
		RenderHUD(m_bHUDPanel, false, m_bHUDCaption, m_bHUDSelection, m_engine.GetPlayTime());
		m_engine.EndFrame();
	}
}

void CAdVisuoView::RenderScene(bool bHUDSelection)
{
	CAdVisuoRenderer renderer(&m_engine);

	AVCOLOR active = { 1, 0.86f, 0.47f }, inactive = { 1, 1, 1 };
	m_screen.Prepare(inactive, active, bHUDSelection);

	for (AVULONG i = 0; i < m_screen.GetCount(); i++)
		if (m_screen.IsEnabled(i))
		{
			m_screen.Prepare(i, bHUDSelection);
			renderer.Render(GetProject(), GetCamera(m_screen.GetCamera(i)));
		}
}

void CAdVisuoView::RenderHUD(bool bHUDPanel, bool bHUDClockOnly, bool bHUDCaption, bool bHUDSelection, AVLONG nTime)
{
	m_screen.Prepare();

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
			m_screen.GetViewport(i, x0, x1, y0, y1, bHUDSelection);

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
		m_hud.SetItemStatus(CHUD::HIT_PLAY, m_engine.IsPlaying() && !m_engine.IsPaused() ? 2 : 0);
		m_hud.SetItemStatus(CHUD::HIT_FULL_SCREEN, ::AVGetMainWnd()->IsFullScreen() ? 2 : 0);
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
								AVLONG nTimeFrom, AVLONG nTimeTo, bool bShowCaptions, bool bShowClock, CBUpdate fn)
{
	m_bLock = true;
	bool bResult = true;

	// Prepare...
	AVFLOAT fAR = m_screen.GetAspectRatio();
	m_screen.SetAspectRatio((AVFLOAT)nResX / (AVFLOAT)nResY);
	OnAdjustCameras();

	Rewind(nTimeFrom);

	try
	{
//		m_engine.Stop();
		m_engine.StartTargetToVideo(CSize(nResX, nResY), lpszFilename, nFPS);

		// first frame - to initialise
		m_script.Play();
		AVLONG t = nTimeFrom;
		m_engine.Proceed(t);

		m_engine.SetTargetOffScreen();
		m_hud.SetPos(CPoint(nResX/2 - 256, nResY - 64));
		m_hud.SetAltPos(CPoint(nResX, nResY));
		m_sprite.OnResize();
		m_engine.BeginFrame();
		RenderScene();
		RenderHUD(bShowClock, true, bShowCaptions, false, t);
		m_engine.EndFrame();
		t += 1000 / nFPS;

		AVGetMainWnd()->SetActiveWindow();

		// all other frames
		for (t; t <= nTimeTo; t += m_engine.GetAccel() * 1000 / nFPS)
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
			m_engine.SetPlayTime(t);
			m_engine.Proceed(t);
			m_engine.ProceedAux(t);

			m_engine.SetTargetOffScreen();
			m_engine.BeginFrame();
			RenderScene();
			RenderHUD(bShowClock, true, bShowCaptions, false, t);
			m_engine.EndFrame();

			bool bStop = false, bPreview = false;
			fn(L"Rendering to file...", t, bStop, bPreview);
			if (bStop) break;

			if (bPreview)
			{
				m_engine.SetTargetToScreen();
				m_engine.BeginFrame();
				RenderScene();
				RenderHUD(bShowClock, true, bShowCaptions, false, t);
				m_engine.EndFrame();
			}
		}

		bool bStop = false, bPreview = false;
		fn(L"Finalising...", t, bStop, bPreview);

		m_engine.SetTargetToScreen();
		m_engine.BeginFrame();
		RenderScene();
		RenderHUD(false, true, bShowCaptions, false, 0);
		m_engine.EndFrame();
		for (int i = 0; i < 40; i++)
		{
			m_engine.SetTargetOffScreen();
			m_engine.BeginFrame();
			RenderScene();
			RenderHUD(false, false, bShowCaptions, false, 0);
			m_engine.EndFrame();
		}
		m_engine.DoneTargetOffScreen();
	}
	catch (...) 
	{ 
		m_engine.DoneTargetOffScreen();
		m_engine.Stop();
		bResult = false;
	}

	Stop();

	m_screen.SetAspectRatio(fAR);
	OnAdjustViewSize();
	m_bLock = false;
	return bResult;
}

bool CAdVisuoView::RenderToBitmap(LPCTSTR pFilename)
{
	AVULONG x0, x1, y0, y1;
	m_screen.Get(x0, x1, y0, y1);

	m_engine.StartTargetToImage(CSize(x1 - x0, y1 - y0), pFilename);
	m_engine.BeginFrame();
	RenderScene();
	// RenderHUD(true, true, true, false);
	m_engine.EndFrame();
	m_engine.DoneTargetOffScreen();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// Camera Control

void CAdVisuoView::OnDrag(int dx, int dy, int dz, bool bShift, bool bCtrl, bool bAlt)
{
	if (!GetCurCamera()) return;

	// if (bCtrl) if (abs(dx) > abs(dy)) dy = 0; else dx = 0;

	switch (SETTINGS::nNavMode)
	{
	case 0:
	case 2:
		// CCTV & Ghost Mode
		if (dx != 0) !bShift ? GetCurCamera()->Pan(DEG2RAD((AVFLOAT)dx/5.0f)) : GetCurCamera()->Move(-(AVFLOAT)dx, 0, 0);
		if (dy != 0) !bShift ? GetCurCamera()->Tilt(DEG2RAD(-(AVFLOAT)dy/5.0f)) : GetCurCamera()->Move(0, 0, -(AVFLOAT)dy);
		if (dz != 0) !bShift ? GetCurCamera()->Zoom(DEG2RAD((AVFLOAT)dz / 2.0f)) : GetCurCamera()->Zoom(DEG2RAD((AVFLOAT)dz / 2.0f));
		break;
	case 1:
		// Walk Mode
		if (dx != 0 && !bShift) GetCurCamera()->Pan(DEG2RAD((AVFLOAT)dx/5.0f));
		if (dy != 0 && !bShift) GetCurCamera()->Move(0, -(AVFLOAT)dy, 0);
		if (dx != 0 &&  bShift) GetCurCamera()->Move(-(AVFLOAT)dx, 0, 0);
		if (dy != 0 &&  bShift) GetCurCamera()->Move(0, 0, -(AVFLOAT)dy);
		if (dz != 0) GetCurCamera()->Tilt((AVFLOAT)dz/10.0f);
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
	AVFLOAT fAngle = nSpan * (AVFLOAT)M_PI/180/30;
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
//	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


///////////////////////////////////////////////////////////////////////////////////////
// Camera


void CAdVisuoView::OnSelCamera(UINT nCmd)
{
	if (GetProject()->GetLiftGroupsCount() == 0) return;

	AVLONG nCamera = nCmd - ID_SELCAMERA_1;
	if (nCamera >= 0 && nCamera < N_CAMERAS)
	{
		m_screen.SetCurCamera(nCamera);
		GetCurCamera()->Adjust(m_screen.GetCurAspectRatio());
	}
}

void CAdVisuoView::OnUpdateSelCamera(CCmdUI *pCmdUI)
{
	if (GetProject()->GetLiftGroupsCount() == 0) 
	{
		pCmdUI->Enable(FALSE);
		return;
	}

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
		if (nCmd >= ID_STOREY_MENU + 1000 && nCmd < ID_STOREY_MENU + 1300)
			GetCurCamera()->AnimateTo(CAMLOC_STOREY, nCmd - ID_STOREY_MENU - 1000, GetViewAspectRatio());
		else if (nCmd >= ID_CAMERA_LIFT_MENU + 2000 && nCmd < ID_CAMERA_LIFT_MENU + 2200)
			GetCurCamera()->AnimateTo(CAMLOC_LIFT, nCmd - ID_CAMERA_LIFT_MENU - 2000, GetViewAspectRatio());
		else if (nCmd >= ID_CAMERA_GROUP_MENU + 3000 && nCmd < ID_CAMERA_GROUP_MENU + 3200)
			GetCurCamera()->AnimateTo(CAMLOC_LIFTGROUP, nCmd - ID_CAMERA_GROUP_MENU - 3000, GetViewAspectRatio());
		else switch (nCmd)
		{
			case ID_CAMERA_OVERHEAD:		GetCurCamera()->AnimateTo(CAMLOC_OVERHEAD, 0, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    0, GetViewAspectRatio()); break;
			case ID_CAMERA_CENTRALREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    1, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTREAR:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    2, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTSIDE:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    3, GetViewAspectRatio()); break;
			case ID_CAMERA_RIGHTFRONT:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    4, GetViewAspectRatio()); break;
			case ID_CAMERA_CENTRALFRONT:	GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    5, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTFRONT:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    6, GetViewAspectRatio()); break;
			case ID_CAMERA_LEFTSIDE:		GetCurCamera()->AnimateTo(CAMLOC_LOBBY,    7, GetViewAspectRatio()); break;
			
			case ID_CAMERA_LIFT_MENU:		break;
			case ID_CAMERA_LIFTRIGHT:		GetCurCamera()->AnimateTo(CAMLOC_LIFT, GetCurCamera()->GetLift()-1, GetViewAspectRatio()); break;
			case ID_CAMERA_LIFTLEFT:		GetCurCamera()->AnimateTo(CAMLOC_LIFT, GetCurCamera()->GetLift()+1, GetViewAspectRatio()); break;

			case ID_CAMERA_GROUP_MENU:		break;
			case ID_CAMERA_GROUPRIGHT:		GetCurCamera()->AnimateTo(CAMLOC_LIFTGROUP, CAMLOC_PREV, GetViewAspectRatio()); break;
			case ID_CAMERA_GROUP_LEFT:		GetCurCamera()->AnimateTo(CAMLOC_LIFTGROUP, CAMLOC_NEXT, GetViewAspectRatio()); break;

			case ID_STOREY_MENU:			break;
			case ID_STOREY_ONEUP:			GetCurCamera()->AnimateTo(CAMLOC_STOREY, CAMLOC_NEXT, GetViewAspectRatio()); break;
			case ID_STOREY_ONEDOWN:			GetCurCamera()->AnimateTo(CAMLOC_STOREY, CAMLOC_PREV, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_FRONT:		GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, 0, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_REAR:		GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, 1, GetViewAspectRatio()); break;
			case ID_CAMERA_EXT_SIDE:		if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 2) 
												GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, m_nCamExtSideOption = 3, GetViewAspectRatio());
											else if (desc.camloc == CAMLOC_OUTSIDE && desc.index == 3)
												GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, m_nCamExtSideOption = 2, GetViewAspectRatio());
											else 
												GetCurCamera()->AnimateTo(CAMLOC_OUTSIDE, m_nCamExtSideOption, GetViewAspectRatio()); 
											break;
		}
	}
	else
	{
		// With Control Key - fast motion
		if (nCmd >= ID_STOREY + 1000 && nCmd < ID_STOREY + 1200)
			GetCurCamera()->MoveTo(CAMLOC_STOREY, nCmd - ID_STOREY - 1000, GetViewAspectRatio());
		else if (nCmd >= ID_CAMERA_LIFT_MENU + 2000 && nCmd < ID_CAMERA_LIFT_MENU + 2200)
			GetCurCamera()->MoveTo(CAMLOC_LIFT, nCmd - ID_CAMERA_LIFT_MENU - 2000, GetViewAspectRatio());
		else if (nCmd >= ID_CAMERA_GROUP_MENU + 3000 && nCmd < ID_CAMERA_GROUP_MENU + 3200)
			GetCurCamera()->MoveTo(CAMLOC_LIFTGROUP, nCmd - ID_CAMERA_GROUP_MENU - 3000, GetViewAspectRatio());
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
			case ID_CAMERA_GROUPRIGHT:		GetCurCamera()->MoveTo(CAMLOC_LIFTGROUP, GetCurCamera()->GetLiftGroup()-1, GetViewAspectRatio()); break;
			case ID_CAMERA_GROUP_LEFT:		GetCurCamera()->MoveTo(CAMLOC_LIFTGROUP, GetCurCamera()->GetLiftGroup()+1, GetViewAspectRatio()); break;
			case ID_STOREY_ONEUP:			GetCurCamera()->MoveTo(CAMLOC_STOREY, GetProject()->GetLiftGroup(GetCurCamera()->GetLiftGroup())->GetFloorUp(GetCurCamera()->GetStorey()), GetViewAspectRatio()); break;
			case ID_STOREY_ONEDOWN:			GetCurCamera()->MoveTo(CAMLOC_STOREY, GetProject()->GetLiftGroup(GetCurCamera()->GetLiftGroup())->GetFloorDown(GetCurCamera()->GetStorey()), GetViewAspectRatio()); break;
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
	else if (pCmdUI->m_nID >= ID_CAMERA_GROUP_MENU + 3000 && pCmdUI->m_nID < ID_CAMERA_GROUP_MENU + 3200)
		pCmdUI->SetCheck(pCmdUI->m_nID - ID_CAMERA_GROUP_MENU - 3000 == GetCurCamera()->GetLiftGroup());
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
	case ID_CAMERA_LIFTLEFT:		pCmdUI->Enable(desc.camloc == CAMLOC_LIFT && GetCurCamera()->GetLift() < (AVLONG)GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->GetShaftCount() - 1); break;
	
	case ID_CAMERA_GROUPRIGHT:		pCmdUI->Enable(GetCurCamera()->GetLiftGroup() > 0); break;
	case ID_CAMERA_GROUP_LEFT:		pCmdUI->Enable(GetCurCamera()->GetLiftGroup() < (AVLONG)GetProject()->GetLiftGroupsCount() - 1); break;

	case ID_STOREY_ONEUP:			pCmdUI->Enable(desc.camloc != CAMLOC_LIFT && GetCurCamera()->GetStorey() < (AVLONG)GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->GetHighestStoreyServed()); break;
	case ID_STOREY_ONEDOWN:			pCmdUI->Enable(desc.camloc != CAMLOC_LIFT && GetCurCamera()->GetStorey() > (AVLONG)GetProject()->GetLiftGroup(GetCurLiftGroupIndex())->GetLowestStoreyServed());break;

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
	pCmdUI->Enable(GetCurCamera() != NULL);
	if (GetCurCamera() == NULL) return;

	if (GetProject()->GetLiftGroupsCount() == 0) return;

	CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
	if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
	{
		auto pSubItems = &pButton->GetSubItems();
		INT_PTR n = pSubItems->GetSize();
		if (n && (*pSubItems)[n - 1] == m_pbutFloor && m_nLGButFloor == GetCurLiftGroupIndex())
			return;		// we already have this menu in place

		// create the floors menu
		pButton->RemoveAllSubItems();
		m_nLGButFloor = GetCurLiftGroupIndex();
		CLiftGroupVis *pLiftGroup = GetProject()->GetLiftGroup(m_nLGButFloor);
		for (AVLONG i = pLiftGroup->GetStoreyCount() - 1; i >= 0; i--)
		{
			if (!pLiftGroup->IsStoreyServed(i)) continue;

			m_pbutFloor = new CMFCRibbonButton(ID_STOREY_MENU + 1000 + i, pLiftGroup->GetStorey(i)->GetName().c_str());
			m_pbutFloor->SetDefaultMenuLook();
			pButton->AddSubItem(m_pbutFloor);

			if ((AVLONG)i < (AVLONG)pLiftGroup->GetStoreyCount() - 10)
			{
				if (pLiftGroup->GetStoreyCount() > 60 && i > 30) i -= 4;
				if (pLiftGroup->GetStoreyCount() > 120 && i > 100) i -= 5;
			}
		}
	}
}

void CAdVisuoView::OnUpdateCameraLiftMenu(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCurCamera() != NULL);
	if (GetCurCamera() == NULL) return;

	if (GetProject()->GetLiftGroupsCount() == 0) return;

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
		for (AVULONG i = 0; i < pLiftGroup->GetShaftCount(); i++)
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

void CAdVisuoView::OnUpdateCameraGroupMenu(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetProject()->GetLiftGroupsCount() >= 2);

	if (GetProject()->GetLiftGroupsCount() == 0) return;

	CMFCRibbonButton *pButton = ((CMFCRibbonButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);
	if (pButton && pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonButton)))
	{
		auto pSubItems = &pButton->GetSubItems();
		INT_PTR n = pSubItems->GetSize();
		if (n && (*pSubItems)[n - 1] == m_pbutGroup)
			return;		// we already have this menu in place

		// create the floors menu
		pButton->RemoveAllSubItems();
		for (AVULONG i = 0; i < GetProject()->GetLiftGroupsCount(); i++)
		{
			m_pbutGroup = new CMFCRibbonButton(ID_CAMERA_GROUP_MENU + 3000 + i, GetProject()->GetLiftGroup(i)->GetName().c_str());
			m_pbutGroup->SetDefaultMenuLook();
			pButton->AddSubItem(m_pbutGroup);
		}
	}
}

void CAdVisuoView::OnTenancyMenu()
{
}

void CAdVisuoView::OnUpdateTenancyMenu(CCmdUI *pCmdUI)
{
}

void CAdVisuoView::OnScenarioMenu()
{
	Stop();
	CRibbonScenarioButton::ClearDirtyFlag();
}

void CAdVisuoView::OnUpdateScenarioMenu(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_engine.IsReady() && !m_engine.IsPlaying()); 

	if (!pCmdUI->m_pOther)
		return;

	CRibbonScenarioButton *pButton = ((CRibbonScenarioButton*)((CMFCRibbonCmdUI*)pCmdUI)->m_pUpdated);

	if (pButton->GetProject() != GetProject())
		pButton->SetProject(GetProject());
}

///////////////////////////////////////////////////////////////////////////////////////
// Actions

void CAdVisuoView::OnActionPlay()	{ if ( m_engine.IsPlaying()) return; Play();  OutText(L"Visualisation started..."); }
void CAdVisuoView::OnActionPause()	{ if (!m_engine.IsPlaying()) return; Pause(); OutText(m_engine.IsPaused() ? L"Visualisation paused..." : L"Visualisation resumed..."); }
void CAdVisuoView::OnActionStop()	{ if (!m_engine.IsPlaying()) return; Stop();  OutText(L"Visualisation stopped..."); }

void CAdVisuoView::OnActionSlowdown()		{ m_engine.SlowDown(); }
void CAdVisuoView::OnActionSpeedup()		{ m_engine.SpeedUp(); }
void CAdVisuoView::OnActionNormalpace()		{ m_engine.ResetAccel(); }

void CAdVisuoView::OnUpdateActionPlay(CCmdUI *pCmdUI)		{ pCmdUI->Enable(m_engine.IsReady() && !m_engine.IsPlaying()); pCmdUI->SetCheck(m_engine.IsPlaying()); }
void CAdVisuoView::OnUpdateActionPause(CCmdUI *pCmdUI)		{ pCmdUI->Enable(m_engine.IsPlaying()); pCmdUI->SetCheck(m_engine.IsPaused()); }
void CAdVisuoView::OnUpdateActionStop(CCmdUI *pCmdUI)		{ pCmdUI->Enable(m_engine.IsPlaying()); }
void CAdVisuoView::OnUpdateActionSlowdown(CCmdUI *pCmdUI)	{ pCmdUI->Enable(m_engine.IsPlaying()); }
void CAdVisuoView::OnUpdateActionSpeedup(CCmdUI *pCmdUI)	{ pCmdUI->Enable(m_engine.IsPlaying()); }
void CAdVisuoView::OnUpdateActionNormalpace(CCmdUI *pCmdUI)	{ pCmdUI->Enable(m_engine.IsPlaying()); }

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
	CMDIFrameWndEx *pMainWnd = ::AVGetMainWnd();
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

		CRect rect;
		GetDesktopWindow()->GetWindowRect(rect);
		pMainWnd->SetWindowPos(&wndTopMost, -13, -54, rect.Width() + 26, rect.Height() + 54 + 13, SWP_SHOWWINDOW);

		g_bFullScreen = true;
		m_engine.ResetDevice(NULL);
	}
	else
	{
		pMainWnd->ShowFullScreen();
		pMainWnd->ShowWindow(SW_MAXIMIZE);
		if (!m_bMaximised) pMainWnd->ShowWindow(SW_RESTORE);

		pMainWnd->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		m_engine.ResetDevice(m_hWnd);
		g_bFullScreen = false;
	}

	OnAdjustViewSize();

	if (!pMainWnd->IsFullScreen()) 
		Sleep(1500);
}

void CAdVisuoView::OnUpdateViewFullScreen(CCmdUI *pCmdUI)
{
	CMDIFrameWndEx *pMainWnd = ::AVGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)));
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(pMainWnd->IsFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////
// HUD specific

void CAdVisuoView::OnActionPlayspecial()
{
	if (!m_engine.IsReady())
		return;

	if (!m_engine.IsPlaying())
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
	AVGetMainWnd()->EnableWindow(FALSE);
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
	AVGetMainWnd()->EnableWindow(TRUE);
	AVGetMainWnd()->SetActiveWindow();
}

void CAdVisuoView::OnActionSavestill()
{
	CFileDialog dlg(FALSE, L"jpg", L"*.jpg", OFN_OVERWRITEPROMPT, L"JPG Files (*.jpg)|*.jpg|BMP Files (*.bmp)|*.bmp|Targa Files (*.tga)|*.tga|PNG Files (*.png)|*.png|All Files|*.*||");
	if (dlg.DoModal() == IDOK)
		RenderToBitmap(dlg.GetPathName());
}

void CAdVisuoView::OnUpdateActionRender(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_engine.IsPlaying() && !AVGetMainWnd()->IsFullScreen());
	pCmdUI->SetCheck(m_engine.IsPlaying());
}

void CAdVisuoView::OnUpdateActionSavestill(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!AVGetMainWnd()->IsFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////
// Modes

void CAdVisuoView::OnNavigationCctv()									{ SETTINGS::nNavMode = 0; }
void CAdVisuoView::OnUpdateNavigationCctv(CCmdUI *pCmdUI)				{ pCmdUI->SetRadio(SETTINGS::nNavMode == 0); pCmdUI->Enable(FALSE); }
void CAdVisuoView::OnNavigationWalk()									{ SETTINGS::nNavMode = 1; }
void CAdVisuoView::OnUpdateNavigationWalk(CCmdUI *pCmdUI)				{ pCmdUI->SetRadio(SETTINGS::nNavMode == 1); pCmdUI->Enable(FALSE); }
void CAdVisuoView::OnNavigationGhost()									{ SETTINGS::nNavMode = 2; 
																		/*	// TEMPORARY CODE!!!
																			OutText(L"Loading complete");
																			for each (CLiftGroup *pGroup in GetProject()->GetLiftGroups())
																				for each (CSim *pSim in pGroup->GetSims())
																					OutText(L"SIM - Passengers: %d, journeys: %d", pSim->GetPassengerCount(), pSim->GetJourneyTotalCount());
																			OutText(L"Storing details to advisuo.txt");

																			std::wofstream myfile;
																			myfile.open ("c:\\users\\jarek\\desktop\\advisuo.txt");
																			for each (CLiftGroupVis *pGroup in GetProject()->GetLiftGroups())
																			{
																				myfile << L"LIFT GROUP: " << pGroup->GetName() << std::endl;
																				for each (CSimVis *pSim in pGroup->GetSims())
																				{
																					myfile << L"  SCENARIO: " << pSim->GetScenarioName() << std::endl;
																					for each (CLift *pLift in pSim->GetLifts())
																					{
																						myfile << L"    LIFT: " << pLift->GetId() << std::endl;
																						for each (JOURNEY j in pLift->GetJourneys())
																							if (j.m_floorTo != (AVULONG)-1)
																								myfile << L"      JOURNEY: from " << j.m_floorFrom << L" to " << j.m_floorTo << L", " << j.m_timeGo/1000 << L" => " << j.m_timeDest/1000 << L"; doorcycles: " << j.StringifyDoorCycles() << std::endl;
																					}
																					myfile << L"  - PASSENGERS:" << std::endl;
																					std::sort(pSim->GetPassengers().begin(), pSim->GetPassengers().end(), [](CPassenger *p1, CPassenger *p2) -> bool 
																					{ 
																						if (p1->GetLiftId() == p2->GetLiftId())
																							return p1->GetLoadTime() < p2->GetLoadTime(); 
																						else
																							return p1->GetLiftId() < p2->GetLiftId(); 
																					});
																					for each (CPassenger *pPassenger in pSim->GetPassengers())
																						myfile << L"    PASSENGER: lift: " << pPassenger->GetLiftId() << L" from " << pPassenger->GetArrivalFloor() << L" to " << pPassenger->GetDestFloor() << L", " << pPassenger->GetLoadTime() << L" => " << pPassenger->GetUnloadTime() << std::endl;
																				}
																			}
																			myfile.close();
																			OutText(L"Done."); */
																		}
void CAdVisuoView::OnUpdateNavigationGhost(CCmdUI *pCmdUI)				{ pCmdUI->SetRadio(SETTINGS::nNavMode == 2); /*pCmdUI->Enable(GetDocument()->IsDownloadComplete());*/ }

void CAdVisuoView::OnCharacterNocolourcoding()							{ SETTINGS::nColouringMode = 0; }
void CAdVisuoView::OnUpdateCharacterNocolourcoding(CCmdUI *pCmdUI)		{ pCmdUI->SetCheck(SETTINGS::nColouringMode == 0); }
void CAdVisuoView::OnCharacterCurrentwaitingtime()						{ SETTINGS::nColouringMode = 1; }
void CAdVisuoView::OnUpdateCharacterCurrentwaitingtime(CCmdUI *pCmdUI)	{ pCmdUI->SetCheck(SETTINGS::nColouringMode == 1); }
void CAdVisuoView::OnCharacterExpectedwaitingtime()						{ SETTINGS::nColouringMode = 2; }
void CAdVisuoView::OnUpdateCharacterExpectedwaitingtime(CCmdUI *pCmdUI)	{ pCmdUI->SetCheck(SETTINGS::nColouringMode == 2); }

void CAdVisuoView::OnUpdateFps(CCmdUI *pCmdUI)
{
	CString str;
	AVULONG fps = m_engine.GetFPS();
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
		CDlgMaterials *pDlg = new CDlgMaterials(&m_engine);
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
	AVLONG a = 0;
	m_instRec.Play(a);
}

void CAdVisuoView::OnFileSimulation()
{
	AVULONG nProjectId = (*GetProject())[L"ProjectId"];
	AVULONG nSimId = GetProject()->GetSimulationId();
	AVGetApp()->LoadSimulation(nProjectId, nSimId, true);
}

void CAdVisuoView::OnFileDownload()
{
	AVULONG nProjectId = (*GetProject())[L"ProjectId"];
	AVULONG nSimId = GetProject()->GetSimulationId();
	AVGetApp()->LoadSimulation(nProjectId, nSimId, false);
}


void CAdVisuoView::OnUpdateFileDownload(CCmdUI *pCmdUI)
{
	if (!AVGetApp()->IsLogged())
		pCmdUI->Enable(FALSE);
}


void CAdVisuoView::OnUpdateFileSimulation(CCmdUI *pCmdUI)
{
	if (!AVGetApp()->IsLogged())
		pCmdUI->Enable(FALSE);
}
