// HUD.cpp

#include "StdAfx.h"
#include "HUD.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// CHUD

CHUD::CHUD(CSprite *pSprite) : m_pSprite(pSprite), m_pFont(NULL), m_plateAlt(pSprite)
{
	m_nTime = m_nTimeLoaded = m_nTimeSim = m_nTimeSlider = 0;
	m_strTime = /*m_strTimeLoaded =*/ m_strTimeSim = _helperTime2Str(0);

	static LPCTSTR s_pFilenames[] = 
	{
		L"",
		L"plate_0.bmp",
		L"plate_1.bmp",
		L"plate_2.bmp",
		L"plate_3.bmp",
		L"icons_32x32.bmp",
		L"icons_16x16.bmp",
		L"icons_8x16.bmp",
		L"icons_8x8.bmp"
	};
	memcpy(m_pFilenames, s_pFilenames, sizeof(m_pFilenames));

	memset(m_pTextures, 0, sizeof(m_pTextures));

	static HUDITEM s_pItems[] = 
	{
		{ CPoint(84, 46), CSize(308, 12), TEX_NONE,  0, 0, ID_ACTION_REWIND },	// slider
		{ CPoint(28, 30), CSize(32, 32),  TEX_32x32, 0, 0, ID_ACTION_PLAYSPECIAL },
		{ CPoint(62, 44), CSize(16, 16),  TEX_16x16, 0, 0, ID_ACTION_STOP },
		{ CPoint(448, 44), CSize(8, 16),  TEX_8x16,  0, 0, ID_ACTION_SLOWDOWN },
		{ CPoint(456, 44), CSize(8, 16),  TEX_8x16,  2, 0, ID_ACTION_NORMALPACE },
		{ CPoint(464, 44), CSize(8, 16),  TEX_8x16,  4, 0, ID_ACTION_SPEEDUP },
		{ CPoint(476, 44), CSize(16, 16), TEX_16x16, 2, 0, ID_VIEW_FULLSCREEN }, // full screen
		{ CPoint(464, 26), CSize(8, 8),   TEX_8x8,   0, 0, ID_HUD_PINDOWN }, // pin down
		{ CPoint(476, 26), CSize(8, 8),   TEX_8x8,   4, 0, ID_HUD_PANEL },	// close HUD
		{ CPoint(16, 24), CSize(480, 40), TEX_NONE,  0, 0, 0}	// background
	};
	memcpy(m_pItems, s_pItems, sizeof(m_pItems));
	m_nHit = HIT_NONE;
	m_nAutoHideTime = 6500;
	m_bVisible = true;
	m_bAutoHide = true;
	KeepReady();
}

CHUD::~CHUD()
{
	for (AVULONG i = 0; i < TEX_MAX; i++)
		if (m_pTextures[i]) delete m_pTextures[i];
	if (m_pFont) delete m_pFont;
}

void CHUD::Initialise()
{
	m_plateAlt.SetParams((_stdPathModels + L"plateSE.bmp").c_str(), 0xFF0000FF, 0x80FFFFFF, 12, TRUE, FALSE, L"System", 0xFF000000, 16, true, CSize(2, 2));
	for (AVULONG i = 0; i < TEX_MAX; i++)
		if (*m_pFilenames[i])
		{
			m_pTextures[i] = m_pSprite->LoadTexture((_stdPathModels + m_pFilenames[i]).c_str(), 0xFF0000FF);
			m_pTextures[i]->SetColor(0x80FFFFFF);
		}
	m_pFont = m_pSprite->CreateFont(12, TRUE, FALSE, L"System");
	m_pFont->SetColor(0xFF000000);
	KeepReady();
}

void CHUD::Draw(CPoint ptMouse)
{
	// check visibility & auto-hide
	if (!m_bVisible) return;
	AVULONG nAH = GetAH() / 25;

	// draw alt time plate
	if (nAH > 0 /*&& m_nTime > 0*/)
	{
		CSize size = m_plateAlt.Calc(m_strTime);
		m_ptAlt.y += 40 - min(nAH, 40);
		m_plateAlt.Draw(m_ptAlt - size, size, m_strTime);
		m_ptAlt.y -= 40 - min(nAH, 40);
	}

	// auto-hide amendment
	if (nAH > 40) return;
	m_pt.y += nAH;

	// check hit test
	HIT nHit = HitTest(ptMouse, false);

	// draw background
	m_pSprite->Draw(m_pTextures[TEX_0], m_pt);

	// draw buttons
	for (AVULONG i = 0; i < HIT_MAX; i++)
	{
		HUDITEM *p = m_pItems + i;
		if (p->nTexId == TEX_NONE) continue;
		AVULONG nIndex = p->nIndex + p->nStatus;
		if (i == nHit) nIndex |= 1;
		CRect rect(CPoint(p->sz.cx * nIndex), p->sz);
		m_pSprite->Draw(m_pTextures[p->nTexId], m_pt + p->pt, rect);
	}

	// draw the slider
	CRect rect(m_pItems[HIT_SLIDER].pt, m_pItems[HIT_SLIDER].sz);
	AVULONG w = rect.Width();

	AVFLOAT fLoaded = (AVFLOAT)m_nTimeLoaded / m_nTimeSim; if (fLoaded < 0) fLoaded = 0; if (fLoaded > 1) fLoaded = 1;
	AVFLOAT fPlay   = (AVFLOAT)m_nTime / m_nTimeSim;       if (fPlay < 0) fPlay = 0; if (fPlay > 1) fPlay = 1;
	AVFLOAT fSlider = (AVFLOAT)m_nTimeSlider / m_nTimeSim; if (fSlider < 0) fSlider = 0; if (fSlider > 1) fSlider = 1;
	AVULONG nLoaded = (AVULONG)ceil((AVFLOAT)w * fLoaded);
	AVULONG nPlay   = (AVULONG)ceil((AVFLOAT)w * fPlay);
	AVULONG nSlider = (AVULONG)ceil((AVFLOAT)w * fSlider);

	rect.right = rect.left + nLoaded;
	m_pSprite->Draw(m_pTextures[TEX_1], m_pt + rect.TopLeft(), rect);
	rect.right = rect.left + nPlay;
	m_pSprite->Draw(m_pTextures[TEX_2], m_pt + rect.TopLeft(), rect);

	// display digits
	m_pSprite->Draw(m_pFont, m_pt + CPoint(rect.right - 22, 28), m_strTime);
	m_pSprite->Draw(m_pFont, m_pt + CPoint(398, 44), m_strTimeSim);

	// slider drag
	if (!m_strTimeSlider.IsEmpty()) 
	{
		rect.left += nSlider;
		rect.right = rect.left + 2;
		rect.top--; rect.bottom++;
		m_pSprite->Draw(m_pTextures[TEX_3], m_pt + rect.TopLeft(), rect);
		rect.left += 8;
		m_pSprite->Draw(m_pFont, m_pt + CPoint(min(rect.left, 340), 44), m_strTimeSlider);
	}

	// undo auto-hide amendment
	m_pt.y -= nAH;
}

