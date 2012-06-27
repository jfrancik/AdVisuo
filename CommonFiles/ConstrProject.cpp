// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "ConstrProject.h"
#include "ConstrBuilding.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProject

void CProjectConstr::Construct()
{
	m_pElem = CreateElement(NULL);
	m_pElemSite = CreateElement(NULL);

	if (!m_pElem) return;

	m_pElem->Create(NULL, CElem::ELEM_PROJECT, (LPOLESTR)GetProjectInfo(PRJ_PROJECT_NAME).c_str(), Vector(0));
	m_pElemSite->Create(GetElement(), CElem::ELEM_SITE, (LPOLESTR)GetProjectInfo(PRJ_BUILDING_NAME).c_str(), Vector(0));

	float y = GetBuilding()->GetShaftLinesCount() == 2 ? GetBuilding()->GetShaft(GetBuilding()->GetShaftCount()-1)->GetBox().RearExt() : GetBuilding()->GetBox().RearExt();
	for each (CSim *pSim in m_sims)
	{
		CBuildingConstr *pBuilding = (CBuildingConstr*)pSim->GetBuilding();
		y -= pBuilding->GetShaftLinesCount() == 2 ? pBuilding->GetShaft(pBuilding->GetShaftCount()-1)->GetBox().RearExt() : pBuilding->GetBox().RearExt();
		float x = -(GetBuilding()->GetBox().WidthExt() - pBuilding->GetBox().WidthExt()) / 2;

		pBuilding->Deconstruct();
		pBuilding->Construct(Vector(x, y, 0));

		pSim->SetOffsetVector(Vector(x, y, 0));
		y += pBuilding->GetShaft(0)->GetBox().RearExt();
	}
}

void CProjectConstr::Deconstruct()
{
	delete m_pElem;
	delete m_pElemSite;
}
