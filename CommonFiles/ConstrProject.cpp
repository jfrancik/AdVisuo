// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrProject.h"
#include "ConstrLftGroup.h"
#include "BaseSimClasses.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBone / CElem

CElem::CElem(CProject *pProject, CLftGroup *pLftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)	
	: m_pProject(pProject), m_pLftGroup(pLftGroup), m_pParent(pParent)
{
	OLECHAR _name[257];
	_snwprintf_s(_name, 256, name, LOWORD(i), HIWORD(i));
	m_name = _name;
}

CElem::~CElem()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProjectConstr

void CProjectConstr::Construct()
{
	m_pElem = CreateElement(NULL, NULL, CElem::ELEM_PROJECT, (LPOLESTR)GetProjectInfo(PRJ_PROJECT_NAME).c_str(), 0, Vector(0));
	m_pElemSite = CreateElement(NULL, GetElement(), CElem::ELEM_SITE, (LPOLESTR)GetProjectInfo(PRJ_BUILDING_NAME).c_str(), 0, Vector(0));

	if (!m_pElem) return;

	BOX _box = GetLftGroup(0)->GetTotalAreaBox();

	float y = GetLftGroup(0)->GetTotalAreaBox().RearExt();
	for each (CLftGroupConstr *pLftGroup in m_groups)
	{
		float x = -(GetLftGroup(0)->GetBox().WidthExt() - pLftGroup->GetBox().WidthExt()) / 2;
		y -= pLftGroup->GetTotalAreaBox().RearExt();

		pLftGroup->Deconstruct();
		pLftGroup->Construct(Vector(x, y, 0));

		if (pLftGroup->GetSim())
			pLftGroup->GetSim()->SetOffsetVector(Vector(x, y, 0));
		y = pLftGroup->GetTotalAreaBox().FrontExt();
	}
}

void CProjectConstr::Deconstruct()
{
	delete m_pElem;
	delete m_pElemSite;
}
