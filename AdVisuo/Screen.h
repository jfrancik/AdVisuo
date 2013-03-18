// Screen.h

#pragma once

#include <freewill.h>
interface IRenderer;

// All AVFLOAT type coordinates are 0..1 and are scaled to the current window/screen size

#define RATIO_16_9		16.0f / 9.0f
#define RATIO_16_10		16.0f / 10.0f
#define RATIO_4_3		4.0f / 3.0f

class CScreen
{
protected:
	IRenderer *m_pRenderer;
	
	// screen divider (either vertical or horizontal)
	struct DIVIDER
	{
	private:
		AVFLOAT val, delta;
		bool bMovable;

	public:
		operator AVFLOAT()								{ return val + delta; }
		DIVIDER(AVFLOAT val = 0, bool bMovable = true) : val(val), delta(0), bMovable(bMovable) { }

		bool IsMovable()								{ return bMovable; }

		void SetDelta(AVFLOAT delta)					{ this->delta = delta; }
		void ApplyMinMax(AVFLOAT fMin, AVFLOAT fMax)	{ delta = max(delta, fMin-val); delta = min(delta, fMax-val); }
		void Commit()									{ val += delta; delta = 0; }
		void Rollback()									{ delta = 0; }
	};

	// screen view port (pane)
	struct VIEWPORT
	{
		AVULONG x0, x1, y0, y1;		// these are indices to the array of dividers, not the actual co-ordinates
		AVULONG nCamera;			// camera index
		bool bEnabled;

		VIEWPORT(AVULONG x0, AVULONG x1, AVULONG y0, AVULONG y1, AVULONG nCamera, bool bEnabled)
			: x0(x0), x1(x1), y0(y0), y1(y1), nCamera(nCamera), bEnabled(bEnabled) { }
		VIEWPORT() { }
	};


	// X screen dividers - left to right
	AVULONG m_nXDivs;					// number of dividers (# of columns + 1)
	DIVIDER *m_pXDivs;					// array of dividers

	// Y screen dividers - upper to lower
	AVULONG m_nYDivs;					// number of dividers (# of columns + 1)
	DIVIDER *m_pYDivs;					// array of dividers

	// width of the frame
	AVLONG m_nFrame;

	// array of viewports
	AVULONG m_nVP;
	VIEWPORT *m_pVP;

	// active viewport
	AVULONG m_nActiveVP;

	// configuration id
	AVULONG m_nConfig;

	// required aspect ratio
	AVFLOAT m_fAspect;

public:
	CScreen(IRenderer *pRenderer = NULL, AVULONG nCols = 2, AVULONG nRows = 2, AVLONG nFrame = 1);
	~CScreen();

	void SetRenderer(IRenderer *pRenderer);

	void SetConfig(AVULONG n)					{ m_nConfig = n; OnSetConfig(n); }
	AVULONG GetConfig()							{ return m_nConfig; }
	void SetAspectRatio(AVFLOAT fAspect)		{ m_fAspect = fAspect; }
	AVFLOAT GetAspectRatio()					{ return m_fAspect; }
	virtual void OnSetConfig(AVULONG) = 0;

	AVULONG GetCount()							{ return m_nVP; }
	bool IsEnabled(AVULONG i)					{ return m_pVP[i].bEnabled; }
	AVULONG GetCamera(AVULONG i)				{ return m_pVP[i].nCamera; }
	void SetCamera(AVULONG i, AVULONG nCam)		{ m_pVP[i].nCamera = nCam; }

	AVULONG GetCurCamera()						{ return GetCamera(GetActive()); }
	void SetCurCamera(AVULONG nCam)				{ SetCamera(GetActive(), nCam); }
	
	// get, for the i'th viewport, the standard absolute dimension (0,0 - 1,1, plus pixel offsets)
	void GetViewport(AVULONG i, AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1, AVLONG &nmx0, AVLONG &nmx1, AVLONG &nmy0, AVLONG &nmy1, bool bShowSelFrame);
	
	void GetSelFrame(AVULONG i, AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1, AVLONG &nmx0, AVLONG &nmx1, AVLONG &nmy0, AVLONG &nmy1);
	// get, for the i'th viewport, the screen coordinates
	void GetViewport(AVULONG i, AVULONG &x0, AVULONG &x1, AVULONG &y0, AVULONG &y1, bool bShowSelFrame);
	// get the screen coordinates for entire usable screen
	void Get(AVULONG &x0, AVULONG &x1, AVULONG &y0, AVULONG &y1);


	void ApplyAspectRatio(AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1);

	AVFLOAT GetAspectRatio(AVULONG i);
	AVFLOAT GetCurAspectRatio()					{ return GetAspectRatio(GetActive()); }
	
	void Prepare();
	void Prepare(FWCOLOR frame, FWCOLOR active_frame, bool bShowSelFrame);
	bool Prepare(AVULONG i, bool bShowSelFrame);

	AVULONG GetActive()							{ return m_nActiveVP; }
	void SetActive(AVULONG nIndex)				{ m_nActiveVP = nIndex; }

	enum HIT { HIT_NONE, HIT_VIEW, HIT_XDIV, HIT_YDIV, HIT_XYDIV };
	void HitTest(CPoint &point, enum HIT &nHit, AVULONG &nIndexX, AVULONG &nIndexY);
	
	void OnDrag(CPoint &point, enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY);
	void OnDragCommit(enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY);
	void OnDragRollback(enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY);
};

class CScreen2x2 : public CScreen
{
public:

	enum VIEWPORTS { VP_FULL, VP_UP_LT, VP_UP_RT, VP_LW_LT, VP_LW_RT, VP_UP, VP_LW, VP_LT, VP_RT, VP_EXT_LT, VP_EXT_RT, VP_MAX };
	enum SCREENCFG { SC_SINGLE, SC_HORIZONTAL, SC_VERTICAL, SC_TRIPLE_LEFT, SC_TRIPLE_RIGHT, SC_QUADRUPLE, SC_MAX };

	CScreen2x2(IRenderer *pRenderer = NULL, AVLONG nFrame = 1);
	virtual void OnSetConfig(AVULONG);
};