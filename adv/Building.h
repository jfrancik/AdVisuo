// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include "../CommonFiles/DBTools.h"

class CBuilding : public CBuildingBase
{
public:
	CBuilding() : CBuildingBase() { }

	struct SHAFT : public CBuildingBase::SHAFT
	{
		SHAFT(CBuilding *pBuilding) : CBuildingBase::SHAFT(pBuilding)	{ }
	};
	struct STOREY : public CBuildingBase::STOREY
	{
		STOREY(CBuilding *pBuilding) : CBuildingBase::STOREY(pBuilding)	{ }
	};

	virtual SHAFT *CreateShaft()			{ return new SHAFT(this); }
	virtual STOREY *CreateStorey()			{ return new STOREY(this); }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return (STOREY*)CBuildingBase::GetGroundStorey(i); }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db, ULONG nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectID);

	// IFC
	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);
};
