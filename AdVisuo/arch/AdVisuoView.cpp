// AdVisuoView.cpp - a part of the AdVisuo Client Software

// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// AdVisuoView.cpp : implementation of the CAdVisuoView class
//

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

// CAdVisuoView

IMPLEMENT_DYNCREATE(CAdVisuoView, CView)

BEGIN_MESSAGE_MAP(CAdVisuoView, CView)
	ON_COMMAND_RANGE(ID_CAMERA, ID_CAMERA_LIFT, &CAdVisuoView::OnCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CAMERA, ID_CAMERA_LIFT, &CAdVisuoView::OnUpdateCamera)
	ON_COMMAND(ID_STOREY_ONEUP, &CAdVisuoView::OnStoreyOneup)
	ON_COMMAND(ID_STOREY_ONEDOWN, &CAdVisuoView::OnStoreyOnedown)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_UPDATE_COMMAND_UI(ID_STOREY_ONEDOWN, &CAdVisuoView::OnUpdateStoreyOnedown)
	ON_UPDATE_COMMAND_UI(ID_STOREY_ONEUP, &CAdVisuoView::OnUpdateStoreyOneup)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_ACTION_RENDER, &CAdVisuoView::OnActionRender)
	ON_COMMAND(ID_ACTION_SAVESTILL, &CAdVisuoView::OnActionSavestill)
	ON_COMMAND(ID_ACTION_PLAY, &CAdVisuoView::OnActionPlay)
END_MESSAGE_MAP()

// CAdVisuoView construction/destruction

CAdVisuoView::CAdVisuoView()
{
	m_pCamera = NULL;
	m_pChgStoreyAction = NULL;
}

CAdVisuoView::~CAdVisuoView()
{
	if (m_pChgStoreyAction) m_pChgStoreyAction->Release();
}

BOOL CAdVisuoView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// CAdVisuoView Attributes

// CAdVisuoView drawing

void CAdVisuoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void CAdVisuoView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	OnTimerChgFloor();
	if (((CMDIFrameWnd*)(AfxGetApp()->m_pMainWnd))->MDIGetActive() == GetParentFrame())
		InvalidateRect(NULL, FALSE);
}

bool bGlobalFlag = false;

