// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"

class CProject : public CProjectBase
{
public:
	CProject();

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db);
	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);
	static HRESULT CleanUpAll(dbtools::CDataBase db);
	static HRESULT DropTables(dbtools::CDataBase db);
};
