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
		HRESULT Store(dbtools::CDataBase db, ULONG nBuildingID, AVFLOAT MachRoomSlab, AVFLOAT LiftBeamHeight, LIFT_STRUCTURE Structure, AVFLOAT IntDivBeamWidth);
		HRESULT LoadFromConsole(dbtools::CDataBase::SELECT &sel);
		HRESULT LoadFromVisualisation(dbtools::CDataBase::SELECT &sel, AVFLOAT &MachRoomSlab, AVFLOAT &LiftBeamHeight, LIFT_STRUCTURE &Structure, AVFLOAT &IntDivBeamWidth, AVFLOAT &IntDivBeamHeight);
	};
	struct STOREY : public CBuildingBase::STOREY
	{
		HRESULT Store(dbtools::CDataBase db, ULONG nBuildingID);
		HRESULT LoadFromConsole(dbtools::CDataBase::SELECT &sel);
		HRESULT LoadFromVisualisation(dbtools::CDataBase::SELECT &sel);
	};

	virtual SHAFT *CreateShaft()			{ return new SHAFT; }
	virtual STOREY *CreateStorey()			{ return new STOREY; }
	SHAFT *GetShaft(AVULONG i)				{ return (SHAFT*)CBuildingBase::GetShaft(i); }
	STOREY *GetStorey(AVULONG i)			{ return (STOREY*)CBuildingBase::GetStorey(i); }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db, ULONG nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationID);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectID);

	// IFC
	HRESULT SaveAsIFC(LPCOLESTR pFileName, bool bBrep = true, bool bPresentation = false);
};
