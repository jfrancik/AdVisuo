// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrProject.h"
#include "ConstrLiftGroup.h"
#include "BaseSimClasses.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBone / CElem

CElem::CElem(CProject *pProject, CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)	
	: m_pProject(pProject), m_pLiftGroup(pLiftGroup), m_pParent(pParent)
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
	if (GetLiftGroupsCount() == 0) return;

	BOX _box = GetLiftGroup(0)->GetTotalAreaBox();

	float y = GetLiftGroup(0)->GetTotalAreaBox().RearExt();
	for each (CLiftGroupConstr *pLiftGroup in GetLiftGroups())
	{
		float x = -(GetLiftGroup(0)->GetBox().WidthExt() - pLiftGroup->GetBox().WidthExt()) / 2;
		y -= pLiftGroup->GetTotalAreaBox().RearExt();

		pLiftGroup->Deconstruct();
		pLiftGroup->Construct(Vector(x, y, 0));

		for each (CSim *pSim in pLiftGroup->GetSims())
			pSim->SetOffsetVector(Vector(x, y, 0));
		
		y = pLiftGroup->GetTotalAreaBox().FrontExt();
	}
}

void CProjectConstr::Deconstruct()
{
	delete m_pElem;
	delete m_pElemSite;
}
