// Material.h

#pragma once

class CBuilding;
using namespace std;

enum MATERIALS
{
	MAT_BACKGROUND, MAT_LOBBY_0, MAT_LOBBY_1, MAT_LOBBY_2, MAT_LOBBY_3, MAT_LOBBY_4, MAT_LOBBY_5, MAT_LOBBY_6, MAT_LOBBY_7, 
	MAT_FLOOR, MAT_CEILING, MAT_DOOR, MAT_LIFT_DOOR, MAT_OPENING, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, MAT_SHAFT, MAT_BEAM,
	MAT_NUM
};

struct MATERIAL
{
	AVULONG m_index;
	bool m_bSolid;
	COLORREF m_color;
	AVFLOAT m_alpha;
	wstring m_fname;
	AVFLOAT m_fUTile;
	AVFLOAT m_fVTile;

	LPCOLESTR GetLabel();
	void Set(COLORREF color, AVFLOAT alpha = 1.0f)									{ m_bSolid = true; m_color = color; m_alpha = alpha; }
	void Set(LPCOLESTR fname, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha = 1.0f)	{ m_bSolid = false; m_fname = fname; m_fUTile = fUTile; m_fVTile = fVTile; m_alpha = alpha; }
	void SetupBackground(CBuilding *pBuilding);
	void Setup(CBuilding *pBuilding, AVULONG nItem, AVULONG i = 0);
	void Setup(CBuilding *pBuilding);
};

class CMaterialManager
{
	CBuilding *m_pBuilding;
	MATERIAL m_materials[MAT_NUM];
public:
	CMaterialManager(CBuilding *pBuilding);

	MATERIAL &operator[](int i)		{ return m_materials[i]; }
	LPCOLESTR GetLabel(int i)		{ return m_materials[i].GetLabel(); }
	void Set(MATERIALS nMat, COLORREF color, AVFLOAT alpha = 1.0f);
	void Set(MATERIALS nMat, LPCOLESTR fname, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha = 1.0f);
	void Setup(MATERIALS nMat);
};