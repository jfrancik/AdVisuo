// Screen.cpp

#include "StdAfx.h"
#include "Screen.h"

#include <fwrender.h>

#pragma warning (disable:4995)
#pragma warning (disable:4996)
 

CScreen::CScreen(IRenderer *pRenderer, AVULONG nCols, AVULONG nRows, AVLONG nFrame)
{
	ASSERT(nRows > 0 && nCols > 0);

	m_pRenderer = NULL;
	SetRenderer(pRenderer);

	m_nXDivs = nCols + 1;
	m_pXDivs = new DIVIDER[m_nXDivs];
	for (AVULONG i = 0; i <= nCols; i++)
		m_pXDivs[i] = DIVIDER((AVFLOAT)i / (AVFLOAT)nCols, i > 0 && i < nCols);
	
	m_nYDivs = nRows + 1;
	m_pYDivs = new DIVIDER[m_nYDivs];
	for (AVULONG i = 0; i <= nRows; i++)
		m_pYDivs[i] = DIVIDER((AVFLOAT)i / (AVFLOAT)nRows, i > 0 && i < nRows);
	
	m_nFrame = nFrame;

	m_nActiveVP = 0;
	m_nConfig = 0;
	m_fAspect = 0;
}

CScreen::~CScreen()
{
	delete [] m_pXDivs;
	delete [] m_pYDivs;
	delete [] m_pVP;
	if (m_pRenderer) m_pRenderer->Release();
}

void CScreen::SetRenderer(IRenderer *pRenderer)
{
	if (m_pRenderer) m_pRenderer->Release();
	m_pRenderer = pRenderer;
	if (m_pRenderer) m_pRenderer->AddRef();
}

CScreen2x2::CScreen2x2(IRenderer *pRenderer, AVLONG nFrame) : CScreen(pRenderer, 2, 2, nFrame)
{
	m_nVP = VP_MAX;
	m_pVP = new VIEWPORT[VP_MAX];
	m_pVP[VP_FULL]    = VIEWPORT(0, 2, 0, 2, 0, true);
	m_pVP[VP_UP_LT]   = VIEWPORT(0, 1, 0, 1, 0, false);
	m_pVP[VP_UP_RT]   = VIEWPORT(1, 2, 0, 1, 1, false);
	m_pVP[VP_LW_LT]   = VIEWPORT(0, 1, 1, 2, 3, false);
	m_pVP[VP_LW_RT]   = VIEWPORT(1, 2, 1, 2, 2, false);
	m_pVP[VP_UP]      = VIEWPORT(0, 2, 0, 1, 4, false);
	m_pVP[VP_LW]      = VIEWPORT(0, 2, 1, 2, 5, false);
	m_pVP[VP_LT]      = VIEWPORT(0, 1, 0, 2, 0, false);	// cameras outside?
	m_pVP[VP_RT ]     = VIEWPORT(1, 2, 0, 2, 3, false);	// cameras outside?
	m_pVP[VP_EXT_LT]  = VIEWPORT(0, 1, 0, 2, 6, false);	// external cameras?
	m_pVP[VP_EXT_RT ] = VIEWPORT(1, 2, 0, 2, 7, false);	// external cameras?
}

void CScreen2x2::OnSetConfig(AVULONG n)
{
	bool configs[SC_MAX][VP_MAX] =
	{
		{  true, false, false, false, false, false, false, false, false, false, false },	// SC_SINGLE
		{ false, false, false, false, false,  true,  true, false, false, false, false },	// SC_HORIZONTAL
		{ false, false, false, false, false, false, false,  true,  true, false, false },	// SC_VERTICAL
		{ false, false,  true, false,  true, false, false, false, false,  true, false },	// SC_TRIPLE_LEFT
		{ false,  true, false,  true, false, false, false, false, false, false,  true },	// SC_TRIPLE_RIGHT
		{ false,  true,  true,  true,  true, false, false, false, false, false, false },	// SC_QUADRUPLE
	};
	
	if (n >= SC_MAX) return;

	m_nActiveVP = 0;
	for (AVULONG i = 0; i < m_nVP; i++)
		if (configs[n][i])
		{
			m_pVP[i].bEnabled = true;
			if (!m_nActiveVP) m_nActiveVP = i;
		}
		else
			m_pVP[i].bEnabled = false;
}

