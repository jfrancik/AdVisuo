// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "VisProject.h"
#include "VisElem.h"
#include "VisLiftGroup.h"
#include "VisSim.h"
#include "VisLift.h"
#include "VisPassenger.h"

#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////
// CProjectVis Implementation

CLiftGroup *CProjectVis::CreateLiftGroup(AVULONG iIndex)
{ 
	return new CLiftGroupVis(this, iIndex); 
}

CElem *CProjectVis::CreateElement(CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)			
{ 
	return new CElemVis(this, pLiftGroup, pParent, nElemId, name, i, vec); 
}


//////////////////////////////////////////////////////////////////////////////////
// FW Specific

void CProjectVis::StoreConfig()
{ 
	for each (CLiftGroupVis *pGroup in GetLiftGroups())
		pGroup->StoreConfig();
}

//////////////////////////////////////////////////////////////////////////////////
// Error Messages

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
		case E_PRJ_NOT_AUTHORISED: return L"user not authorised to browse projects";
		case E_PRJ_INTERNAL:	return L"internal error";
		default:				return L"unidentified error";
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Load from XML

void CProjectVis::Load(xmltools::CXmlReader reader)
{
	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			reader >> ME;
			ResolveMe();
		}
		else
		if (reader.getName() == L"AVLiftGroup")
		{
			CLiftGroupVis *pGroup = AddLiftGroup();
			reader >> *pGroup ;
			pGroup ->ResolveMe();
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			AVULONG nLiftGroupId = reader[L"LiftGroupId"];
			CLiftGroupVis *pGroup = FindLiftGroup(nLiftGroupId);
			if (!pGroup || pGroup->GetSim(0)) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLiftGroup::STOREY *pStorey = pGroup->AddStorey();
			reader >> *pStorey;
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			AVULONG nLiftGroupId = reader[L"LiftGroupId"];
			CLiftGroupVis *pGroup = FindLiftGroup(nLiftGroupId);
			if (!pGroup || pGroup->GetSim(0)) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLiftGroup::SHAFT *pShaft = pGroup->AddShaft();
			reader >> *pShaft;
		}
		else
		if (reader.getName() == L"AVSim")
		{
			AVULONG nLiftGroupId = reader[L"LiftGroupId"];
			CLiftGroupVis *pGroup = FindLiftGroup(nLiftGroupId);
			if (!pGroup /*|| pGroup->GetSim()*/) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);

			pGroup->AddExtras();
			CSimVis *pSim = pGroup->AddSim();
			pGroup->ResolveMe();
			pGroup->Create();
			pGroup->Scale(0.04f);

			reader >> *pSim;
			pSim->ResolveMe();

			for (AVULONG i = 0; i < pGroup->GetLiftCount(); i++)
				pSim->AddLift(pSim->CreateLift(i));
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			AVULONG nSimId = reader[L"SimID"];
			CSimVis *pSim = FindSim(nSimId);
			if (!pSim || !pSim->GetLiftGroup()) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLiftGroupVis *pGroup = pSim->GetLiftGroup();

			JOURNEY journey;
			AVULONG nLiftID = reader[L"LiftID"];
			journey.m_shaftFrom = reader[L"ShaftFrom"];
			journey.m_shaftTo = reader[L"ShaftTo"];
			journey.m_floorFrom = reader[L"FloorFrom"];
			journey.m_floorTo = reader[L"FloorTo"];
			journey.m_timeGo = reader[L"TimeGo"];
			journey.m_timeDest = reader[L"TimeDest"];
			journey.ParseDoorCycles(reader[L"DC"]);
				  
			if (nLiftID >= pGroup->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pGroup->GetShaftCount() || journey.m_shaftTo >= pGroup->GetShaftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);
			if (nLiftID >= pSim->GetLiftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);

			pSim->GetLift(nLiftID)->AddJourney(journey);
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			AVULONG nSimId = reader[L"SimID"];
			CSimVis *pSim = FindSim(nSimId);
			if (!pSim || !pSim->GetLiftGroup()) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);

			CPassengerVis *pPassenger = (CPassengerVis*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();
			pSim->AddPassenger(pPassenger);
		}
	}
}

void CProjectVis::LoadIndex(xmltools::CXmlReader reader, std::vector<CProjectVis*> &prjs)
{
	while (reader.read())
		if (reader.getName() == L"AVProject")
		{
			CProjectVis *pPrj = new CProjectVis();
			reader >> *pPrj;
			pPrj->ResolveMe();
			prjs.push_back(pPrj);
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

void CProjectVis::Store(xmltools::CXmlWriter writer)
{
	writer.write(L"AVProject", *this);
	for (AVULONG i = 0; i < GetLiftGroupsCount(); i++)
	{
		writer.write(L"AVLiftGroup", *GetLiftGroup(i));
	
		for (ULONG i = 0; i < GetLiftGroup(i)->GetShaftCount(); i++)
			writer.write(L"AVShaft", *GetLiftGroup(i)->GetShaft(i));

		for (ULONG i = 0; i < GetLiftGroup(i)->GetStoreyCount(); i++)
			writer.write(L"AVFloor", *GetLiftGroup(i)->GetStorey(i));

		for (ULONG i = 0; i < GetLiftGroup(i)->GetSim(0)->GetLiftCount(); i++)
			for (ULONG j = 0; j < GetLiftGroup(i)->GetSim(0)->GetLift(i)->GetJourneyCount(); j++)
			{
				JOURNEY *pJ = GetLiftGroup(i)->GetSim(0)->GetLift(i)->GetJourney(j);
				writer[L"LiftID"] = GetLiftGroup(i)->GetSim(0)->GetLift(i)->GetId();
				writer[L"ShaftFrom"] = pJ->m_shaftFrom;
				writer[L"ShaftTo"] = pJ->m_shaftTo;
				writer[L"FloorFrom"] = pJ->m_floorFrom;
				writer[L"FloorTo"] = pJ->m_floorTo;
				writer[L"TimeGo"] = pJ->m_timeGo;
				writer[L"TimeDest"] = pJ->m_timeDest;
				writer[L"DC"] = pJ->StringifyDoorCycles();
				writer.write(L"AVJourney");
			}

		for (ULONG i = 0; i < GetLiftGroup(i)->GetSim(0)->GetPassengerCount(); i++)
			writer.write(L"AVPassenger", *GetLiftGroup(i)->GetSim(0)->GetPassenger(i));
	}
}