void CAdVisuoView::OnDraw(CDC *pDC)
{
	CAdVisuoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	// Executed once to initialise the engine
	static bool bProcInDrawFlag = false;
	if (!bProcInDrawFlag && pDoc->IsSimReady() && !pDoc->IsEngineReady())
	{
		bProcInDrawFlag = true;
		CWaitCursor wait_cursor;
		CRect rect;
		GetClientRect(rect);
		::FillRect(pDC->m_hDC, rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
		pDoc->CreateFreeWill(m_hWnd);
		pDoc->CreatePlates();
		pDoc->CreateBuilding(0);
		if (GetCamera())
			CreateCamera();
		BeginFrame();
		RenderScene();
		EndFrame();
		pDoc->PrepareSim();
		bProcInDrawFlag = false;
		return;
	}

	// if anything went wrong or is not yet ready...
	if (bProcInDrawFlag || !pDoc->IsEngineReady() || !pDoc->IsSimReady() /*|| !GetCamera() || !GetCamera()->IsReady() || !GetCamera()->m_pCamera*/)
	{
		CRect rect;
		GetClientRect(rect);
		::FillRect(pDC->m_hDC, rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
		return;
	}

	ASSERT(!bProcInDrawFlag);

	// Executed each time camera is changed
	if (GetCamera() && !GetCamera()->IsReady())
		CreateCamera();

	BeginFrame();
	RenderScene();
	RenderPlates(0, !pDoc->IsPlaying());
	EndFrame();
}

bool CAdVisuoView::CreateCamera()
{
	IRenderer *pRenderer = GetRenderer();
	FWFLOAT fAspectRatio;
	pRenderer->PutWindow(m_hWnd);
	pRenderer->GetAspectRatio(&fAspectRatio);
	GetCamera()->Create(fAspectRatio);
	return true;
}

void CAdVisuoView::BeginFrame()
{
	IRenderer *pRenderer = GetRenderer();

	if (m_hWnd) pRenderer->PutWindow(m_hWnd);

	GetDocument()->m_pScene->PutCamera(GetCamera()->m_pCamera);
	GetCamera()->m_pCamera->Render(pRenderer);

	pRenderer->BeginFrame();
}

void CAdVisuoView::EndFrame()
{
	GetRenderer()->EndFrame();
}

void CAdVisuoView::RenderScene()
{
	IRenderer *pRenderer = GetRenderer();
	CBuilding *pBuilding = &GetDocument()->m_building;
	CSim *pSim = &GetDocument()->m_sim;

	// my own display list goes here... instead of m_pScene->Render(pRenderer);
	GetDocument()->m_pLight1->Render(pRenderer);
	GetDocument()->m_pLight2->Render(pRenderer);
	pSim->RenderPassengers(pRenderer, 0);
	for (FWULONG i = 0; i < pBuilding->GetLiftCount(); i++)
		pBuilding->GetLiftObj(i)->Render(pRenderer);
	for (FWULONG i = 0; i < pBuilding->GetStoreyCount(); i++)
		pBuilding->GetStoreyObj(i)->Render(pRenderer);
	for (FWULONG i = 0; i < pBuilding->GetStoreyCount(); i++)
	{
		pBuilding->GetFrontWallObj(i)->Render(pRenderer);
		pBuilding->GetRearWallObj(i)->Render(pRenderer);
	}
	pSim->RenderPassengers(pRenderer, 1);
}

void CAdVisuoView::RenderPlates(FWULONG nFont, bool bDrawPlay, bool bDrawTime, AVLONG nTime)
{
	CAdVisuoDoc* pDoc = GetDocument();
	IRenderer *pRenderer = GetRenderer();

	ITransform *pT;
	pDoc->m_pScene->CreateCompatibleTransform(&pT);

	// Reset Projection Transform
	FWFLOAT fAspect;
	pRenderer->GetAspectRatio(&fAspect);
	pT->FromPerspectiveLH(0.93, 0.99999, 2, fAspect);
	pRenderer->PutProjectionTransform(pT);

	// draw play (conditional)
	if (bDrawPlay)
	{
		pT->FromIdentity(); 
		pRenderer->PutViewTransform(pT);
		(m_bPlayHover ? pDoc->m_pObjPlatePlayDark : pDoc->m_pObjPlatePlay)->Render(pRenderer);
	}

	FWCOLOR color = { 0.0f, 0.0f, 0.0f, 1.0f };
	FWULONG cx, cy;
	pRenderer->GetViewSize(&cx, &cy);

	// Draw Plate
	pT->FromTranslationXYZ(0, -0.5f+0.00, 0); pRenderer->PutViewTransform(pT);
	pDoc->m_pObjPlateText->Render(pRenderer);

	// Draw Description Text
//	wchar_t buf[256];
//	pRenderer->DrawText(nFont, GetCamera()->Description(buf, 255), 5, cy-5, color, 0, 2);
	
	// Draw Time
	if (bDrawTime)
	{
		CString str;
		if (nTime >= 0)
			str.Format(L"%d.%02d", nTime/1000, (nTime/10)%100);
		else
			str.Format(L"-%d.%02d", -nTime/1000, (-nTime/10)%100);
		pRenderer->DrawText(nFont, (LPOLESTR)(LPCOLESTR)str, cx-5, cy-5, color, 2, 2);
	}
	
	pT->Release();
}

void CAdVisuoView::RenderPlates(FWULONG nFont, bool bDrawPlay)
{
	if (GetDocument()->IsPlaying())
		RenderPlates(nFont, bDrawPlay, true, GetDocument()->GetPlayTime() + GetDocument()->m_sim.GetTimeLowerBound());
	else
		RenderPlates(nFont, bDrawPlay, false);
}

void CAdVisuoView::OnActionRender()
{
	// display Video Params dlg
	CDlgVideo dlg;
	dlg.m_resX = 800;//1920;
	dlg.m_resY = 600;//1080;
	dlg.m_timeFrom = 0;
	dlg.m_timeTo = 0;
	if (dlg.DoModal() != IDOK)
		return;
	if (dlg.m_timeTo == 0) dlg.m_timeTo = 0x7fffffff;

	// Prepare...
	IRenderer *pRenderer = GetRenderer();
	GetDocument()->Play(); GetRenderer()->Stop();

	// Stop GUI, display VideoCtrl window
	AfxGetMainWnd()->EnableWindow(FALSE);
	CDlgVideoCtrl ctrl(this);
	ctrl.Create(IDD_ADV_VIDEOCTRL);
	ctrl.SetWindowPos(NULL, 15, 120, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ctrl.ShowWindow(SW_SHOW);
	ctrl.SetStatus(L"Initialising...", FALSE);
	ctrl.m_wndPreview.SetCheck(BST_CHECKED);

	pRenderer->InitOffScreen(dlg.m_resX, dlg.m_resY);
	GetDocument()->m_pFWDevice->EnableErrorException(TRUE);

	try
	{
		pRenderer->OpenMovieFile(L"output.avi", 25);
		pRenderer->SetFont(1, 16, 6, true, false, L"Arial");

		//#ADRIAN
		AVULONG T = 60000;
		T = 318400;

		for (AVULONG t = -GetDocument()->m_sim.GetTimeLowerBound(); t < T; t+=40)
			GetDocument()->Proceed(t);
		GetDocument()->m_pRenderer->PutPlayTime(T);
		GetDocument()->OnActionPause();

		// initial motion
		IAction *pAction;
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 10000, 0, GetCamera()->m_pHandleBone, -150.0f, 400, 240);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
		// two floors
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 89000, 3000, GetCamera()->m_pHandleBone, -200.0f, 500, 0);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		// express zone
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 318200, 0, GetCamera()->m_pHandleBone, 0, 0, 40);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 319400, 53600, GetCamera()->m_pHandleBone, 0, 0, 5040);
		AVFLOAT s = 5.0400f/0.04f;
		pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 1.0f, 2.5f/s, 1.0f/s, 1.5f/s);

		// upper zone jumps
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 377700, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 388900, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 399300, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 410500, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 421700, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 431300, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 442500, 4000, GetCamera()->m_pHandleBone, 0, 0, 160);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 452200, 2000, GetCamera()->m_pHandleBone, 0, 0, 80);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);

		// distant view
		ITransform *pT;
		GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT);
		pT->FromRotationZ(-0.4);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 520000, 500, GetCamera()->m_pHandleBone, pT);
		pT->FromRotationX(-0.2);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 520500, 500, GetCamera()->m_pHandleBone, pT);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 521000, 1000, GetCamera()->m_pHandleBone, 0, 3400, -480.0f);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);

		// rear view
		pT->FromRotationZ(3.1415f);
		pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 600000, 20000, GetCamera()->m_pHandleBone, pT);

		pT->Release();
		//#END-OF-ADRIAN

		AVULONG t = T;

		for (t; t <= dlg.m_timeTo; t += 40)
		{
		//#ADRIAN
			if (t >= 170000 && t < 300000)
				t += 280;
			if (t >= 540000)
				t += 280;
		//#END-OF-ADRIAN

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

			GetDocument()->Proceed(t - GetDocument()->m_sim.GetTimeLowerBound());

			pRenderer->SetTargetOffScreen();
			BeginFrame();
			RenderScene();
			RenderPlates(1, false, true, t);
			EndFrame();
			AfxGetMainWnd()->SetActiveWindow();
			bGlobalFlag = true;

			ctrl.SetStatus(L"Rendering to file...", TRUE);
			ctrl.SetTime(t);
			if (ctrl.m_bStopping) break;

			if (ctrl.m_wndPreview.GetCheck() == BST_CHECKED)
			{
				pRenderer->SetTargetToScreen();
				BeginFrame();
				RenderScene();
				RenderPlates(0, false, true, t);
				EndFrame();
			}
		}

		ctrl.SetStatus(L"Finalising...", FALSE);

		pRenderer->SetTargetToScreen();
		BeginFrame();
		RenderScene();
		RenderPlates(0, false, true, t);
		EndFrame();
		for (int i = 0; i < 40; i++)
		{
			pRenderer->SetTargetOffScreen();
			BeginFrame();
			RenderScene();
			RenderPlates(1, false, true, t);
			EndFrame();
		}

		pRenderer->CloseMovieFile();
		pRenderer->DoneOffScreen();
	}
	catch (FWERROR *) 
	{ 
		//h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); 
		pRenderer->CloseMovieFile();
		pRenderer->DoneOffScreen();
	}
	ctrl.DestroyWindow();
	bGlobalFlag = false;
	AfxGetMainWnd()->EnableWindow(TRUE);
	AfxGetMainWnd()->SetActiveWindow();
	GetDocument()->m_pFWDevice->EnableErrorException(FALSE);
	GetDocument()->OnActionStop();
}

