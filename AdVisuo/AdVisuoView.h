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
#include "Engine.h"

#include <functional>

#define N_CAMERAS	8						// number of cameras

class CAdVisuoView : public CView, ILostDeviceObserver
{
	CEngine m_engine;

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

	// Modes
	AVULONG m_nWalkMode;						// Walk or CCTV mode
	AVULONG m_nColouringMode;					// Character Colouring Mode

	// Hit test auxiliaries
	CScreen::HIT m_hit;							// hit test result - screen outside HUD
	AVULONG m_nHitX, m_nHitY;					// aux hit indices
	CPoint m_ptHit;								// last drag position

	// Other auxiliaries
	DWORD m_nKeyScanTime;						// used internally by OnScanKey
	bool m_bMaximised;							// used to keep the state of the main app window while full screen mode

	// UI - specific
	AVLONG m_nAspectImageIndex;					// image index for the ID_VIEW_ASPECT button
	CMFCRibbonBaseElement *m_pbutFloor;			// last generated "Change Floor" button
	AVULONG m_nLGButFloor;						// used to refresh the menu after LiftGroup changed
	CMFCRibbonBaseElement *m_pbutLift;			// last generated "Lift Camera" button
	CMFCRibbonBaseElement *m_pbutGroup;			// last generated "Lift Camera" button
	AVULONG m_nCamExtSideOption;				// used to interchange between left and right side vision

protected: // create from serialization only
	CAdVisuoView();
	DECLARE_DYNCREATE(CAdVisuoView)
public:
	virtual ~CAdVisuoView();

public:
	
	// Attributes
	CAdVisuoDoc *GetDocument() const			{ return reinterpret_cast<CAdVisuoDoc*>(m_pDocument); }
	CProjectVis *GetProject() const				{ return GetDocument()->GetProject(); }

	// Initialisation and Life-Cycle
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual void OnInitialUpdate();
	void OnAdjustViewSize();
	void OnAdjustCameras();

	// ILostDeviceObserver implementation
	virtual void OnLostDevice();
	virtual void OnResetDevice();

	// Mode Configuration
	AVULONG GetWalkMode()						{ return m_nWalkMode; }
	AVULONG GetColouringMode()					{ return m_nColouringMode; }
	void SetWalkMode(AVULONG n);
	void SetColouringMode(AVULONG n);

	// Play Control
	void Play();
	void Stop();
	void Pause();
	void Rewind(AVULONG nMSec);		// rewinds the simulation to the given point

	CEngine *GetEngine()						{ return &m_engine; }

	CCamera *GetCamera(int i)					{ return i < N_CAMERAS ? m_pCamera[i] : NULL; }
	CCamera *GetCurCamera()						{ return GetCamera(m_screen.GetCurCamera()); }
	
	AVULONG GetCurLiftGroupIndex()				{ return GetCurCamera()->GetLiftGroup(); }

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
	void RenderScene(bool bHUDSelection = false);
	void RenderHUD(bool bHUDPanel, bool bHUDClockOnly, bool bHUDCaption, bool bHUDSelection, AVLONG nTime);

	// Snapshot and Video Rendering
	typedef std::function<void(LPCTSTR strStatus, AVULONG nTime, bool &bStop, bool &bPreview)> CBUpdate;
	bool RenderToVideo(LPCTSTR lpszFilename, AVULONG nFPS, AVULONG nResX, AVULONG nResY, AVLONG nTimeFrom, AVLONG nTimeTo, bool bShowCaptions, bool bShowClock, CBUpdate fn);
	bool RenderToBitmap(LPCTSTR pFilename);

	// Camera Control
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
	afx_msg void OnUpdateCameraGroupMenu(CCmdUI *pCmdUI);

	// Tenancy
	afx_msg void OnTenancyMenu();
	afx_msg void OnUpdateTenancyMenu(CCmdUI *pCmdUI);

	// Scenario
	afx_msg void OnScenarioMenu();
	afx_msg void OnUpdateScenarioMenu(CCmdUI *pCmdUI);

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

	// Modes
	afx_msg void OnNavigationCctv();
	afx_msg void OnUpdateNavigationCctv(CCmdUI *pCmdUI);
	afx_msg void OnNavigationWalk();
	afx_msg void OnUpdateNavigationWalk(CCmdUI *pCmdUI);
	afx_msg void OnNavigationGhost();
	afx_msg void OnUpdateNavigationGhost(CCmdUI *pCmdUI);
	afx_msg void OnCharacterNocolourcoding();
	afx_msg void OnUpdateCharacterNocolourcoding(CCmdUI *pCmdUI);
	afx_msg void OnCharacterCurrentwaitingtime();
	afx_msg void OnUpdateCharacterCurrentwaitingtime(CCmdUI *pCmdUI);
	afx_msg void OnCharacterExpectedwaitingtime();
	afx_msg void OnUpdateCharacterExpectedwaitingtime(CCmdUI *pCmdUI);
	
	// Status Bar
	afx_msg void OnUpdateStatusbarPane2(CCmdUI *pCmdUI);
//public:
	afx_msg void OnUpdateViewAspect(CCmdUI *pCmdUI);
	afx_msg void OnViewMaterials();
	afx_msg void OnUpdateViewMaterials(CCmdUI *pCmdUI);
	afx_msg void OnRecScript();
	afx_msg void OnUpdateRecScript(CCmdUI *pCmdUI);
	afx_msg void OnRecRecord();
	afx_msg void OnRecPlay();
public:
	afx_msg void OnFileSimulation();
	afx_msg void OnFileDownload();
	afx_msg void OnUpdateFileDownload(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSimulation(CCmdUI *pCmdUI);
};
   

