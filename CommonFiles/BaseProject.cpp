// BaseClasses.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseProject.h"
#include "BaseLiftGroup.h"
#include "BaseSimClasses.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CProject

CProject::CProject()
{
	m_nId = 0;
	m_nSimulationId = 0;
	m_nAVVersionId = 0;
	m_nMaxTime = 0;
	m_nTimeSaved = 0;
	m_bRevitCompatibilityMode_ = false;
}

CProject::~CProject()
{
	for each (CLiftGroup *pGroup in m_groups)
		delete pGroup;
}

AVLONG CProject::GetSimulationStartTime()
{
	AVLONG t = 0;
	for each (CLiftGroup *pGroup in GetLiftGroups())
		for each (CSim *pSim in pGroup->GetSims())
			for each (CPassenger *pPassenger in pSim->GetPassengers())
				if (pPassenger->GetSpawnTime() < t)
					t = pPassenger->GetSpawnTime();
	return t;
}

AVULONG CProject::GetMaxStoreyCount()
{
	return GetLiftGroupsCount() ? GetLiftGroup(0)->GetStoreyCount() : 0;
}

AVULONG CProject::GetMaxBasementStoreyCount()
{
	return GetLiftGroupsCount() ? GetLiftGroup(0)->GetBasementStoreyCount() : 0;
}

AVULONG CProject::GetMaxShaftCount()
{
	auto i = std::max_element(m_groups.begin(), m_groups.end(), [] (CLiftGroup* p1, CLiftGroup* p2) -> bool { return p1->GetShaftCount() < p2->GetShaftCount(); } );
	return (i == m_groups.end()) ? 0 : (*i)->GetShaftCount();
}

CLiftGroup *CProject::AddLiftGroup()
{
	CLiftGroup *p = CreateLiftGroup(m_groups.size());
	m_groups.push_back(p);
	return p;
}

CLiftGroup *CProject::FindLiftGroup(int id)
{
	auto i = std::find_if(m_groups.begin(), m_groups.end(), [id] (CLiftGroup* p) -> bool { return (p->GetId() == id); } );
	return (i == m_groups.end()) ? NULL : *i;
}

CSim *CProject::FindSim(int id)
{
	for each (CLiftGroup *pGroup in GetLiftGroups())
	{
		std::vector<CSim*> &sims = pGroup->GetSims();
		auto i = std::find_if(sims.begin(), sims.end(), [id] (CSim* pSim) -> bool { return (pSim->GetId() == id); } );
		if (i != sims.end()) return *i;
	}
	return NULL;
}

void CProject::ResolveMe()
{
	m_nId = ME[L"ID"];
	m_nSimulationId = ME[L"SimulationId"];
	m_nAVVersionId = ME[L"AVVersionId"];

	m_nMaxTime = ME[L"MaxSimulationTime"];
	m_nTimeSaved = ME[L"TimeSaved"];
}

std::wstring CProject::GetProjectInfo(PRJ_INFO what)
{
	switch (what)
	{
		case PRJ_NAME:			return ME[L"ProjectName"];
		case PRJ_NUMBER:		return ME[L"ProjectNo"];
		case PRJ_LIFT_DESIGNER:	return ME[L"LiftDesigner"]; 
		case PRJ_CHECKED_BY:	return ME[L"CheckedBy"];
		case PRJ_CLIENT_NAME:	return ME[L"ClientCompanyName"];
		case PRJ_BUILDING_NAME:	return ME[L"BuildingName"];
		case PRJ_CITY:			return ME[L"City"];
		case PRJ_COUNTY:		return ME[L"StateCounty"];
		case PRJ_COUNTRY:		return ME[L"Country"];
		case PRJ_LB_RGN:		return ME[L"LBRegionDistrict"];
		case PRJ_POST_CODE:		return ME[L"PostalZipCode"];

		case PRJ_FOLDER_NAME:	return ME[L"ProjectFolderName"];

		case PRJ_CREATED_BY:	return ME[L"CreatedBy"];
		case PRJ_CREATED_DATE:	return ME[L"CreatedDate"];
		case PRJ_MODIFIED_BY:	return ME[L"LastModifiedBy"];
		case PRJ_MODIFIED_DATE:	return ME[L"LastModifiedDate"];
		
		case SIM_NAME:			return ME[L"SimName"];
		case SIM_COMMENTS:		return ME[L"SimComments"];

		case SIM_CREATED_BY:	return ME[L"SimCreatedBy"];
		case SIM_CREATED_DATE:	return ME[L"SimCreatedDate"];
		case SIM_MODIFIED_BY:	return ME[L"SimLastModifiedBy"];
		case SIM_MODIFIED_DATE:	return ME[L"SimLastModifiedDate"];

		default:				return L"(unknown)";
	}
}

void CProject::Scale(AVFLOAT fScale)
{ 
	for each (CLiftGroup *pGroup in m_groups) 
		pGroup->Scale(fScale); 
}

void CProject::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)
{ 
	for each (CLiftGroup *pGroup in m_groups) 
		pGroup->Move(x, y, z); 
}

