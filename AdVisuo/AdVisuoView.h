// AdVisuoView.h - a part of the AdVisuo Client Software

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

// AdVisuoView.h : interface of the CAdVisuoView class
//


#pragma once

#include "AdVisuoDoc.h"
#include "AdVisuoRenderer.h"
#include "Camera.h"
#include "Screen.h"
#include "Sprite.h"
#include "HUD.h"
#include "Script.h"
#include "Dialogs.h"

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer
#include <fwaction.h>	// actions

#include <functional>

#define N_CAMERAS	10						// number of cameras

class CAdVisuoView : public CView
{
	// FreeWill Objects
	IFWDevice *m_pFWDevice;					// FreeWill Device
	IRenderer *m_pRenderer;					// The Renderer
	IScene *m_pScene;						// The Scene
	IBody *m_pBody;							// The Body
	ISceneLightDir *m_pLight1;				// light 1
	ISceneLightDir *m_pLight2;				// light 2

	IAction *m_pActionTick;					// The Clock Tick Action...
	IAction *m_pAuxActionTick;				// The Clock Tick Action for Camera Animation...
	AVULONG m_nAuxTimeRef;					// The Clock Value for m_pAuxActionTick

	// HUD
	CSprite m_sprite;							// sprite - DX object used to draw all 
	CTextPlate m_plateCam;						// text plates - camera descriptions
	CHUD m_hud;									// main HUD console

	// special - used by RenderToVideo to lock regular screen updates
	bool m_bLock;
	
	// Cameras
	CCamera *m_pCamera[N_CAMERAS];

	// Screen Configuration Object
	CScreen2x2 m_screen;

	// Script
	CScript m_script;
	CScriptEvent m_instRec;
	friend class CScriptEvent;

	// Flags
	bool m_bSelectOnWheel, m_bSelectOnMove;		// specify when focus is switched between viewports (always false at the moment)
	bool m_bHUDPanel;							// diaplay HUD view panel
	bool m_bHUDCaption;							// diaplay HUD view captions
	bool m_bHUDSelection;						// display sub-view selection

	// Hit test auxiliaries
	CScreen::HIT m_hit;							// hit test result - screen outside HUD
	AVULONG m_nHitX, m_nHitY;					// aux hit indices
	CPoint m_ptHit;								// last drag position

	// Frame Rate calculation
	static DWORD c_fpsNUM;						// Frame per Second rate calculation
	DWORD *m_pfps;
	int m_nfps;

	// Other auxiliaries
	DWORD m_nKeyScanTime;						// used internally by OnScanKey
	bool m_bMaximised;							// used to keep the state of the main app window while full screen mode
	
	// UI - specific
	AVLONG m_nAspectImageIndex;					// image index for the ID_VIEW_ASPECT button
	CMFCRibbonBaseElement *m_pbutFloor;			// last generated "Change Floor" button
	CMFCRibbonBaseElement *m_pbutLift;			// last generated "Lift Camera" button
	AVULONG m_nCamExtSideOption;				// used to interchange between left and right side vision

protected: // create from serialization only
	CAdVisuoView();
	DECLARE_DYNCREATE(CAdVisuoView)
public:
	virtual ~CAdVisuoView();

public:
	
	// Attributes
	CAdVisuoDoc *GetDocument() const			{ return reinterpret_cast<CAdVisuoDoc*>(m_pDocument); }
	bool IsEngineReady()						{ return m_pBody != NULL; }

	CString GetDiagnosticMessage();

	// Initialisation and Life-Cycle
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual void OnInitialUpdate();

	void OnLostDevice();
	void OnResetDevice();

	// Sim Initialisation
	void PrepareSim();

	// FreeWill Initialisation
	bool CreateFreeWill(HWND m_hWnd);

	void CreateCamera(int i)					{ if (i >= N_CAMERAS) return; DeleteCamera(i); m_pCamera[i] = new CCamera(GetDocument()->GetProject()->GetBuilding(0), i); m_pCamera[i]->Create(); }
	void DeleteCamera(int i)					{ if (i >= N_CAMERAS) return; if (m_pCamera[i]) delete m_pCamera[i]; m_pCamera[i] = NULL; }
	CCamera *GetCamera(int i)					{ return i < N_CAMERAS ? m_pCamera[i] : NULL; }
	void SetCamera(int i, CCamera *pCamera)		{ if (i >= N_CAMERAS) return; DeleteCamera(i); m_pCamera[i] = pCamera; }

	CCamera *GetCurCamera()						{ return GetCamera(m_screen.GetCurCamera()); }

	void DeleteAllCameras()						{ for (int i = 0; i < N_CAMERAS; i++) DeleteCamera(i); }
	void CreateAllCameras()						{ for (int i = 0; i < N_CAMERAS; i++) CreateCamera(i); }

	AVFLOAT GetScreenAspectRatio()				{ return (AVFLOAT)GetSystemMetrics(SM_CXSCREEN) / (AVFLOAT)GetSystemMetrics(SM_CYSCREEN); }
	AVFLOAT GetWindowAspectRatio()				{ CRect rect; GetClientRect(rect); return (AVFLOAT)rect.Width() / (AVFLOAT)rect.Height(); }
	AVFLOAT GetViewAspectRatio()				{ return m_screen.GetCurAspectRatio(); }
	AVFLOAT GetViewAspectRatio(AVULONG i)		{ return m_screen.GetAspectRatio(i); }


