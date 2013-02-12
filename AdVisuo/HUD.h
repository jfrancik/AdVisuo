// HUD.h

#pragma once

#include "Sprite.h"
#include "resource.h"

class CHUD
{
public:
	enum TEX { TEX_NONE, TEX_0, TEX_1, TEX_2, TEX_3, TEX_32x32, TEX_16x16, TEX_8x16, TEX_8x8, TEX_MAX };
	enum HIT { HIT_NONE = -1, HIT_SLIDER = 0, HIT_PLAY, HIT_STOP, HIT_SLOWDOWN, HIT_NORMAL_PACE, HIT_SPEEDUP, HIT_FULL_SCREEN, HIT_PIN, HIT_CLOSE, HIT_BACK, HIT_MAX };

private:
	CSprite *m_pSprite;		// sprite used for HUD
	CPoint m_pt;			// panel position
	
	CPoint m_ptAlt;			// alt clock position
	CTextPlate m_plateAlt;	// text plate - alt clock device

	AVLONG m_nTime, m_nTimeLoaded, m_nTimeSim, m_nTimeSlider;				// time information - current, loaded, simulation total, HUD slider display
	CString m_strTime, /*m_strTimeLoaded,*/ m_strTimeSim, m_strTimeSlider;	// time information in textual form

	struct HUDITEM
	{
		CPoint pt;
		CSize sz;
		enum TEX nTexId;
		AVULONG nIndex;
		AVULONG nStatus;
		ULONG nCmd;
	};
	LPCTSTR m_pFilenames[TEX_MAX];
	CSprite::CTexture *m_pTextures[TEX_MAX];
	CSprite::CFont *m_pFont;
	HUDITEM m_pItems[HIT_MAX];

	enum HIT m_nHit;	// HitTest result while Drag begin-commot process

	// auto-hide and close
	AVULONG m_nAutoHideTime;
	bool m_bVisible;
	bool m_bAutoHide;
	DWORD m_nTick;

public:
	CHUD(CSprite *pSprite);
	~CHUD();

	void Initialise();

	void SetPos(CPoint pt)									{ m_pt = pt; }
	void SetAltPos(CPoint pt)								{ m_ptAlt = pt; }
	void SetItemStatus(AVULONG nItem, AVULONG nStatus)		{ m_pItems[nItem].nStatus = nStatus; }
	void SetTime(AVLONG t)									{ m_nTime       = max(0, t); m_strTime       = _helperTime2StrX(t); }
	void SetLoadedTime(AVLONG t)							{ m_nTimeLoaded = max(0, t); /*m_strTimeLoaded = _helperTime2Str(t);*/ }
	void SetSimulationTime(AVLONG t)						{ m_nTimeSim    = max(0, t); m_strTimeSim    = _helperTime2Str(t); }

	void Draw(CPoint ptMouse);
	enum HIT HitTest(CPoint pt, bool bAH = true);	// set bAH to false to allow auto-hide counting...
	
	void KeepReady();								// keep ready to avoid auto-hide... HitTest calls this if bAH=trur
	void Hide();									// forces immediate hiding
	AVULONG GetAH();								// 0 = shown, more than 0 - time since auto hide applies

	bool OnDragBegin(CPoint pt);					// client should call it from WM_LBUTTONDOWN and get mouse capture if the function returns true
	void OnDrag(CPoint pt);							// on mouse move, use to test if it is dragging; set bSlider = false to disable slider HUD
	void OnDragCommit(CPoint pt, HWND hWnd);		// will post commands to hWnd is something successfully clicked
	bool IsDragging()					{ return m_nHit != HIT_NONE; }
	AVULONG GetSliderTime()				{ return m_nTimeSlider; }

	void OnToggle();
	void OnPinDown();
	bool IsVisible()					{ return m_bVisible; }
	void OnLostDevice();
	void OnResetDevice();

private:
	CString _helperTime2Str(AVLONG t);
	CString _helperTime2StrX(AVLONG t);
};
