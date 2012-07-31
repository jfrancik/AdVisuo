// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrProject.h"
#include "ConstrBuilding.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBone / CElem

CElem::CElem(CProject *pProject, CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)	
	: m_pProject(pProject), m_pBuilding(pBuilding), m_pParent(pParent)
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

	BOX _box = GetBuilding()->GetTotalAreaBox();

	float y = GetBuilding()->GetTotalAreaBox().RearExt();
	for each (CSim *pSim in m_sims)
	{
		CBuildingConstr *pBuilding = (CBuildingConstr*)pSim->GetBuilding();
		
		float x = -(GetBuilding()->GetBox().WidthExt() - pBuilding->GetBox().WidthExt()) / 2;
		y -= pBuilding->GetTotalAreaBox().RearExt();

		pBuilding->Deconstruct();
		pBuilding->Construct(Vector(x, y, 0));

		pSim->SetOffsetVector(Vector(x, y, 0));
		y = pBuilding->GetTotalAreaBox().FrontExt();
	}
}

void CProjectConstr::Deconstruct()
{
	delete m_pElem;
	delete m_pElemSite;
}