	// Timer, Cursor and Size Handlers
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	// Rendering
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view

protected:
	void BeginFrame();
	void RenderScene(bool bHUDSelection = false);
	void RenderHUD(bool bHUDPanel, bool bHUDClockOnly, bool bHUDCaption, bool bHUDSelection, AVLONG nTime = 0);
	void EndFrame();

	// Snapshot and Video Rendering
	typedef std::function<void(LPCTSTR strStatus, AVULONG nTime, bool &bStop, bool &bPreview)> CBUpdate;
	bool RenderToVideo(LPCTSTR lpszFilename, AVULONG nFPS, AVULONG nResX, AVULONG nResY, AVULONG nTimeFrom, AVULONG nTimeTo, bool bShowCaptions, bool bShowClock, CBUpdate fn);
	bool RenderToBitmap(LPCTSTR pFilename, enum FW_RENDER_BITMAP fmt);


	// Proceeding the Animation
	bool Proceed(FWULONG nMSec);	// proceed the simulation to the given time stamp; returns false if simulation is finished
	bool Proceed();					// proceed the simulation to the current play time; returns false if simulation is finished

	// Play Control
	void Play();					// starts the simulation
	void Rewind(FWULONG nMSec);		// rewinds the simulation to the given point
	void Pause()					{ if (IsEngineReady()) m_pRenderer->Pause(); }
	void Stop()						{ if (IsEngineReady()) m_pRenderer->Stop(); }
	bool IsPlaying()				{ return IsEngineReady() && (m_pRenderer->IsPlaying() == S_OK); }
	bool IsPaused()					{ return IsEngineReady() && (m_pRenderer->IsPaused() == S_OK); }
	FWFLOAT GetAccel()				{ FWFLOAT accel; m_pRenderer->GetAccel(&accel); return accel; }
	void PutAccel(FWFLOAT accel)	{ m_pRenderer->PutAccel(accel); }
	FWULONG GetPlayTime()			{ FWULONG nTime; if (!IsPlaying()) return 0; m_pRenderer->GetPlayTime(&nTime); return nTime; }
	FWULONG GetFPS();

	// Auxiliary Player - used for camera animation
	void AuxPlay(IAction **pAuxAction);
	bool IsAuxPlaying();

	// Camera Control
	void OnAdjustCameras();
	void OnDrag(int dx, int dy, int dz, bool bShift, bool bCtrl, bool bAlt);
	void OnScanKey();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	// Mouse
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	// Camera
	afx_msg void OnSelCamera(UINT nCmd);
	afx_msg void OnUpdateSelCamera(CCmdUI *pCmdUI);
	afx_msg void OnLayout(UINT nCmd);
	afx_msg void OnUpdateLayout(CCmdUI *pCmdUI);
	afx_msg void OnCamera(UINT);
	afx_msg void OnUpdateCamera(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStoreyMenu(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCameraLiftMenu(CCmdUI *pCmdUI);

	// Action
	afx_msg void OnActionPlay();
	afx_msg void OnUpdateActionPlay(CCmdUI *pCmdUI);
	afx_msg void OnActionPause();
	afx_msg void OnUpdateActionPause(CCmdUI *pCmdUI);
	afx_msg void OnActionStop();
	afx_msg void OnUpdateActionStop(CCmdUI *pCmdUI);
	afx_msg void OnActionSlowdown();
	afx_msg void OnUpdateActionSlowdown(CCmdUI *pCmdUI);
	afx_msg void OnActionSpeedup();
	afx_msg void OnUpdateActionSpeedup(CCmdUI *pCmdUI);
	afx_msg void OnActionNormalpace();
	afx_msg void OnUpdateActionNormalpace(CCmdUI *pCmdUI);

	// View
	afx_msg void OnViewAspectRatio(UINT nCmd);
	afx_msg void OnUpdateViewAspectRatio(CCmdUI *pCmdUI);
	afx_msg void OnViewFullScreen();
	afx_msg void OnUpdateViewFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnHudPanel();
	afx_msg void OnUpdateHudPanel(CCmdUI *pCmdUI);
	afx_msg void OnHudCaption();
	afx_msg void OnUpdateHudCaption(CCmdUI *pCmdUI);
	afx_msg void OnHudSelect();
	afx_msg void OnUpdateHudSelect(CCmdUI *pCmdUI);

	// HUD specific
	afx_msg void OnActionPlayspecial();
	afx_msg void OnActionRewind();
	afx_msg void OnHudPinDown();
	
	// File	
	afx_msg void OnActionRender();
	afx_msg void OnUpdateActionRender(CCmdUI *pCmdUI);
	afx_msg void OnActionSavestill();
	afx_msg void OnUpdateActionSavestill(CCmdUI *pCmdUI);

	// Status Bar
	afx_msg void OnUpdateStatusbarPane2(CCmdUI *pCmdUI);
public:
	afx_msg void OnUpdateViewAspect(CCmdUI *pCmdUI);
	afx_msg void OnViewMaterials();
	afx_msg void OnUpdateViewMaterials(CCmdUI *pCmdUI);
	afx_msg void OnRecScript();
	afx_msg void OnUpdateRecScript(CCmdUI *pCmdUI);
	afx_msg void OnRecRecord();
	afx_msg void OnRecPlay();
};
   

