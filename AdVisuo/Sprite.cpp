// Sprite.cpp

#include "StdAfx.h"
#include "Sprite.h"

#include <fwrender.h>

#pragma warning (disable:4995)
#pragma warning (disable:4996)
 

CSprite::CSprite(IRenderer *pRenderer)
{
	m_pRenderer = NULL;
	m_pDevice = NULL;
	m_pSprite = NULL;

	SetRenderer(pRenderer);

	m_vecUserScale = m_vecScrScale = D3DXVECTOR2(1, 1);
	m_vecTrans = D3DXVECTOR2(0, 0);
	m_bDirty = true;
}

CSprite::~CSprite()
{
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pDevice) m_pDevice->Release();
	if (m_pSprite) m_pSprite->Release();
}

CSprite::CFont::CFont(IDirect3DDevice9 *pDevice, AVLONG nHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName)
{
	m_pFont = NULL;
	m_color = D3DCOLOR_COLORVALUE(1, 1, 1, 1);
	D3DXCreateFont(pDevice, nHeight, 0, bBold ? FW_BOLD : FW_NORMAL, 0, bItalic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, pFaceName, &m_pFont);
}

CSprite::CFont::~CFont()
{
	if (m_pFont) m_pFont->Release();
}

CSprite::CTexture::CTexture(IDirect3DDevice9 *pDevice, LPCTSTR strFilename, D3DCOLOR ColorKey)
{
	m_pTexture = NULL;
	m_color = D3DCOLOR_COLORVALUE(1, 1, 1, 1);
	m_w = m_h = 0;

	HRESULT h = D3DXCreateTextureFromFileEx(pDevice, strFilename, 0, 0, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, ColorKey, NULL, NULL, &m_pTexture);
	if SUCCEEDED(h)
	{
		D3DSURFACE_DESC desc;
		m_pTexture->GetLevelDesc(0, &desc);
		m_w = desc.Width; m_h = desc.Height;
	}
}

CSprite::CTexture::~CTexture()
{
	if (m_pTexture) m_pTexture->Release();
}

void CSprite::SetRenderer(IRenderer *pRenderer)
{
	if (m_pRenderer) m_pRenderer->Release();
	m_pRenderer = pRenderer;
	if (!m_pRenderer) return;
	m_pRenderer->AddRef();
	m_pRenderer->GetDeviceHandle(1, (FWHANDLE*)&m_pDevice);
	if (m_pDevice)
	{
		if (m_pSprite) m_pSprite->Release();
		D3DXCreateSprite(m_pDevice, &m_pSprite);
		m_bDirty = true;
	}
}

CSprite::CTexture *CSprite::LoadTexture(LPCTSTR  strFilename, D3DCOLOR ColorKey)
{
	return m_pDevice ? new CTexture(m_pDevice, strFilename, ColorKey) : NULL;
}

CSprite::CFont *CSprite::CreateFont(AVLONG nHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName)
{
	return m_pDevice ? new CFont(m_pDevice, nHeight, bBold, bItalic, pFaceName) : NULL;
}

void CSprite::Begin(DWORD nFlags)
{
	if (m_bDirty) calcTransforms();
	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
}

void CSprite::Draw(CTexture *p, CPoint pt, const RECT *pRect)
{
	D3DXVECTOR3 pos((AVFLOAT)pt.x, (AVFLOAT)pt.y, 0);
	m_pSprite->Draw(*p, pRect, NULL, &pos, *p);
}

void CSprite::Draw(CFont *p, CPoint pt, LPCWSTR pText, DWORD nFormat)
{
	ID3DXFont *pFont = *p;
	RECT rect;
	rect.left = rect.right = pt.x; rect.top = rect.bottom = pt.y; 
	pFont->DrawText(m_pSprite, pText, -1, &rect, DT_CALCRECT | nFormat, *p);
	pFont->DrawText(m_pSprite, pText, -1, &rect, nFormat, *p);
}

void CSprite::Draw(CFont *p, RECT *pRect, LPCWSTR pText, DWORD nFormat)
{
	ID3DXFont *pFont = *p;
	pFont->DrawText(m_pSprite, pText, -1, pRect, nFormat, *p);
}