void CScreen::GetViewport(AVULONG i, AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1, AVLONG &nmx0, AVLONG &nmx1, AVLONG &nmy0, AVLONG &nmy1, bool bShowSelFrame)
{
	nmx0 = nmx1 = nmy0 = nmy1 = 0;
	if (i >= m_nVP)
	{
		// exception: VP out of range
		x0 = y0 = 0;
		x1 = y1 = 1;
	}
	else if (i != 0 && bShowSelFrame && i == m_nActiveVP)
	{
		// active VP - frame around!
		VIEWPORT *pVP = m_pVP + i;
		x0 = m_pXDivs[pVP->x0]; if (x0 > 0.001) nmx0 =  m_nFrame; else nmx0 =  2*m_nFrame; 
		x1 = m_pXDivs[pVP->x1]; if (x1 < 0.999) nmx1 = -m_nFrame; else nmx1 = -2*m_nFrame;
		y0 = m_pYDivs[pVP->y0]; if (y0 > 0.001) nmy0 =  m_nFrame; else nmy0 =  2*m_nFrame;
		y1 = m_pYDivs[pVP->y1]; if (y1 < 0.999) nmy1 = -m_nFrame; else nmy1 = -2*m_nFrame;
	}
	else
	{
		// regular VP
		VIEWPORT *pVP = m_pVP + i;
		x0 = m_pXDivs[pVP->x0]; if (x0 > 0.001) nmx0 = m_nFrame;
		x1 = m_pXDivs[pVP->x1]; if (x1 < 0.999) nmx1 = -m_nFrame;
		y0 = m_pYDivs[pVP->y0]; if (y0 > 0.001) nmy0 = m_nFrame;
		y1 = m_pYDivs[pVP->y1]; if (y1 < 0.999) nmy1 = -m_nFrame;
	}

	// calculate for the required aspect ratio
	ApplyAspectRatio(x0, x1, y0, y1);
}

void CScreen::GetSelFrame(AVULONG i, AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1, AVLONG &nmx0, AVLONG &nmx1, AVLONG &nmy0, AVLONG &nmy1)
{
	nmx0 = nmx1 = nmy0 = nmy1 = 0;
	if (i >= m_nVP)
	{
		// exception: VP out of range
		x0 = y0 = 0;
		x1 = y1 = 1;
	}
	else
	{
		// size including the highlighted frame around the active VP
		VIEWPORT *pVP = m_pVP + i;
		x0 = m_pXDivs[pVP->x0]; if (x0 > 0.001) nmx0 = -m_nFrame; else nmx0 =  0; 
		x1 = m_pXDivs[pVP->x1]; if (x1 < 0.999) nmx1 =  m_nFrame; else 0;
		y0 = m_pYDivs[pVP->y0]; if (y0 > 0.001) nmy0 = -m_nFrame; else 0;
		y1 = m_pYDivs[pVP->y1]; if (y1 < 0.999) nmy1 =  m_nFrame; else 0;
	}

	// calculate for the required aspect ratio
	ApplyAspectRatio(x0, x1, y0, y1);
}

void CScreen::ApplyAspectRatio(AVFLOAT &x0, AVFLOAT &x1, AVFLOAT &y0, AVFLOAT &y1)
{
	if (m_fAspect == 0) return;
	
	AVULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);
	AVFLOAT fAR = (AVFLOAT)x / (AVFLOAT)y;
	if (fAR > m_fAspect)
	{
		AVFLOAT f = m_fAspect / fAR;
		x0 = (1 - f) / 2 + f * x0;
		x1 = (1 - f) / 2 + f * x1;
	}
	else if (fAR < m_fAspect)
	{
		AVFLOAT f = fAR / m_fAspect;
		y0 = (1 - f) / 2 + f * y0;
		y1 = (1 - f) / 2 + f * y1;
	}
}

void CScreen::GetViewport(AVULONG i, AVULONG &x0, AVULONG &x1, AVULONG &y0, AVULONG &y1, bool bShowSelFrame)
{
	AVFLOAT fx0, fx1, fy0, fy1;
	AVLONG nmx0, nmx1, nmy0, nmy1;
	AVULONG x, y;
	GetViewport(i, fx0, fx1, fy0, fy1, nmx0, nmx1, nmy0, nmy1, bShowSelFrame);
	m_pRenderer->GetViewSize(&x, &y);
	x0 = (AVULONG)(x * fx0) + nmx0;
	x1 = (AVULONG)(x * fx1) + nmx1;
	y0 = (AVULONG)(y * fy0) + nmy0;
	y1 = (AVULONG)(y * fy1) + nmy1;
}

void CScreen::Get(AVULONG &x0, AVULONG &x1, AVULONG &y0, AVULONG &y1)
{
	AVFLOAT fx0 = 0, fx1 = 1, fy0 = 0, fy1 = 1;
	AVULONG x, y;
	ApplyAspectRatio(fx0, fx1, fy0, fy1);
	m_pRenderer->GetViewSize(&x, &y);
	x0 = (AVULONG)(x * fx0);
	x1 = (AVULONG)(x * fx1);
	y0 = (AVULONG)(y * fy0);
	y1 = (AVULONG)(y * fy1);
}

AVFLOAT CScreen::GetAspectRatio(AVULONG i)
{
	AVULONG x0, x1, y0, y1;
	GetViewport(i, x0, x1, y0, y1, false);
	return (AVFLOAT)(x1 - x0) / (AVFLOAT)(y1 - y0);
}

void CScreen::Prepare()
{
	m_pRenderer->PutViewport(0, 0, 1, 1, 0, 0, 0, 0);
}

