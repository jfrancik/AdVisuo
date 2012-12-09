// Material.cpp

#include "StdAfx.h"
#include "Material.h"
#include "VisLftGroup.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>

#include "freewilltools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)

LPCOLESTR MATERIAL::GetLabel()
{
	static LPCOLESTR labels[] = {
		L"Background",
		L"Lobby walls 0",
		L"Lobby walls 1",
		L"Lobby walls 2",
		L"Lobby walls 3",
		L"Lobby walls 4",
		L"Lobby walls 5",
		L"Lobby walls 6",
		L"Lobby walls 7",
		L"Lobby floors",
		L"Lobby ceilings",
		L"Shaft doors",
		L"Lift doors",
		L"Openings",
		L"Lift walls",
		L"Lift floors",
		L"Lift ceilings",
		L"Shaft walls",
		L"Division beams"
	};
	return labels[m_index]; 
}
 
void MATERIAL::SetupBackground(CLftGroupVis *pLftGroup)
{
	if (!m_bSolid) return;
	FWCOLOR color;
	color.r = (AVFLOAT)GetRValue(m_color) / 255.0f;
	color.g = (AVFLOAT)GetGValue(m_color) / 255.0f;
	color.b = (AVFLOAT)GetBValue(m_color) / 255.0f;
	color.a = 1;

	pLftGroup->GetRenderer()->PutBackColor(color);
}

void MATERIAL::Setup(CLftGroupVis *pLftGroup, AVULONG nItem, AVULONG i)
{
	if (m_bSolid)
	{
		FWCOLOR color;
		color.r = (AVFLOAT)GetRValue(m_color) / 255.0f;
		color.g = (AVFLOAT)GetGValue(m_color) / 255.0f;
		color.b = (AVFLOAT)GetBValue(m_color) / 255.0f;
		color.a = m_alpha;
		
		IMaterial *pMaterial = pLftGroup->GetMaterial(nItem, i);
		if (pMaterial)
		{
			pMaterial->SetDiffuseColor(color);
			pMaterial->SetAmbientColor(color);
			pMaterial->SetSpecularColor(color);
		}
		else
			pLftGroup->SetMaterial(nItem, i, color.r, color.g, color.b, m_alpha);
	}
	else
		pLftGroup->SetMaterial(nItem, i, (AVSTRING)m_fname.c_str(), m_fUTile, m_fVTile, m_alpha);
}

void MATERIAL::Setup(CLftGroupVis *pLftGroup)
{
	switch (m_index)
	{
	case MAT_BACKGROUND:	// Background
		SetupBackground(pLftGroup);
		break;
	case MAT_LOBBY_0:		// Lobby walls 0
		Setup(pLftGroup, CElem::WALL_FRONT, 0);
		Setup(pLftGroup, CElem::WALL_REAR, 0);
		Setup(pLftGroup, CElem::WALL_SIDE, 0);
		break;
	case MAT_LOBBY_1:		// Lobby walls 1
		Setup(pLftGroup, CElem::WALL_FRONT, 1);
		Setup(pLftGroup, CElem::WALL_REAR, 1);
		Setup(pLftGroup, CElem::WALL_SIDE, 1);
		break;
	case MAT_LOBBY_2:		// Lobby walls 2
		Setup(pLftGroup, CElem::WALL_FRONT, 2);
		Setup(pLftGroup, CElem::WALL_REAR, 2);
		Setup(pLftGroup, CElem::WALL_SIDE, 2);
		break;
	case MAT_LOBBY_3:		// Lobby walls 3
		Setup(pLftGroup, CElem::WALL_FRONT, 3);
		Setup(pLftGroup, CElem::WALL_REAR, 3);
		Setup(pLftGroup, CElem::WALL_SIDE, 3);
		break;
	case MAT_LOBBY_4:		// Lobby walls 4
		Setup(pLftGroup, CElem::WALL_FRONT, 4);
		Setup(pLftGroup, CElem::WALL_REAR, 4);
		Setup(pLftGroup, CElem::WALL_SIDE, 4);
		break;
	case MAT_LOBBY_5:		// Lobby walls 5
		Setup(pLftGroup, CElem::WALL_FRONT, 5);
		Setup(pLftGroup, CElem::WALL_REAR, 5);
		Setup(pLftGroup, CElem::WALL_SIDE, 5);
		break;
	case MAT_LOBBY_6:		// Lobby walls 6
		Setup(pLftGroup, CElem::WALL_FRONT, 6);
		Setup(pLftGroup, CElem::WALL_REAR, 6);
		Setup(pLftGroup, CElem::WALL_SIDE, 6);
		break;
	case MAT_LOBBY_7:		// Lobby walls 7
		Setup(pLftGroup, CElem::WALL_FRONT, 7);
		Setup(pLftGroup, CElem::WALL_REAR, 7);
		Setup(pLftGroup, CElem::WALL_SIDE, 7);
		break;
	case MAT_FLOOR:			// Lobby floors
		Setup(pLftGroup, CElem::WALL_FLOOR);
		break;
	case MAT_CEILING:		// Lobby ceilings
		Setup(pLftGroup, CElem::WALL_CEILING);
		break;
	case MAT_DOOR:			// Shaft doors
		Setup(pLftGroup, CElem::WALL_DOOR);
		break;
	case MAT_LIFT_DOOR:		// Lift doors
		Setup(pLftGroup, CElem::WALL_LIFT_DOOR);
		break;
	case MAT_OPENING:		// Openings
		Setup(pLftGroup, CElem::WALL_OPENING);
		break;
	case MAT_LIFT:			// Lift walls
		Setup(pLftGroup, CElem::WALL_LIFT);
		break;
	case MAT_LIFT_FLOOR:	// Lift floors
		Setup(pLftGroup, CElem::WALL_LIFT_FLOOR);
		break;
	case MAT_LIFT_CEILING:	// Lift ceiling
		Setup(pLftGroup, CElem::WALL_LIFT_CEILING);
		break;
	case MAT_SHAFT:			// Shaft Walls
		Setup(pLftGroup, CElem::WALL_SHAFT);
		break;
	case MAT_BEAM:			// Shaft Walls
		Setup(pLftGroup, CElem::WALL_BEAM);
		break;
	}
}

CMaterialManager::CMaterialManager(CLftGroupVis *pLftGroup) : m_pLftGroup(pLftGroup)
{
	for (int i = 0; i < MAT_NUM; i++)
		m_materials[i].m_index = i;
}

void CMaterialManager::Set(MATERIALS nMat, COLORREF color, AVFLOAT alpha)
{
	m_materials[nMat].Set(color, alpha);
	m_materials[nMat].Setup(m_pLftGroup);
}

void CMaterialManager::Set(MATERIALS nMat, LPCOLESTR fname, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha)
{
	m_materials[nMat].Set(fname, fUTile, fVTile, alpha);
	m_materials[nMat].Setup(m_pLftGroup);
}

void CMaterialManager::Setup(MATERIALS nMat)
{
	m_materials[nMat].Setup(m_pLftGroup);
}