void CSprite::CalcTextRect(CFont *p, RECT *pRect, LPCWSTR pText, DWORD nFormat)
{
	ID3DXFont *pFont = *p;
	pFont->DrawText(m_pSprite, pText, -1, pRect, DT_CALCRECT | nFormat, *p);
}

void CSprite::End()
{
	m_pSprite->End();
}

void CSprite::OnResize()
{
	AVULONG x, y, X, Y;
	m_pRenderer->GetViewSize(&x, &y);
	m_pRenderer->GetBackBufferSize(&X, &Y);
	m_vecScrScale = D3DXVECTOR2((AVFLOAT)X / (AVFLOAT)x, (AVFLOAT)Y / (AVFLOAT)y);
	m_bDirty = true;
}

void CSprite::OnLostDevice()
{
	m_pSprite->OnLostDevice();
}

void CSprite::OnResetDevice()
{
	m_pSprite->OnResetDevice();
}

void CSprite::calcTransforms()
{
	D3DXVECTOR2 vecScale = D3DXVECTOR2(m_vecUserScale.x * m_vecScrScale.x, m_vecUserScale.y * m_vecScrScale.y);
	D3DXMATRIX mat;
	D3DXMatrixTransformation2D(&mat, NULL, 0.0f, &vecScale, NULL, NULL, &m_vecTrans);
	m_pSprite->SetTransform(&mat);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// CTextPlate

CTextPlate::CTextPlate(CSprite *pSprite) : m_pSprite(pSprite), m_pTexture(NULL), m_pFont(NULL), m_nSlopeWidth(0), m_bSlopeOnLeft(false), m_offsetText(0, 0)
{
}

CTextPlate::~CTextPlate()
{
	if (m_pTexture) delete m_pTexture;
	if (m_pFont) delete m_pFont;
}

void CTextPlate::SetParams(LPCTSTR strFilename, D3DCOLOR colorKey, D3DCOLOR color,
		  	       AVLONG nFontHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName, D3DCOLOR colorText,
			       AVULONG nSlopeWidth, bool bSlopeOnLeft, CSize offsetText)
{
	m_pTexture = m_pSprite->LoadTexture(strFilename, colorKey);
	m_pTexture->SetColor(color);
	m_pFont = m_pSprite->CreateFont(nFontHeight, bBold, bItalic, pFaceName);
	m_pFont->SetColor(colorText);
	m_nSlopeWidth = nSlopeWidth;
	m_bSlopeOnLeft = bSlopeOnLeft;
	m_offsetText = offsetText;
}

CSize CTextPlate::Calc(LPCWSTR pText, DWORD nFormat)
{
	CRect rect(0, 0, 0, 0);
	m_pSprite->CalcTextRect(m_pFont, rect, pText, nFormat);
	return CSize(rect.Width() + m_offsetText.cx + max((LONG)m_nSlopeWidth, m_offsetText.cx), rect.Height() + 2 * m_offsetText.cy);
}

void CTextPlate::Draw(CPoint pt, CSize sz, LPCWSTR pText, DWORD nFormat)
{
	CRect rect(CPoint(0, 0), sz);
	rect.MoveToXY(m_bSlopeOnLeft ? 0 : m_pTexture->GetWidth() - sz.cx, m_pTexture->GetHeight() - sz.cy); 
	m_pSprite->Draw(m_pTexture, pt, rect);

	CPoint ptt(pt.x + (m_bSlopeOnLeft ? max((LONG)m_nSlopeWidth, m_offsetText.cx) : m_offsetText.cx), pt.y + m_offsetText.cy);
	CSize  szt(sz.cx - m_offsetText.cx - max((LONG)m_nSlopeWidth, m_offsetText.cx), sz.cy - 2 * m_offsetText.cy);
	m_pSprite->Draw(m_pFont, CRect(ptt, szt), pText, nFormat);
}

void CTextPlate::Draw(CPoint pt, LPCWSTR pText, DWORD nFormat)
{
	Draw(pt, Calc(pText, nFormat), pText, nFormat);
}

void CTextPlate::OnLostDevice()
{
	m_pSprite->OnLostDevice();
	m_pTexture->OnLostDevice();
	m_pFont->OnLostDevice();
}

void CTextPlate::OnResetDevice()
{
	m_pSprite->OnResetDevice();
	m_pTexture->OnResetDevice();
	m_pFont->OnResetDevice();
}