void CScreen::Prepare(FWCOLOR frame, FWCOLOR active_frame, bool bShowSelFrame)
{
	FWCOLOR color;

	m_pRenderer->GetBackColor(&color);
	
	FWCOLOR black = { 0, 0, 0 };
	m_pRenderer->PutBackColor(black);
	m_pRenderer->PutViewport(0, 0, 1, 1, 0, 0, 0, 0);
	m_pRenderer->Clear();

	AVFLOAT x0 = 0, x1 = 1, y0 = 0, y1 = 1;
	ApplyAspectRatio(x0, x1, y0, y1);
	m_pRenderer->PutBackColor(frame);
	m_pRenderer->PutViewport(x0, y0, x1-x0, y1-y0, 0, 0, 0, 0);
	m_pRenderer->Clear();
	
	if (bShowSelFrame)
	{
		AVFLOAT x0, x1, y0, y1;
		AVLONG nmx0, nmx1, nmy0, nmy1;
		GetSelFrame(m_nActiveVP, x0, x1, y0, y1, nmx0, nmx1, nmy0, nmy1);

		m_pRenderer->PutBackColor(active_frame);
		m_pRenderer->PutViewport(x0, y0, x1-x0, y1-y0, nmx0, nmy0, nmx1-nmx0, nmy1-nmy0);
		m_pRenderer->Clear();
	}

	m_pRenderer->PutBackColor(color);
}

bool CScreen::Prepare(AVULONG i, bool bShowSelFrame)
{
	if (i >= m_nVP) return false;
	
	VIEWPORT *p = m_pVP + i;
	if (!p->bEnabled) return false;

	AVFLOAT x0, x1, y0, y1;
	AVLONG nmx0, nmx1, nmy0, nmy1;
	GetViewport(i, x0, x1, y0, y1, nmx0, nmx1, nmy0, nmy1, bShowSelFrame);

	m_pRenderer->PutViewport(x0, y0, x1-x0, y1-y0, nmx0, nmy0, nmx1-nmx0, nmy1-nmy0);
	m_pRenderer->Clear();

	return true;
}

void CScreen::HitTest(CPoint &point, enum HIT &nHit, AVULONG &nIndexX, AVULONG &nIndexY)
{
	nHit = HIT_NONE;
	nIndexX = nIndexY = 0;

	AVULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);

	for (AVULONG i = 0; i < m_nVP; i++)
	{
		VIEWPORT *pVP = m_pVP + i;
		if (!pVP->bEnabled) continue;

		AVULONG x0, x1, y0, y1;
		GetViewport(i, x0, x1, y0, y1, false);
		if ((ULONG)point.x > x0 && (ULONG)point.x < x1 && (ULONG)point.y > y0 && (ULONG)point.y < y1)
		{
			nHit = HIT_VIEW;
			nIndexX = i;
			return;
		}
	}

	for (AVULONG i = 0; i < m_nXDivs; i++)
		if (m_pXDivs[i].IsMovable() && abs(point.x - (AVLONG)(x * m_pXDivs[i])) <= m_nFrame)
		{
			nHit = HIT_XDIV;
			nIndexX = i;
		}

	for (AVULONG i = 0; i < m_nYDivs; i++)
		if (m_pYDivs[i].IsMovable() && abs(point.y - (AVLONG)(y * m_pYDivs[i])) <= m_nFrame)
		{
			nHit = (nHit == HIT_NONE) ? HIT_YDIV : HIT_XYDIV;
			nIndexY = i;
		}

}

void CScreen::OnDrag(CPoint &delta, enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY)
{
	AVULONG x, y;
	m_pRenderer->GetViewSize(&x, &y);

	if (nHit == HIT_XDIV || nHit == HIT_XYDIV)
	{
		m_pXDivs[nIndexX].SetDelta((AVFLOAT)delta.x / (AVFLOAT)x);
		m_pXDivs[nIndexX].ApplyMinMax(nIndexX > 0 ? m_pXDivs[nIndexX-1] + 0.1f : 0, nIndexX < m_nXDivs-1 ? m_pXDivs[nIndexX+1] - 0.1f : 1); 
	}
	if (nHit == HIT_YDIV || nHit == HIT_XYDIV)
	{
		m_pYDivs[nIndexY].SetDelta((AVFLOAT)delta.y / (AVFLOAT)y);
		m_pYDivs[nIndexY].ApplyMinMax(nIndexY > 0 ? m_pYDivs[nIndexY-1] + 0.1f : 0, nIndexY < m_nYDivs-1 ? m_pYDivs[nIndexY+1] - 0.1f : 1);
	}
}

void CScreen::OnDragCommit(enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY)
{
	if (nHit == HIT_XDIV || nHit == HIT_XYDIV)
		m_pXDivs[nIndexX].Commit();
	if (nHit == HIT_YDIV || nHit == HIT_XYDIV)
		m_pYDivs[nIndexY].Commit();
}

void CScreen::OnDragRollback(enum HIT &nHit, AVULONG nIndexX, AVULONG nIndexY)
{
	if (nHit == HIT_XDIV || nHit == HIT_XYDIV)
		m_pXDivs[nIndexX].Rollback();
	if (nHit == HIT_YDIV || nHit == HIT_XYDIV)
		m_pYDivs[nIndexY].Rollback();
}