void CAdVisuoView::OnActionSavestill()
{
	IRenderer *pRenderer = GetRenderer();

	CFileDialog dlg(FALSE, L"jpg", L"*.jpg", OFN_OVERWRITEPROMPT, L"JPG Files (*.jpg)|*.jpg|BMP Files (*.bmp)|*.bmp|Targa Files (*.tga)|*.tga|PNG Files (*.png)|*.png|All Files|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		enum RENDER_BITMAP bmp = RENDER_JPG;
		CString ext = dlg.GetFileExt();
		if (ext.CompareNoCase(L"bmp") == 0) bmp = RENDER_BMP;
		if (ext.CompareNoCase(L"jpg") == 0) bmp = RENDER_JPG;
		if (ext.CompareNoCase(L"tga") == 0) bmp = RENDER_TGA;
		if (ext.CompareNoCase(L"png") == 0) bmp = RENDER_PNG;

		pRenderer->InitOffScreen(0, 0);
		pRenderer->OpenStillFile(dlg.GetFileName(), bmp);
		pRenderer->SetFont(1, 32, 12, true, false, L"Arial");

		BeginFrame();
		RenderScene();
		RenderPlates(0, false);
		EndFrame();

		pRenderer->DoneOffScreen();
		return;
	}
}

void CAdVisuoView::OnDrag(int dx, int dy, int dz, bool bShift, bool bCtrl, bool bAlt)
{
	if (m_pChgStoreyAction) return;		// camera travelling between storeys...

	if (bCtrl) if (abs(dx) > abs(dy)) dy = 0; else dx = 0;

	switch (GetAdVisuoApp()->GetWalkMode())
	{
	case 0:
		// CCTV Mode
		if (dx != 0) OnCameraPan(DEG2RAD((FWFLOAT)dx/5.0f));
		if (dy != 0) OnCameraTilt(DEG2RAD(-(FWFLOAT)dy/5.0f));
		if (dz != 0) OnCameraZoom(DEG2RAD((FWFLOAT)dz / 2.0f));
		break;
	case 1:
		// Walk Mode
		if (dx != 0 && !bShift) OnCameraPan(DEG2RAD((FWFLOAT)dx/5.0f));
		if (dy != 0 && !bShift) OnCameraMove(0, -(FWFLOAT)dy, 0);
		if (dx != 0 &&  bShift) OnCameraMove(-(FWFLOAT)dx, 0, 0);
		if (dy != 0 &&  bShift) OnCameraMove(0, 0, -(FWFLOAT)dy);
		if (dz != 0) OnCameraTilt((FWFLOAT)dz/10.0f);
		break;
	case 2:
		// Ghost Mode
		break;
	}
}

