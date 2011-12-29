// Sprite.h

#pragma once

#include <D3d9.h>
#include <D3dx9core.h>
#include <freewill.h>
interface IRenderer;

namespace avsprite { };

class CSprite
{
public:

	class CFont
	{
		ID3DXFont *m_pFont;
		D3DCOLOR m_color;
		CFont(IDirect3DDevice9 *pDevice, AVLONG nHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName);
		friend class ::CSprite;
	public:
		~CFont();
		void SetColor(D3DCOLOR color)		{ m_color = color; }
		operator ID3DXFont*()				{ return m_pFont; }
		operator D3DCOLOR()					{ return m_color; }
		void OnLostDevice()					{ m_pFont->OnLostDevice(); }
		void OnResetDevice()				{ m_pFont->OnResetDevice(); }
	};

	class CTexture
	{
		IDirect3DTexture9 *m_pTexture;
		D3DCOLOR m_color;
		AVULONG m_w, m_h;			// size of the texture
		CTexture(IDirect3DDevice9 *pDevice, LPCTSTR strFilename, D3DCOLOR ColorKey = 0);
		friend class ::CSprite;
	public:
		~CTexture();
		AVULONG GetWidth()					{ return m_w; }
		AVULONG GetHeight()					{ return m_h; }
		void SetColor(D3DCOLOR color)		{ m_color = color; }
		operator IDirect3DTexture9*()		{ return m_pTexture; }
		operator D3DCOLOR()					{ return m_color; }
		void OnLostDevice()					{  }
		void OnResetDevice()				{  }
	};

protected:
	IRenderer *m_pRenderer;
	IDirect3DDevice9 *m_pDevice;
	ID3DXSprite *m_pSprite;

	// transforms
	D3DXVECTOR2 m_vecUserScale, m_vecScrScale;
	D3DXVECTOR2 m_vecTrans;
	bool m_bDirty;	// if set, imposes the transforms to be recalculated
	
public:
	CSprite(IRenderer *pRenderer = NULL);
	~CSprite();
	void SetRenderer(IRenderer *pRenderer);

	CTexture *LoadTexture(LPCTSTR strFilename, D3DCOLOR ColorKey = 0);
	CFont *CreateFont(AVLONG nHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName);

	void SetScale(AVFLOAT sx = 1, AVFLOAT sy = 1)						{ m_vecUserScale = D3DXVECTOR2(sx, sy); m_bDirty = true; }
	void SetTransform(AVULONG x = 0, AVULONG y = 0)						{ m_vecTrans = D3DXVECTOR2((AVFLOAT)x, (AVFLOAT)y); m_bDirty = true; }

	void Begin(DWORD nFlags = D3DXSPRITE_ALPHABLEND);
	void Draw(CTexture *p, CPoint pt, const RECT *pRect = NULL);
	void Draw(CFont *p, CPoint pt, LPCWSTR pText, DWORD nFormat = 0);
	void Draw(CFont *p, RECT *pRect, LPCWSTR pText, DWORD nFormat = 0);
	void CalcTextRect(CFont *p, RECT *pRect, LPCWSTR pText, DWORD nFormat = 0);
	void End();


	// Event Handlers
	void OnResize();
	void OnLostDevice();
	void OnResetDevice();

protected:
	// toolbox
	void calcTransforms();
};

class CTextPlate
{
	CSprite *m_pSprite;
	CSprite::CTexture *m_pTexture;
	CSprite::CFont *m_pFont;
	AVULONG m_nSlopeWidth;
	bool m_bSlopeOnLeft;
	CSize m_offsetText;

public:
	CTextPlate(CSprite *pSprite);
	~CTextPlate();

	void SetParams(LPCTSTR strFilename, D3DCOLOR colorKey, D3DCOLOR color,
		  	       AVLONG nFontHeight, BOOL bBold, BOOL bItalic, LPCWSTR pFaceName, D3DCOLOR colorText,
			       AVULONG nSlopeWidth, bool bSlopeOnLeft, CSize offsetText);

	CSize Calc(LPCWSTR pText, DWORD nFormat = 0);
	void Draw(CPoint pt, CSize sz, LPCWSTR pText, DWORD nFormat = 0);
	void Draw(CPoint pt, LPCWSTR pText, DWORD nFormat = 0);

	void OnLostDevice();
	void OnResetDevice();
};

