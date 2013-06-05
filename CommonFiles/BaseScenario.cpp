// BaseScenario.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseScenario.h"
//#include "BaseSimClasses.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenario

CScenario::CScenario(CProject *pProject)
{
	m_pProject = pProject;
	m_nId = 0;
	m_nScenarioId = 0;
}

CScenario::~CScenario()
{
}

void CScenario::ResolveMe()
{
	m_nId = ME[L"ID"];
	m_nScenarioId = ME[L"TrafficScenarioId"];
	m_name = ME[L"Name"];
}

//std::wstring CProject::GetProjectInfo(PRJ_INFO what)
//{
//	switch (what)
//	{
//		case PRJ_PROJECT_NAME: return ME[L"ProjectName"];
//		case PRJ_BUILDING_NAME: return ME[L"BuildingName"];
//		case PRJ_LANGUAGE: return ME[L"Language"];
//		case PRJ_UNITS: return ME[L"MeasurementUnits"];
//		case PRJ_COMPANY: return ME[L"ClientCompany"];
//		case PRJ_CITY: return ME[L"City"];
//		case PRJ_LB_RGN: return ME[L"LBRegionDistrict"];
//		case PRJ_COUNTY: return ME[L"County"];
//		case PRJ_DESIGNER: return ME[L"LiftDesigner"];
//		case PRJ_COUNTRY: return ME[L"Country"];
//		case PRJ_CHECKED_BY: return ME[L"CheckedBy"];
//		case PRJ_POST_CODE: return ME[L"PostalZipCode"];
//		default: return L"(unknown)";
//	}
//}