void CAdVisuoView::OnCameraPan(FWFLOAT f)
{
	if (!GetCamera() || !GetCamera()->m_pHandleBone) return;

	ITransform *pT1 = NULL, *pT2 = NULL;
	GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT1);
	GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT2);
	pT1->FromRotationZ(f);
	GetCamera()->m_pHandleBone->GetLocalTransform(pT2);
	pT1->Multiply(pT2);
	GetCamera()->m_pHandleBone->PutLocalTransform(pT1);
	pT1->Release();
	pT2->Release();
}

void CAdVisuoView::OnCameraTilt(FWFLOAT f)
{
	if (!GetCamera() || !GetCamera()->m_pCamera) return;

	ITransform *pT1 = NULL, *pT2 = NULL;
	GetCamera()->m_pCamera->CreateCompatibleTransform(&pT1);
	GetCamera()->m_pCamera->CreateCompatibleTransform(&pT2);
	pT1->FromRotationX(f);
	GetCamera()->m_pCamera->GetLocalTransform(pT2);
	pT1->Multiply(pT2);
	GetCamera()->m_pCamera->PutLocalTransform(pT1);
	pT1->Release();
	pT2->Release();
}

void CAdVisuoView::OnCameraZoom(FWFLOAT f)
{
	if (!GetCamera() || !GetCamera()->m_pCamera) return;

	FWFLOAT fFOV, fNear, fFar, fAspect;
	GetCamera()->m_pCamera->GetPerspective(&fFOV, &fNear, &fFar, &fAspect);
	fFOV -= f;
	GetCamera()->m_pCamera->PutPerspective(fFOV, fNear, fFar, fAspect);
}

