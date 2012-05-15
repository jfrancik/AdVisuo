// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseBuilding.h"
#include "../CommonFiles/DBTools.h"

class CBuilding : public CBuildingBase
{
public:
	CBuilding() : CBuildingBase() { }

	struct STOREY : public CBuildingBase::STOREY
	{
		STOREY(CBuilding *pBuilding, AVULONG nId) : CBuildingBase::STOREY(pBuilding, nId)	{ }
	};
	struct SHAFT : public CBuildingBase::SHAFT
	{
		SHAFT(CBuilding *pBuilding, AVULONG nId) : CBuildingBase::SHAFT(pBuilding, nId)	{ }
	};
	struct LIFT : public CBuildingBase::LIFT
	{
		LIFT(CBuilding *pBuilding, AVULONG nId) : CBuildingBase::LIFT(pBuilding, nId)	{ }
	};

	virtual STOREY *CreateStorey(AVULONG nId)	{ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)		{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)		{ return new LIFT(this, nId); }
	STOREY *GetStorey(AVULONG i)				{ return (STOREY*)CBuildingBase::GetStorey(i); }
	STOREY *GetGroundStorey(AVULONG i = 0)		{ return (STOREY*)CBuildingBase::GetGroundStorey(i); }
	SHAFT *GetShaft(AVULONG i)					{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	LIFT *GetLift(AVULONG i)					{ return (LIFT*)CBuildingBase::GetLift(i); }

public:
	// IO
	static HRESULT LoadNumberFromConsole(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nNumber);
	static HRESULT LoadNumberFromVisualisation(dbtools::CDataBase db, ULONG nProjectID, ULONG &nNumber);
	HRESULT Store(dbtools::CDataBase db, ULONG nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectID);

	// IFC
	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);
};