CHUD::HIT CHUD::HitTest(CPoint pt, bool bAH)
{
	pt -= m_pt;

	HIT hit = HIT_NONE;

	CRect rect(16, 24, 496, 64);
	if (rect.PtInRect(pt))
	{
		hit = HIT_BACK;
		KeepReady();
	}

	if (!m_bVisible) return HIT_NONE;

	for (AVULONG i = 0; i < HIT_MAX; i++)
	{
		HUDITEM *p = m_pItems + i;
		CRect rect(p->pt, p->sz);
		if (rect.PtInRect(pt))
			return (enum HIT)i;
	}
	return hit;
}

void CHUD::KeepReady()
{
	m_nTick = ::GetTickCount();
}

void CHUD::Hide()
{
	m_nTick = 0;
}

AVULONG CHUD::GetAH()
{
	if (!m_bAutoHide) return 0;
	if (!m_bVisible) return m_nAutoHideTime + m_nAutoHideTime;
	AVULONG nAH = ::GetTickCount() - m_nTick;
	return (nAH > m_nAutoHideTime) ? nAH - m_nAutoHideTime : 0;
}

bool CHUD::OnDragBegin(CPoint pt)
{
	m_nHit = HitTest(pt);
	OnDrag(pt);
	return m_nHit != HIT_NONE;
}

void CHUD::OnDrag(CPoint pt)
{
	if (m_nHit == HIT_SLIDER && HitTest(pt) == HIT_SLIDER)
	{
		AVULONG x = pt.x - m_pt.x - m_pItems[HIT_SLIDER].pt.x;
		m_nTimeSlider = m_nTimeSim * x / m_pItems[HIT_SLIDER].sz.cx;
		if (m_nTimeSlider >= m_nTimeLoaded)
			m_strTimeSlider.Empty();
		else
			m_strTimeSlider = _helperTime2Str(m_nTimeSlider);
	}
	else
		m_strTimeSlider.Empty();
}

void CHUD::OnDragCommit(CPoint pt, HWND hWnd)
{
	m_strTimeSlider.Empty();
	if (HitTest(pt) == m_nHit)
	{
		ULONG nCmd = m_pItems[m_nHit].nCmd;
		if (nCmd)
			::PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(nCmd, 0), (LPARAM)0);
	}
}

void CHUD::OnToggle()
{
	m_bVisible = !m_bVisible;
	KeepReady();
}

void CHUD::OnPinDown()
{
	m_bAutoHide = !m_bAutoHide;
	m_pItems[HIT_PIN].nStatus = m_bAutoHide ? 0 : 2;
}

void CHUD::OnLostDevice()
{
	m_pSprite->OnLostDevice();
	m_plateAlt.OnLostDevice();
	for (AVULONG i = 0; i < TEX_MAX; i++)
		if (m_pTextures[i]) m_pTextures[i]->OnLostDevice();
	m_pFont->OnLostDevice();
}

void CHUD::OnResetDevice()
{
	m_pSprite->OnResetDevice();
	m_plateAlt.OnResetDevice();
	for (AVULONG i = 0; i < TEX_MAX; i++)
		if (m_pTextures[i]) m_pTextures[i]->OnResetDevice();
	m_pFont->OnResetDevice();
}

CString CHUD::_helperTime2Str(AVLONG t)
{
	CString str;
	if (t > -1000)
		str.Format(L"%d:%02d:%02d", (t/3600000), (t/60000)%60, (t/1000)%60);		//, (-t/10)%100);
	else
		str = L"0:00:00";
		//str.Format(L"-%d:%02d:%02d", (-t/3600000), (-t/60000)%60, (-t/1000)%60);	//, (-t/10)%100);
	return str;
}

CString CHUD::_helperTime2StrX(AVLONG t)
{
	CString str;
	if (t > 0)
		str.Format(L"%d:%02d:%02d.%01d", (t/3600000), (t/60000)%60, (t/1000)%60, (t/100)%10);		//, (-t/10)%100);
	else
		str = L"0:00:00.0";
		//str.Format(L"-%d:%02d:%02d", (-t/3600000), (-t/60000)%60, (-t/1000)%60);	//, (-t/10)%100);
	return str;
}