void CAdVisuoView::OnCameraMove(FWFLOAT x, FWFLOAT y, FWFLOAT z)
{
	if (!GetCamera() || !GetCamera()->m_pHandleBone) return;

	ITransform *pT1 = NULL, *pT2 = NULL;
	GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT1);
	GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT2);
	pT1->FromTranslationXYZ(x, y, z);
	GetCamera()->m_pHandleBone->GetLocalTransform(pT2);
	pT1->Multiply(pT2);
	GetCamera()->m_pHandleBone->PutLocalTransform(pT1);
	pT1->Release();
	pT2->Release();
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


// CAdVisuoView diagnostics

#ifdef _DEBUG
void CAdVisuoView::AssertValid() const
{
	CView::AssertValid();
}

void CAdVisuoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CAdVisuoView message handlers

void CAdVisuoView::OnCamera(UINT nCmd)
{
	if (!GetCamera()) return;
	GetCamera()->Destroy();
	GetCamera()->m_nCameraId = nCmd - ID_CAMERA;
}

void CAdVisuoView::OnUpdateCamera(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetCamera() && GetCamera()->m_nCameraId == pCmdUI->m_nID - ID_CAMERA);
}

void CAdVisuoView::OnStoreyOneup()
{
	if (!GetCamera() || m_pChgStoreyAction) return;
	m_nChgStorey = GetCamera()->m_nStorey + 1;
	GetCamera()->m_pHandleBone->PushState();
	CParam params[] = { (FWPUNKNOWN)NULL, 0, 1000, GetCamera()->m_pHandleBone, 0, 0, GetDocument()->m_building.GetStorey(GetCamera()->m_nStorey)->SH };
	GetDocument()->m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&m_pChgStoreyAction);
	m_pChgStoreyAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	m_nChgStoreyTime = ::GetTickCount();
}

void CAdVisuoView::OnStoreyOnedown()
{
	if (!GetCamera() || m_pChgStoreyAction) return;
	m_nChgStorey = GetCamera()->m_nStorey - 1;
	GetCamera()->m_pHandleBone->PushState();
	CParam params[] = { (FWPUNKNOWN)NULL, 0, 1000, GetCamera()->m_pHandleBone, 0, 0, -GetDocument()->m_building.GetStorey(GetCamera()->m_nStorey-1)->SH };
	GetDocument()->m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&m_pChgStoreyAction);
	m_pChgStoreyAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	m_nChgStoreyTime = ::GetTickCount();
}

void CAdVisuoView::OnTimerChgFloor()
{
	if (!GetCamera() || !m_pChgStoreyAction)
		return;

	FWULONG nMSec = GetTickCount() - m_nChgStoreyTime;
	m_pChgStoreyAction->SendEvent(nMSec, EVENT_TICK, nMSec, NULL);
	FWFLOAT fPhase;
	ACTION_EVENT ev = { NULL, NULL, nMSec };
	m_pChgStoreyAction->GetPhase(&ev, &fPhase);
	if (fPhase >= 1.0)
	{
		m_pChgStoreyAction->Release();
		m_pChgStoreyAction = NULL;
		GetCamera()->m_pHandleBone->PopState();
		GetCamera()->ChangeStorey(m_nChgStorey);
	}
}

void CAdVisuoView::OnUpdateStoreyOnedown(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCamera() && !m_pChgStoreyAction && !GetCamera()->IsLowestStorey());
}

