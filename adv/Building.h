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
	};
	struct STOREY : public CBuildingBase::STOREY
	{
	};

	virtual SHAFT *CreateShaft()			{ return new SHAFT; }
	virtual STOREY *CreateStorey()			{ return new STOREY; }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db, ULONG nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationID, AVFLOAT fScale);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectID, AVFLOAT fScale);

	// IFC
	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);
};
