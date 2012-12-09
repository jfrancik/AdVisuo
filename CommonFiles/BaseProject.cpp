// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseProject.h"
#include "BaseLftGroup.h"
#include "BaseSimClasses.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProject

CProject::CProject()
{
	m_nId = 0;
	m_nSimulationId = 0;
	m_nAVVersionId = 0;
}

CProject::~CProject()
{
	DeleteLftGroups();
}

CLftGroup *CProject::FindLftGroup(int id)
{
	auto i = std::find_if(m_groups.begin(), m_groups.end(), [id] (CLftGroup* p) -> bool { return (p->GetId() == id); } );
	return (i == m_groups.end()) ? NULL : *i;
}

CLftGroup *CProject::AddLftGroup()
{
	CLftGroup *p = CreateLftGroup(m_groups.size());
	m_groups.push_back(p);
	return p;
}

void CProject::DeleteLftGroups()
{
	for each (CLftGroup *pGroup in m_groups)
		delete pGroup;
}

CSim *CProject::GetSim(int i)
{
	return m_groups[i]->GetSim();
}

CSim *CProject::FindSim(int id)
{
	auto i = std::find_if(m_groups.begin(), m_groups.end(), [id] (CLftGroup* p) -> bool { CSim *pSim = p->GetSim(); if (pSim) return (pSim->GetId() == id); else return false; } );
	return (i == m_groups.end()) ? NULL : (*i)->GetSim();
}

void CProject::ResolveMe()
{
	m_nId = ME[L"ID"];
	m_nSimulationId = ME[L"SimulationId"];
	m_nAVVersionId = ME[L"AVVersionId"];
}

std::wstring CProject::GetProjectInfo(PRJ_INFO what)
{
	switch (what)
	{
		case PRJ_PROJECT_NAME: return ME[L"ProjectName"];
		case PRJ_BUILDING_NAME: return ME[L"BuildingName"];
		case PRJ_LANGUAGE: return ME[L"Language"];
		case PRJ_UNITS: return ME[L"MeasurementUnits"];
		case PRJ_COMPANY: return ME[L"ClientCompany"];
		case PRJ_CITY: return ME[L"City"];
		case PRJ_LB_RGN: return ME[L"LBRegionDistrict"];
		case PRJ_COUNTY: return ME[L"County"];
		case PRJ_DESIGNER: return ME[L"LiftDesigner"];
		case PRJ_COUNTRY: return ME[L"Country"];
		case PRJ_CHECKED_BY: return ME[L"CheckedBy"];
		case PRJ_POST_CODE: return ME[L"PostalZipCode"];
		default: return L"(unknown)";
	}
}

void CProject::Scale(AVFLOAT fScale)
{ 
	for each (CLftGroup *pGroup in m_groups) 
		pGroup->Scale(fScale); 
}

void CProject::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{ 
	for each (CLftGroup *pGroup in m_groups) 
		pGroup->Move(x, y, z); 
}
