// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "Project.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)

std::wstring _prj_error::ErrorMessage()
{
	switch (_error)
	{
		case E_PRJ_NOT_FOUND:	return L"project not found";
		case E_PRJ_NO_BUILDING:	return L"corrupt or missing building structure";
		case E_PRJ_PASSENGERS:	return L"no hall calls found";
		case E_PRJ_LIFTS:		return L"inconsistent building structure: too many or too few lifts";
		case E_PRJ_FLOORS:		return L"inconsistent building structure: too many or too few floors";
		case E_PRJ_LIFT_DECKS:	return L"inconsistent building structure: wrong number of lift decks";
		case E_PRJ_FILE_STRUCT:	return L"data appear in wrong sequence within the simulation file";
		case E_PRJ_INTERNAL:	return L"internal error";
		default:				return L"unidentified error";
	}
}

CProject::CProject() : CProjectBase()
{
	m_nDefault = 0;
	Append();
}

CProject::~CProject()
{
	for (int i = 0; i < GetSize(); i++)
	{
		delete m_buildings[i];
		delete m_sims[i];
	}
}

int CProject::Append()
{
	CBuilding *pBuilding = new CBuilding();
	CSim *pSim = new CSim(pBuilding);
	m_buildings.push_back(pBuilding);
	m_sims.push_back(pSim);
	m_phases.push_back(PHASE_NONE);
	return GetSize() - 1;
}