void CAdVisuoView::OnUpdateStoreyOneup(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCamera() && !m_pChgStoreyAction && !GetCamera()->IsHighestStorey());
}


void CAdVisuoView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_drag = point;

	// check MOP (Mouse Over Play button)
	CRect rect;
	GetClientRect(rect);
	long x = (rect.left + rect.right) / 2 - point.x;
	long y = (rect.top + rect.bottom) / 2 - point.y;
	m_bPlayClicked = sqrt((double)x * x + y * y) < rect.Height() * 0.165;

	CView::OnLButtonDown(nFlags, point);
}

void CAdVisuoView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		OnDrag(point.x - m_drag.x, point.y - m_drag.y, 0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
		ReleaseCapture();
	}

	// check MOP (Mouse Over Play button)
	CRect rect;
	GetClientRect(rect);
	long x = (rect.left + rect.right) / 2 - point.x;
	long y = (rect.top + rect.bottom) / 2 - point.y;
	if (m_bPlayClicked && sqrt((double)x * x + y * y) < rect.Height() * 0.165)
		/*GetDocument()->*/OnActionPlay();

	CView::OnLButtonUp(nFlags, point);
}

void CAdVisuoView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		OnDrag(point.x - m_drag.x, point.y - m_drag.y, 0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
		m_drag = point;
	}

	// check MOP (Mouse Over Play button)
	CRect rect;
	GetClientRect(rect);
	long x = (rect.left + rect.right) / 2 - point.x;
	long y = (rect.top + rect.bottom) / 2 - point.y;
	m_bPlayHover = sqrt((double)x * x + y * y) < rect.Height() * 0.165;

	CView::OnMouseMove(nFlags, point);
}


BOOL CAdVisuoView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OnDrag(0, 0, zDelta/120, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CAdVisuoView::OnActionPlay()
{
	if (GetDocument()->IsPlaying()) return;
	GetDocument()->OnActionPlay();
	if (!GetDocument()->IsPlaying()) return;

	//#ADRIAN
	AVULONG T = 60000;
	T = 315400;

	for (AVULONG t = -GetDocument()->m_sim.GetTimeLowerBound(); t < T; t+=400)
		GetDocument()->Proceed(t);
	GetDocument()->m_pRenderer->PutPlayTime(T);


	// initial motion
	IAction *pAction;
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 10000, 0, GetCamera()->m_pHandleBone, -150.0f, 400, 240);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	// two floors
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 89000, 3000, GetCamera()->m_pHandleBone, -200.0f, 500, 0);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	// express zone
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 318200, 0, GetCamera()->m_pHandleBone, 0, 0, 40);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 319400, 53600, GetCamera()->m_pHandleBone, 0, 0, 5040);
	AVFLOAT s = 5.0400f/0.04f;
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 1.0f, 2.5f/s, 1.0f/s, 1.5f/s);

	// upper zone jumps
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 377700, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 388900, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 399300, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 410500, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 421700, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 431300, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 442500, 4800, GetCamera()->m_pHandleBone, 0, 0, 160);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.1600f/0.04f, 2.5f, 1.0f, 1.5f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 452200, 2000, GetCamera()->m_pHandleBone, 0, 0, 80);
	pAction->SetEnvelopeEx((ACTION_ENVELOPE)666, 0.0800f/0.04f, 2.5f, 1.0f, 1.5f);

	// distant view
	ITransform *pT;
	GetCamera()->m_pHandleBone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(-0.4);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 520000, 500, GetCamera()->m_pHandleBone, pT);
	pT->FromRotationX(-0.2);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 520500, 500, GetCamera()->m_pHandleBone, pT);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Move", GetDocument()->m_pActionTick, 521000, 1000, GetCamera()->m_pHandleBone, 0, 3400, -480.0f);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.25f, 0.3f);

	// rear view
	pT->FromRotationZ(3.1415f);
	pAction = (IAction*)FWCreateObjWeakPtr(GetDocument()->m_pActionTick->FWDevice(), L"Action", L"Rotate", GetDocument()->m_pActionTick, 600000, 20000, GetCamera()->m_pHandleBone, pT);

	pT->Release();
}
